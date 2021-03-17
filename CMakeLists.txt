cmake_minimum_required (VERSION 3.5.1)

project(hermes LANGUAGES CXX)

message( STATUS "CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}" )

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_compile_options(
    "-Wall"
    "-Werror"
    $<$<CONFIG:Debug>:--coverage>
)

link_libraries(
    $<$<CONFIG:Debug>:--coverage>
)

add_subdirectory(src)
add_subdirectory(ut)