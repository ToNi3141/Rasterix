add_library(ft60x STATIC
    FT60XBusConnector.cpp
)

if (CMAKE_HOST_WIN32)
    # Note: Use powershell to create the visual studio project as a 32 bit version to create the 32 bit DLL for 32 bit games.
    # C:\<pwd>\Rasterix\build> cmake -G "Visual Studio 16 2019" -A Win32 ../.
    set(FTDI_LIBRARY_DIR ${CMAKE_CURRENT_SOURCE_DIR}/ftd3xx/win/Win32)
    set(FTDI_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/ftd3xx/win)
    set(FTDI_DYNAMIC_LIBRARY_PATH ${FTDI_LIBRARY_DIR}/FTD3XX.dll)
    set(FTDI_LIB_PATH ${FTDI_LIBRARY_DIR}/FTD3XX.lib)
    configure_file(${FTDI_DYNAMIC_LIBRARY_PATH} ${CMAKE_BINARY_DIR}/ COPYONLY)
    configure_file(${FTDI_LIB_PATH} ${CMAKE_BINARY_DIR}/ COPYONLY)
endif()

if (CMAKE_HOST_APPLE)
    set(CMAKE_OSX_ARCHITECTURES "x86_64") # The FTDI library is only available for X86 (at least for mac os)
    set(FTDI_LIBRARY_DIR ${CMAKE_CURRENT_SOURCE_DIR}/ftd3xx/osx)
    set(FTDI_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/ftd3xx/osx)
    set(FTDI_DYNAMIC_LIBRARY_PATH ${FTDI_LIBRARY_DIR}/libftd3xx.dylib)
    set(FTDI_LIB_PATH ${FTDI_DYNAMIC_LIBRARY_PATH})
    configure_file(${FTDI_DYNAMIC_LIBRARY_PATH} ${CMAKE_BINARY_DIR}/ COPYONLY)
endif()

target_link_libraries(ft60x PRIVATE ${FTDI_LIB_PATH} gl)
target_include_directories(ft60x PUBLIC ${FTDI_INCLUDE_DIR} .)