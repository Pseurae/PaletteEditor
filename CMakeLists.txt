cmake_minimum_required(VERSION 3.17)
project(foo)

set(CMAKE_CXX_STANDARD 20)

add_subdirectory(third_party)

if(MSVC)
    SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /ENTRY:mainCRTStartup")
    set(APPLICATION_TYPE WIN32)
endif()

add_executable(
    palette-editor ${APPLICATION_TYPE}
    src/editor.cpp
    src/palette.cpp
    src/main.cpp
)
target_link_libraries(palette-editor PUBLIC ${OPENGL_LIBRARIES} glfw imgui tinyfiledialogs)