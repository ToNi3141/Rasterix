// Rasterix
// https://github.com/ToNi3141/Rasterix
// Copyright (c) 2022 ToNi3141

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

#ifndef RENDERER_HPP
#define RENDERER_HPP

#include <stdint.h>
#include <array>
#include "Vec.hpp"
#include "IRenderer.hpp"
#include "IBusConnector.hpp"
#include "DisplayList.hpp"
#include "Rasterizer.hpp"
#include <string.h>
#include "DisplayListAssembler.hpp"
#include <future>
#include <algorithm>
#include "TextureMemoryManager.hpp"

// Screen
// <-----------------X_RESOLUTION--------------------------->
// +--------------------------------------------------------+ ^
// |        ^                                               | |
// |        | LINE_RESOLUTION        DISPLAY_LINES          | |
// |        |                                               | |
// |        v                                               | |
// |<------------------------------------------------------>| Y
// |                                                        | _
// |                                 DISPLAY_LINES          | R
// |                                                        | E
// |                                                        | S
// |<------------------------------------------------------>| O
// |                                                        | L
// |                                 DISPLAY_LINES          | U
// |                                                        | T
// |                                                        | I
// |<------------------------------------------------------>| O
// |                                                        | N
// |                                 DISPLAY_LINES          | |
// |                                                        | |
// |                                                        | |
// +--------------------------------------------------------+ v
// This renderer collects all triangles in a single display list. It will create for each display line a unique display list where
// all triangles and operations are stored, which belonging to this display line. This is probably the fastest method to do this
// but requires much more memory because of lots of duplicated data.
// The BUS_WIDTH is used to calculate the alignment in the display list.
template <uint32_t DISPLAY_LIST_SIZE = 2048, uint16_t DISPLAY_LINES = 1, uint16_t LINE_RESOLUTION = 128, uint16_t BUS_WIDTH = 32, uint16_t MAX_NUMBER_OF_TEXTURES = 64>
class Renderer : public IRenderer
{
public:
    Renderer(IBusConnector& busConnector)
        : m_busConnector(busConnector)
    {
        for (auto& entry : m_displayListAssembler)
        {
            entry.clearAssembler();
        }

        setTexEnvColor({{0, 0, 0, 0}});
        setClearColor({{0, 0, 0, 0}});
        setClearDepth(65535);
        setFogColor({{255, 255, 255, 255}});
        std::array<float, 33> fogLut{};
        std::fill(fogLut.begin(), fogLut.end(), 1.0f);
        setFogLut(fogLut, 0.0f, std::numeric_limits<float>::max());

        // Initialize the render thread by running it once
        m_renderThread = std::async([&](){
            return true;
        });
    }

    virtual bool drawTriangle(const Vec4& v0,
                              const Vec4& v1,
                              const Vec4& v2,
                              const Vec4& tc0,
                              const Vec4& tc1,
                              const Vec4& tc2,
                              const Vec4& c0,
                              const Vec4& c1,
                              const Vec4& c2) override
    {
        Rasterizer::RasterizedTriangle triangleConf;

        if (!Rasterizer::rasterize(triangleConf, v0, tc0, c0, v1, tc1, c1, v2, tc2, c2))
        {
            // Triangle is not visible
            return true;
        }

        for (uint32_t i = 0; i < DISPLAY_LINES; i++)
        {
            const uint16_t currentScreenPositionStart = i * LINE_RESOLUTION;
            const uint16_t currentScreenPositionEnd = (i + 1) * LINE_RESOLUTION;
            if (Rasterizer::checkIfTriangleIsInBounds(triangleConf,
                                                      currentScreenPositionStart,
                                                      currentScreenPositionEnd))
            {
                Rasterizer::RasterizedTriangle *triangleConfDl = m_displayListAssembler[i + (DISPLAY_LINES * m_backList)].drawTriangle();
                if (triangleConfDl != nullptr)
                {
                    Rasterizer::calcLineIncrement(*triangleConfDl,
                                                  triangleConf,
                                                  currentScreenPositionStart,
                                                  currentScreenPositionEnd);
                }
                else
                {
                    return false;
                }
            }
        }
        return true;
    }

