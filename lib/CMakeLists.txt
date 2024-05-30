add_subdirectory(3rdParty)
add_subdirectory(gl)
add_subdirectory(driver)
add_subdirectory(stubs)
add_subdirectory(utils)

if (CMAKE_HOST_WIN32)
    add_subdirectory(wgl)
endif()

if (BUILD_ZYNQ_EMBEDDED_LINUX AND BUILD_SHARED_LIBRARY)
    add_subdirectory(glx)
endif()