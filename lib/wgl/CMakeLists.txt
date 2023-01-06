add_library(wgl SHARED
    wgl.cpp
    gl.def
)
SET(CMAKE_LINK_DEF_FILE_FLAG "/DEF:gl.def")
target_link_libraries(wgl PRIVATE gl spdlog::spdlog ft60x span)
