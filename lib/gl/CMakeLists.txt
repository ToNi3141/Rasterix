add_library(gl STATIC
    RRXGL.cpp
    glImpl.cpp
    gl.cpp
    glu.cpp
    vertexpipeline/VertexPipeline.cpp
    vertexpipeline/RenderObj.cpp
    transform/Lighting.cpp
    transform/Clipper.cpp
    transform/TexGen.cpp
    transform/ViewPort.cpp
    transform/MatrixStore.cpp
    transform/Culling.cpp
    transform/PrimitiveAssembler.cpp
    transform/Stencil.cpp
    pixelpipeline/PixelPipeline.cpp
    pixelpipeline/Fogging.cpp
    pixelpipeline/Texture.cpp
    renderer/Rasterizer.cpp
    renderer/Renderer.cpp
)

target_link_libraries(gl PRIVATE spdlog::spdlog span)
target_include_directories(gl PUBLIC .)

add_compile_definitions(
    RRX_CORE_TMU_COUNT=${RRX_CORE_TMU_COUNT}
    RRX_CORE_MAX_TEXTURE_SIZE=${RRX_CORE_MAX_TEXTURE_SIZE}
    RRX_CORE_ENABLE_MIPMAPPING=${RRX_CORE_ENABLE_MIPMAPPING}
    RRX_CORE_MAX_DISPLAY_WIDTH=${RRX_CORE_MAX_DISPLAY_WIDTH}
    RRX_CORE_MAX_DISPLAY_HEIGHT=${RRX_CORE_MAX_DISPLAY_HEIGHT}
    RRX_CORE_FRAMEBUFFER_SIZE_IN_PIXEL_LG=${RRX_CORE_FRAMEBUFFER_SIZE_IN_PIXEL_LG}
    RRX_CORE_USE_FLOAT_INTERPOLATION=${RRX_CORE_USE_FLOAT_INTERPOLATION}
    RRX_CORE_NUMBER_OF_TEXTURE_PAGES=${RRX_CORE_NUMBER_OF_TEXTURE_PAGES}
    RRX_CORE_NUMBER_OF_TEXTURES=${RRX_CORE_NUMBER_OF_TEXTURES}
    RRX_CORE_TEXTURE_PAGE_SIZE=${RRX_CORE_TEXTURE_PAGE_SIZE}
    RRX_CORE_GRAM_MEMORY_LOC=${RRX_CORE_GRAM_MEMORY_LOC}
    RRX_CORE_COLOR_BUFFER_LOC_0=${RRX_CORE_COLOR_BUFFER_LOC_0}
    RRX_CORE_COLOR_BUFFER_LOC_1=${RRX_CORE_COLOR_BUFFER_LOC_1}
    RRX_CORE_COLOR_BUFFER_LOC_2=${RRX_CORE_COLOR_BUFFER_LOC_2}
    RRX_CORE_DEPTH_BUFFER_LOC=${RRX_CORE_DEPTH_BUFFER_LOC}
    RRX_CORE_STENCIL_BUFFER_LOC=${RRX_CORE_STENCIL_BUFFER_LOC}
    RRX_CORE_THREADED_RASTERIZATION=${RRX_CORE_THREADED_RASTERIZATION}
    RRX_CORE_ENABLE_VSYNC=${RRX_CORE_ENABLE_VSYNC}
)
