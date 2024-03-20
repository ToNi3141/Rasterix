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
    ViewPort.cpp
    MatrixStack.cpp
    Culling.cpp
)

target_link_libraries(gl PRIVATE spdlog::spdlog)
target_include_directories(gl PUBLIC .)
