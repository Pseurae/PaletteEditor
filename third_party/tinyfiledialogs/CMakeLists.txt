cmake_minimum_required(VERSION 3.16)
project(imgui)

set(CMAKE_CXX_STANDARD 17)

add_library(
    tinyfiledialogs
    tinyfiledialogs.c
)
target_include_directories(tinyfiledialogs PUBLIC .)
