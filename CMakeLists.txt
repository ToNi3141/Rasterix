cmake_minimum_required(VERSION 3.14...3.25)

# Selects the platform to build for
option(RRX_BUILD_NATIVE "Sets up a native build for the current platform" OFF)
option(RRX_BUILD_ZYNQ_EMBEDDED_LINUX "Sets up a cross build environment for embedded linux with zynq drivers" OFF)
option(RRX_BUILD_RPPICO "Sets up a cross compilation build for the RPPico" OFF)
# Enables the examples
option(RRX_BUILD_EXAMPLES "Builds the examples" OFF)
# Builds a dynamic library
option(RRX_BUILD_SHARED_LIBRARY "Builds a dynamic loadable library" OFF)
# Selects the bus connector
option(RRX_DRIVER_FT60X "Includes a driver for a FT60X bus connector" OFF)
option(RRX_DRIVER_VERILATOR "Includes a driver for the Verilator simulation" OFF)
option(RRX_DRIVER_DMA_PROXY "Includes a driver for the DMA Proxy bus connector" OFF)
option(RRX_DRIVER_STANDALONE_XIL_CDMA "Includes a driver for the Xilinx CDMA bus connector" OFF)
# Enables logging
option(RRX_ENABLE_SPDLOG "Enables logging via the spdlog library" OFF)

# Hardware defines
set(RRX_CORE_TMU_COUNT "1" CACHE STRING "The number of TMUs in the hardware")
# Texture settings
set(RRX_CORE_MAX_TEXTURE_SIZE "256" CACHE STRING "The maximum width of the texture")
set(RRX_CORE_ENABLE_MIPMAPPING "true" CACHE STRING "Enables mipmapping")
# Display settings
set(RRX_CORE_MAX_DISPLAY_WIDTH "1024" CACHE STRING "The maximum width of the display")
set(RRX_CORE_MAX_DISPLAY_HEIGHT "600" CACHE STRING "The maximum height of the display")
set(RRX_CORE_FRAMEBUFFER_SIZE_IN_PIXEL_LG "16" CACHE STRING "The log2(size) of the framebuffer in 16 bit words / in pixel")
# Rasterizer settings
set(RRX_CORE_USE_FLOAT_INTERPOLATION "false" CACHE STRING "Enables float interpolation")
# Texture memory settings
set(RRX_CORE_NUMBER_OF_TEXTURE_PAGES "7280" CACHE STRING "The number of texture pages")
set(RRX_CORE_NUMBER_OF_TEXTURES "7280" CACHE STRING "The number of textures (typically less or the same as RRX_CORE_NUMBER_OF_TEXTURE_PAGES)")
set(RRX_CORE_TEXTURE_PAGE_SIZE "4096" CACHE STRING "The size of a texture page")
# Memory RAM location. This is used as memory offset for all device memory  
# address calculations. Mostly useful for architectures with shared memory
set(RRX_CORE_GRAM_MEMORY_LOC "0x0E000000" CACHE STRING "The location of the GRAM memory")
# Framebuffer memory location
set(RRX_CORE_COLOR_BUFFER_LOC_0 "0x01E00000" CACHE STRING "The location of the color buffer when the RRX is disabled")
set(RRX_CORE_COLOR_BUFFER_LOC_1 "0x01E00000" CACHE STRING "The location of the first color buffer")
set(RRX_CORE_COLOR_BUFFER_LOC_2 "0x01C00000" CACHE STRING "The location of the second color buffer")
set(RRX_CORE_DEPTH_BUFFER_LOC "0" CACHE STRING "The location of the depth buffer")
set(RRX_CORE_STENCIL_BUFFER_LOC "0" CACHE STRING "The location of the stencil buffer")
# Misc
set(RRX_CORE_THREADED_RASTERIZATION "false" CACHE STRING "Enables the threaded rasterization. Can improve the performance on multi core linux systems.")

set(CMAKE_CXX_STANDARD 17)

if (NOT RRX_BUILD_RPPICO)
    project(
        Rasterix
        VERSION 0.1
    )
endif()

if (RRX_BUILD_RPPICO)
        add_link_options(-Wl,--print-memory-usage)
        # initialize the SDK based on PICO_SDK_PATH
        # note: this must happen before project()
        include(pico_sdk_import.cmake)
        project(
            RasterixRPPico
            VERSION 0.1
        )
        # initialize the Raspberry Pi Pico SDK
        pico_sdk_init()
endif()

if (CMAKE_HOST_APPLE AND RRX_BUILD_NATIVE)
    set(CMAKE_OSX_ARCHITECTURES "x86_64") # The FTDI library is only available for X86 (at least for mac os)
endif()

if (RRX_BUILD_ZYNQ_EMBEDDED_LINUX AND RRX_BUILD_SHARED_LIBRARY)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
endif()

add_compile_definitions(SPDLOG_ACTIVE_LEVEL=3)

if (RRX_BUILD_EXAMPLES)
    add_subdirectory(example)
endif()
add_subdirectory(lib)
