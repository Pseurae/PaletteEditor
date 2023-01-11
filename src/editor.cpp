#define GL_SILENCE_DEPRECATION
#include <GLFW/glfw3.h>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "tinyfiledialogs.h"

#include "editor.hpp"
#include "palette.hpp"

#include <algorithm>
#include <fstream>

#if defined(__APPLE__)
asm(
    ".const_data\n"
    ".globl _sFont_ProggyClean\n"
    ".align 4\n"
    "_sFont_ProggyClean:\n"
    "   .incbin \"ProggyClean.ttf\"\n"
    ".globl _sFont_ProggyCleanLen\n"
    ".align 4\n"
    "_sFont_ProggyCleanLen:\n"
    "   .long . - _sFont_ProggyClean\n"
);
#else
asm(
    ".section .rodata\n"
    ".global sFont_ProggyClean\n"
    ".align 4\n"
    "sFont_ProggyClean:\n"
    "   .incbin \"ProggyClean.ttf\"\n"
    ".global sFont_ProggyCleanLen\n"
    ".align 4\n"
    "sFont_ProggyCleanLen:\n"
    "   .long sFont_ProggyCleanLen - sFont_ProggyClean"
);
#endif

extern const unsigned char sFont_ProggyClean[];
extern const unsigned long sFont_ProggyCleanLen;

