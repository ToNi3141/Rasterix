add_executable(minimal main.cpp)

add_definitions(-DSPDLOG_ACTIVE_LEVEL=3)

if (CMAKE_HOST_WIN32)
    target_link_libraries(minimal PRIVATE gl spdlog::spdlog "${CMAKE_BINARY_DIR}/FTD3XX.lib")
endif()

if (CMAKE_HOST_APPLE)
    target_link_libraries(minimal PRIVATE gl spdlog::spdlog "${CMAKE_BINARY_DIR}/libftd3xx.dylib")
endif()
