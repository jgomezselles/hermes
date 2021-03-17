###
# Set the minimum required version of CMAKE
cmake_minimum_required (VERSION 3.5.1)

###
# Add hermer module path to cmake module path
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake/")

###
# Set the project name
project(hermes LANGUAGES CXX)

###
# Print the build type
message( STATUS "CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}" )

###
# Set c++ standard version
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

###
# Set compile flags, --coverage will only be added in Debug
add_compile_options(
    "-Wall"
    "-Werror"
    $<$<CONFIG:Debug>:--coverage>
)

###
# Set link flags, --coverage will only be added in Debug
link_libraries(
    $<$<CONFIG:Debug>:--coverage>
)

add_subdirectory(src)
add_subdirectory(ut)

##########
# Staging
##########
add_custom_target(staging)
add_dependencies(staging hermes)

add_custom_command(
TARGET staging POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_SOURCE_DIR}/staging/bin

    COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_CURRENT_BINARY_DIR}/src/hermes
        ${CMAKE_SOURCE_DIR}/staging/bin/hermes

    COMMAND STAGING_AREA=${CMAKE_SOURCE_DIR}/staging mkstaging
        ${CMAKE_SOURCE_DIR}/staging/bin/hermes
)
