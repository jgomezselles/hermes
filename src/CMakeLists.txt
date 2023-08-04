add_subdirectory(http2_client)
add_subdirectory(config)
add_subdirectory(sender)
add_subdirectory(stats)
add_subdirectory(script)

add_executable(hermes main.cpp)

target_link_libraries(hermes
PRIVATE
    # Keep the list of files sorted from down to top.
    ## hermes internal libs
    hermes-sender
    hermes-config
    hermes-http2-client
    hermes-script
    hermes-stats

    ## OTel libs
    opentelemetry_exporter_ostream_span
    opentelemetry_trace
    opentelemetry_resources
    opentelemetry_version
    opentelemetry_common

    ## nghttp
    nghttp2_asio
    nghttp2

    ## boost
    boost_thread
    boost_filesystem
    boost_system

    ## others
    ssl
    crypto
    dl
    pthread
)
