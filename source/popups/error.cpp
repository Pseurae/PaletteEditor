#include "popups/error.hpp"
#include <imgui.h>

namespace Popups
{
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
