if (BUILD_NATIVE)
    add_subdirectory(ft60x)
    add_subdirectory(ft2232)
    add_subdirectory(verilator)
endif()

if (BUILD_ZYNQ_EMBEDDED_LINUX)
    add_subdirectory(dmaproxy)
endif()