add_library(hermes-stats
STATIC
    stats.cpp
)

target_include_directories(hermes-stats
PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/src/config

INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}
)
