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
    static constexpr std::size_t TMU_COUNT { RRX_CORE_TMU_COUNT };
    static constexpr std::size_t MAX_TEXTURE_SIZE { RRX_CORE_MAX_TEXTURE_SIZE };
    static constexpr bool ENABLE_MIPMAPPING { RRX_CORE_ENABLE_MIPMAPPING };
    
    // Display Settings
    static constexpr std::size_t MAX_DISPLAY_WIDTH { RRX_CORE_MAX_DISPLAY_WIDTH };
    static constexpr std::size_t MAX_DISPLAY_HEIGHT { RRX_CORE_MAX_DISPLAY_HEIGHT };
    static constexpr std::size_t FRAMEBUFFER_SIZE_IN_WORDS { RRX_CORE_FRAMEBUFFER_SIZE_IN_WORDS };

    // Rasterizer settings
    static constexpr bool USE_FLOAT_INTERPOLATION { RRX_CORE_USE_FLOAT_INTERPOLATION };

    // Bus Settings
    static constexpr std::size_t CMD_STREAM_WIDTH { RRX_CORE_CMD_STREAM_WIDTH };

    // Texture Memory Settings
    static constexpr std::size_t NUMBER_OF_TEXTURE_PAGES { RRX_CORE_NUMBER_OF_TEXTURE_PAGES };
    static constexpr std::size_t NUMBER_OF_TEXTURES { RRX_CORE_NUMBER_OF_TEXTURES };
    static constexpr std::size_t TEXTURE_PAGE_SIZE { RRX_CORE_TEXTURE_PAGE_SIZE };

    // Memory RAM location. This is used as memory offset for all device memory  
    // address calculations. Mostly useful for architectures with shared memory
    static constexpr uint32_t GRAM_MEMORY_LOC { RRX_CORE_GRAM_MEMORY_LOC }; // Shares memory with linux

    // Framebuffer Memory Location
    static constexpr FramebufferType FRAMEBUFFER_TYPE { RRX_CORE_FRAMEBUFFER_TYPE };
    static constexpr uint32_t COLOR_BUFFER_LOC_1 { RRX_CORE_COLOR_BUFFER_LOC_1 };
    static constexpr uint32_t COLOR_BUFFER_LOC_2 { RRX_CORE_COLOR_BUFFER_LOC_2 };
    static constexpr uint32_t DEPTH_BUFFER_LOC { RRX_CORE_DEPTH_BUFFER_LOC };
    static constexpr uint32_t STENCIL_BUFFER_LOC { RRX_CORE_STENCIL_BUFFER_LOC };
};

} // namespace rr

#endif // _RENDER_CONFIG_HPP_
