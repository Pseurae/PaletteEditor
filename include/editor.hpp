#ifndef EDITOR_HPP
#define EDITOR_HPP

#include <string>
#include <list>

#include "palette.hpp"
#include "popups.hpp"

#include <functional>

struct GLFWwindow;

class Editor
{
public:
    Editor();
    ~Editor();

    void Loop(void);
private:
    void InitGLFW(void);
    void InitImGui(void);

    void StartFrame(void);
    void Frame(void);
    void EndFrame(void);

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

    void ProcessShortcuts(int key, int mod);

    PopupManager m_PopupManager;
    GLFWwindow *m_Window;
    bool m_Logger = false;
};

#endif // EDITOR_HPP