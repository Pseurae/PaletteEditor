#pragma once

#include <string>
#include <list>

#include "palette.hpp"
#include <functional>

struct GLFWwindow;
namespace Actions { class EditActionBase; }

struct Popup
{
    enum PopupMode : char
    {
        None,
        Prompt,
        Error
    };

    using Action = std::function<void(void)>;
    std::string m_PopupMessage;
    Action m_YesAction;
    Action m_NoAction;
    PopupMode m_Mode;

    void Popups(void);
    void ShowPrompt(const std::string &msg, Action yes_action, Action no_action);
    void ShowError(const std::string &msg, Action ok_action);
};

using ActionList = std::list<std::shared_ptr<Actions::EditActionBase>>;

class Editor
{
private:
    Popup m_PopupCtrl;
    GLFWwindow *m_Window;
    Palette *m_Palette;
    float m_DPIScaling;
    std::string m_LoadedFile;
    bool m_Dirty = false;
    bool m_Logger = false;

    ActionList m_RedoStack;
    ActionList m_UndoStack;

public:
    Editor();
    ~Editor();

    void InitGLFW(void);
    void InitImGui(void);

    void StartFrame(void);
    void Frame(void);
    void EndFrame(void);

    void Loop(void);

    void ExitImGui(void);
    void ExitGLFW(void);

    void Logger(void);
    void MenuBar(void);
    void DetailsBar(void);
    void PaletteEditor(void);
    void StatusBar(void);

    void OpenPalette(const char *);
    void PromptOpenPalette(void);
    void SavePalette(bool);

    void ResizePalette(int size);

    void RegisterAction(Actions::EditActionBase *action);
    void Undo(void);
    void Redo(void);
    
    void ClearUndoStack(void);
    void ClearRedoStack(void);

    void ProcessShortcuts(int key, int mod);

    friend struct Actions::EditActionBase;
};

namespace Actions
{
struct EditActionBase
{
    EditActionBase() = default;
    virtual ~EditActionBase() = default;

    Palette *GetPalette(Editor *editor);

    virtual void Undo(Editor *) = 0;
    virtual void redo(Editor *) = 0;
    virtual std::string to_string() = 0;
    virtual void PrintDetails(Editor *) = 0;
};

struct ChangeColorCount : EditActionBase
{
    int old_size;
    int new_size;
    float *old_colors;

    ChangeColorCount(int old_size, int new_size, ColorList *old_colors);
    ~ChangeColorCount();

    virtual void redo(Editor *) override;
    virtual void Undo(Editor *) override;
    virtual std::string to_string() override { return "ChangeColorCount"; };
    virtual void PrintDetails(Editor *) override;
};

struct ModifyColor : EditActionBase
{
    int idx;
    float old_color[3];
    float new_color[3];

    ModifyColor(int idx, float *old_color, float *new_color);

    virtual void redo(Editor *) override;
    virtual void Undo(Editor *) override;
    virtual std::string to_string() override { return "ModifyColor"; };
    virtual void PrintDetails(Editor *) override;
};

struct SwapColors : EditActionBase
{
    int old_idx, new_idx;
    SwapColors(int old_idx, int new_idx) : old_idx(old_idx), new_idx(new_idx) {}
    virtual void redo(Editor *) override;
    virtual void Undo(Editor *) override;
    virtual std::string to_string() override { return "SwapColors"; };
    virtual void PrintDetails(Editor *) override;
};
}