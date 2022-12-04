add_library(gl SHARED 
    Clipper.cpp
    gl.cpp
    glu.cpp
    IceGL.cpp
    Lighting.cpp
    PixelPipeline.cpp
    Rasterizer.cpp
    RenderObj.cpp
    TexGen.cpp
    VertexPipeline.cpp
    wgl.cpp
    gl.def
)
SET(CMAKE_LINK_DEF_FILE_FLAG "/DEF:gl.def")
add_definitions(-DSPDLOG_ACTIVE_LEVEL=3) 
target_link_libraries(gl PRIVATE spdlog::spdlog "${CMAKE_BINARY_DIR}/FTD3XX.lib") #  "${CMAKE_BINARY_DIR}/FTD3XX.lib" # TODO: Remove Windows Dependency
