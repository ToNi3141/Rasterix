#ifndef _RENDER_CONFIG_HPP_
#define _RENDER_CONFIG_HPP_

#include <cstdint>

namespace rr
{
enum class FramebufferType
{
    INTERNAL_TO_STREAM,
    INTERNAL_TO_MEMORY,
    EXTERNAL_MEMORY_TO_STREAM,
    EXTERNAL_MEMORY_DOUBLE_BUFFER
};

struct RenderConfigRRXIFSimulation
{
    // TMU settings
    static constexpr uint8_t TMU_COUNT { 2 };
    static constexpr uint16_t MAX_TEXTURE_SIZE { 256 };

    // Display Settings
    static constexpr uint16_t MAX_DISPLAY_WIDTH { 640 };
    static constexpr uint16_t MAX_DISPLAY_HEIGHT { 480 };
    static constexpr uint32_t INTERNAL_FRAMEBUFFER_SIZE { 64 * 1024 };

    // Bus Settings
    static constexpr uint16_t CMD_STREAM_WIDTH { 64 };

    // Texture Memory Settings
    static constexpr uint32_t NUMBER_OF_TEXTURE_PAGES { 7680 };
    static constexpr uint32_t NUMBER_OF_TEXTURES { NUMBER_OF_TEXTURE_PAGES };
    static constexpr uint32_t TEXTURE_PAGE_SIZE { 4096 };

    // Memory RAM location. This is used as memory offset for all device memory  
    // address calculations. Mostly useful for architectures with shared memory
    static constexpr uint32_t GRAM_MEMORY_LOC { 0x0 }; // No shared memory

    // Framebuffer Memory Location
    static constexpr FramebufferType FRAMEBUFFER_TYPE { FramebufferType::INTERNAL_TO_STREAM };
    static constexpr uint32_t COLOR_BUFFER_LOC_1 { 0 };
    static constexpr uint32_t COLOR_BUFFER_LOC_2 { 0 };
    static constexpr uint32_t DEPTH_BUFFER_LOC { 0 };
    static constexpr uint32_t STENCIL_BUFFER_LOC { 0 };
};

struct RenderConfigRRXEFSimulation
{
    // TMU settings
    static constexpr uint8_t TMU_COUNT { 2 };
    static constexpr uint16_t MAX_TEXTURE_SIZE { 256 };

    // Display Settings
    static constexpr uint16_t MAX_DISPLAY_WIDTH { 640 };
    static constexpr uint16_t MAX_DISPLAY_HEIGHT { 480 };
    static constexpr uint32_t INTERNAL_FRAMEBUFFER_SIZE { 2 * 1024 * 1024 };

    // Bus Settings
    static constexpr uint16_t CMD_STREAM_WIDTH { 64 };

    // Texture Memory Settings
    static constexpr uint32_t NUMBER_OF_TEXTURE_PAGES { 6912 };
    static constexpr uint32_t NUMBER_OF_TEXTURES { NUMBER_OF_TEXTURE_PAGES };
    static constexpr uint32_t TEXTURE_PAGE_SIZE { 4096 };

    // Memory RAM location. This is used as memory offset for all device memory  
    // address calculations. Mostly useful for architectures with shared memory
    static constexpr uint32_t GRAM_MEMORY_LOC { 0x0 }; // No shared memory

    // Framebuffer Memory Location
    static constexpr FramebufferType FRAMEBUFFER_TYPE { FramebufferType::EXTERNAL_MEMORY_TO_STREAM };
    static constexpr uint32_t COLOR_BUFFER_LOC_1 { 0x01E00000 };
    static constexpr uint32_t COLOR_BUFFER_LOC_2 { 0x01C00000 };
    static constexpr uint32_t DEPTH_BUFFER_LOC { 0x01A00000 };
    static constexpr uint32_t STENCIL_BUFFER_LOC { 0x01900000 };
};

struct RenderConfigRRXIFCModA7
{
    // TMU settings
    static constexpr uint8_t TMU_COUNT { 1 };
    static constexpr uint16_t MAX_TEXTURE_SIZE { 128 };

