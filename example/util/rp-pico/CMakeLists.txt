add_library(runner INTERFACE)

target_link_libraries(runner INTERFACE pico_stdlib gl spdlog::spdlog hardware_spi hardware_dma span)

target_include_directories(runner INTERFACE .)
