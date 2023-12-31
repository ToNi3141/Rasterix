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


#ifndef TEXTUREMEMORYMANAGER_HPP
#define TEXTUREMEMORYMANAGER_HPP
#include <array>
#include "IRenderer.hpp"
#include <functional>
#include <spdlog/spdlog.h>
#include <span>
#include <optional>

namespace rr
{
template <class RenderConfig>
class TextureMemoryManager 
{
public:
    static constexpr uint32_t MAX_TEXTURE_SIZE { RenderConfig::MAX_TEXTURE_SIZE * RenderConfig::MAX_TEXTURE_SIZE * 2 * 2 };
    static constexpr uint32_t TEXTURE_PAGE_SIZE { RenderConfig::TEXTURE_PAGE_SIZE };

    TextureMemoryManager()
    {
        for (auto& texture : m_textures)
        {
            texture.inUse = false;
            texture.requiresDelete = false;
            texture.requiresUpload = false;
        }
    }

    virtual std::pair<bool, uint16_t> createTexture() 
    {
        for (uint32_t i = 1; i < m_textureLut.size(); i++)
        {
            if (!m_textureLut[i])
            {
                m_textureLut[i] = allocTexture();
                m_textures[i].requiresUpload = false;
                m_textures[i].requiresDelete = false;
                setTextureWrapModeS(i, IRenderer::TextureWrapMode::REPEAT);
                setTextureWrapModeT(i, IRenderer::TextureWrapMode::REPEAT);
                enableTextureMagFiltering(i, true);
                return {true, i};
            }
        }
        return {false, 0};
    }

    bool updateTexture(const uint16_t texId, const IRenderer::TextureObjectMipmap& textureObject) 
    {
        bool ret = true;
        uint32_t textureSlot = *m_textureLut[texId];
        const uint32_t textureSlotOld = *m_textureLut[texId];

        if (m_textures[textureSlot].requiresUpload)
        {
            m_textures[textureSlot].requiresDelete = true;
            std::optional<uint32_t> newTextureSlot = allocTexture();

            if (!newTextureSlot)
            {
                return false;
            }

            textureSlot = *newTextureSlot;
            m_textureLut[texId] = newTextureSlot;
            SPDLOG_DEBUG("Use new texture slot {} for texId {}", i, texId);
        }
        m_textures[textureSlot].textures = textureObject;

        m_textures[textureSlot].requiresUpload = true;
        m_textures[textureSlot].requiresDelete = false;

        m_textures[textureSlot].tmuConfig.setWarpModeS(m_textures[textureSlotOld].tmuConfig.getWrapModeS());
        m_textures[textureSlot].tmuConfig.setWarpModeT(m_textures[textureSlotOld].tmuConfig.getWrapModeT());
        m_textures[textureSlot].tmuConfig.setEnableMagFilter(m_textures[textureSlotOld].tmuConfig.getEnableMagFilter());

        m_textures[textureSlot].tmuConfig.setPixelFormat(textureObject[0].getPixelFormat());
        m_textures[textureSlot].tmuConfig.setTextureWidth(textureObject[0].width);
        m_textures[textureSlot].tmuConfig.setTextureHeight(textureObject[0].height);
 
        // Allocate memory pages
        const uint32_t textureSize = m_textures[textureSlot].getTextureSize();
        const uint32_t texturePages = (textureSize/ TEXTURE_PAGE_SIZE) + 1;
        SPDLOG_DEBUG("Use number of pages: {}", texturePages);
        ret = allocPages(m_textures[textureSlot], texturePages);
        if (!ret)
        {
            SPDLOG_ERROR("Run out of memory during page allocation");
            deallocPages(m_textures[textureSlot]);
        }

        return ret;
    }

    void setTextureWrapModeS(const uint16_t texId, IRenderer::TextureWrapMode mode)
    {
        Texture& tex = m_textures[*m_textureLut[texId]];
        tex.tmuConfig.setWarpModeS(mode);
    }

    void setTextureWrapModeT(const uint16_t texId, IRenderer::TextureWrapMode mode)
    {
        Texture& tex = m_textures[*m_textureLut[texId]];
        tex.tmuConfig.setWarpModeT(mode);
    }

    void enableTextureMagFiltering(const uint16_t texId, bool filter)
    {
        Texture& tex = m_textures[*m_textureLut[texId]];
        tex.tmuConfig.setEnableMagFilter(filter);
    }

    bool textureValid(const uint16_t texId) const 
    {
        const Texture& tex = m_textures[*m_textureLut[texId]];
        return (texId != 0) && tex.inUse;
    }

    TmuTextureReg getTmuConfig(const uint16_t texId) const
    {
        const Texture& tex = m_textures[*m_textureLut[texId]];
        return tex.tmuConfig;
    }

    std::span<const uint16_t> getPages(const uint16_t texId) const
    {
        if (textureValid(texId))
        {
            const Texture& tex = m_textures[*m_textureLut[texId]];
            return { tex.pageTable.data(), tex.pages };
        }
        return {};
    }

    uint32_t getTextureDataSize(const uint16_t texId) const
    {
        if (textureValid(texId))
        {
            const Texture& tex = m_textures[*m_textureLut[texId]];
            return tex.pages * TEXTURE_PAGE_SIZE;
        }
        return 0;
    }

