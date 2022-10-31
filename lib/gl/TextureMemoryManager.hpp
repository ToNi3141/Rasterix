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


#ifndef TEXTUREMEMORYMANAGER_HPP
#define TEXTUREMEMORYMANAGER_HPP
#include <array>
#include "IRenderer.hpp"
#include <functional>

template <uint16_t MAX_NUMBER_OF_TEXTURES = 64>
class TextureMemoryManager 
{
public:
    static constexpr uint32_t MAX_TEXTURE_SIZE = 256 * 256 * 2;
    struct TextureMeta 
    {
        const bool valid;
        const uint32_t addr;
        const uint32_t size;
        const uint32_t tmuConfig;
        const IRenderer::TextureObject::PixelFormat pixelFormat;
    };

    TextureMemoryManager()
    {
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
    }

    virtual std::pair<bool, uint16_t> createTexture() 
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

    bool updateTexture(const uint16_t texId, const IRenderer::TextureObject& textureObject) 
    {
        uint32_t textureSlot = m_textureLut[texId];

        // Check if the current texture contains any pixels. If yes, a new texture must be allocated because the current texture
        // might be used in the display list. If it does not contain any pixels, then this texture will for sure not be used
        // in a display list and the current object can be safely reused.
        if (m_textures[textureSlot].pixels || (textureSlot == 0))
        {
            // Slot 0 has a special meaning. It will never be used and never be deleted.
            if (textureSlot != 0)
            {
                m_textures[textureSlot].requiresDelete = true;
            }
            
            for (uint32_t i = 1; i < m_textures.size(); i++)
            {
                if (m_textures[i].inUse == false)
                {
                    textureSlot = i;
                    m_textureLut[texId] = textureSlot;
                    break;
                }
            }
        }

        m_textures[textureSlot].pixels = textureObject.pixels;
        m_textures[textureSlot].pixelFormat = textureObject.getPixelFormat();
        m_textures[textureSlot].size = textureObject.width * textureObject.height * 2;
        m_textures[textureSlot].inUse = true;
        m_textures[textureSlot].requiresUpload = true;
        m_textures[textureSlot].requiresDelete = false;
        m_textures[textureSlot].intendedPixelFormat = textureObject.intendedPixelFormat;
        m_textures[textureSlot].tmuConfig.reg.wrapModeS = textureObject.wrapModeS;
        m_textures[textureSlot].tmuConfig.reg.wrapModeT = textureObject.wrapModeT;
        m_textures[textureSlot].tmuConfig.reg.enableMagFilter = textureObject.enableMagFilter;
        m_textures[textureSlot].tmuConfig.reg.texWidth = (1 << (static_cast<uint32_t>(log2f(static_cast<float>(textureObject.width))) - 1));
        m_textures[textureSlot].tmuConfig.reg.texHeight = (1 << (static_cast<uint32_t>(log2f(static_cast<float>(textureObject.height))) - 1));
        return true;
    }

    TextureMeta getTextureMeta(const uint16_t texId)
    {
        Texture& tex = m_textures[m_textureLut[texId]];
        if (!tex.inUse || texId == 0)
        {
            return { false, 0, 0, 0, IRenderer::TextureObject::PixelFormat::RGBA4444 };
        }
        return { true, MAX_TEXTURE_SIZE * m_textureLut[texId], tex.size, tex.tmuConfig.serialized, tex.pixelFormat };
    }

    IRenderer::TextureObject getTexture(const uint16_t texId)
    {
        const uint32_t textureSlot = m_textureLut[texId];
        if (!m_textures[textureSlot].inUse)
        {
            return {};
        }
        return { m_textures[textureSlot].pixels,
            static_cast<uint16_t>(m_textures[textureSlot].tmuConfig.reg.texWidth * 2),
            static_cast<uint16_t>(m_textures[textureSlot].tmuConfig.reg.texHeight * 2),
            m_textures[textureSlot].tmuConfig.reg.wrapModeS,
            m_textures[textureSlot].tmuConfig.reg.wrapModeT,
            m_textures[textureSlot].tmuConfig.reg.enableMagFilter,
            m_textures[textureSlot].intendedPixelFormat };
    }

    bool deleteTexture(const uint16_t texId) 
    {
        const uint32_t texLutId = m_textureLut[texId];
        m_textureLut[texId] = 0;
        m_textures[texLutId].requiresDelete = true;
        return true;
    }

    bool uploadTextures(const std::function<bool(std::shared_ptr<const uint16_t> texAddr, uint32_t gramAddr, uint32_t texSize)> uploader) 
    {
        // Upload textures
        for (uint32_t i = 0; i < m_textures.size(); i++)
        {
            Texture& texture = m_textures[i];
            if (texture.requiresUpload && texture.pixels)
            {
                if (uploader(texture.pixels, i * MAX_TEXTURE_SIZE, texture.size)) 
                {
                    texture.requiresUpload = false;
                }
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
                texture.pixels = std::shared_ptr<const uint16_t>();
            }
        }
        return true;
    }

private:
    struct Texture
    {
        bool inUse;
        bool requiresUpload;
        bool requiresDelete;
        std::shared_ptr<const uint16_t> pixels;
        uint32_t size;
        IRenderer::TextureObject::PixelFormat pixelFormat;
        IRenderer::TextureObject::IntendedInternalPixelFormat intendedPixelFormat; // Stores the intended pixel format. Has no actual effect here other than to cache a value.
        union 
        {
            struct __attribute__ ((__packed__)) TmuTextureConfig
            {
                uint8_t texWidth : 8;
                uint8_t texHeight : 8;
                IRenderer::TextureObject::TextureWrapMode wrapModeS : 1;
                IRenderer::TextureObject::TextureWrapMode wrapModeT : 1;
                bool enableMagFilter : 1;
            } reg;
            uint32_t serialized;
        } tmuConfig;
    };

    // Texture memory allocator
    std::array<Texture, MAX_NUMBER_OF_TEXTURES> m_textures;
    std::array<uint32_t, MAX_NUMBER_OF_TEXTURES> m_textureLut;
};

#endif
