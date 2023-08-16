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
    Popup(const std::string &name, bool modal) : m_Name(name), m_Modal(modal) { }
    virtual void Draw() = 0;
    constexpr const auto &GetName() const { return m_Name; }
    constexpr const bool IsModal() const { return m_Modal; }
private:
    std::string m_Name;
    bool m_Modal;
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
            m_OpenedPopups.push_front(std::make_shared<T>());
        }
        else if constexpr (requires { T(std::forward<Args>(args)...); })
        {
            m_OpenedPopups.push_front(std::make_shared<T>(std::forward<Args>(args)...));
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
private:
    void UpdatePopupStates();
    void DrawAllPopups();

    std::list<std::string> m_PopupsToOpen;
    std::list<std::shared_ptr<Popup>> m_OpenedPopups;
};

#endif // POPUPS_HPP
