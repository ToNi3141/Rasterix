add_library(gl STATIC
    RRXGL.cpp
    glImpl.cpp
    gl.cpp
    glu.cpp
    vertexpipeline/Lighting.cpp
    vertexpipeline/Clipper.cpp
    vertexpipeline/TexGen.cpp
    vertexpipeline/VertexPipeline.cpp
    vertexpipeline/ViewPort.cpp
    vertexpipeline/MatrixStack.cpp
    vertexpipeline/Culling.cpp
    vertexpipeline/PrimitiveAssembler.cpp
    vertexpipeline/RenderObj.cpp
    pixelpipeline/Stencil.cpp
    pixelpipeline/PixelPipeline.cpp
    pixelpipeline/Fogging.cpp
    pixelpipeline/Texture.cpp
    renderer/Rasterizer.cpp
)

target_link_libraries(gl PRIVATE spdlog::spdlog)
target_include_directories(gl PUBLIC .)

add_compile_definitions(
    RRX_TMU_COUNT=${RRX_TMU_COUNT}
    RRX_MAX_TEXTURE_SIZE=${RRX_MAX_TEXTURE_SIZE}
    RRX_ENABLE_MIPMAPPING=${RRX_ENABLE_MIPMAPPING}
    RRX_MAX_DISPLAY_WIDTH=${RRX_MAX_DISPLAY_WIDTH}
    RRX_MAX_DISPLAY_HEIGHT=${RRX_MAX_DISPLAY_HEIGHT}
    RRX_FRAMEBUFFER_SIZE_IN_WORDS=${RRX_FRAMEBUFFER_SIZE_IN_WORDS}
    RRX_USE_FLOAT_INTERPOLATION=${RRX_USE_FLOAT_INTERPOLATION}
    RRX_CMD_STREAM_WIDTH=${RRX_CMD_STREAM_WIDTH}
    RRX_NUMBER_OF_TEXTURE_PAGES=${RRX_NUMBER_OF_TEXTURE_PAGES}
    RRX_NUMBER_OF_TEXTURES=${RRX_NUMBER_OF_TEXTURES}
    RRX_TEXTURE_PAGE_SIZE=${RRX_TEXTURE_PAGE_SIZE}
    RRX_GRAM_MEMORY_LOC=${RRX_GRAM_MEMORY_LOC}
    RRX_FRAMEBUFFER_TYPE=${RRX_FRAMEBUFFER_TYPE}
    RRX_COLOR_BUFFER_LOC_1=${RRX_COLOR_BUFFER_LOC_1}
    RRX_COLOR_BUFFER_LOC_2=${RRX_COLOR_BUFFER_LOC_2}
    RRX_DEPTH_BUFFER_LOC=${RRX_DEPTH_BUFFER_LOC}
    RRX_STENCIL_BUFFER_LOC=${RRX_STENCIL_BUFFER_LOC}
)
