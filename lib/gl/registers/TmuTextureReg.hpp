// Rasterix
// https://github.com/ToNi3141/Rasterix
// Copyright (c) 2023 ToNi3141

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


#ifndef _TMU_TEXTURE_REG_
#define _TMU_TEXTURE_REG_

#include <functional>
#include <cstdint>
#include <cmath>

namespace rr
{
class TmuTextureReg
{
public:
    enum class TextureWrapMode : uint32_t
    {
        REPEAT,
        CLAMP_TO_EDGE
    };

    enum class PixelFormat : uint32_t
    {
        RGBA4444,
        RGBA5551,
        RGB565
    };

    TmuTextureReg() = default;
    void setTextureWidth(const uint16_t val) { m_regVal.fields.texWidth = (1 << (static_cast<uint32_t>(log2f(static_cast<float>(val))) - 1)); }
    void setTextureHeight(const uint16_t val) { m_regVal.fields.texHeight = (1 << (static_cast<uint32_t>(log2f(static_cast<float>(val))) - 1)); }
    void setWarpModeS(const TextureWrapMode val) { m_regVal.fields.wrapModeS = static_cast<uint32_t>(val); }
    void setWarpModeT(const TextureWrapMode val) { m_regVal.fields.wrapModeT = static_cast<uint32_t>(val); }
    void setEnableMagFilter(const bool val) { m_regVal.fields.enableMagFilter = val; }
    void setPixelFormat(const PixelFormat val) { m_regVal.fields.pixelFormat = static_cast<uint32_t>(val); }

    uint16_t getTextureWidth() const { return m_regVal.fields.texWidth * 2; }
    uint16_t getTextureHeight() const { return m_regVal.fields.texHeight * 2; }
    TextureWrapMode getWrapModeS() const { return static_cast<TextureWrapMode>(m_regVal.fields.wrapModeS); }
    TextureWrapMode getWrapModeT() const { return static_cast<TextureWrapMode>(m_regVal.fields.wrapModeT); }
    bool getEnableMagFilter() const { return m_regVal.fields.enableMagFilter; }
    PixelFormat getPixelFormat() const { return static_cast<PixelFormat>(m_regVal.fields.pixelFormat); }

    void setTmu(const uint8_t tmu) { m_offset = tmu * TMU_OFFSET; }
    uint32_t serialize() const { return m_regVal.data; }
    uint32_t getAddr() const { return 0xB + m_offset; }
private:
    static constexpr uint8_t TMU_OFFSET { 3 };
    union RegVal
    {
        #pragma pack(push, 1)
        struct RegContent
        {
            RegContent()
                : texWidth(0)
                , texHeight(0)
                , wrapModeS(static_cast<uint32_t>(TextureWrapMode::REPEAT))
                , wrapModeT(static_cast<uint32_t>(TextureWrapMode::REPEAT))
                , enableMagFilter(false)
                , pixelFormat(static_cast<uint32_t>(PixelFormat::RGBA4444))
            { }

            uint32_t texWidth : 8;
            uint32_t texHeight : 8;
            uint32_t wrapModeS : 1;
            uint32_t wrapModeT : 1;
            uint32_t enableMagFilter : 1;
            uint32_t pixelFormat : 4;
        } fields {};
        uint32_t data;
        #pragma pack(pop)
    } m_regVal;
    uint8_t m_offset { 0 };
};
} // namespace rr

#endif // _TMU_TEXTURE_REG_
