###
# Set the minimum required version of CMAKE
cmake_minimum_required (VERSION 3.5.1)

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