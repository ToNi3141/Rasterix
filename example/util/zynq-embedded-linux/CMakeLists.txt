add_library(runner INTERFACE)

target_link_libraries(runner INTERFACE gl spdlog::spdlog dmaproxy span threadrunner)

target_include_directories(runner INTERFACE .)
