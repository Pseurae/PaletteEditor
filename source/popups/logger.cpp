#include "popups/logger.hpp"
#include "context.hpp"

#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>

namespace Popups
{
    Logger::Logger() : Popup("Logger", true, true, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove) {}

    void Logger::PreDraw()
    {
        auto pos = ImGui::GetMainViewport()->Pos;
        auto size = ImGui::GetWindowSize();

        auto center = ImVec2(pos.x + size.x * 0.5f, pos.y + size.y * 0.5f);

        ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
        ImGui::SetNextWindowSize(size * 0.75f, ImGuiCond_Appearing);
    }

    void Logger::Draw()
    {
        auto printActions = [this](const auto &list) {
            ImGui::BeginChild("###list", ImVec2(0.0f, ImGui::GetMainViewport()->Size.y * 0.65f), true, ImGuiWindowFlags_NoDecoration);
            {
                for (auto it = list.rbegin(); it != list.rend(); ++it)
                {
                    auto action = *it;
                    ImGui::Text("%s", action->ToString().c_str());
                    ImGui::SameLine();

                    ImGui::BeginGroup();
                    action->PrintDetails();
                    ImGui::EndGroup();
                }
            }
            ImGui::EndChild();
        };

        ImGui::BeginTabBar("LoggerActions");

        if (ImGui::BeginTabItem("Undo"))
        {
            printActions(Context::GetContext().actionRegister.GetUndoStack());
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Redo"))
        {
            printActions(Context::GetContext().actionRegister.GetRedoStack());
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }
}