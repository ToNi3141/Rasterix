cmake_minimum_required(VERSION 3.14...3.25)

project(
    Rasterix
    VERSION 0.1
    LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_OSX_ARCHITECTURES "x86_64") # The FTDI library is only available for X86 (at least for mac os)

add_subdirectory(example)
add_subdirectory(lib)
