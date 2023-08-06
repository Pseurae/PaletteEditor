#define GL_SILENCE_DEPRECATION
#include <GLFW/glfw3.h>

#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui.h"
#include "imgui_internal.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "tinyfiledialogs.h"

#include "editor.hpp"
#include "palette.hpp"

#include <iostream>
#include <algorithm>
#include <fstream>

void Popup::Popups(void)
{
    ImGuiWindowFlags popupflags = ImGuiWindowFlags_AlwaysAutoResize;
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10.0f, 4.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10.0f, 10.0f));

    switch (this->m_Mode)
    {
    case PopupMode::None:
        break;
    case PopupMode::Prompt:
        ImGui::OpenPopup("Prompt");
        break;
    case PopupMode::Error:
        ImGui::OpenPopup("Error");
        break;    
    }

    ImGui::SetNextWindowSize(ImVec2(ImGui::GetMainViewport()->Size.x / 4 * 3, 0), ImGuiCond_Always);
    ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    if (ImGui::BeginPopupModal("Prompt", NULL, popupflags))
    {
        ImGui::TextWrapped("%s", m_PopupMessage.c_str());

        ImGui::Spacing();

        if (ImGui::Button("Yes"))
        {
            if (m_YesAction != nullptr)
                m_YesAction();
            ImGui::CloseCurrentPopup();
        }

        ImVec2 button_size = ImGui::GetItemRectSize();

        ImGui::SameLine();

        if (ImGui::Button("No", button_size))
        {
            if (m_NoAction != nullptr)
                m_NoAction();
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }

    ImGui::SetNextWindowSize(ImVec2(ImGui::GetMainViewport()->Size.x / 4 * 3, 0), ImGuiCond_Always);
    ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    if (ImGui::BeginPopupModal("Error", NULL, popupflags))
    {
        ImGui::TextWrapped("%s", m_PopupMessage.c_str());

        if (ImGui::Button("Ok"))
        {
            if (m_YesAction != nullptr)
                m_YesAction();
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

    ImGui::PopStyleVar(2);
    this->m_Mode = PopupMode::None;
}

void Popup::ShowPrompt(const std::string &msg, Popup::Action yes_action, Popup::Action no_action)
{
    m_PopupMessage = msg;
    m_YesAction = yes_action;
    m_NoAction = no_action;
    m_Mode = PopupMode::Prompt;
}

void Popup::ShowError(const std::string& msg, Popup::Action ok_action)
{
    m_PopupMessage = msg;
    m_YesAction = ok_action;
    m_Mode = PopupMode::Error;
}

static void glfw_error_callback(int error, const char *description)
{
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

Editor::Editor()
{
    this->InitGLFW();
    this->InitImGui();
    m_Palette = new Palette();
}

Editor::~Editor()
{
    this->ExitImGui();
    this->ExitGLFW();
    this->ClearUndoStack();
    this->ClearRedoStack();
    delete m_Palette;
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
        if (editor->m_Dirty)
        {
            glfwSetWindowShouldClose(window, GLFW_FALSE);
            editor->m_PopupCtrl.ShowPrompt("There are unsaved changes.\nDo you want to quit?", [window](){
                glfwSetWindowShouldClose(window, GLFW_TRUE);
            }, nullptr);
        }
    });

    m_DPIScaling = std::max((float)fb_w / win_w, (float)fb_h / win_h);
}

void Editor::InitImGui(void)
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();

    ImFontConfig font_cfg;
    font_cfg.FontDataOwnedByAtlas = false;
    // io.Fonts->AddFontFromMemoryTTF((void *)sFont_ProggyClean, sFont_ProggyCleanLen, 13.0f * m_DPIScaling, &font_cfg);
    // io.FontGlobalScale /= m_DPIScaling;

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

    ImGui::Begin("PalEditor", NULL, windowflags);
    this->Logger();
    m_PopupCtrl.Popups();
    this->MenuBar();
    ImGui::End();

    this->StatusBar();
}

