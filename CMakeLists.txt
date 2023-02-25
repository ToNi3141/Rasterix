# Note: Use powershell to create the visual studio project as a 32 bit version to create the 32 bit DLL for 32 bit games.
# C:\<pwd>\Rasterix\build> cmake -G "Visual Studio 16 2019" -A Win32 ../.

cmake_minimum_required(VERSION 3.14...3.25)
option(BUILD_NATIVE "Sets up a native build for the current platform" OFF)
option(BUILD_CROSS "Sets up a cross compilation build for the RP2040" OFF)

set(CMAKE_CXX_STANDARD 20)

if (BUILD_CROSS)
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

if (BUILD_NATIVE)
    project(
        Rasterix
        VERSION 0.1
    )
endif()

if (CMAKE_HOST_APPLE AND BUILD_NATIVE)
    set(CMAKE_OSX_ARCHITECTURES "x86_64") # The FTDI library is only available for X86 (at least for mac os)
endif()

add_compile_definitions(SPDLOG_ACTIVE_LEVEL=3)

add_subdirectory(example)
add_subdirectory(lib)
