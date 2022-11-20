add_subdirectory(3rdParty)
add_subdirectory(gl)

if (CMAKE_HOST_WIN32)
    target_include_directories(gl PUBLIC driver gl ftd3xx/win)
    link_directories(${CMAKE_BINARY_DIR}/ftd3xx/win/x64)
endif()

if (CMAKE_HOST_APPLE)
    target_include_directories(gl PUBLIC driver gl ftd3xx/osx)
    link_directories(${CMAKE_BINARY_DIR}/ftd3xx/osx)
endif()
