add_library(hermes-sender
STATIC
    sender.cpp
    timer_impl.cpp
)

target_include_directories(hermes-sender
PRIVATE
    ${CMAKE_SOURCE_DIR}/src/http2_client
    ${CMAKE_SOURCE_DIR}/src/config
    ${CMAKE_SOURCE_DIR}/src/script

INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}
)
