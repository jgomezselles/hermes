add_library(hermes-o11y
STATIC
    observability.cpp
    metrics.cpp
    tracer.cpp
)

target_include_directories(hermes-o11y
PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}

INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}
)
