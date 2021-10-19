target_sources( unit-test
PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/script_test.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/script_queue_test.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/script_reader_test.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/json_reader_test.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/script_functions_test.cpp
)
