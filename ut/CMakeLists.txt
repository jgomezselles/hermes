add_executable( unit-test "")

target_link_libraries( unit-test
PRIVATE
        hermes-sender
        hermes-config
        hermes-http2-client
        hermes-script
        hermes-stats
        nghttp2_asio
        nghttp2
        boost_thread
        boost_filesystem
        boost_system
        ssl
        crypto
        dl
        gtest
        gmock
        gtest_main
        pthread
)

add_subdirectory(helpers)
include_directories(helpers)
add_subdirectory(script)
add_subdirectory(stats)
add_subdirectory(sender)
add_subdirectory(http2_client)
