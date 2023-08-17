#include "actions/modify_color.hpp"
#include "context.hpp"
#include <imgui.h>

namespace Actions
{
    void ModifyColor::Apply()
    {
        Context::GetContext().palette[m_Index] = m_NewColor;
    }

    void ModifyColor::Revert()
    {
        Context::GetContext().palette[m_Index] = m_OldColor;
    }

    void ModifyColor::PrintDetails()
    {
        ImVec4 col;

        ImGui::Text("%d", m_Index);
        ImGui::SameLine();

        col = ImVec4(m_OldColor[0], m_OldColor[1], m_OldColor[2], 1.0);
        ImGui::ColorButton("##old", col, 0, ImVec2(15.0f, 15.0f));

        ImGui::SameLine();
        ImGui::Text("->");
        ImGui::SameLine();

        col = ImVec4(m_NewColor[0], m_NewColor[1], m_NewColor[2], 1.0);
        ImGui::ColorButton("##new", col, 0, ImVec2(15.0f, 15.0f));
    }
}