cmake_minimum_required(VERSION 3.17)
project(foo)

set(CMAKE_CXX_STANDARD 20)

find_package(OpenGL REQUIRED)

add_subdirectory(external/glfw)
add_subdirectory(external/imgui)
add_subdirectory(external/nfd)

if(MSVC)
    SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /ENTRY:mainCRTStartup")
    set(APPLICATION_TYPE WIN32)
endif()

add_executable(
    palette-editor ${APPLICATION_TYPE}

    source/actions/change_color_count.cpp
    source/actions/modify_color.cpp
    source/actions/swap_colors.cpp

    source/popups/combine.cpp
    source/popups/error.cpp
    source/popups/logger.cpp
    source/popups/prompt.cpp

    source/actions.cpp
    source/context.cpp
    source/editor.cpp
    source/palette.cpp
    source/popups.cpp
    source/main.cpp
)
target_include_directories(palette-editor PUBLIC include)
target_link_libraries(palette-editor PUBLIC ${OPENGL_LIBRARIES} glfw imgui nfd)