void Editor::Frame(void)
{
    ImGui::Begin("PalEditor", NULL, 0);
    this->DetailsBar();
    ImGui::Spacing();
    this->PaletteEditor();
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

void Editor::Logger(void)
{
    ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImGui::GetMainViewport()->Size * 0.75, ImGuiCond_Appearing);
    int flags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove;
    bool tmp = true;

    auto printActions = [this](const ActionList &list) {
        ImGui::BeginChild("###list", ImVec2(0.0f, 0.0f), true, ImGuiWindowFlags_NoDecoration);
        {
            for (auto it = list.rbegin(); it != list.rend(); ++it)
            {
                auto action = *it;
                ImGui::Text("%s", action->to_string().c_str());
                ImGui::SameLine();

                ImGui::BeginGroup();
                action->PrintDetails(this);
                ImGui::EndGroup();
            }
        }
        ImGui::EndChild();
    };

    if (ImGui::BeginPopupModal("Logger", &tmp, flags))
    {
        ImGui::BeginTabBar("LoggerActions");

        if (ImGui::BeginTabItem("Undo"))
        {
            printActions(m_UndoStack);
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Redo"))
        {
            printActions(m_RedoStack);
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();

        ImGui::EndPopup();
    }

    if (this->m_Logger)
        ImGui::OpenPopup("Logger");

    this->m_Logger = false;
}

#if defined(__APPLE__)
static const char *sText_FileShortcuts[] =
{
    "Cmd+O",
    "Cmd+S",
    "Cmd+Shift+S",
    "Cmd+Q"
};
#else
static const char *sText_FileShortcuts[] =
{
    "Ctrl+O",
    "Ctrl+S",
    "Ctrl+Shift+S",
    "Ctrl+Q"
};
#endif

void Editor::MenuBar(void)
{
    if (ImGui::BeginMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("Open", sText_FileShortcuts[0])) 
            {
                this->PromptOpenPalette();
            }
            if (ImGui::MenuItem("Save", sText_FileShortcuts[1])) 
            {
                this->SavePalette(false);
            }
            if (ImGui::MenuItem("Save As", sText_FileShortcuts[2]))
            {
                this->SavePalette(true);
            }
            if (ImGui::MenuItem("Logger", nullptr))
            {
                this->m_Logger = true;
            }
            if (ImGui::MenuItem("Quit", sText_FileShortcuts[3]))
            {
                const char *s;
                if (this->m_Dirty)
                    s = "There are unsaved changes.\nDo you want to quit?";
                else
                    s = "Do you want to quit?";

                m_PopupCtrl.ShowPrompt(s, [this](){
                    glfwSetWindowShouldClose(this->m_Window, GLFW_TRUE);
                }, nullptr);
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Edit"))
        {
#if defined(__APPLE__)
            if (ImGui::MenuItem("Undo", "Cmd+Z", false, !m_UndoStack.empty())) this->Undo();
            if (ImGui::MenuItem("Redo", "Cmd+R", false, !m_RedoStack.empty())) this->Redo();
#else
            if (ImGui::MenuItem("Undo", "Ctrl+Z", false, !m_UndoStack.empty())) this->Undo();
            if (ImGui::MenuItem("Redo", "Ctrl+R", false, !m_RedoStack.empty())) this->Redo();
#endif
            ImGui::EndMenu();
        }

        ImGui::EndMenuBar();
    }
}

void Editor::DetailsBar(void)
{
    int old_num = m_Palette->GetPaletteSize();
    int num_colors = m_Palette->GetPaletteSize();

    if (ImGui::InputInt("No. of Colors", &num_colors))
        num_colors = std::min(std::max(1, num_colors), 255);

    if (ImGui::IsItemDeactivatedAfterEdit() && old_num != num_colors)
        this->ResizePalette(num_colors);

    ImGui::TextWrapped("Path:\n%s", m_LoadedFile.empty() ? "No file opened." : m_LoadedFile.c_str());
}

static void SwapColors(Color &c1, Color &c2)
{
    Color tmp = c2;
    c2 = c1;
    c1 = tmp;
}

void Editor::PaletteEditor(void)
{
    static bool has_cached_color = false;
    static float cached_color[3] = { 0.0f };

    ColorList &colorlist = m_Palette->GetColorList();

    ImGui::BeginChild("Colors", ImVec2(0.0f, 0.0f), true, ImGuiWindowFlags_AlwaysAutoResize);
    for (int i = 0; i < colorlist.size(); i++)
    {
        char label[10];
        snprintf(label, 10, "Color #%i", i);

        ImGui::ColorEdit3(label, colorlist.at(i).data(), ImGuiColorEditFlags_NoDragDrop);

        if (ImGui::IsItemActivated())
        {
            if (!has_cached_color)
                memcpy(cached_color, colorlist.at(i).data(), sizeof(float) * 3);

            has_cached_color = true;
        }

        if (ImGui::IsItemDeactivatedAfterEdit())
        {
            this->RegisterAction(new Actions::ModifyColor(i, cached_color, colorlist.at(i).data()));
            has_cached_color = false;
            m_Dirty = true;
        }

        if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
        {
            ImGui::SetDragDropPayload("ColorDND", &i, sizeof(int));

            const float *col = colorlist.at(i).data();
            const ImVec4 col_v4(col[0], col[1], col[2], 1.0f);
            ImGui::ColorButton("##preview", col_v4); ImGui::SameLine();
            ImGui::Text("Color");
            ImGui::EndDragDropSource();
        }

        if (ImGui::BeginDragDropTarget())
        {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ColorDND"))
            {
                IM_ASSERT(payload->DataSize == sizeof(int));
                int target = *(const int*)payload->Data;
                SwapColors(colorlist.at(i), colorlist.at(target));
                this->RegisterAction(new Actions::SwapColors(i, target));
                m_Dirty = true;
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
        ImGui::Text("Action Stack: %lu (Undo) | %lu (Redo)", m_UndoStack.size(), m_RedoStack.size());
        ImGui::EndMenuBar();
    }

    ImGui::End();
    ImGui::PopStyleVar();
}

static constexpr const char *sFilterPatterns[] = { "*.pal" };

void Editor::OpenPalette(const char *path)
{

    std::string new_file = std::string(path, strlen(path));
    std::ifstream palfile(new_file, std::ios::in);

    if (palfile.is_open())
    {
        try
        {
            m_Palette->LoadFromFile(palfile);
        }
        catch (const char *c)
        {
            this->m_PopupCtrl.ShowError(c, nullptr);
            return;
        }
    }

    ClearUndoStack();
    ClearRedoStack();

    palfile.close();

    m_LoadedFile = new_file;
    m_Dirty = false;
}

void Editor::PromptOpenPalette(void)
{
    if (this->m_Dirty)
    {
        m_PopupCtrl.ShowPrompt(
            "Opening another palette will discard unsaved changes.\nDo you want to continue?",
            [this](void)
            {
                char *path = tinyfd_openFileDialog("Open Palette File", NULL, 1, sFilterPatterns, "Palette Files (*.pal)", 0);
                if (!path)
                    return;
                this->OpenPalette(path);
            },
            nullptr
        );
    }
    else
    {
        char *path = tinyfd_openFileDialog("Open Palette File", NULL, 1, sFilterPatterns, "Palette Files (*.pal)", 0);
        if (!path)
            return;
        this->OpenPalette(path);
    }
}

void Editor::SavePalette(bool promptFilepath)
{
    if (m_LoadedFile.empty() || promptFilepath)
    {
        char *path = tinyfd_saveFileDialog("Save Palette File", NULL, 1, sFilterPatterns, "Test");
        if (!path)
            return;

        m_LoadedFile = std::string(path, strlen(path));
    }

    std::ofstream palfile(m_LoadedFile, std::ios::out);
    if (palfile.is_open())
    {
        try
        {
            m_Palette->SaveToFile(palfile);
        }
        catch (const char *c)
        {
            printf("%s\n", c);
            return;
        }
    }

    m_Dirty = false;
    palfile.close();
}

void Editor::ResizePalette(int size)
{
    int old_size = m_Palette->GetPaletteSize();
    this->RegisterAction(new Actions::ChangeColorCount(old_size, size, &m_Palette->GetColorList()));
    m_Palette->ResizePalette(size);
    m_Dirty = true;
}

void Editor::RegisterAction(Actions::EditActionBase *action)
{
    m_UndoStack.push_front(std::shared_ptr<Actions::EditActionBase>(action));

    if (m_UndoStack.empty())
        return;

    while (m_UndoStack.size() > 30)
        m_UndoStack.pop_back();

    this->ClearRedoStack();
}

void Editor::Undo(void)
{
    if (m_UndoStack.empty())
        return;

    auto action = m_UndoStack.front();
    action->Undo(this);
    m_UndoStack.pop_front();
    m_RedoStack.push_front(std::shared_ptr<Actions::EditActionBase>(action));
}

void Editor::Redo(void)
{
    if (m_RedoStack.empty())
        return;

    auto action = m_RedoStack.front();
    action->redo(this);
    m_RedoStack.pop_front();
    m_UndoStack.push_front(std::shared_ptr<Actions::EditActionBase>(action));
}

void Editor::ClearUndoStack(void)
{
    m_UndoStack.clear();
}

void Editor::ClearRedoStack(void)
{
    m_RedoStack.clear();
}

void Editor::ProcessShortcuts(int key, int mods)
{
    if (mods & (GLFW_MOD_CONTROL | GLFW_MOD_SUPER))
    {
        switch (key)
        {
        case GLFW_KEY_O:
            this->PromptOpenPalette();
            break;
        case GLFW_KEY_S:
            if (mods & GLFW_MOD_SHIFT)
                this->SavePalette(true);
            else
                this->SavePalette(false);
            break;
        case GLFW_KEY_Z:
            this->Undo();
            break;
        case GLFW_KEY_R:
            this->Redo();
            break;
        }
    }
}

namespace Actions
{
    Palette *EditActionBase::GetPalette(Editor *editor)
    {
        return editor->m_Palette;
    }

    ChangeColorCount::ChangeColorCount(int old_size, int new_size, ColorList *old_colors)
    {
        this->old_size = old_size;
        this->new_size = new_size;
        if (old_size > new_size)
        {
            this->old_colors = static_cast<float *>(calloc((old_size - new_size), sizeof(float) * 3));

            for (int i = new_size; i < old_size; i++)
            {
                memcpy(this->old_colors + i * 3, old_colors->at(i).data(), sizeof(float) * 3);
            }
        }
        else
        {
            this->old_colors = nullptr;
        }
    }

    ChangeColorCount::~ChangeColorCount()
    {
        if (this->old_colors != nullptr)
            free(this->old_colors);
    }

    void ChangeColorCount::Undo(Editor *editor)
    {
        Palette *palette = this->GetPalette(editor);
        palette->ResizePalette(old_size);
        if (this->old_colors != nullptr)
        {
            for (int i = new_size; i < old_size; i++)
            {
                memcpy(palette->GetColorList().at(i).data(), this->old_colors + i * 3, sizeof(float) * 3);
            }
        }
    }

    void ChangeColorCount::redo(Editor *editor)
    {
        Palette *palette = this->GetPalette(editor);
        palette->ResizePalette(new_size);
    }

    void ChangeColorCount::PrintDetails(Editor *editor)
    {
        ImGui::Text("%d -> %d", old_size, new_size);
    }

    ModifyColor::ModifyColor(int idx, float *old_color, float *new_color)
    {
        this->idx = idx;
        memcpy(this->old_color, old_color, sizeof(float) * 3);
        memcpy(this->new_color, new_color, sizeof(float) * 3);
    }

    void ModifyColor::Undo(Editor *editor)
    {
        Palette *palette = this->GetPalette(editor);
        float *color = palette->GetColorList().at(idx).data();
        memcpy(color, this->old_color, sizeof(float) * 3);
    }

    void ModifyColor::redo(Editor *editor)
    {
        Palette *palette = this->GetPalette(editor);
        float *color = palette->GetColorList().at(idx).data();
        memcpy(color, this->new_color, sizeof(float) * 3);
    }

    void ModifyColor::PrintDetails(Editor *editor)
    {
        ImVec4 col;

        ImGui::Text("%d", idx);
        ImGui::SameLine();

        col = ImVec4(old_color[0], old_color[1], old_color[2], 1.0);
        ImGui::ColorButton("##old", col, 0, ImVec2(15.0f, 15.0f));

        ImGui::SameLine();
        ImGui::Text("->");
        ImGui::SameLine();

        col = ImVec4(new_color[0], new_color[1], new_color[2], 1.0);
        ImGui::ColorButton("##new", col, 0, ImVec2(15.0f, 15.0f));
    }

    void SwapColors::Undo(Editor *editor)
    {
        Palette *palette = this->GetPalette(editor);
        ::SwapColors(palette->GetColorList().at(old_idx), palette->GetColorList().at(new_idx));
    }

    void SwapColors::redo(Editor *editor)
    {
        Palette *palette = this->GetPalette(editor);
        ::SwapColors(palette->GetColorList().at(old_idx), palette->GetColorList().at(new_idx));
    }

    void SwapColors::PrintDetails(Editor *editor)
    {
        float *col;
        ImVec4 col_v4;
        Palette *palette = this->GetPalette(editor);

        col = palette->GetColorList().at(new_idx).data();
        col_v4 = ImVec4(col[0], col[1], col[2], 1.0f);

        ImGui::Text("%d", old_idx);
        ImGui::SameLine();

        ImGui::ColorButton("##first", col_v4, 0, ImVec2(15.0f, 15.0f));
        ImGui::SameLine();

        ImGui::Text("->");
        ImGui::SameLine();

        col = palette->GetColorList().at(old_idx).data();
        col_v4 = ImVec4(col[0], col[1], col[2], 1.0f);

        ImGui::Text("%d", new_idx);
        ImGui::SameLine();
        ImGui::ColorButton("##second", col_v4, 0, ImVec2(15.0f, 15.0f));
    }
}