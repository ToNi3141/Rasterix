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

#ifndef RENDERER_BUCKETS_HPP
#define RENDERER_BUCKETS_HPP

#include <stdint.h>
#include <array>
#include <stdint.h>
#include <array>
#include "Vec.hpp"
#include "IRenderer.hpp"
#include "IBusConnector.hpp"
#include "DisplayList.hpp"
#include "Rasterizer.hpp"

// Screen
// <-----------------X_RESOLUTION--------------------------->
// +--------------------------------------------------------+ ^
// |        ^                                               | |
// |        | LINE_RESOLUTION        DISPLAY_LINES          | |
// |        |                                               | |
// |        v                                               | |
// |<------------------------------------------------------>| |
// |                                                        | Y
// |                                 DISPLAY_LINES          | _
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
// This renderer puts the triangles in buckets and uploads the single buckets. This should be faster then the normal renderer which
// has one big display list and creates during upload the buckets. With this approach the renderer is more memory efficient but
// has less performance.
template <uint32_t DISPLAY_LIST_SIZE = 2048, uint16_t DISPLAY_LINES = 2, uint16_t LINE_RESOLUTION = 64, uint16_t BUS_WIDTH = 16>
class RendererBuckets : public IRenderer
{
public:
    RendererBuckets(IBusConnector& busConnector)
        : m_busConnector(busConnector)
    {
    }

    virtual bool drawTriangle(const Vec4 &v0,
                        const Vec4 &v1,
                        const Vec4 &v2,
                        const Vec2& st0,
                        const Vec2& st1,
                        const Vec2& st2,
                        const Vec4i& color) override
    {
        bool blendColor = false;
        Rasterizer::RasterizedTriangle triangleConf;

        if (!Rasterizer::rasterize(triangleConf, v0, st0, v1, st1, v2, st2))
        {
            // Triangle is not visible
            return true;
        }

        if (blendColor)
        {
            triangleConf.triangleStaticColor = m_color;
        }
        else
        {
            triangleConf.triangleStaticColor = 0x0;
        }
#ifndef NO_PERSP_CORRECT
        triangleConf.triangleConfiguration = TRIANGLE_CONFIGURATION_ENABLE_W_INTERPOLATION
                | TRIANGLE_CONFIGURATION_TEXTURE_INTERPOLATION;
#endif

        // Handle the first case in a special manner, because this case is really fast to calculate. Just check
        // for the bounding box and if the triangle is in this bounding box, just render it without forther calculations
        if (triangleConf.bbStartY < LINE_RESOLUTION)
        {
            const bool retVal = appendStreamCommand(m_displayList[m_backList][0], StreamCommand::TRIANGLE_STREAM, triangleConf);
            if (retVal == false)
            {
                // Out of memory error
                return false;
            }
        }

        for (uint32_t i = 1; i < DISPLAY_LINES; i++)
        {
            const uint16_t currentScreenPositionStart = i * LINE_RESOLUTION;
            const uint16_t currentScreenPositionEnd = (i + 1) * LINE_RESOLUTION;
            Rasterizer::RasterizedTriangle tmp; // Workaround for unaligned access when using triangleConfDl directly
            if (Rasterizer::calcLineIncrement(tmp, triangleConf, currentScreenPositionStart,
                                            currentScreenPositionEnd)) 
            {
                const bool retVal = appendStreamCommand(m_displayList[m_backList][i], StreamCommand::TRIANGLE_STREAM, tmp);
                if (retVal == false)
                {
                    // Out of memory error
                    return false;
                }
            }
        }

        // Should have a really low performance impact to trigger a upload after each triangle...
        uploadDisplayList();
        return true;
    }