    // Display Settings
    static constexpr uint16_t MAX_DISPLAY_WIDTH { 320 };
    static constexpr uint16_t MAX_DISPLAY_HEIGHT { 240 };
    static constexpr uint32_t INTERNAL_FRAMEBUFFER_SIZE { 32 * 1024 };

    // Bus Settings
    static constexpr uint16_t CMD_STREAM_WIDTH { 32 };

    // Texture Memory Settings
    static constexpr uint32_t NUMBER_OF_TEXTURE_PAGES { 128 };
    static constexpr uint32_t NUMBER_OF_TEXTURES { NUMBER_OF_TEXTURE_PAGES };
    static constexpr uint32_t TEXTURE_PAGE_SIZE { 4096 };

    // Memory RAM location. This is used as memory offset for all device memory  
    // address calculations. Mostly useful for architectures with shared memory
    static constexpr uint32_t GRAM_MEMORY_LOC { 0x0 }; // No shared memory

    // Framebuffer Memory Location
    static constexpr FramebufferType FRAMEBUFFER_TYPE { FramebufferType::INTERNAL_TO_STREAM };
    static constexpr uint32_t COLOR_BUFFER_LOC_1 { 0 };
    static constexpr uint32_t COLOR_BUFFER_LOC_2 { 0 };
    static constexpr uint32_t DEPTH_BUFFER_LOC { 0 };
    static constexpr uint32_t STENCIL_BUFFER_LOC { 0 };
};

struct RenderConfigRRXIFNexys
{
    // TMU settings
    static constexpr uint8_t TMU_COUNT { 2 };
    static constexpr uint16_t MAX_TEXTURE_SIZE { 256 };
    
    // Display Settings
    static constexpr uint16_t MAX_DISPLAY_WIDTH { 1024 };
    static constexpr uint16_t MAX_DISPLAY_HEIGHT { 600 };
    static constexpr uint32_t INTERNAL_FRAMEBUFFER_SIZE { 256 * 1024 };

    // Bus Settings
    static constexpr uint16_t CMD_STREAM_WIDTH { 128 };

    // Texture Memory Settings
    static constexpr uint32_t NUMBER_OF_TEXTURE_PAGES { 7680 };
    static constexpr uint32_t NUMBER_OF_TEXTURES { NUMBER_OF_TEXTURE_PAGES };
    static constexpr uint32_t TEXTURE_PAGE_SIZE { 4096 };

    // Memory RAM location. This is used as memory offset for all device memory  
    // address calculations. Mostly useful for architectures with shared memory
    static constexpr uint32_t GRAM_MEMORY_LOC { 0x0 }; // No shared memory

    // Framebuffer Memory Location
    static constexpr FramebufferType FRAMEBUFFER_TYPE { FramebufferType::INTERNAL_TO_MEMORY };
    static constexpr uint32_t COLOR_BUFFER_LOC_1 { 0x01E00000 };
    static constexpr uint32_t COLOR_BUFFER_LOC_2 { 0 };
    static constexpr uint32_t DEPTH_BUFFER_LOC { 0 };
    static constexpr uint32_t STENCIL_BUFFER_LOC { 0 };
};

struct RenderConfigRRXEFNexys
{
    // TMU settings
    static constexpr uint8_t TMU_COUNT { 2 };
    static constexpr uint16_t MAX_TEXTURE_SIZE { 256 };
    
    // Display Settings
    static constexpr uint16_t MAX_DISPLAY_WIDTH { 1024 };
    static constexpr uint16_t MAX_DISPLAY_HEIGHT { 600 };
    static constexpr uint32_t INTERNAL_FRAMEBUFFER_SIZE { 600 * 2 * 1024 };

    // Bus Settings
    static constexpr uint16_t CMD_STREAM_WIDTH { 128 };

    // Texture Memory Settings
    static constexpr uint32_t NUMBER_OF_TEXTURE_PAGES { 6400 };
    static constexpr uint32_t NUMBER_OF_TEXTURES { NUMBER_OF_TEXTURE_PAGES };
    static constexpr uint32_t TEXTURE_PAGE_SIZE { 4096 };

    // Memory RAM location. This is used as memory offset for all device memory  
    // address calculations. Mostly useful for architectures with shared memory
    static constexpr uint32_t GRAM_MEMORY_LOC { 0x0 }; // No shared memory

