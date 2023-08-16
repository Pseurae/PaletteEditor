#include "actions/modify_color.hpp"
#include "context.hpp"

namespace Actions
{
    void ModifyColor::Apply()
    {
        Context::palette[m_Index] = m_NewColor;
    }

    void ModifyColor::Revert()
    {
        Context::palette[m_Index] = m_OldColor;
    }
}