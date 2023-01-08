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
#include <tcb/span.hpp>

namespace rr
{
template <uint16_t MAX_NUMBER_OF_TEXTURES = 64, std::size_t PAGE_SIZE = 4096, std::size_t NUMBER_OF_PAGES = 7680>
class TextureMemoryManager 
{
public:
    static constexpr uint32_t MAX_TEXTURE_SIZE { IRenderer::MAX_TEXTURE_SIZE_PX * IRenderer::MAX_TEXTURE_SIZE_PX * 2 };
    static constexpr uint32_t TEXTURE_PAGE_SIZE { PAGE_SIZE };

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
                setTextureWrapModeS(i, IRenderer::TextureWrapMode::REPEAT);
                setTextureWrapModeT(i, IRenderer::TextureWrapMode::REPEAT);
                enableTextureMagFiltering(i, true);
                return {true, i};
            }
        }
        return {false, 0};
    }

    bool updateTexture(const uint16_t texId, const IRenderer::TextureObject& textureObject) 
    {
        bool ret = false;
        uint32_t textureSlot = m_textureLut[texId];
        const uint32_t textureSlotOld = m_textureLut[texId];
        const uint32_t textureSizeInBytes = textureObject.width * textureObject.height * 2;

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
            
            // Search new texture
            for (uint32_t i = 1; i < m_textures.size(); i++)
            {
                if (m_textures[i].inUse == false)
                {
                    textureSlot = i;
                    m_textureLut[texId] = textureSlot;
                    SPDLOG_DEBUG("Use new texture slot {} for texId {}", i, texId);

                    // Allocate memory pages
                    const uint32_t texturePages = (std::max)(static_cast<uint32_t>(1), (textureSizeInBytes / TEXTURE_PAGE_SIZE));
                    SPDLOG_DEBUG("Use number of pages: {}", texturePages);
                    ret = allocPages(m_textures[textureSlot], texturePages);
                    if (!ret)
                    {
                        SPDLOG_ERROR("Run out of memory during page allocation");
                        deallocPages(m_textures[textureSlot]);
                    }
                    break;
                }
            }
            if (!ret)
            {
                SPDLOG_ERROR("Run out of memory during texture allocation");
            }
        }
        m_textures[textureSlot].tmuConfig.reg.wrapModeS = m_textures[textureSlotOld].tmuConfig.reg.wrapModeS;
        m_textures[textureSlot].tmuConfig.reg.wrapModeT = m_textures[textureSlotOld].tmuConfig.reg.wrapModeT;
        m_textures[textureSlot].tmuConfig.reg.enableMagFilter = m_textures[textureSlotOld].tmuConfig.reg.enableMagFilter;

        m_textures[textureSlot].pixels = textureObject.pixels;
        m_textures[textureSlot].size = textureSizeInBytes;
        m_textures[textureSlot].inUse = true;
        m_textures[textureSlot].requiresUpload = true;
        m_textures[textureSlot].requiresDelete = false;
        m_textures[textureSlot].intendedPixelFormat = textureObject.intendedPixelFormat;
        m_textures[textureSlot].tmuConfig.reg.pixelFormat = static_cast<uint32_t>(textureObject.getPixelFormat());
        m_textures[textureSlot].tmuConfig.reg.texWidth = (1 << (static_cast<uint32_t>(log2f(static_cast<float>(textureObject.width))) - 1));
        m_textures[textureSlot].tmuConfig.reg.texHeight = (1 << (static_cast<uint32_t>(log2f(static_cast<float>(textureObject.height))) - 1));
        return ret;
    }

    void setTextureWrapModeS(const uint16_t texId, IRenderer::TextureWrapMode mode)
    {
        Texture& tex = m_textures[m_textureLut[texId]];
        tex.tmuConfig.reg.wrapModeS = static_cast<uint32_t>(mode);
    }

    void setTextureWrapModeT(const uint16_t texId, IRenderer::TextureWrapMode mode)
    {
        Texture& tex = m_textures[m_textureLut[texId]];
        tex.tmuConfig.reg.wrapModeT = static_cast<uint32_t>(mode);
    }

    void enableTextureMagFiltering(const uint16_t texId, bool filter)
    {
        Texture& tex = m_textures[m_textureLut[texId]];
        tex.tmuConfig.reg.enableMagFilter = filter;
    }

    bool textureValid(const uint16_t texId) const 
    {
        const Texture& tex = m_textures[m_textureLut[texId]];
        return (texId != 0) && tex.inUse;
    }

    uint32_t getTmuConfig(const uint16_t texId) const
    {
        const Texture& tex = m_textures[m_textureLut[texId]];
        return tex.tmuConfig.serialized;
    }

    bool useTexture(const uint16_t texId, const std::function<bool(const uint32_t bufferIndex, const uint32_t addr, const uint32_t size)> appendToDisplayList)
    {
        Texture& tex = m_textures[m_textureLut[texId]];
        bool ret { textureValid(texId) };
        const uint32_t texSize { (std::min)(TEXTURE_PAGE_SIZE, tex.size) }; // In case it is just a small texture, just upload a part of the page.
        for (uint32_t i = 0; i < tex.pages; i++)
        {
            ret = ret && appendToDisplayList(i * TEXTURE_PAGE_SIZE, tex.pageTable[i] * TEXTURE_PAGE_SIZE, texSize); 
        }
        return ret;
    }

    tcb::span<const uint16_t> getPages(const uint16_t texId) const
    {
        if (textureValid(texId))
        {
            const Texture& tex = m_textures[m_textureLut[texId]];
            return { tex.pageTable.data(), tex.pages };
        }
        return {};
    }

    uint32_t getTextureDataSize(const uint16_t texId) const
    {
        if (textureValid(texId))
        {
            const Texture& tex = m_textures[m_textureLut[texId]];
            return tex.size;
        }
        return 0;
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
            static_cast<IRenderer::TextureObject::IntendedInternalPixelFormat>(m_textures[textureSlot].intendedPixelFormat) };
    }

    bool deleteTexture(const uint16_t texId) 
    {
        const uint32_t texLutId = m_textureLut[texId];
        m_textureLut[texId] = 0;
        m_textures[texLutId].requiresDelete = true;
        return true;
    }

    bool uploadTextures(const std::function<bool(const uint16_t* texAddr, uint32_t gramAddr, uint32_t texSize)> uploader) 
    {
        // Upload textures
        for (uint32_t i = 0; i < m_textures.size(); i++)
        {
            Texture& texture = m_textures[i];
            if (texture.requiresUpload && texture.pixels)
            {
                bool ret { true };
                const uint32_t texturePageSize = (std::min)(texture.size, TEXTURE_PAGE_SIZE);
                for (std::size_t j = 0; j < texture.pages; j++)
                {
                    ret = ret && uploader(texture.pixels.get() + (j * (TEXTURE_PAGE_SIZE / 2)), static_cast<uint32_t>(texture.pageTable[j]) * TEXTURE_PAGE_SIZE, texturePageSize);
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
                texture.pixels = std::shared_ptr<const uint16_t>();
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
        static constexpr std::size_t MAX_NUMBER_OF_PAGES { MAX_TEXTURE_SIZE / PAGE_SIZE };
        bool inUse;
        bool requiresUpload;
        bool requiresDelete;
        std::array<uint16_t, MAX_NUMBER_OF_PAGES> pageTable {};
        uint8_t pages { 0 };
        std::shared_ptr<const uint16_t> pixels;
        uint32_t size;
        IRenderer::TextureObject::IntendedInternalPixelFormat intendedPixelFormat; // Stores the intended pixel format. Has no actual effect here other than to cache a value.
        union 
        {
            #pragma pack(push, 1)
            struct TmuTextureConfig
            {
                uint32_t texWidth : 8;
                uint32_t texHeight : 8;
                uint32_t wrapModeS : 1;
                uint32_t wrapModeT : 1;
                uint32_t enableMagFilter : 1;
                uint32_t pixelFormat : 4;
            } reg;
            uint32_t serialized;
            #pragma pack(pop)
        } tmuConfig;
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

    // Texture memory allocator
    std::array<Texture, MAX_NUMBER_OF_TEXTURES> m_textures;
    std::array<uint32_t, MAX_NUMBER_OF_TEXTURES> m_textureLut;
    std::array<PageEntry, NUMBER_OF_PAGES> m_pageTable {};
};

} // namespace rr
#endif
