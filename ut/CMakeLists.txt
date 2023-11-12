execute_process(
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/scripts/
        COMMAND ${CMAKE_SOURCE_DIR}/scripts/generate_certs_for_ut_server.sh
        RESULT_VARIABLE rv
)

if(NOT rv EQUAL 0)
    message(WARNING_ERROR " Failed generating certificates. Some tests won't work!")
endif()

add_executable( unit-test "")

target_link_libraries( unit-test
PRIVATE
        ## hermes internal libs
        hermes-sender
        hermes-config
        hermes-http2-client
        hermes-script
        hermes-stats
        hermes-o11y

        ## OTel libs
        opentelemetry_exporter_otlp_http_metric
        opentelemetry_exporter_otlp_http
        opentelemetry_exporter_otlp_http_client
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

        ## gtest
        gtest
        gmock
        gtest_main

        ## others
        curl
        protobuf
        ssl
        crypto
        dl
        pthread
)

add_subdirectory(script)
add_subdirectory(stats)
add_subdirectory(sender)
add_subdirectory(http2_client)
add_subdirectory(o11y)
