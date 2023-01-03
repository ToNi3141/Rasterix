add_executable(minimal main.cpp)

add_definitions(-DSPDLOG_ACTIVE_LEVEL=3)

target_link_libraries(minimal PRIVATE gl spdlog::spdlog ft60x span)