    virtual void commit() override
    {
        // Check if the previous rendering has finished. If not, block till it is finished.
        if (m_renderThread.valid() && (m_renderThread.get() != true))
        {
            // TODO: In the unexpected case, that the render thread fails, this should handle this error somehow
            return;
        }

        // Switch the display lists
        if (m_backList == 0)
        {
            m_backList = 1;
            m_frontList = 0;
        }
        else
        {
            m_backList = 0;
            m_frontList = 1;
        }

        // Prepare all display lists
        bool ret = true;
        for (uint32_t i = 0; i < DISPLAY_LINES; i++)
        {
            ret = ret && m_displayListAssembler[i + (DISPLAY_LINES * m_frontList)].commit();
        }

        // Upload textures
        m_textureManager.uploadTextures([&](std::shared_ptr<const uint16_t> texAddr, uint32_t gramAddr, uint32_t texSize)
        {
            static constexpr uint32_t TEX_UPLOAD_SIZE = TextureMemoryManager<>::MAX_TEXTURE_SIZE + ListAssembler::uploadCommandSize();
            DisplayListAssembler<TEX_UPLOAD_SIZE, BUS_WIDTH / 8> uploader;
            uploader.updateTexture(gramAddr, texAddr, texSize);

            while (!m_busConnector.clearToSend())
                ;
            m_busConnector.writeData(uploader.getDisplayList()->getMemPtr(), uploader.getDisplayList()->getSize());
            return true;
        });

        // Render image (in new thread)
        if (ret)
        {
            m_renderThread = std::async([&](){
                for (int32_t i = DISPLAY_LINES - 1; i >= 0; i--)
                {
                    while (!m_busConnector.clearToSend())
                        ;
                    const typename ListAssembler::List *list = m_displayListAssembler[i + (DISPLAY_LINES * m_frontList)].getDisplayList();
                    m_busConnector.writeData(list->getMemPtr(), list->getSize());
                    m_displayListAssembler[i + (DISPLAY_LINES * m_frontList)].clearAssembler();
                }
                return true;
            });
        }
    }

    virtual bool clear(bool colorBuffer, bool depthBuffer) override
    {
        bool ret = true;
        for (uint32_t i = 0; i < DISPLAY_LINES; i++)
        {
            ret = ret && m_displayListAssembler[i + (DISPLAY_LINES * m_backList)].clear(colorBuffer, depthBuffer);
        }
        return ret;
    }

    virtual bool setClearColor(const Vec4i& color) override
    {
        return writeToReg(ListAssembler::SET_COLOR_BUFFER_CLEAR_COLOR, convertColor(color));
    }

    virtual bool setClearDepth(uint16_t depth) override
    {
        return writeToReg(ListAssembler::SET_DEPTH_BUFFER_CLEAR_DEPTH, depth);
    }

    virtual bool setFragmentPipelineConfig(const FragmentPipelineConf& pipelineConf) override 
    {
        return writeToReg(ListAssembler::SET_FRAGMENT_PIPELINE_CONFIG, pipelineConf.serialize());
    }

    virtual bool setTexEnv(const TMU target, const TexEnvConf& texEnvConfig) override
    {
        (void)target; // Only TMU0 is supported
        return writeToReg(ListAssembler::SET_TMU0_TEX_ENV, texEnvConfig.serialize());
    }

    virtual bool setTexEnvColor(const Vec4i& color) override
    {
        return writeToReg(ListAssembler::SET_TMU0_TEX_ENV_COLOR, convertColor(color));
    }

    virtual bool setFogColor(const Vec4i& color) override
    {
        return writeToReg(ListAssembler::SET_FOG_COLOR, convertColor(color));
    }

