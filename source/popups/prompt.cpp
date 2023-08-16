#include "popups/prompt.hpp"
#include <imgui.h>

namespace Popups
{
    void Prompt::Draw()
    {
        ImGui::TextWrapped("%s", m_Message.c_str());

        ImGui::Spacing();

        if (ImGui::Button("Yes"))
        {
            if (m_YesCallback) m_YesCallback();
            ImGui::CloseCurrentPopup();
        }

        ImVec2 button_size = ImGui::GetItemRectSize();

        ImGui::SameLine();

        if (ImGui::Button("No", button_size))
        {
            if (m_NoCallback) m_NoCallback();
            ImGui::CloseCurrentPopup();
        }
    }
}