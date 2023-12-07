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
