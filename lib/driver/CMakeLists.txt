if (RRX_DRIVER_FT60X)
    add_subdirectory(ft60x)
endif()

if (RRX_DRIVER_DMA_PROXY)
    add_subdirectory(dmaproxy)
endif()

if (RRX_DRIVER_VERILATOR)
    add_subdirectory(verilator)
endif()

if (RRX_DRIVER_STANDALONE_XIL_CDMA)
    add_subdirectory(standalonexilcdma)
endif()