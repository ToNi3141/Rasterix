// Rasterix
// https://github.com/ToNi3141/Rasterix
// Copyright (c) 2024 ToNi3141

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

#ifndef TEXTURE_HPP_
#define TEXTURE_HPP_

#include "math/Vec.hpp"
#include "Enums.hpp"
#include "renderer/Renderer.hpp"
#include <optional>

namespace rr
{
class Texture
{
public:
    using TexEnv = TexEnvReg;

    Texture(Renderer& renderer);

    bool updateTexture();
    TextureObjectMipmap& getTexture();
    bool useTexture();
    bool isTextureValid(const uint16_t texId) const { return m_renderer.isTextureValid(texId); };
    std::pair<bool, uint16_t> createTexture() { return m_renderer.createTexture(); }
    bool createTextureWithName(const uint16_t texId) { return m_renderer.createTextureWithName(texId); };
    bool deleteTexture(const uint16_t texture) { return m_renderer.deleteTexture(texture); }
    void setBoundTexture(const uint16_t val);
    void setTexWrapModeS(const TextureWrapMode mode) { m_renderer.setTextureWrapModeS(m_tmu, m_tmuConf[m_tmu].boundTexture, mode); }
    void setTexWrapModeT(const TextureWrapMode mode) { m_renderer.setTextureWrapModeT(m_tmu, m_tmuConf[m_tmu].boundTexture, mode); }
    void setEnableMagFilter(const bool val) { m_renderer.enableTextureMagFiltering(m_tmu, m_tmuConf[m_tmu].boundTexture, val); }
    void setEnableMinFilter(const bool val) { m_renderer.enableTextureMinFiltering(m_tmu, m_tmuConf[m_tmu].boundTexture, val); }

    bool setTexEnvMode(const TexEnvMode mode);
    TexEnv& texEnv() { return m_tmuConf[m_tmu].texEnvConf; }
    bool setTexEnvColor(const Vec4& color);

    void activateTmu(const std::size_t tmu);
    std::size_t getActiveTmu() const { return m_tmu; }

private:
    struct TmuConfig
    {
        // Textures
        uint16_t boundTexture { 0 };

        // TMU
        TexEnvMode texEnvMode { TexEnvMode::REPLACE };
        TexEnv texEnvConf {};
        TexEnv texEnvConfUploaded {};
    };

    Renderer& m_renderer;

    // TMU
    std::array<TmuConfig, RenderConfig::TMU_COUNT> m_tmuConf {};
    std::size_t m_tmu { 0 };
    std::optional<TextureObjectMipmap> m_textureObjectMipmap {};
};

} // namespace rr
#endif // TEXTURE_HPP_
