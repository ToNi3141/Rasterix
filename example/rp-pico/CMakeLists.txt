add_executable(minimal
    minimal.cpp
)

# Add pico_stdlib library which aggregates commonly used features
target_link_libraries(minimal pico_stdlib gl spdlog::spdlog)

# create map/bin/hex/uf2 file in addition to ELF.
pico_add_extra_outputs(minimal)