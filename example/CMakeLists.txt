if (BUILD_NATIVE)
    add_subdirectory(minimal)
endif()

if (BUILD_RP2040)
    add_subdirectory(rp-pico)
endif()