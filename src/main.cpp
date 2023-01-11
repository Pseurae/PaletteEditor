#include <stdio.h>
#define GL_SILENCE_DEPRECATION
#include <GLFW/glfw3.h>

#include <algorithm>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "tinyfiledialogs.h"
#include "editor.hpp"

int main(int argc, char *argv[])
{
    Editor *app = new Editor();
    app->Loop();
    delete app;
    return 0;
}
