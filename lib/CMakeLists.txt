add_subdirectory(3rdParty)
add_subdirectory(gl)
add_subdirectory(driver)
add_subdirectory(stubs)

if (CMAKE_HOST_WIN32)
    add_subdirectory(wgl)
endif()