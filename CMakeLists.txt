# Note: Use powershell to create the visual studio project as a 32 bit version to create the 32 bit DLL for 32 bit games.
# C:\<pwd>\Rasterix\build> cmake -G "Visual Studio 16 2019" -A Win32 ../.

cmake_minimum_required(VERSION 3.14...3.25)

project(
    Rasterix
    VERSION 0.1
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)

if (CMAKE_HOST_APPLE)
    set(CMAKE_OSX_ARCHITECTURES "x86_64") # The FTDI library is only available for X86 (at least for mac os)
endif()

add_subdirectory(example)
add_subdirectory(lib)
