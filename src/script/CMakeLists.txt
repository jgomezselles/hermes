add_library(hermes-script
STATIC
    script_queue.cpp
    script.cpp
    json_reader.cpp
    script_reader.cpp
)

target_include_directories(hermes-script
PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/src/config

INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}
)
