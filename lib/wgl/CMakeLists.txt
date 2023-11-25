add_library(wgl SHARED
    wgl.cpp
    gl.def
)
SET(CMAKE_LINK_DEF_FILE_FLAG "/DEF:gl.def")
target_link_libraries(wgl PRIVATE gl spdlog::spdlog ft60x)

if (VARIANT_RRXIF)
    add_compile_definitions(VARIANT_RRXIF=1)
endif()
if (VARIANT_RRXEF)
    add_compile_definitions(VARIANT_RRXEF=1)
endif()