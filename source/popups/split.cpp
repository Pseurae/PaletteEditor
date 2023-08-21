#include "popups/split.hpp"

#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>

#include "context.hpp"

namespace Popups
{
    Split::Split() : Popup("split", true, true, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoDecoration)
    {}

    void Split::PreDraw()
    {
        auto pos = ImGui::GetMainViewport()->Pos;
        auto size = ImGui::GetWindowSize();

        auto center = ImVec2(pos.x + size.x * 0.5f, pos.y + size.y * 0.5f);
        ImGui::SetNextWindowPos(center, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
        ImGui::SetNextWindowSize(size * 0.9f, ImGuiCond_Always);
    }

    void Split::Load()
    {
        auto &palette = Context::GetContext().palette;

        size_t numFiles = palette.size() / m_NumColors;
        size_t remaining = palette.size() % m_NumColors;

        for (size_t i = 0; i < palette.size(); ++i)
        {
            if (i % m_NumColors == 0)
            {
                auto &ctx = Context::CreateNewContext();
                ctx.isDirty = true;
                if (i / m_NumColors != numFiles)
                    ctx.palette.resize(m_NumColors);
                else
                    ctx.palette.resize(remaining);
            }

            Context::GetContext().palette[i % m_NumColors] = palette[i];
        }
    }

    void Split::Draw()
    {
        if (ImGui::Button("Load"))
        {
            Load();
            ImGui::CloseCurrentPopup();
        }

        ImGui::SameLine();
        
        if (ImGui::Button("Cancel"))
            ImGui::CloseCurrentPopup();

        ImGui::Spacing();

        int numColors = Context::GetContext().palette.size();
        ImGui::InputInt("Total Colors", &numColors, 0, 0, ImGuiInputTextFlags_ReadOnly);
    
        int oldNumColors = m_NumColors;
        if (ImGui::InputInt("Colors Per File", &oldNumColors, 1, 5))
            m_NumColors = (size_t)std::min(std::max(1ULL, (size_t)oldNumColors), Context::GetContext().palette.size());

        ImGui::Spacing();

        auto windowWidth = ImGui::GetContentRegionAvail().x;

        ImGui::BeginGroup();
        ImGui::Dummy(ImVec2(windowWidth, ImGui::GetFrameHeight() * 0.25f));

        ImGui::Dummy(ImVec2(ImGui::GetFrameHeight() * 0.25f, 0.0f));
        ImGui::SameLine();
        ImGui::Text("This will load %i file(s) into the editor.", (int)std::ceilf(numColors / (m_NumColors * 1.0f)));
        ImGui::SameLine();
        ImGui::Dummy(ImVec2(ImGui::GetFrameHeight() * 0.25f, 0.0f));

        ImGui::Dummy(ImVec2(0.0f, ImGui::GetFrameHeight() * 0.25f));
        ImGui::EndGroup();

        ImGui::GetWindowDrawList()->AddRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), IM_COL32_WHITE, ImGui::GetFrameHeight() * 0.25f);

        ImGui::Spacing();
    }
}