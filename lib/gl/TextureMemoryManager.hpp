#ifndef TEXTUREMEMORYMANAGER_HPP
#define TEXTUREMEMORYMANAGER_HPP
#include <array>
#include "IRenderer.hpp"
#include <functional>

template <uint16_t MAX_NUMBER_OF_TEXTURES = 64>
class TextureMemoryManager 
{
public:
    static constexpr uint32_t MAX_TEXTURE_SIZE = 128 * 128 * 2;
    struct TextureObject 
    {
        bool valid;
        uint32_t addr;
        uint32_t size;
        uint32_t tmuConfig;
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

    bool updateTexture(const uint16_t texId, 
                       std::shared_ptr<const uint16_t> pixels, 
                       const uint16_t texWidth,
                       const uint16_t texHeight,
                       const IRenderer::TextureWrapMode wrapModeS,
                       const IRenderer::TextureWrapMode wrapModeT,
                       const bool enableMagFilter) 
    {
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
            m_textures[newTextureSlot].size = texWidth * texHeight * 2;
            m_textures[newTextureSlot].inUse = true;
            m_textures[newTextureSlot].requiresUpload = true;
            m_textures[newTextureSlot].tmuConfig.reg.wrapModeS = wrapModeS;
            m_textures[newTextureSlot].tmuConfig.reg.wrapModeT = wrapModeT;
            m_textures[newTextureSlot].tmuConfig.reg.enableMagFilter = enableMagFilter;
            m_textures[newTextureSlot].tmuConfig.reg.texWidth = (1 << (static_cast<uint32_t>(log2(static_cast<float>(texWidth))) - 1));
            m_textures[newTextureSlot].tmuConfig.reg.texHeight = (1 << (static_cast<uint32_t>(log2(static_cast<float>(texHeight))) - 1));
            return true;
        }
        return false;
    }

    TextureObject getTexture(const uint16_t texId) 
    {
        Texture& tex = m_textures[m_textureLut[texId]];
        if (!tex.inUse || texId == 0)
        {
            return { false, 0, 0, 0 };
        }
        return { true, MAX_TEXTURE_SIZE * m_textureLut[texId], tex.size, tex.tmuConfig.serialized };
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
            if (texture.requiresUpload && texture.gramAddr)
            {
                if (uploader(texture.gramAddr, i * MAX_TEXTURE_SIZE, texture.size)) 
                {
                    texture.gramAddr = std::shared_ptr<const uint16_t>();
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
                texture.gramAddr = std::shared_ptr<const uint16_t>(); // Probably not needed anymore, because when the texture is uploaded, it is deleted anyway
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
        std::shared_ptr<const uint16_t> gramAddr; // Refactor to another name. gramAddr is miss leading
        uint32_t size;
        union 
        {
            struct __attribute__ ((__packed__)) TmuTextureConfig
            {
                uint8_t texWidth : 8;
                uint8_t texHeight : 8;
                IRenderer::TextureWrapMode wrapModeS : 1;
                IRenderer::TextureWrapMode wrapModeT : 1;
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