find_package(OpenGL REQUIRED)

include_directories(${OPENGL_INCLUDE_DIRS})

set(GLFW_BUILD_EXAMPLES OFF)
set(GLFW_BUILD_TESTS OFF)
set(GLFW_BUILD_DOCS OFF)
set(GLFW_INSTALL OFF)

add_subdirectory(glfw)
add_subdirectory(imgui)
add_subdirectory(tinyfiledialogs)