void Popup::Popups(void)
{
    ImGuiWindowFlags popupflags = ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse;
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10.0f, 4.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10.0f, 10.0f));

    ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    if (ImGui::BeginPopupModal("Prompt", NULL, popupflags))
    {
        ImGui::Text("%s", this->m_PopupMessage.c_str());

        ImGui::Spacing();

        if (ImGui::Button("Yes"))
        {
            if (this->m_YesAction != nullptr)
                this->m_YesAction();
            ImGui::CloseCurrentPopup();
        }

        ImVec2 button_size = ImGui::GetItemRectSize();

        ImGui::SameLine();

        if (ImGui::Button("No", button_size))
        {
            if (this->m_NoAction != nullptr)
                this->m_NoAction();
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }

    ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    if (ImGui::BeginPopupModal("Error", NULL, popupflags))
    {
        ImGui::Text("%s", this->m_PopupMessage.c_str());

        if (ImGui::Button("Ok"))
        {
            if (this->m_YesAction != nullptr)
                this->m_YesAction();
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

    ImGui::PopStyleVar(2);

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

    this->m_Mode = PopupMode::None;
}

void Popup::ShowPrompt(std::string msg, Popup::Action yes_action, Popup::Action no_action)
{
    this->m_PopupMessage = msg;
    this->m_YesAction = yes_action;
    this->m_NoAction = no_action;

    this->m_Mode = PopupMode::Prompt;
}

void Popup::ShowError(std::string msg, Popup::Action ok_action)
{
    this->m_PopupMessage = msg;
    this->m_YesAction = ok_action;

    this->m_Mode = PopupMode::Error;
}

static void glfw_error_callback(int error, const char *description)
{
    fprintf(stderr, "Glfw Error %d: %s\n", error, description);
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

    glfwSetWindowUserPointer(this->m_Window, this);
    glfwSetWindowSizeLimits(this->m_Window, 400, 400, 400, 400);

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

    m_DPIScaling = std::max((float)fb_w / win_w, (float)fb_h / win_h);
}

void Editor::InitImGui(void)
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();

    ImFontConfig font_cfg;
    font_cfg.FontDataOwnedByAtlas = false;
    io.Fonts->AddFontFromMemoryTTF((void *)sFont_ProggyClean, sFont_ProggyCleanLen, 13.0f * m_DPIScaling, &font_cfg);
    io.FontGlobalScale /= m_DPIScaling;

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
    ImGui::SetNextWindowSize(viewport->Size);

    ImGui::Begin("PalEditor", NULL, windowflags);
    this->m_PopupCtrl.Popups();
    this->MenuBar();
    ImGui::End();
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
    while (!glfwWindowShouldClose(this->m_Window))
    {
        if (glfwGetWindowAttrib(this->m_Window, GLFW_ICONIFIED) || 
            !glfwGetWindowAttrib(this->m_Window, GLFW_FOCUSED) ||
            !glfwGetWindowAttrib(this->m_Window, GLFW_VISIBLE))
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
    glfwDestroyWindow(this->m_Window);
    glfwTerminate();
}

#if defined(__APPLE__)
static const char *sText_FileShortcuts[3] =
{
    "Cmd+O",
    "Cmd+S",
    "Cmd+Shift+S",
};
#else
static const char *sText_FileShortcuts[3] =
{
    "Ctrl+O",
    "Ctrl+S",
    "Ctrl+Shift+S",
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
                if (this->m_Dirty)
                {
                    m_PopupCtrl.ShowPrompt(
                        "Opening another palette will discard unsaved changes.\nDo you want to continue?",
                        [this](void)
                        {
                            this->OpenPalette();
                        },
                        []() {}
                    );
                }
                else
                {
                    OpenPalette();
                }
            }
            if (ImGui::MenuItem("Save", sText_FileShortcuts[1])) 
            {
                SavePalette(false);
            }
            if (ImGui::MenuItem("Save As", sText_FileShortcuts[2]))
            {
                SavePalette(true);
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
    // ImGui::BeginChild("Details", ImVec2(0.0f, 0.0f), true, ImGuiWindowFlags_AlwaysAutoResize);
    int num_colors = m_Palette->GetPaletteSize();

    if (ImGui::InputInt("No. of Colors", &num_colors))
    {
        num_colors = std::min(std::max(1, num_colors), 255);
    }

    if (ImGui::IsItemDeactivatedAfterEdit())
        this->ResizePalette(num_colors);

    ImGui::TextWrapped("Path: %s", m_LoadedFile.empty() ? "No file opened." : m_LoadedFile.c_str());
    // ImGui::EndChild();
}

void Editor::PaletteEditor(void)
{
    static bool has_cached_color = false;
    static float cached_color[3] = { 0.0f };

    ColorList *colorlist = m_Palette->GetColorList();

    ImGui::BeginChild("Colors", ImVec2(0.0f, 0.0f), true, ImGuiWindowFlags_AlwaysAutoResize);
    for (int i = 0; i < colorlist->size(); i++)
    {
        char label[10];
        snprintf(label, 10, "Color #%i", i);

        ImGui::ColorEdit3(label, colorlist->at(i).data());

        if (ImGui::IsItemActivated())
        {
            if (!has_cached_color)
                memcpy(cached_color, colorlist->at(i).data(), sizeof(float) * 3);
            has_cached_color = true;
        }

        if (ImGui::IsItemDeactivatedAfterEdit())
        {
            this->RegisterAction(new Actions::ModifyColor(i, cached_color, colorlist->at(i).data()));
            m_Dirty = true;
            has_cached_color = false;
        }
    }

    ImGui::EndChild();
}

static constexpr const char *sFilterPatterns[] = { "*.pal" };

void Editor::OpenPalette(void)
{
    char *path = tinyfd_openFileDialog("Open Palette File", NULL, 1, sFilterPatterns, "Palette Files (*.pal)", 0);
    if (!path)
        return;

    std::string new_file = std::string(path, strlen(path));
    std::ifstream palfile(new_file, std::ios::in);

    if (palfile.is_open())
    {
        try
        {
            m_Palette->LoadFromFile(palfile);
        }
        catch (std::string c)
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
    this->RegisterAction(new Actions::ChangeColorCount(old_size, size, m_Palette->GetColorList()));
    m_Palette->ResizePalette(size);
    m_Dirty = true;
}

void Editor::RegisterAction(Actions::EditActionBase *action)
{
    m_UndoStack.push_front(action);

    if (m_UndoStack.empty())
        return;

    while (m_UndoStack.size() > 30)
    {
        delete m_UndoStack.back();
        m_UndoStack.pop_back();
    }

    this->ClearRedoStack();
}

void Editor::Undo(void)
{
    if (m_UndoStack.empty())
        return;

    Actions::EditActionBase *action = m_UndoStack.front();
    action->undo(this);
    m_UndoStack.pop_front();
    m_RedoStack.push_front(action);
}

void Editor::Redo(void)
{
    if (m_RedoStack.empty())
        return;

    Actions::EditActionBase *action = m_RedoStack.front();
    action->redo(this);
    m_RedoStack.pop_front();
    m_UndoStack.push_front(action);
}

void Editor::ClearUndoStack(void)
{
    while (m_UndoStack.size())
    {
        delete m_UndoStack.back();
        m_UndoStack.pop_back();
    }
}

void Editor::ClearRedoStack(void)
{
    while (m_RedoStack.size())
    {
        delete m_RedoStack.back();
        m_RedoStack.pop_back();
    }
}

void Editor::ProcessShortcuts(int key, int mods)
{
    if (mods & (GLFW_MOD_CONTROL | GLFW_MOD_SUPER))
    {
        switch (key)
        {
        case GLFW_KEY_O:
            if (this->m_Dirty)
                m_PopupCtrl.ShowPrompt(
                    "Opening another palette will discard unsaved changes.\nDo you want to continue?",
                    [this](void) { this->OpenPalette(); }, nullptr
                );
            else
                OpenPalette();
            break;
        case GLFW_KEY_S:
            if (mods & GLFW_MOD_SHIFT)
                SavePalette(true);
            else
                SavePalette(false);
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

    void ChangeColorCount::undo(Editor *editor)
    {
        Palette *palette = this->GetPalette(editor);
        palette->ResizePalette(old_size);
        if (this->old_colors != nullptr)
        {
            for (int i = new_size; i < old_size; i++)
            {
                memcpy(palette->GetColorList()->at(i).data(), this->old_colors + i * 3, sizeof(float) * 3);
            }
        }
    }

    void ChangeColorCount::redo(Editor *editor)
    {
        Palette *palette = this->GetPalette(editor);
        palette->ResizePalette(new_size);
    }

    ModifyColor::ModifyColor(int idx, float *old_color, float *new_color)
    {
        this->idx = idx;
        memcpy(this->old_color, old_color, sizeof(float) * 3);
        memcpy(this->new_color, new_color, sizeof(float) * 3);
    }

    void ModifyColor::undo(Editor *editor)
    {
        Palette *palette = this->GetPalette(editor);
        float *color = palette->GetColorList()->at(idx).data();
        memcpy(color, this->old_color, sizeof(float) * 3);
    }

    void ModifyColor::redo(Editor *editor)
    {
        Palette *palette = this->GetPalette(editor);
        float *color = palette->GetColorList()->at(idx).data();
        memcpy(color, this->new_color, sizeof(float) * 3);
    }
}