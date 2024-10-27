if (RRX_BUILD_NATIVE)
    add_subdirectory(native)
endif()

if (RRX_BUILD_RPPICO)
    add_subdirectory(rp-pico)
endif()

if (RRX_BUILD_ZYNQ_EMBEDDED_LINUX)
    add_subdirectory(zynq-embedded-linux)
endif()