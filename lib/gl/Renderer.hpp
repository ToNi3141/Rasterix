// RasteriCEr
// https://github.com/ToNi3141/RasteriCEr
// Copyright (c) 2021 ToNi3141

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
// This renderer collects all triangles in a single display list. Later, when the display list is uploaded, this renderer
// will create sub display lists for each display line. This approach is more memory efficient because every triangle is saved
// only once, but because it has to reinterpret the display list during upload, it is slower then the approach in the
// RendererBuckets. This renderer will preallocate for every display line one display list and will dispatch each incoming
// triangle to the buckets. It should be faster but it is less memory efficient because a triangle is potentially saved
// several times.
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

        for (auto& texture : m_textures)
        {
            texture.inUse = false;
            texture.requiresDelete = false;
            texture.requiresUpload = false;
        }

        for (auto& t : m_textureLut)
        {
            t = 0;
        }

        // Unfortunately the Arduino compiler is too old and does not support C++20 default member initializers in bit fields
#ifndef NO_PERSP_CORRECT
        m_confReg2.perspectiveCorrectedTextures = true;
#else
        m_confReg2.perspectiveCorrectedTextures = false;
#endif

        setDepthFunc(TestFunc::LESS);
        setDepthMask(false);
        setColorMask(true, true, true, true);
        setAlphaFunc(TestFunc::ALWAYS, 0xf);
        setTexEnv(TexEnvTarget::TEXTURE_ENV, TexEnvParamName::TEXTURE_ENV_MODE, TexEnvParam::MODULATE);
        setBlendFunc(BlendFunc::ONE, BlendFunc::ZERO);
        setLogicOp(LogicOp::COPY);
        setTexEnvColor({{0, 0, 0, 0}});
        setClearColor({{0, 0, 0, 0}});
        setClearDepth(65535);

        // Initialize the render thread by running it once
        m_renderThread = std::async([&](){
            return true;
        });
    }

    virtual bool drawTriangle(const Vec4& v0,
                              const Vec4& v1,
                              const Vec4& v2,
                              const Vec2& st0,
                              const Vec2& st1,
                              const Vec2& st2,
                              const Vec4i& color) override
    {
        Rasterizer::RasterizedTriangle triangleConf;

        if (!Rasterizer::rasterize(triangleConf, v0, st0, v1, st1, v2, st2))
        {
            // Triangle is not visible
            return true;
        }

        triangleConf.triangleStaticColor = convertColor(color);

        bool ret = true;
        for (uint32_t i = 0; i < DISPLAY_LINES; i++)
        {
            // TODO: The copy of triangleConfDl when added to the assember is not needed. We could preallocate that from the display list
            Rasterizer::RasterizedTriangle triangleConfDl;
            const uint16_t currentScreenPositionStart = i * LINE_RESOLUTION;
            const uint16_t currentScreenPositionEnd = (i + 1) * LINE_RESOLUTION;
            if (Rasterizer::calcLineIncrement(triangleConfDl, triangleConf, currentScreenPositionStart,
                                                currentScreenPositionEnd))
            {
                ret = ret && m_displayListAssembler[i + (DISPLAY_LINES * m_backList)].drawTriangle(triangleConfDl);
            }
        }
        return ret;
    }

    virtual void commit() override
    {
        if (m_renderThread.get() != true)
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
        for (uint32_t i = 0; i < m_textures.size(); i++)
        {
            Texture& texture = m_textures[i];
            if (texture.requiresUpload)
            {
                static constexpr uint32_t TEX_UPLOAD_SIZE = MAX_TEXTURE_SIZE + ListAssembler::uploadCommandSize();
                DisplayListAssembler<TEX_UPLOAD_SIZE, BUS_WIDTH / 8> uploader;
                uploader.updateTexture(i * MAX_TEXTURE_SIZE, texture.gramAddr, texture.width * texture.height * 2);

                while (!m_busConnector.clearToSend())
                    ;
                m_busConnector.writeData(uploader.getDisplayList()->getMemPtr(), uploader.getDisplayList()->getSize());
                texture.requiresUpload = false;
            }
        }

        // Collect garbage textures
        for (uint32_t i = 0; i < m_textures.size(); i++)
        {
            Texture& texture = m_textures[i];
            if (texture.requiresDelete)
            {
                texture.requiresDelete = false;
                texture.inUse = false;
                texture.gramAddr = std::shared_ptr<const uint16_t>();
            }
        }

        // Render image
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

    virtual bool setDepthMask(const bool flag) override
    {
        m_confReg1.depthMask = flag;
        return writeToReg(ListAssembler::SET_CONF_REG1, m_confReg1);
    }

    virtual bool enableDepthTest(const bool enable) override
    {
        m_confReg1.enableDepthTest = enable;
        return writeToReg(ListAssembler::SET_CONF_REG1, m_confReg1);
    }

    virtual bool setColorMask(const bool r, const bool g, const bool b, const bool a) override
    {
        m_confReg1.colorMaskA = a;
        m_confReg1.colorMaskB = b;
        m_confReg1.colorMaskG = g;
        m_confReg1.colorMaskR = r;
        return writeToReg(ListAssembler::SET_CONF_REG1, m_confReg1);
    }

    virtual bool setDepthFunc(const TestFunc func) override
    {
        m_confReg1.depthFunc = func;
        return writeToReg(ListAssembler::SET_CONF_REG1, m_confReg1);
    }

    virtual bool setAlphaFunc(const TestFunc func, const uint8_t ref) override
    {
        m_confReg1.alphaFunc = func;
        m_confReg1.referenceAlphaValue = ref;
        return writeToReg(ListAssembler::SET_CONF_REG1, m_confReg1);
    }

    virtual bool setTexEnv(const TexEnvTarget target, const TexEnvParamName pname, const TexEnvParam param) override
    {
        (void)target; // Only TEXTURE_ENV is supported
        (void)pname; // Only GL_TEXTURE_ENV_MODE is supported
        m_confReg2.texEnvFunc = param;
        return writeToReg(ListAssembler::SET_CONF_REG2, m_confReg2);
    }

    virtual bool setBlendFunc(const BlendFunc sfactor, const BlendFunc dfactor) override
    {
        m_confReg2.blendFuncSFactor = sfactor;
        m_confReg2.blendFuncDFactor = dfactor;
        return writeToReg(ListAssembler::SET_CONF_REG2, m_confReg2);
    }

    virtual bool setLogicOp(const LogicOp opcode) override
    {
        (void)opcode;
        return false;
    }

    virtual bool setTexEnvColor(const Vec4i& color) override
    {
        return writeToReg(ListAssembler::SET_TEX_ENV_COLOR, convertColor(color));
    }

    virtual bool setTextureWrapModeS(const TextureWrapMode mode)  override
    {
        m_confReg2.texClampS = mode == TextureWrapMode::CLAMP_TO_EDGE;
        return writeToReg(ListAssembler::SET_CONF_REG2, m_confReg2);
    }

    virtual bool setTextureWrapModeT(const TextureWrapMode mode) override
    {
        m_confReg2.texClampT = mode == TextureWrapMode::CLAMP_TO_EDGE;
        return writeToReg(ListAssembler::SET_CONF_REG2, m_confReg2);
    }

    virtual std::pair<bool, uint16_t> createTexture() override
    {
        for (uint32_t i = 1; i < m_textureLut.size(); i++)
        {
            if (m_textureLut[i] == 0)
            {
                return {true, i};
            }
        }
        return {false, 0};
    }

    virtual bool updateTexture(const uint16_t texId, std::shared_ptr<const uint16_t> pixels, const uint16_t texWidth, const uint16_t texHeight) override
    {
        if (texWidth != texHeight)
            return false;

        const uint32_t textureSlot = m_textureLut[texId];

        // If the current ID has already a bound texture slot, then we have to mark it for deletion.
        // We can't just update a texture, because even deleted textures could be used as long as the rasterizer runs.
        if (textureSlot != 0)
        {
            m_textures[textureSlot].requiresDelete = true;
        }

        uint32_t newTextureSlot = 0;
        for (uint32_t i = 1; i < m_textures.size(); i++)
        {
            if (m_textures[i].inUse == false)
            {
                newTextureSlot = i;
                break;
            }
        }

        if (newTextureSlot != 0)
        {
            m_textureLut[texId] = newTextureSlot;

            m_textures[newTextureSlot].gramAddr = pixels;
            m_textures[newTextureSlot].width = texWidth;
            m_textures[newTextureSlot].height = texHeight;
            m_textures[newTextureSlot].inUse = true;
            m_textures[newTextureSlot].requiresUpload = true;
            return true;
        }
        return false;
    }

    virtual bool useTexture(const uint16_t texId) override 
    {
        Texture& tex = m_textures[m_textureLut[texId]];
        if (!tex.inUse || texId == 0 || !tex.gramAddr)
        {
            return false;
        }

        bool ret = true;
        for (uint32_t i = 0; i < DISPLAY_LINES; i++)
        {
            ret = ret && m_displayListAssembler[i + (DISPLAY_LINES * m_backList)].useTexture(MAX_TEXTURE_SIZE * m_textureLut[texId],
                                                                                             tex.width * tex.height * 2,
                                                                                             tex.width,
                                                                                             tex.height);
        }
        return ret;
    }

    virtual bool deleteTexture(const uint16_t texId) override 
    {
        const uint32_t texLutId = m_textureLut[texId];
        m_textureLut[texId] = 0;
        m_textures[texLutId].requiresDelete = true;
        return true;
    }

private:
    static constexpr uint32_t MAX_TEXTURE_SIZE = 128 * 128 * 2;

    struct Texture
    {
        bool inUse;
        bool requiresUpload;
        bool requiresDelete;
        std::shared_ptr<const uint16_t> gramAddr;
        uint16_t width;
        uint16_t height;
    };

    using ListAssembler = DisplayListAssembler<DISPLAY_LIST_SIZE, BUS_WIDTH / 8>;

    static uint16_t convertColor(const Vec4i color)
    {
        Vec4i colorShift{color};
        colorShift >>= 4;
        uint16_t colorInt =   (static_cast<uint16_t>(colorShift[3]) << 0)
                | (static_cast<uint16_t>(colorShift[2]) << 4)
                | (static_cast<uint16_t>(colorShift[1]) << 8)
                | (static_cast<uint16_t>(colorShift[0]) << 12);
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

    // Texture memory allocator
    std::array<Texture, MAX_NUMBER_OF_TEXTURES> m_textures;
    std::array<uint32_t, MAX_NUMBER_OF_TEXTURES> m_textureLut;

    IBusConnector& m_busConnector;

    struct __attribute__ ((__packed__)) ConfReg1
    {
        bool enableDepthTest : 1;
        IRenderer::TestFunc depthFunc : 3;
        IRenderer::TestFunc alphaFunc : 3;
        uint8_t referenceAlphaValue : 4;
        bool depthMask : 1;
        bool colorMaskA : 1;
        bool colorMaskB : 1;
        bool colorMaskG : 1;
        bool colorMaskR : 1;
    } m_confReg1;

    struct __attribute__ ((__packed__)) ConfReg2
    {
        bool perspectiveCorrectedTextures : 1;
        IRenderer::TexEnvParam texEnvFunc : 3;
        IRenderer::BlendFunc blendFuncSFactor : 4;
        IRenderer::BlendFunc blendFuncDFactor : 4;
        bool texClampS : 1;
        bool texClampT : 1;
    } m_confReg2;

    std::future<bool> m_renderThread;
};

#endif // RENDERER_HPP
