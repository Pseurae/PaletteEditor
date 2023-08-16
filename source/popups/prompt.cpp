#include "popups/prompt.hpp"
#include <imgui.h>

namespace Popups
{
    Prompt::Prompt(const std::string &name, const std::string &message, PopupCallback yes, PopupCallback no) : 
            Popup(name, true, false, ImGuiWindowFlags_NoDecoration), m_Message(message), m_YesCallback(yes), m_NoCallback(no) {}

    void Prompt::PreDraw()
    {
        ImGui::SetNextWindowSize(ImVec2(ImGui::GetMainViewport()->Size.x / 4 * 3, 0), ImGuiCond_Always);
        ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    }

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