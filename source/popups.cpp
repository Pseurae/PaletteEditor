#include "popups.hpp"
#include <imgui.h>

void PopupManager::UpdateAndDraw()
{
    UpdatePopupStates();
    DrawAllPopups();
}

bool PopupManager::IsAnyPopupOpen()
{
    return m_OpenedPopups.size() > 0;
}

void PopupManager::ProcessShortcuts(int key, int mods)
{
    for (auto &popup : m_OpenedPopups)
        popup->ProcessShortcuts(key, mods);
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
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10.0f, 4.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10.0f, 10.0f));

    for (auto &popup : m_OpenedPopups)
    {
        popup->PreDraw();
        bool tmp = true;

        int popupFlags = popup->GetPopupFlags() | ImGuiWindowFlags_AlwaysAutoResize;
        bool canDraw = popup->IsModal() ? 
            ImGui::BeginPopupModal(popup->GetName().c_str(), popup->ShowCloseButton() ? &tmp : nullptr, popupFlags) : 
            ImGui::BeginPopup(popup->GetName().c_str(), popupFlags);

        if (canDraw)
        {
            if (popup->GetCloseFlag())
                ImGui::CloseCurrentPopup();

            popup->Draw();
            ImGui::EndPopup();
        }
    }

    ImGui::PopStyleVar(2);
}
