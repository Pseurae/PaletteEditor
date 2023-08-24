#include "actions.hpp"

void ActionRegister::Undo()
{
    if (m_UndoStack.empty())
        return;

    auto action = m_UndoStack.front();
    action->Revert();
    m_UndoStack.pop_front();
    m_RedoStack.push_front(action);
}

void ActionRegister::Redo()
{
    if (m_RedoStack.empty())
        return;

    auto action = m_RedoStack.front();
    action->Apply();
    m_RedoStack.pop_front();
    m_UndoStack.push_front(action);
}

void ActionRegister::ClearUndoStack()
{
    m_UndoStack.clear();
}

void ActionRegister::ClearRedoStack()
{
    m_RedoStack.clear();
}
