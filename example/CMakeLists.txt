add_subdirectory(util)
add_subdirectory(stencilShadow)
add_subdirectory(minimal)

if (NOT RRX_BUILD_RPPICO) 
    # exclude it from the RPPico build, because the memory is too small
    add_subdirectory(mipmap)
endif()
