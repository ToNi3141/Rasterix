add_subdirectory(3rdParty)
add_subdirectory(gl)

# TODO: Remove OS X specific stuff
target_include_directories(gl PUBLIC driver gl ftd3xx/osx)
link_directories(${CMAKE_BINARY_DIR}/ftd3xx/osx)
