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

struct RenderConfig
{
    // TMU settings
    static constexpr uint8_t TMU_COUNT { RRX_TMU_COUNT };
    static constexpr uint16_t MAX_TEXTURE_SIZE { RRX_MAX_TEXTURE_SIZE };
    static constexpr bool ENABLE_MIPMAPPING { RRX_ENABLE_MIPMAPPING };
    
    // Display Settings
    static constexpr uint16_t MAX_DISPLAY_WIDTH { RRX_MAX_DISPLAY_WIDTH };
    static constexpr uint16_t MAX_DISPLAY_HEIGHT { RRX_MAX_DISPLAY_HEIGHT };
    static constexpr uint32_t FRAMEBUFFER_SIZE_IN_WORDS { RRX_FRAMEBUFFER_SIZE_IN_WORDS };

    // Rasterizer settings
    static constexpr bool USE_FLOAT_INTERPOLATION { RRX_USE_FLOAT_INTERPOLATION };

    // Bus Settings
    static constexpr uint16_t CMD_STREAM_WIDTH { RRX_CMD_STREAM_WIDTH };

    // Texture Memory Settings
    static constexpr uint32_t NUMBER_OF_TEXTURE_PAGES { RRX_NUMBER_OF_TEXTURE_PAGES };
    static constexpr uint32_t NUMBER_OF_TEXTURES { RRX_NUMBER_OF_TEXTURES };
    static constexpr uint32_t TEXTURE_PAGE_SIZE { RRX_TEXTURE_PAGE_SIZE };

    // Memory RAM location. This is used as memory offset for all device memory  
    // address calculations. Mostly useful for architectures with shared memory
    static constexpr uint32_t GRAM_MEMORY_LOC { RRX_GRAM_MEMORY_LOC }; // Shares memory with linux

    // Framebuffer Memory Location
    static constexpr FramebufferType FRAMEBUFFER_TYPE { RRX_FRAMEBUFFER_TYPE };
    static constexpr uint32_t COLOR_BUFFER_LOC_1 { RRX_COLOR_BUFFER_LOC_1 };
    static constexpr uint32_t COLOR_BUFFER_LOC_2 { RRX_COLOR_BUFFER_LOC_2 };
    static constexpr uint32_t DEPTH_BUFFER_LOC { RRX_DEPTH_BUFFER_LOC };
    static constexpr uint32_t STENCIL_BUFFER_LOC { RRX_STENCIL_BUFFER_LOC };
};

} // namespace rr

#endif // _RENDER_CONFIG_HPP_