    virtual bool setFogLut(const std::array<float, 33>& fogLut, float start, float end) override
    {
        union Value {
            uint64_t val;
            struct {
                int32_t a;
                int32_t b;
            } numbers;
            struct {
                float a;
                float b;
            } floats;
        };

        bool ret = true;

        std::array<uint64_t, 33> arr;

        // The verilog code is not able to handle float values smaller than 1.0f.
        // So, if start is smaller than 1.0f, set the lower bound to 1.0f which will
        // the set x to 1.
        const float lutLowerBound = start < 1.0f ? 1.0f : start;;
        const float lutUpperBound = end;

        // Add bounds to the lut value
        Value bounds;
        bounds.floats.a = lutLowerBound;
        bounds.floats.b = lutUpperBound;
        arr[0] = bounds.val;

        // Calculate the lut entries
        for (std::size_t i = 0; i < arr.size() - 1; i++)
        {
            float f = fogLut[i];
            float fn = fogLut[i + 1];

            const float diff = fn - f;
            const float step = diff / 256.0f;

            Value lutEntry;
            lutEntry.numbers.a = static_cast<int32_t>(step * powf(2, 30));
            lutEntry.numbers.b = static_cast<int32_t>(f * powf(2, 30));

            arr[i + 1] = lutEntry.val;
        }

        // Upload data to the display lists
        for (uint32_t i = 0; i < DISPLAY_LINES; i++)
        {
            ret = ret && m_displayListAssembler[i + (DISPLAY_LINES * m_backList)].template writeArray<uint64_t, arr.size()>(ListAssembler::SET_FOG_LUT, arr);
        }
        return ret;
    }

    virtual std::pair<bool, uint16_t> createTexture() override
    {
        return m_textureManager.createTexture();
    }

    virtual bool updateTexture(const uint16_t texId, 
                               std::shared_ptr<const uint16_t> pixels, 
                               const uint16_t texWidth,
                               const uint16_t texHeight,
                               const TextureWrapMode wrapModeS,
                               const TextureWrapMode wrapModeT,
                               const bool enableMagFilter) override
    {
        return m_textureManager.updateTexture(texId, pixels, texWidth, texHeight, wrapModeS, wrapModeT, enableMagFilter);
    }

    virtual bool useTexture(const TMU target, const uint16_t texId) override 
    {
        (void)target;
        typename TextureManager::TextureObject tex = m_textureManager.getTexture(texId);
        bool ret = tex.valid;
        for (uint32_t i = 0; i < DISPLAY_LINES; i++)
        {
            ret = ret && m_displayListAssembler[i + (DISPLAY_LINES * m_backList)].useTexture(0, tex.addr, tex.size);
            ret = ret && m_displayListAssembler[i + (DISPLAY_LINES * m_backList)].writeRegister(ListAssembler::SET_TMU0_TEXTURE_CONFIG, tex.tmuConfig);
        }
        return ret;
    }

    virtual bool deleteTexture(const uint16_t texId) override 
    {
        return m_textureManager.deleteTexture(texId);
    }

    virtual bool setFeatureEnableConfig(const FeatureEnableConf& featureEnable) override
    {
        return writeToReg(ListAssembler::SET_FEATURE_ENABLE, featureEnable.serialize());
    }

private:
    using ListAssembler = DisplayListAssembler<DISPLAY_LIST_SIZE, BUS_WIDTH / 8>;
    using TextureManager = TextureMemoryManager<MAX_NUMBER_OF_TEXTURES>;

    static uint32_t convertColor(const Vec4i color)
    {
        uint32_t colorInt =   (static_cast<uint32_t>(0xff & color[3]) << 0)
                | (static_cast<uint32_t>(0xff & color[2]) << 8)
                | (static_cast<uint32_t>(0xff & color[1]) << 16)
                | (static_cast<uint32_t>(0xff & color[0]) << 24);
        return colorInt;
    }

    template <typename TArg>
    bool writeToReg(uint32_t regIndex, const TArg& regVal)
    {
        bool ret = true;
        for (uint32_t i = 0; i < DISPLAY_LINES; i++)
        {
            ret = ret && m_displayListAssembler[i + (DISPLAY_LINES * m_backList)].writeRegister(regIndex, regVal);
        }
        return ret;
    }

    std::array<ListAssembler, DISPLAY_LINES * 2> m_displayListAssembler;
    uint8_t m_frontList = 0;
    uint8_t m_backList = 1;

    IBusConnector& m_busConnector;

    TextureManager m_textureManager;

    std::future<bool> m_renderThread;
};

#endif // RENDERER_HPP
