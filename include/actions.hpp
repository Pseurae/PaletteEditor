#ifndef ACTIONS_HPP
#define ACTIONS_HPP

#include <string>
#include <list>
#include <memory>
#include <concepts>

class Editor;

class Action
{
public:
    Action() = default;
    virtual ~Action() = default;

    virtual void Revert() = 0;
    virtual void Apply() = 0;
    virtual std::string ToString() = 0;
    virtual void PrintDetails() = 0;
};

class ActionRegister
{
public:
    template<std::derived_from<Action> T, typename... Args>
    void RegisterAction(Args... args)
    {
        if constexpr (std::is_abstract_v<T>)
        {
            []<bool flag = false>() {
                static_assert(flag, "Action is still an abstract class.");
            }();
        }
        else if constexpr (std::is_empty_v<T>)
        {
            m_UndoStack.push_front(std::make_shared<T>());
        }
        else if constexpr (requires { T(std::forward<Args>(args)...); })
        {
            m_UndoStack.push_front(std::make_shared<T>(std::forward<Args>(args)...));
        }
        else
        {
            []<bool flag = false>() {
                static_assert(flag, "Action cannot be constructed from given parameters.");
            }();
        }

        m_UndoStack.front()->Apply();

        while (m_UndoStack.size() > 30)
            m_UndoStack.pop_back();

        m_RedoStack.clear();
    }

    void Undo();
    void Redo();

    void ClearUndoStack();
    void ClearRedoStack();

    bool CanUndo() { return !m_UndoStack.empty(); }
    bool CanRedo() { return !m_RedoStack.empty(); }

    const auto &GetRedoStack() const { return m_RedoStack; }
    const auto &GetUndoStack() const { return m_UndoStack; }
private:
    std::list<std::shared_ptr<Action>> m_RedoStack, m_UndoStack;
};

#endif // ACTIONS_HPP