add_executable(minimal main.cpp)

target_link_libraries(minimal PRIVATE runner)

if (BUILD_RP2040)
    # create map/bin/hex/uf2 file in addition to ELF.
    pico_add_extra_outputs(minimal)
endif()

if (VARIANT_RRXIF)
    add_compile_definitions(VARIANT_RRXIF=1)
endif()
if (VARIANT_RRXEF)
    add_compile_definitions(VARIANT_RRXEF=1)
endif()