    // Framebuffer Memory Location
    static constexpr FramebufferType FRAMEBUFFER_TYPE { FramebufferType::EXTERNAL_MEMORY_DOUBLE_BUFFER };
    static constexpr uint32_t COLOR_BUFFER_LOC_1 { 0x01E00000 };
    static constexpr uint32_t COLOR_BUFFER_LOC_2 { 0x01C00000 };
    static constexpr uint32_t DEPTH_BUFFER_LOC { 0x01A00000 };
    static constexpr uint32_t STENCIL_BUFFER_LOC { 0x01900000 };
};

struct RenderConfigRRXEFZynq
{
    // TMU settings
    static constexpr uint8_t TMU_COUNT { 2 };
    static constexpr uint16_t MAX_TEXTURE_SIZE { 256 };
    
    // Display Settings
    static constexpr uint16_t MAX_DISPLAY_WIDTH { 1024 };
    static constexpr uint16_t MAX_DISPLAY_HEIGHT { 600 };
    static constexpr uint32_t INTERNAL_FRAMEBUFFER_SIZE { 2 * 1024 * 1024 };

    // Bus Settings
    static constexpr uint16_t CMD_STREAM_WIDTH { 64 };

    // Texture Memory Settings
    static constexpr uint32_t NUMBER_OF_TEXTURE_PAGES { 6912 };
    static constexpr uint32_t NUMBER_OF_TEXTURES { NUMBER_OF_TEXTURE_PAGES };
    static constexpr uint32_t TEXTURE_PAGE_SIZE { 4096 };

    // Memory RAM location. This is used as memory offset for all device memory  
    // address calculations. Mostly useful for architectures with shared memory
    static constexpr uint32_t GRAM_MEMORY_LOC { 0x0E000000 }; // Shares memory with linux

    // Framebuffer Memory Location
    static constexpr FramebufferType FRAMEBUFFER_TYPE { FramebufferType::EXTERNAL_MEMORY_DOUBLE_BUFFER };
    static constexpr uint32_t COLOR_BUFFER_LOC_1 { 0x01E00000 };
    static constexpr uint32_t COLOR_BUFFER_LOC_2 { 0x01C00000 };
    static constexpr uint32_t DEPTH_BUFFER_LOC { 0x01A00000 };
    static constexpr uint32_t STENCIL_BUFFER_LOC { 0x01900000 };
};

struct RenderConfigRRXIFZynq
{
    // TMU settings
    static constexpr uint8_t TMU_COUNT { 1 };
    static constexpr uint16_t MAX_TEXTURE_SIZE { 256 };
    
    // Display Settings
    static constexpr uint16_t MAX_DISPLAY_WIDTH { 1024 };
    static constexpr uint16_t MAX_DISPLAY_HEIGHT { 600 };
    static constexpr uint32_t INTERNAL_FRAMEBUFFER_SIZE { 128 * 1024 };

    // Bus Settings
    static constexpr uint16_t CMD_STREAM_WIDTH { 64 };

    // Texture Memory Settings
    static constexpr uint32_t NUMBER_OF_TEXTURE_PAGES { 7680 };
    static constexpr uint32_t NUMBER_OF_TEXTURES { NUMBER_OF_TEXTURE_PAGES };
    static constexpr uint32_t TEXTURE_PAGE_SIZE { 4096 };

    // Memory RAM location. This is used as memory offset for all device memory  
    // address calculations. Mostly useful for architectures with shared memory
    static constexpr uint32_t GRAM_MEMORY_LOC { 0x0E000000 }; // Shares memory with linux

    // Framebuffer Memory Location
    static constexpr FramebufferType FRAMEBUFFER_TYPE { FramebufferType::INTERNAL_TO_MEMORY };
    static constexpr uint32_t COLOR_BUFFER_LOC_1 { 0x01E00000 };
    static constexpr uint32_t COLOR_BUFFER_LOC_2 { 0 };
    static constexpr uint32_t DEPTH_BUFFER_LOC { 0 };
    static constexpr uint32_t STENCIL_BUFFER_LOC { 0 };
};


} // namespace rr

#endif // _RENDER_CONFIG_HPP_
