add_library(gl STATIC 
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
)
add_definitions(-DSPDLOG_ACTIVE_LEVEL=0)
target_link_libraries(gl PRIVATE spdlog::spdlog)