#include "popups/error.hpp"
#include <imgui.h>

namespace Popups
{
    Error::Error(const std::string &name, const std::string &message, PopupCallback ok) : 
        Popup(name, true, false, ImGuiWindowFlags_NoDecoration), m_Message(message), m_OkCallback(ok) {}

    void Error::PreDraw()
    {
        ImGui::SetNextWindowSize(ImVec2(ImGui::GetMainViewport()->Size.x / 4 * 3, 0), ImGuiCond_Always);
        ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    }

    void Error::Draw()
    {
        ImGui::TextWrapped("%s", m_Message.c_str());

        if (ImGui::Button("Ok"))
        {
            if (m_OkCallback) m_OkCallback();
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}
