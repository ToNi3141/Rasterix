add_library(spdlog INTERFACE)
add_library(spdlog::spdlog ALIAS spdlog)
target_include_directories(spdlog INTERFACE .)
