add_subdirectory(3rdParty)
add_subdirectory(gl)
add_subdirectory(driver)
add_subdirectory(stubs)
add_subdirectory(utils)
add_subdirectory(threadrunner)

if (CMAKE_HOST_WIN32 AND RIX_BUILD_SHARED_LIBRARY)
    add_subdirectory(wgl)
endif()

if (RIX_BUILD_ZYNQ_EMBEDDED_LINUX AND RIX_BUILD_SHARED_LIBRARY)
    add_subdirectory(glx)
endif()