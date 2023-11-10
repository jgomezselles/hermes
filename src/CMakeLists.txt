add_subdirectory(http2_client)
add_subdirectory(config)
add_subdirectory(sender)
add_subdirectory(stats)
add_subdirectory(script)
add_subdirectory(o11y)

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
    hermes-o11y

    ## OTel libs

    opentelemetry_exporter_otlp_http_metric
    opentelemetry_exporter_otlp_http_client
    opentelemetry_exporter_otlp_http
    opentelemetry_http_client_curl
    opentelemetry_otlp_recordable
    opentelemetry_proto

    opentelemetry_metrics
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
    curl
    protobuf
    ssl
    crypto
    dl
    pthread
)
