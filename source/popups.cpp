#include "popups.hpp"
#include <imgui.h>

void PopupManager::UpdateAndDraw()
{
    UpdatePopupStates();
    DrawAllPopups();
}

void PopupManager::UpdatePopupStates()
{
    m_PopupsToOpen.remove_if([](const auto &name) {
        if (ImGui::IsPopupOpen(name.c_str()))
            return true;
        else
            ImGui::OpenPopup(name.c_str());

        return false;
    });

    m_OpenedPopups.remove_if([](const auto &popup) {
        if (ImGui::IsPopupOpen(popup->GetName().c_str()))
            return false;

        return true;
    });
}

void PopupManager::DrawAllPopups()
{
    ImGuiWindowFlags popupflags = ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoDecoration;
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10.0f, 4.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10.0f, 10.0f));

    for (auto &popup : m_OpenedPopups)
    {
        ImGui::SetNextWindowSize(ImVec2(ImGui::GetMainViewport()->Size.x / 4 * 3, 0), ImGuiCond_Always);
        ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

        bool canDraw = popup->IsModal() ? 
            ImGui::BeginPopupModal(popup->GetName().c_str(), nullptr, popupflags) : 
            ImGui::BeginPopup(popup->GetName().c_str(), popupflags);

        if (canDraw)
        {
            popup->Draw();
            ImGui::EndPopup();
        }
    }

    ImGui::PopStyleVar(2);
}
