add_library(gl STATIC
    gl.cpp
    glu.cpp
    IceGL.cpp
    vertexpipeline/Lighting.cpp
    vertexpipeline/Clipper.cpp
    vertexpipeline/TexGen.cpp
    vertexpipeline/VertexPipeline.cpp
    vertexpipeline/ViewPort.cpp
    vertexpipeline/MatrixStack.cpp
    vertexpipeline/Culling.cpp
    vertexpipeline/PrimitiveAssembler.cpp
    vertexpipeline/Stencil.cpp
    vertexpipeline/RenderObj.cpp
    pixelpipeline/PixelPipeline.cpp
    pixelpipeline/Fogging.cpp
    pixelpipeline/Texture.cpp
    renderer/Rasterizer.cpp
)

target_link_libraries(gl PRIVATE spdlog::spdlog)
target_include_directories(gl PUBLIC .)
