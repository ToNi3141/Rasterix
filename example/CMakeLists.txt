if (BUILD_NATIVE)
    add_subdirectory(minimal)
endif()

if (BUILD_CROSS)
    add_subdirectory(rp-pico)
endif()