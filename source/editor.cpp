#define GL_SILENCE_DEPRECATION
#include <GLFW/glfw3.h>

#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <nfd.h>
#include <array>

#include "editor.hpp"
#include "fs.hpp"
#include "palette.hpp"
#include "context.hpp"

#include "actions/change_color_count.hpp"
#include "actions/modify_color.hpp"
#include "actions/swap_colors.hpp"

#include "popups/combine.hpp"
#include "popups/error.hpp"
#include "popups/logger.hpp"
#include "popups/prompt.hpp"
#include "popups/split.hpp"

enum 
{
    SHORT_NEW,
    SHORT_OPEN,
    SHORT_SAVE,
    SHORT_SAVE_AS,
    SHORT_QUIT,
    SHORT_UNDO,
    SHORT_REDO,
    SHORT_COMBINE,
    SHORT_SPLIT,
    COUNT_SHORT
};

#if defined(__APPLE__)
#define sText_Modifier "Cmd"
#else
#define sText_Modifier "Ctrl"
#endif

static constexpr std::array<const char*, COUNT_SHORT> sText_FileShortcuts =
{
    sText_Modifier "+N",
    sText_Modifier "+O",
    sText_Modifier "+S",
    sText_Modifier "+Shift+S",
    sText_Modifier "+Q",
    sText_Modifier "+Z",
    sText_Modifier "+R",
    sText_Modifier "+Shift+K",
    sText_Modifier "+Shift+L",
};

#undef sText_Modifier

static void glfw_error_callback(int error, const char *description)
{
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

Editor::Editor()
{
    Context::CreateNewContext();
    this->InitGLFW();
    this->InitImGui();
}

Editor::~Editor()
{
    this->ExitImGui();
    this->ExitGLFW();
}

void Editor::InitGLFW()
{
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
        std::abort();

#if defined(__APPLE__)
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#else
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
#endif

    glfwWindowHint(GLFW_SCALE_TO_MONITOR, GLFW_TRUE);

    m_Window = glfwCreateWindow(400, 400, "Palette Editor", NULL, NULL);
    if (m_Window == NULL)
        std::abort();

    glfwSetWindowUserPointer(m_Window, this);
    glfwSetWindowSizeLimits(m_Window, 400, 400, 400, 400);

    glfwMakeContextCurrent(m_Window);
    glfwSwapInterval(1);

    int win_w, win_h, fb_w, fb_h;
    glfwGetWindowSize(m_Window, &win_w, &win_h);
    glfwGetFramebufferSize(m_Window, &fb_w, &fb_h);

    glfwSetKeyCallback(m_Window, [](GLFWwindow *window, int key, int scancode, int action, int mods) {
        Editor *editor = static_cast<Editor *>(glfwGetWindowUserPointer(window));

        if (action == GLFW_PRESS || action == GLFW_REPEAT)
        {
            editor->ProcessShortcuts(key, mods);
            glfwPollEvents();
            editor->StartFrame();
            editor->Frame();
            editor->EndFrame();
        }
    });

    glfwSetDropCallback(m_Window, [](GLFWwindow *window, int path_count, const char *paths[]) {
        Editor *editor = static_cast<Editor *>(glfwGetWindowUserPointer(window));

        if (path_count <= 0)
            return;

        std::string path = std::string(paths[0], strlen(paths[0]));
        if (path.substr(path.find_last_of(".") + 1) != "pal")
            return;
        editor->OpenPalette(path.c_str());
    });

    glfwSetWindowCloseCallback(m_Window, [](GLFWwindow *window) {
        Editor *editor = static_cast<Editor *>(glfwGetWindowUserPointer(window));
        if (Context::GetContext().isDirty)
        {
            glfwSetWindowShouldClose(window, GLFW_FALSE);
            editor->m_PopupManager.OpenPopup<Popups::Prompt>("dirty_buffer_prompt", "There are unsaved changes.\nDo you want to quit?", [window](){
                glfwSetWindowShouldClose(window, GLFW_TRUE);
            });
        }
    });
}

void Editor::InitImGui(void)
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    io.IniFilename = "palette_editor.ini";

    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(m_Window, true);


#if defined(__APPLE__)
    ImGui_ImplOpenGL3_Init("#version 150");
#else
    ImGui_ImplOpenGL3_Init("#version 130");
#endif
}