    IRenderer::TextureObjectMipmap getTexture(const uint16_t texId)
    {
        const uint32_t textureSlot = *m_textureLut[texId];
        if (!m_textures[textureSlot].inUse)
        {
            return {};
        }
        return m_textures[textureSlot].textures;
    }

    bool deleteTexture(const uint16_t texId) 
    {
        const uint32_t texLutId = *m_textureLut[texId];
        m_textureLut[texId] = std::nullopt;
        m_textures[texLutId].requiresDelete = true;
        return true;
    }

    bool uploadTextures(const std::function<bool(uint32_t gramAddr, const std::span<const uint8_t> data)> uploader) 
    {
        // Upload textures
        for (uint32_t i = 0; i < m_textures.size(); i++)
        {
            Texture& texture = m_textures[i];
            if (texture.requiresUpload)
            {
                bool ret { true };
                std::array<uint8_t, TEXTURE_PAGE_SIZE> buffer;
                uint32_t j = 0;
                for (std::span<const uint8_t> b = texture.getPageData(j, { buffer} ); !b.empty(); b = texture.getPageData(++j, buffer))
                {
                    ret = ret && uploader(static_cast<uint32_t>(texture.pageTable[j]) * TEXTURE_PAGE_SIZE, { buffer });
                }
                texture.requiresUpload = !ret;
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
                texture.textures = {};
                deallocPages(texture);
            }
        }
        return true;
    }

private:
    struct PageEntry
    {
        bool inUse { false };
    };

    struct Texture
    {
        static constexpr std::size_t MAX_NUMBER_OF_PAGES { MAX_TEXTURE_SIZE / TEXTURE_PAGE_SIZE };
        bool inUse;
        bool requiresUpload;
        bool requiresDelete;
        std::array<uint16_t, MAX_NUMBER_OF_PAGES> pageTable {};
        uint8_t pages { 0 };
        IRenderer::TextureObjectMipmap textures {};
        TmuTextureReg tmuConfig {};

        uint32_t getTextureSize() const
        {
            uint32_t counter = 0;
            for (auto &e : textures)
            {
                counter += e.width * e.height * 2;
            }
            return counter;
        }

        std::span<const uint8_t> getPageData(uint32_t page, const std::span<uint8_t>& buffer)
        {
            const uint32_t addr = page * buffer.size();
            uint32_t level = 0;
            uint32_t mipMapAddr = 0;
            for (level = 0; level < textures.size(); level++)
            {
                if (addr < (mipMapAddr + textures[level].width * textures[level].height * 2))
                {
                    mipMapAddr = addr - mipMapAddr;
                    break;
                }
                mipMapAddr += textures[level].width * textures[level].height * 2;
            }

            std::span<const uint8_t> ret {};
            uint32_t bufferSize = 0;
            for (; level < textures.size(); level++)
            {
                const uint32_t texSize = textures[level].width * textures[level].height * 2;
                const uint8_t *pixels = std::reinterpret_pointer_cast<const uint8_t, const uint16_t>(textures[level].pixels).get() + mipMapAddr;
                const uint32_t dataSize = std::min(texSize - mipMapAddr, static_cast<uint32_t>(buffer.size()) - bufferSize);
                std::memcpy(buffer.data() + bufferSize, pixels, dataSize);
                bufferSize += dataSize;
                if (bufferSize == buffer.size())
                {
                    break;
                }
                mipMapAddr = 0; // When arriving to this point, then the current mipmap is fully in the buffer and the next mipmap is selected.
            }
            if (bufferSize != 0)
            {
                ret = { buffer.data(), bufferSize };
            }
            return ret;
        }
    };

    bool allocPages(Texture& tex, const uint32_t numberOfPages)
    {
        std::size_t cp = 0;
        for (std::size_t p = 0; p < m_pageTable.size(); p++)
        {
            if (m_pageTable[p].inUse == false)
            {
                tex.pageTable[cp] = p;
                m_pageTable[p].inUse = true;
                SPDLOG_DEBUG("Use page: {}", p);
                cp++;
                tex.pages = cp;
                if (cp == numberOfPages)
                {
                    break;
                }
                if (cp >= tex.pageTable.size())
                {
                    SPDLOG_ERROR("Texture specific page table overflown");
                    return false;
                }
            }
        }
        return true;
    }

    void deallocPages(Texture& tex)
    {
        for (std::size_t j = 0; j < tex.pages; j++)
        {
            m_pageTable[tex.pageTable[j]].inUse = false;
        }
        tex.pages = 0;
    }

    std::optional<uint32_t> allocTexture()
    {
        for (uint32_t i = 1; i < m_textures.size(); i++)
        {
            if (m_textures[i].inUse == false)
            {
                m_textures[i].inUse = true;
                SPDLOG_DEBUG("Allocating texture {}", i);

                return std::make_optional(i);
            }
        }
        SPDLOG_ERROR("Run out of memory during texture allocation");
        return std::nullopt;
    }

    // Texture memory allocator
    std::array<Texture, RenderConfig::NUMBER_OF_TEXTURES> m_textures;
    std::array<std::optional<uint32_t>, RenderConfig::NUMBER_OF_TEXTURES> m_textureLut {};
    std::array<PageEntry, RenderConfig::NUMBER_OF_TEXTURE_PAGES> m_pageTable {};
};

} // namespace rr
#endif
