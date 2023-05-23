if (BUILD_NATIVE)
    add_subdirectory(native)
endif()

if (BUILD_RP2040)
    add_subdirectory(rp-pico)
endif()

if (BUILD_ZYNQ_EMBEDDED_LINUX)
    add_subdirectory(zynq-embedded-linux)
endif()