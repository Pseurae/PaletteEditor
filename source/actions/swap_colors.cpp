#include "actions/swap_colors.hpp"
#include "context.hpp"
#include "palette.hpp"
#include <imgui.h>

static void SwapColors(Color &color1, Color &color2)
{
    Color tmp = color1;
    color1 = color2;
    color2 = tmp;
}

namespace Actions
{
    void SwapColors::Apply()
    {
        ::SwapColors(Context::palette[m_Start], Context::palette[m_End]);
    }

    void SwapColors::Revert()
    {
        ::SwapColors(Context::palette[m_Start], Context::palette[m_End]);
    }

    void SwapColors::PrintDetails()
    {
        Color color;
        ImVec4 col_v4;

        color = Context::palette[m_Start];
        col_v4 = ImVec4(color[0], color[1], color[2], 1.0f);

        ImGui::Text("%d", m_End);
        ImGui::SameLine();

        ImGui::ColorButton("##first", col_v4, 0, ImVec2(15.0f, 15.0f));
        ImGui::SameLine();

        ImGui::Text("->");
        ImGui::SameLine();

        color = Context::palette[m_End];
        col_v4 = ImVec4(color[0], color[1], color[2], 1.0f);

        ImGui::Text("%d", m_Start);
        ImGui::SameLine();
        ImGui::ColorButton("##second", col_v4, 0, ImVec2(15.0f, 15.0f));
    }
}