void Editor::StartFrame(void)
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGuiWindowFlags windowflags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize;

    ImGuiViewport *viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->Pos);
    ImGui::SetNextWindowSize(viewport->Size - ImVec2(0.0f, ImGui::GetFrameHeight()));

    ImGui::Begin("##PaletteEditor", NULL, windowflags);
    m_PopupManager.UpdateAndDraw();
    this->MenuBar();
    ImGui::End();

    this->StatusBar();
}

void Editor::Frame(void)
{
    ImGui::Begin("##PaletteEditor", NULL, 0);
    auto &openContexts = Context::GetOpenContexts();

    if (!openContexts.empty())
    {
        ImGui::BeginTabBar("##OpenedFiles", ImGuiTabBarFlags_AutoSelectNewTabs | ImGuiTabBarFlags_Reorderable);

        for (size_t i = 0; i < openContexts.size(); ++i)
        {
            auto &ctx = openContexts[i];
            auto name = ctx->loadedFile.empty() ? "Untitled" : fs::GetFilename(ctx->loadedFile);

            bool isOpen = true;
            ImGui::PushID(ctx.get());

            int flags = (ctx->isDirty ? ImGuiTabItemFlags_UnsavedDocument : 0) | ImGuiTabItemFlags_NoTooltip;

            if (ImGui::BeginTabItem(name.c_str(), &isOpen, flags))
            {
                Context::SetContext(i);
                ImGui::EndTabItem();
            }

            if (!isOpen)
            {
                if (Context::GetContext().isDirty)
                {
                    m_PopupManager.OpenPopup<Popups::Prompt>(
                        "dirty_buffer_prompt",
                        "This palette has unsaved changes.\nDo you want to close?",
                        [i](void)
                        {
                            Context::RemoveContext(i);
                        }
                    );
                }
                else
                {
                    Context::RemoveContext(i);
                }
            }
        }

        ImGui::Spacing();

        if (!Context::HasNoContext())
        {
            this->DetailsBar();
            ImGui::Spacing();
            this->PaletteEditor();
        }

        ImGui::EndTabBar();
    }
    else
    {
        ImGui::Spacing();
        ImGui::TextWrapped("No files are opened.");
        ImGui::Spacing();
        ImGui::TextWrapped("Press %s or goto \"Files > New\" to create a new file.", sText_FileShortcuts[SHORT_NEW]);
        ImGui::Spacing();
        ImGui::TextWrapped("Press %s or goto \"Files > Open\" to open an existing file.", sText_FileShortcuts[SHORT_OPEN]);
    }
    ImGui::End();
}

void Editor::EndFrame(void)
{
    int display_w, display_h;

    ImGui::Render();
    glfwGetFramebufferSize(m_Window, &display_w, &display_h);
    glViewport(0, 0, display_w, display_h);
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    glfwSwapBuffers(m_Window);
}

void Editor::Loop(void)
{
    while (!glfwWindowShouldClose(m_Window))
    {
        if (glfwGetWindowAttrib(m_Window, GLFW_ICONIFIED) || 
            !glfwGetWindowAttrib(m_Window, GLFW_FOCUSED) ||
            !glfwGetWindowAttrib(m_Window, GLFW_VISIBLE))
        {
            glfwWaitEvents();
        }
        else
        {
            glfwPollEvents();
            this->StartFrame();
            this->Frame();
            this->EndFrame();
        }
    }
}

void Editor::ExitImGui(void)
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void Editor::ExitGLFW(void)
{
    glfwDestroyWindow(m_Window);
    glfwTerminate();
}

