cmake_minimum_required(VERSION 3.14...3.25)

project(
    Rasterix
    VERSION 0.1
    LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

add_subdirectory(example)
add_subdirectory(lib)

if (CMAKE_HOST_WIN32)
    # Note: Use powershell to create the visual studio project as a 32 bit version to create the 32 bit DLL for 32 bit games.
    # C:\<pwd>\Rasterix\build> cmake -G "Visual Studio 16 2019" -A Win32 ../.
    configure_file(${CMAKE_SOURCE_DIR}/lib/ftd3xx/win/Win32/FTD3XX.dll ${CMAKE_BINARY_DIR}/ COPYONLY)
    configure_file(${CMAKE_SOURCE_DIR}/lib/ftd3xx/win/Win32/FTD3XX.lib ${CMAKE_BINARY_DIR}/ COPYONLY)
endif()

if (CMAKE_HOST_APPLE)
    set(CMAKE_OSX_ARCHITECTURES "x86_64") # The FTDI library is only available for X86 (at least for mac os)
    configure_file(${CMAKE_SOURCE_DIR}/lib/ftd3xx/osx/libftd3xx.dylib ${CMAKE_BINARY_DIR}/ COPYONLY)
endif()
