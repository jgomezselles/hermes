add_library(hermes-http2-client
STATIC
    client_impl.cpp
    connection.cpp
    client_utils.cpp
)

target_include_directories(hermes-http2-client
PRIVATE
    ${CMAKE_SOURCE_DIR}/src/stats
    ${CMAKE_SOURCE_DIR}/src/script
    ${CMAKE_SOURCE_DIR}/src/o11y

INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}
)
