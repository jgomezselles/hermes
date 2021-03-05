add_subdirectory(http2_client)
add_subdirectory(config)
add_subdirectory(sender)
add_subdirectory(stats)
add_subdirectory(script)

#from https://gitlab.kitware.com/cmake/community/-/wikis/doc/cmake/notes/2.6#link-line-generation
add_library(nghttp2_asio STATIC IMPORTED)
add_library(nghttp2 STATIC IMPORTED)
add_library(boost_thread STATIC IMPORTED)
add_library(boost_filesystem STATIC IMPORTED)
add_library(boost_system STATIC IMPORTED)
add_library(ssl STATIC IMPORTED)
add_library(crypto STATIC IMPORTED)
add_library(stdc++ STATIC IMPORTED)

set_property(TARGET nghttp2_asio PROPERTY IMPORTED_LOCATION /usr/local/lib/libnghttp2_asio.a)
set_property(TARGET nghttp2 PROPERTY IMPORTED_LOCATION /usr/local/lib/libnghttp2.a)
set_property(TARGET boost_thread PROPERTY IMPORTED_LOCATION /usr/local/lib/libboost_thread.a)
set_property(TARGET boost_filesystem PROPERTY IMPORTED_LOCATION /usr/local/lib/libboost_filesystem.a)
set_property(TARGET boost_system PROPERTY IMPORTED_LOCATION /usr/local/lib/libboost_system.a)
set_property(TARGET ssl PROPERTY IMPORTED_LOCATION /usr/lib/libssl.a)
set_property(TARGET crypto PROPERTY IMPORTED_LOCATION /usr/lib/libcrypto.a)
set_property(TARGET stdc++ PROPERTY IMPORTED_LOCATION /usr/lib/libstdc++.a)

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
    ssl
    crypto
    dl
    pthread
    stdc++
)
