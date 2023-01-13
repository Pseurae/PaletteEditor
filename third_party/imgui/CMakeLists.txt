cmake_minimum_required(VERSION 3.16)
project(imgui)

set(CMAKE_CXX_STANDARD 17)

find_package(PkgConfig REQUIRED)
find_package(Freetype REQUIRED)
find_package(OpenGL REQUIRED)
find_package(glfw3 3.3 REQUIRED)

if (UNIX)
    find_package(OpenGL REQUIRED)
endif ()

add_library(imgui
    imgui_draw.cpp
    imgui_impl_glfw.cpp
    imgui_impl_opengl3.cpp
    imgui_tables.cpp
    imgui_widgets.cpp
    imgui.cpp
)

target_include_directories(imgui PUBLIC include ${FREETYPE_INCLUDE_DIRS} ${GLFW_INCLUDE_DIRS})
target_link_directories(imgui PUBLIC ${GLFW_LIBRARY_DIRS})
target_include_directories(imgui PUBLIC .)
if (WIN32)
    target_link_libraries(imgui PUBLIC Freetype::Freetype glfw3 opengl32.lib)
elseif (UNIX)
    target_link_libraries(imgui PUBLIC Freetype::Freetype glfw OpenGL::GL)
endif()