    virtual void commit() override
    {
        // Add frame buffer flush command
        for (uint32_t i = 0; i < m_displayList[m_backList].size(); i++)
        {
            StreamCommand *op = m_displayList[m_backList][i].template create<StreamCommand>();
            if (op)
            {
                *op = StreamCommand::CLEAR_AND_COMMIT;
                appendStreamCommand(m_displayList[m_backList][i], StreamCommand::SET_COLOR_BUFFER_CLEAR_COLOR, m_clearColorFramebuffer);
            }
            else
            {
                // In case it was not possible to add the clear and commit command, discard all lists to stay in
                // sync with the display output. Otherwise the hardware will not send the current framebuffer slice
                // to the frame buffer which causes that the image will move because of skiped lines
                for (uint32_t i = 0; i < m_displayList[m_backList].size(); i++)
                    m_displayList[m_backList][i].clear();
                return;
            }
        }

        // Check if all front display lists are empty
        // If no display list is empty, block as long as all the lists from the frontList are transferred
        while (uploadDisplayList())
            ;

        // Enqueue all lists from the back display list
        for (uint32_t i = 0; i < m_displayList[m_backList].size(); i++)
        {
            m_displayList[m_backList][i].enqueue();
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
        
        // Triggers an upload
        uploadDisplayList();
    }

    virtual bool useTexture(const uint16_t* pixels, const uint16_t texWidth, const uint16_t texHeight) override
    {
        // Right now only support for square textures
        if (texWidth != texHeight)
            return false;

        static constexpr uint32_t TEXTURE_UPLOAD_SIZE = 1024;
        DisplayList<TEXTURE_UPLOAD_SIZE * 2, BUS_WIDTH / 8> textureUpload;
        textureUpload.clear();
        StreamCommand *op = textureUpload.template create<StreamCommand>();
        if (texWidth == 256)
            *op = StreamCommand::TEXTURE_STREAM_256x256;
        if (texWidth == 128)
            *op = StreamCommand::TEXTURE_STREAM_128x128;
        if (texWidth == 64)
            *op = StreamCommand::TEXTURE_STREAM_64x64;
        if (texWidth == 32)
            *op = StreamCommand::TEXTURE_STREAM_32x32;

        const uint32_t size = texWidth * texHeight;
        m_busConnector.writeData(textureUpload.getMemPtr(), textureUpload.getSize());
        for (uint32_t i = 0; i < size / TEXTURE_UPLOAD_SIZE; i++)
        {
            textureUpload.clear();
            std::array<uint16_t, TEXTURE_UPLOAD_SIZE> *px = textureUpload.template create<std::array<uint16_t, TEXTURE_UPLOAD_SIZE>>();
            memcpy(px, &pixels[i * (TEXTURE_UPLOAD_SIZE)], TEXTURE_UPLOAD_SIZE * 2);
            m_busConnector.writeData(textureUpload.getMemPtr(), textureUpload.getSize());
        }
        return true;
    }

    virtual bool clear(bool frameBuffer, bool zBuffer) override { return true; }
    virtual bool setClearColor(const Vec4i& color) override
    {
//        m_clearColorFramebuffer |= static_cast<uint32_t>(red & 0xf0) << 4 | static_cast<uint32_t>(green & 0xf0) | static_cast<uint32_t>(blue & 0xf0) >> 4;
        return true;
    }

    virtual bool setClearDepth(uint16_t depth) override
    {
        m_clearColorZBuffer = depth;
        return true;
    }

    virtual bool setTextureWrapModeS(const TextureWrapMode mode)  override
    {
    }

    virtual bool setTextureWrapModeT(const TextureWrapMode mode) override
    {
    }

    virtual bool setColorMask(const bool r, const bool g, const bool b, const bool a)
    {
    }

    virtual bool enableDepthTest(const bool enable) override
    {
    }

private:
    using List = DisplayList<DISPLAY_LIST_SIZE, BUS_WIDTH / 8>;

    enum StreamCommand : uint16_t
    {
        NOP = 0,
        TEXTURE_STREAM_32x32 = 0x1001,
        TEXTURE_STREAM_64x64 = 0x1002,
        TEXTURE_STREAM_128x128 = 0x1004,
        TEXTURE_STREAM_256x256 = 0x1008,
        SET_COLOR_BUFFER_CLEAR_COLOR = 0x2000,
        SET_DEPTH_BUFFER_CLEAR_DEPTH = 0x2001,
        SET_TEST_FUNCS = 0x2002,
        SET_BLEND_FUNCS = 0x2003,
        CLEAR_AND_COMMIT = 0x3000,
        TRIANGLE_STREAM = 0x4000
    };

    static constexpr uint32_t CLEAR_COLOR_ZBUFFER_POS = 16;
    static constexpr uint32_t TRIANGLE_CONFIGURATION_ENABLE_W_INTERPOLATION = (1 << 0);
    static constexpr uint32_t TRIANGLE_CONFIGURATION_TEXTURE_INTERPOLATION = (1 << 1);
    static constexpr uint32_t DISPLAY_BUFFERS = 2; // Note: Right now only two are supported. Other values will not work


    /// @brief This method will try to send a new display list to the hardware, if the bus is clear.
    /// @return true if a upload is in progress
    ///         false no upload is in progress
    bool uploadDisplayList()
    {
        // Check if the bus is clear
        if (!m_busConnector.clearToSend())
            return true;

        // Search a display list to transfer
        static constexpr uint8_t INVALID_INDEX = 0xff;
        uint8_t index = INVALID_INDEX;
        // Upload the display lists in reverse order because in reality the rendered picture is upside down
        for (int32_t i = m_displayList[m_frontList].size() - 1; i >= 0; i--)
        {
            // Check if a display list was send previously. If so, reset this list
            if (m_displayList[m_frontList][i].state() == List::State::TRANSFERRING)
            {
                m_displayList[m_frontList][i].clear();
            }

            if (m_displayList[m_frontList][i].state() == List::State::QUEUED)
            {
                index = i;
                break;
            }
        }

        // Check if a display list was found, if so, transfer it
        if (index != INVALID_INDEX)
        {
        	m_busConnector.startColorBufferTransfer(index);
            m_busConnector.writeData(m_displayList[m_frontList][index].getMemPtr(), m_displayList[m_frontList][index].getSize());
            m_displayList[m_frontList][index].transfer();
        }

        return index != INVALID_INDEX;
    }

    template <typename TDisplayList, typename TArg>
    bool appendStreamCommand(TDisplayList& displayList, const StreamCommand op, const TArg& arg)
    {
        StreamCommand *opDl = displayList.template create<StreamCommand>();
        TArg *argDl = displayList.template create<TArg>();

        if (!(opDl && argDl))
        {
            if (opDl)
            {
                displayList.template remove<StreamCommand>();
            }

            if (argDl)
            {
                displayList.template remove<TArg>();
            }
            // Out of memory error
            return false;
        }

        *opDl = op;
        *argDl = arg;
        return true;
    }

    std::array<std::array<List, DISPLAY_LINES>, DISPLAY_BUFFERS> m_displayList;
    uint8_t m_frontList = 0;
    uint8_t m_backList = 1;

    IBusConnector& m_busConnector;

    uint32_t m_textureId = 0;
    uint16_t m_textureWidth = 64;
    uint16_t m_textureHeight = 64;
    uint16_t m_textureBitDepth = 2;
    uint16_t m_clearColorFramebuffer = 0;
    uint16_t m_clearColorZBuffer = 0xffff;
    uint16_t m_color = 0xffff;
};

#endif // RENDERER_BUCKETS_HPP
