add_subdirectory(3rdParty)
add_subdirectory(gl)
add_subdirectory(driver)

if (CMAKE_HOST_WIN32)
    add_subdirectory(wgl)
endif()