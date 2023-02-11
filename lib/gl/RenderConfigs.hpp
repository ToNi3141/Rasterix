#ifndef _RENDER_CONFIG_HPP_
#define _RENDER_CONFIG_HPP_

#include <cstdint>

namespace rr
{

struct RenderConfigSimulation
{
    // TMU settings
    static constexpr uint8_t TMU_COUNT { 2 };
    static constexpr uint16_t MAX_TEXTURE_SIZE { 256 };

    // Size of the Display List
    static constexpr uint32_t DISPLAYLIST_SIZE { 16 * 1024 * 1024 };

    // Display Settings
    static constexpr uint16_t MAX_DISPLAY_WIDTH { 640 };
    static constexpr uint16_t MAX_DISPLAY_HEIGHT { 480 };
    static constexpr uint32_t INTERNAL_FRAMEBUFFER_SIZE { 64 * 1024 };

    // Bus Settings
    static constexpr uint16_t CMD_STREAM_WIDTH { 64 };

    // Texture Memory Settings
    static constexpr uint16_t NUMBER_OF_TEXTURE_PAGES { 7680 };
    static constexpr uint16_t NUMBER_OF_TEXTURES { NUMBER_OF_TEXTURE_PAGES };
    static constexpr uint16_t TEXTURE_PAGE_SIZE { 4096 };
};

struct RenderConfigRasterixNexys
{
    static constexpr uint8_t TMU_COUNT { 2 };
    static constexpr uint16_t MAX_TEXTURE_SIZE { 256 };
    static constexpr uint32_t DISPLAYLIST_SIZE { 16 * 1024 * 1024 };
    static constexpr uint16_t MAX_DISPLAY_WIDTH { 1024 };
    static constexpr uint16_t MAX_DISPLAY_HEIGHT { 600 };
    static constexpr uint32_t INTERNAL_FRAMEBUFFER_SIZE { 256 * 1024 };
    static constexpr uint16_t CMD_STREAM_WIDTH { 128 };
    static constexpr uint16_t NUMBER_OF_TEXTURE_PAGES { 7680 };
    static constexpr uint16_t NUMBER_OF_TEXTURES { NUMBER_OF_TEXTURE_PAGES };
    static constexpr uint16_t TEXTURE_PAGE_SIZE { 4096 };
};


} // namespace rr

#endif // _RENDER_CONFIG_HPP_
