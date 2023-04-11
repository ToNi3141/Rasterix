if (BUILD_NATIVE)
    add_subdirectory(native)
endif()

if (BUILD_RP2040)
    add_subdirectory(rp-pico)
endif()