cmake_minimum_required(VERSION 3.17)
project(foo)

set(CMAKE_CXX_STANDARD 20)

add_subdirectory(third_party/imgui)
add_subdirectory(third_party/tinyfiledialogs)

add_executable(
    palette-editor
    src/editor.cpp
    src/palette.cpp
    src/main.cpp
)
target_link_libraries(palette-editor imgui tinyfiledialogs)