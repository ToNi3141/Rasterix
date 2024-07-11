if (DRIVER_FT60X)
    add_subdirectory(ft60x)
endif()

if (DRIVER_DMA_PROXY)
    add_subdirectory(dmaproxy)
endif()

if (DRIVER_VERILATOR)
    add_subdirectory(verilator)
endif()

if (DRIVER_STANDALONE_XIL_CDMA)
    add_subdirectory(standalonexilcdma)
endif()