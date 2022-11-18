add_executable(minimal main.cpp)

add_definitions(-DSPDLOG_ACTIVE_LEVEL=0)

# TODO: Remove Mac OS specific stuff
configure_file(${CMAKE_SOURCE_DIR}/lib/ftd3xx/osx/libftd3xx.dylib ${CMAKE_BINARY_DIR}/ COPYONLY)

target_link_libraries(minimal PRIVATE gl spdlog::spdlog "${CMAKE_BINARY_DIR}/libftd3xx.dylib")
