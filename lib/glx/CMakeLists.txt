add_library(GL SHARED
    glx.cpp
)
target_link_libraries(GL PRIVATE spdlog::spdlog dmaproxy)

target_link_libraries(GL PUBLIC "-Wl,--whole-archive ../gl/libgl.a -Wl,--no-whole-archive" gl)

set_target_properties(GL PROPERTIES COMPILE_FLAGS "")

if (VARIANT_RRXIF)
    add_compile_definitions(VARIANT_RRXIF=1)
endif()
if (VARIANT_RRXEF)
    add_compile_definitions(VARIANT_RRXEF=1)
endif()