void Editor::MenuBar(void)
{
    if (ImGui::BeginMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("New", sText_FileShortcuts[SHORT_NEW]))
                Context::CreateNewContext();
            if (ImGui::MenuItem("Open", sText_FileShortcuts[SHORT_OPEN])) 
                PromptOpenPalette();
            if (ImGui::MenuItem("Save", sText_FileShortcuts[SHORT_SAVE])) 
                SavePalette(false);
            if (ImGui::MenuItem("Save As", sText_FileShortcuts[SHORT_SAVE_AS]))
                SavePalette(true);
            if (ImGui::MenuItem("Logger", nullptr))
                m_PopupManager.OpenPopup<Popups::Logger>();
            if (ImGui::MenuItem("Quit", sText_FileShortcuts[SHORT_QUIT]))
            {
                const char *s;
                if (Context::GetContext().isDirty)
                    s = "There are unsaved changes.\nDo you want to quit?";
                else
                    s = "Do you want to quit?";

                m_PopupManager.OpenPopup<Popups::Prompt>("dirty_buffer_prompt", s, [this](){
                    glfwSetWindowShouldClose(this->m_Window, GLFW_TRUE);
                });
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Edit"))
        {
            if (ImGui::MenuItem("Undo", sText_FileShortcuts[SHORT_UNDO], nullptr, Context::GetContext().actionRegister.CanUndo())) 
            {
                Context::GetContext().actionRegister.Undo();
                Context::GetContext().isDirty = true;
            }
            if (ImGui::MenuItem("Redo", sText_FileShortcuts[SHORT_REDO], nullptr, Context::GetContext().actionRegister.CanRedo())) 
            {
                Context::GetContext().actionRegister.Redo();
                Context::GetContext().isDirty = true;
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Others"))
        {
            if (ImGui::MenuItem("Combine Palettes", sText_FileShortcuts[SHORT_COMBINE])) m_PopupManager.OpenPopup<Popups::Combine>();
            if (ImGui::MenuItem("Split Palette", sText_FileShortcuts[SHORT_SPLIT], nullptr, !Context::GetOpenContexts().empty())) m_PopupManager.OpenPopup<Popups::Split>();
            ImGui::EndMenu();
        }

        ImGui::EndMenuBar();
    }
}

void Editor::DetailsBar(void)
{
    int num_colors = Context::GetContext().palette.size();

    if (ImGui::InputInt("No. of Colors", &num_colors))
        num_colors = std::min(std::max(1, num_colors), 256);

    if (ImGui::IsItemDeactivatedAfterEdit() && Context::GetContext().palette.size() != num_colors)
    {
        Context::GetContext().actionRegister.RegisterAction<Actions::ChangeColorCount>(Context::GetContext().palette.size(), (size_t)num_colors);
        Context::GetContext().isDirty = true;
    }

    ImGui::TextWrapped("Path:\n%s", Context::GetContext().loadedFile.empty() ? "No file opened." : Context::GetContext().loadedFile.c_str());
}

void Editor::PaletteEditor(void)
{
    static bool hasCachedColor = false;
    static Color cachedColor;
    auto &palette = Context::GetContext().palette;

    ImGui::BeginChild("Colors", ImVec2(0.0f, 0.0f), true, ImGuiWindowFlags_AlwaysAutoResize);
    for (int i = 0; i < palette.size(); i++)
    {
        char label[20];
        snprintf(label, 20, "Color #%i", i);

        auto &color = palette[i];
        ImGui::ColorEdit3(label, (float *)&color, ImGuiColorEditFlags_NoDragDrop);

        if (ImGui::IsItemActivated())
        {
            if (!hasCachedColor)
                cachedColor = palette[i];

            hasCachedColor = true;
        }

        if (ImGui::IsItemDeactivatedAfterEdit())
        {
            Context::GetContext().actionRegister.RegisterAction<Actions::ModifyColor>(i, cachedColor, color);
            Context::GetContext().isDirty = true;
            hasCachedColor = false;
        }

        if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
        {
            ImGui::SetDragDropPayload("ColorDND", &i, sizeof(size_t));

            const auto &col = palette[i];
            const ImVec4 col_v4(col[0], col[1], col[2], 1.0f);
            ImGui::ColorButton("##preview", col_v4); ImGui::SameLine();
            ImGui::Text("Color");
            ImGui::EndDragDropSource();
        }

        if (ImGui::BeginDragDropTarget())
        {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ColorDND"))
            {
                IM_ASSERT(payload->DataSize == sizeof(size_t));
                int target = *(const size_t*)payload->Data;
                Context::GetContext().actionRegister.RegisterAction<Actions::SwapColors>(i, target);
                Context::GetContext().isDirty = true;
            }
            ImGui::EndDragDropTarget();
        }
    }

    ImGui::EndChild();
}

void Editor::StatusBar(void)
{
    const ImGuiViewport *viewport = ImGui::GetMainViewport();

    auto barPos = ImVec2(viewport->Pos.x, viewport->Pos.y + viewport->Size.y - ImGui::GetFrameHeight());
    auto barSize = ImVec2(viewport->Size.x, ImGui::GetFrameHeight());

    ImGui::SetNextWindowPos(barPos);
    ImGui::SetNextWindowSize(barSize);

    ImGuiWindowFlags flags = 
        ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_MenuBar |
        ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoBackground | 
        ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_AlwaysAutoResize;


    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

    ImGui::Begin("##StatusBar", nullptr, flags);

    ImGui::SetCursorPosX(8);
    if (ImGui::BeginMenuBar())
    {
        const auto &undoStack = Context::GetContext().actionRegister.GetRedoStack(), &redoStack = Context::GetContext().actionRegister.GetUndoStack();
        ImGui::Text("Action Stack: %lu (Undo) | %lu (Redo)", undoStack.size(), redoStack.size());
        ImGui::EndMenuBar();
    }

    ImGui::End();
    ImGui::PopStyleVar();
}

void Editor::OpenPalette(const char *path)
{
    Context::CreateNewContext(path);
}

void Editor::PromptOpenPalette(void)
{
    fs::OpenFilePrompt([this](const char *path) { OpenPalette(path); });
}

void Editor::SavePalette(bool promptFilepath)
{
    if (Context::GetContext().loadedFile.empty() || promptFilepath)
    {
        if (!fs::SaveFilePrompt([](const char *path) { Context::GetContext().loadedFile = path; }))
            return;
    }

    Context::GetContext().palette.SaveToFile(Context::GetContext().loadedFile);
    Context::GetContext().isDirty = false;
}

void Editor::ProcessShortcuts(int key, int mods)
{
    if (m_PopupManager.IsAnyPopupOpen())
    {
        m_PopupManager.ProcessShortcuts(key, mods);
        return;
    }

    if (mods & (GLFW_MOD_CONTROL | GLFW_MOD_SUPER))
    {
        switch (key)
        {
        case GLFW_KEY_N:
            Context::CreateNewContext();
            break;
        case GLFW_KEY_O:
            PromptOpenPalette();
            break;
        case GLFW_KEY_S:
            if (mods & GLFW_MOD_SHIFT)
                SavePalette(true);
            else
                SavePalette(false);
            break;
        case GLFW_KEY_Z:
            Context::GetContext().actionRegister.Undo();
            Context::GetContext().isDirty = true;
            break;
        case GLFW_KEY_R:
            Context::GetContext().actionRegister.Redo();
            Context::GetContext().isDirty = true;
            break;
        case GLFW_KEY_K:
            if (mods & GLFW_MOD_SHIFT)
                m_PopupManager.OpenPopup<Popups::Combine>();
        case GLFW_KEY_L:
            if (mods & GLFW_MOD_SHIFT)
                m_PopupManager.OpenPopup<Popups::Split>();
            break;
        }
    }
}
