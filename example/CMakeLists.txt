add_subdirectory(util)
add_subdirectory(stencilShadow)
add_subdirectory(minimal)

if (!BUILD_RP2040) 
    # exclude it from the RP2040 build, because the memory is too small
    add_subdirectory(mipmap)
endif()

