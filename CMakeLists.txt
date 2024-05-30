cmake_minimum_required(VERSION 3.14...3.25)

# Selects the platform to build for
option(BUILD_NATIVE "Sets up a native build for the current platform" OFF)
option(BUILD_ZYNQ_EMBEDDED_LINUX "Sets up a cross build environment for embedded linux with zynq drivers" OFF)
option(BUILD_RP2040 "Sets up a cross compilation build for the RP2040" OFF)
# Enables the examples
option(BUILD_EXAMPLES "Builds the examples" OFF)
# Builds a dynamic library
option(BUILD_SHARED_LIBRARY "Builds a dynamic loadable library" OFF)
# Selects the bus connector
option(DRIVER_FT60X "Includes a driver for a FT60X bus connector" OFF)
option(DRIVER_VERILATOR "Includes a driver for the Verilator simulation" OFF)
option(DRIVER_DMA_PROXY "Includes a driver for the DMA Proxy bus connector" OFF)
# Selects the hardware variant
option(VARIANT_RRXIF "The renderer uses an internal framebuffer for rendering" OFF)
option(VARIANT_RRXEF "The renderer uses an external framebuffer for rendering" OFF)
# Enables logging
option(ENABLE_SPDLOG "Enables logging via the spdlog library" OFF)

# Hardware defines
set(RRX_TMU_COUNT "1" CACHE STRING "The number of TMUs in the hardware")
# Texture settings
set(RRX_MAX_TEXTURE_SIZE "256" CACHE STRING "The maximum width of the texture")
set(RRX_ENABLE_MIPMAPPING "true" CACHE STRING "Enables mipmapping")
# Display settings
set(RRX_MAX_DISPLAY_WIDTH "1024" CACHE STRING "The maximum width of the display")
set(RRX_MAX_DISPLAY_HEIGHT "600" CACHE STRING "The maximum height of the display")
set(RRX_FRAMEBUFFER_SIZE_IN_WORDS "65536" CACHE STRING "The size of the framebuffer in 16 bit words / in pixel")
# Rasterizer settings
set(RRX_USE_FLOAT_INTERPOLATION "false" CACHE STRING "Enables float interpolation")
# BUS settings
set(RRX_CMD_STREAM_WIDTH "64" CACHE STRING "The width of the command stream")
# Texture memory settings
set(RRX_NUMBER_OF_TEXTURE_PAGES "7280" CACHE STRING "The number of texture pages")
set(RRX_NUMBER_OF_TEXTURES "7280" CACHE STRING "The number of textures (typically less or the same as RRX_NUMBER_OF_TEXTURE_PAGES)")
set(RRX_TEXTURE_PAGE_SIZE "4096" CACHE STRING "The size of a texture page")
# Memory RAM location. This is used as memory offset for all device memory  
# address calculations. Mostly useful for architectures with shared memory
set(RRX_GRAM_MEMORY_LOC "0x0E000000" CACHE STRING "The location of the GRAM memory")
# Framebuffer memory location
set(RRX_FRAMEBUFFER_TYPE "FramebufferType::INTERNAL_TO_MEMORY" CACHE STRING "The type of the framebuffer. Allowed values: INTERNAL_TO_STREAM, INTERNAL_TO_MEMORY, EXTERNAL_MEMORY_TO_STREAM, EXTERNAL_MEMORY_DOUBLE_BUFFER") # TODO
set(RRX_COLOR_BUFFER_LOC_1 "0x01E00000" CACHE STRING "The location of the first color buffer")
set(RRX_COLOR_BUFFER_LOC_2 "0x01C00000" CACHE STRING "The location of the second color buffer")
set(RRX_DEPTH_BUFFER_LOC "0" CACHE STRING "The location of the depth buffer")
set(RRX_STENCIL_BUFFER_LOC "0" CACHE STRING "The location of the stencil buffer")


set(CMAKE_CXX_STANDARD 20)

if (BUILD_EXAMPLES)
    if (BUILD_RP2040 AND BUILD_EXAMPLES)
        add_link_options(-Wl,--print-memory-usage)
        # initialize the SDK based on PICO_SDK_PATH
        # note: this must happen before project()
        include(pico_sdk_import.cmake)
        project(
            RasterixRP2040
            VERSION 0.1
        )
        # initialize the Raspberry Pi Pico SDK
        pico_sdk_init()
    endif()

    if (BUILD_NATIVE OR BUILD_ZYNQ_EMBEDDED_LINUX)
        project(
            Rasterix
            VERSION 0.1
        )
    endif()

    add_subdirectory(example)
endif()

if (BUILD_SHARED_LIBRARY AND !BUILD_EXAMPLES)
    project(
        Rasterix
        VERSION 0.1
    )
endif()

if (CMAKE_HOST_APPLE AND BUILD_NATIVE)
    set(CMAKE_OSX_ARCHITECTURES "x86_64") # The FTDI library is only available for X86 (at least for mac os)
endif()

if (BUILD_ZYNQ_EMBEDDED_LINUX AND BUILD_SHARED_LIBRARY)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
endif()

add_compile_definitions(SPDLOG_ACTIVE_LEVEL=3)

add_subdirectory(lib)
