
cmake_minimum_required(VERSION 3.8)
project(span LANGUAGES CXX)

enable_testing()

add_library(span INTERFACE)
target_sources(span INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include/tcb/span.hpp)
target_include_directories(span INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_features(span INTERFACE cxx_std_11)

set(TCB_SPAN_TEST_CXX_STD 11 CACHE STRING "C++ standard version for testing")

# toni3141: Avoid building tests, causes build error with petalinux 2022
#add_subdirectory(test)
