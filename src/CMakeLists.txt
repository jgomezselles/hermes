add_subdirectory(http2_client)
add_subdirectory(config)
add_subdirectory(sender)
add_subdirectory(stats)
add_subdirectory(script)

add_executable(hermes main.cpp)

target_link_libraries( hermes
PRIVATE
    # Keep the list of files sorted from down to top.
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
    boost_regex
    ssl
    crypto
    dl
    pthread
)
