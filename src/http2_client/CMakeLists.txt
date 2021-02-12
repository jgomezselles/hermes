add_library(hermes-http2-client
STATIC
    client_impl.cc
    connection.cc
)

target_include_directories(hermes-http2-client
PRIVATE
    ${CMAKE_SOURCE_DIR}/src/stats
    ${CMAKE_SOURCE_DIR}/src/script

INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}
)
