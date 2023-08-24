#ifndef POPUPS_HPP
#define POPUPS_HPP

#include <string>
#include <list>
#include <memory>
#include <functional>

class Popup
{
public:
    using PopupCallback = std::function<void()>;
    Popup(const std::string &name, bool modal, bool showCloseButton = false, int flags = 0) : 
        m_Name(name), m_Modal(modal), m_ShowCloseButton(showCloseButton), m_PopupFlags(flags), m_ShouldBeClosed(false) { }
    virtual void Draw() = 0;
    virtual void PreDraw() { }
    virtual void ProcessShortcuts(int key, int mods) { } 
    constexpr const auto &GetName() const { return m_Name; }
    constexpr const bool IsModal() const { return m_Modal; }
    constexpr const int GetPopupFlags() const { return m_PopupFlags; }
    constexpr const bool ShowCloseButton() const { return m_ShowCloseButton; }
    constexpr const bool GetCloseFlag() { return m_ShouldBeClosed; } 
    constexpr void SetCloseFlag(bool shouldClose) { m_ShouldBeClosed = shouldClose; } 
private:
    std::string m_Name;
    bool m_Modal;
    bool m_ShouldBeClosed;
    bool m_ShowCloseButton;
    int m_PopupFlags;
};

class PopupManager
{
public:
    template<std::derived_from<Popup> T, typename... Args>
    void OpenPopup(Args... args)
    {
        if constexpr (std::is_abstract_v<T>)
        {
            []<bool flag = false>() {
                static_assert(flag, "Popup is still an abstract class.");
            }();
        }
        else if constexpr (std::is_empty_v<T>)
        {
            m_OpenedPopups.push_front(std::make_unique<T>());
        }
        else if constexpr (requires { T(std::forward<Args>(args)...); })
        {
            m_OpenedPopups.push_front(std::make_unique<T>(std::forward<Args>(args)...));
        }
        else
        {
            []<bool flag = false>() {
                static_assert(flag, "Popup cannot be constructed from given parameters.");
            }();
        }

        m_PopupsToOpen.push_back(m_OpenedPopups.front()->GetName());
    }

    void UpdateAndDraw();
    bool IsAnyPopupOpen();
    void ProcessShortcuts(int key, int mods);
private:
    void UpdatePopupStates();
    void DrawAllPopups();

    std::list<std::string> m_PopupsToOpen;
    std::list<std::unique_ptr<Popup>> m_OpenedPopups;
};

#endif // POPUPS_HPP
