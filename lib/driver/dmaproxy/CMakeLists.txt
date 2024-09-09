add_library(dmaproxy STATIC
    DMAProxyBusConnector.cpp
)

target_link_libraries(dmaproxy PRIVATE gl spdlog::spdlog span)
target_include_directories(dmaproxy PUBLIC .)