#include "actions/change_color_count.hpp"
#include "context.hpp"

#include <imgui.h>

namespace Actions
{
    ChangeColorCount::ChangeColorCount(size_t _old, size_t _new) : m_OldSize(_old), m_NewSize(_new)
    {
        size_t removed_colors = std::max(0LL, (long long)(_old - _new));
        for (size_t i = 0; i < removed_colors; ++i)
            m_Colors.push_back(Context::GetContext().palette[_new + i]);
    }

    void ChangeColorCount::Apply()
    {
        Context::GetContext().palette.resize(m_NewSize);
    }

    void ChangeColorCount::Revert()
    {
        Context::GetContext().palette.resize(m_OldSize);

        for (size_t i = 0; i < m_Colors.size(); ++i)
            Context::GetContext().palette[m_NewSize + i] = m_Colors[i];
    }

    void ChangeColorCount::PrintDetails()
    {
        ImGui::Text("%d -> %d", m_OldSize, m_NewSize);
    }
}