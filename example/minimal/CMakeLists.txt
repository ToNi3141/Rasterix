add_executable(minimal main.cpp)

add_definitions(-DSPDLOG_ACTIVE_LEVEL=0)

# TODO: Remove Mac OS specific stuff
if (CMAKE_HOST_WIN32)
    configure_file(${CMAKE_SOURCE_DIR}/lib/ftd3xx/win/x64/FTD3XX.dll ${CMAKE_BINARY_DIR}/ COPYONLY)
    configure_file(${CMAKE_SOURCE_DIR}/lib/ftd3xx/win/x64/FTD3XX.lib ${CMAKE_BINARY_DIR}/ COPYONLY)
    target_link_libraries(minimal PRIVATE gl spdlog::spdlog "${CMAKE_BINARY_DIR}/FTD3XX.lib")
endif()

if (CMAKE_HOST_APPLE)
    configure_file(${CMAKE_SOURCE_DIR}/lib/ftd3xx/osx/libftd3xx.dylib ${CMAKE_BINARY_DIR}/ COPYONLY)
    target_link_libraries(minimal PRIVATE gl spdlog::spdlog "${CMAKE_BINARY_DIR}/libftd3xx.dylib")
endif()
