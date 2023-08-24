#include "popups/combine.hpp"
#include "context.hpp"

#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>
#include <fstream>
#include <GLFW/glfw3.h>
#include "fs.hpp"

namespace Popups
{
    Combine::Combine() :
        Popup("CombinePalettes", true, true, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoDecoration), 
        m_Palette(0),
        m_Files{}
    {
    }

    void Combine::CombineFiles()
    {
        m_Palette.clear();
        Palette temp;

        for (auto &path : m_Files)
        {
            temp.LoadFromFile(path);
            m_Palette += temp;
        }
    }

    void Combine::PreDraw()
    {
        auto pos = ImGui::GetMainViewport()->Pos;
        auto size = ImGui::GetWindowSize();

        auto center = ImVec2(pos.x + size.x * 0.5f, pos.y + size.y * 0.5f);
        ImGui::SetNextWindowPos(center, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
        ImGui::SetNextWindowSize(size * 0.9f, ImGuiCond_Always);
    }

    void Combine::Draw()
    {
        float height = ImGui::GetContentRegionAvail().y - ImGui::GetFrameHeightWithSpacing() - ImGui::GetStyle().ItemSpacing.y;
        if (ImGui::BeginChild("##CombineWindow", ImVec2(0.0f, height)))
        {
            ImGui::BeginTabBar("CombineSections", ImGuiTabBarFlags_NoTooltip);

            if (ImGui::BeginTabItem("Files"))
            {
                ImGui::Spacing();
                FileDetails();
                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("Palette"))
            {  
                ImGui::Spacing();
                if (!m_Files.empty())
                {
                    DetailsBar();
                    PaletteEditor();
                }
                else
                {
                    ImGui::Text("No file(s) are selected.");
                }
                ImGui::EndTabItem();
            }

            ImGui::EndTabBar();

            ImGui::EndChild();
        }

        ImGui::Spacing();

        if (ImGui::Button("Load"))
        {
            Load();
            SetCloseFlag(true);
        }

        ImGui::SameLine();
        
        if (ImGui::Button("Cancel"))
            SetCloseFlag(true);
    }

    void Combine::ProcessShortcuts(int key, int mods)
    {
        if (key == GLFW_KEY_ESCAPE)
            SetCloseFlag(true);
        else if (key == GLFW_KEY_ENTER)
        {
            Load();
            SetCloseFlag(true);
        }
    }

    void Combine::Load()
    {
        size_t numFiles = m_Palette.size() / 256;
        size_t remaining = m_Palette.size() % 256;

        for (size_t i = 0; i < m_Palette.size(); ++i)
        {
            int paletteIdx = i % 256;
            if (paletteIdx == 0)
            {
                auto &ctx = Context::CreateNewContext();
                ctx.isDirty = true;
                if (i / 256 != numFiles)
                    ctx.palette.resize(256);
                else
                    ctx.palette.resize(remaining);
            }

            Context::GetContext().palette[paletteIdx] = m_Palette[i];
        }
    }

    void Combine::FileDetails()
    {
        if (ImGui::Button("Add Palette"))
        {
            if (fs::OpenFilePrompt([this](const char *path) { m_Files.push_back(path); }))
                CombineFiles();
        }

        ImGui::Spacing();

        ImGui::BeginChild("##Filenames", ImVec2(0, 0), true);

        for (size_t i = 0; i < m_Files.size(); ++i)
        {
            auto &fpath = m_Files[i];
            auto fname = fs::GetFilename(fpath);
            ImGui::BeginGroup();

            float cursorY = ImGui::GetCursorPosY();
            ImGui::SetCursorPosY(cursorY + ImGui::GetTextLineHeight() / 4.0f);

            float windowWidth = ImGui::GetContentRegionAvail().x;

            ImGui::Text(fname.c_str());

            ImGui::EndGroup();

            if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
            {
                ImGui::SetDragDropPayload("FilenameDND", &i, sizeof(size_t));
                ImGui::Text(fname.c_str());
                ImGui::EndDragDropSource();
            }

            if (ImGui::BeginDragDropTarget())
            {
                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("FilenameDND"))
                {
                    IM_ASSERT(payload->DataSize == sizeof(size_t));
                    size_t target = *(const size_t*)payload->Data;
                    std::string tmp = fpath;
                    fpath = m_Files[target];
                    m_Files[target] = tmp;

                    CombineFiles();
                }
                ImGui::EndDragDropTarget();
            }

            ImGui::SameLine();

            ImGui::SetCursorPosX(ImGui::GetWindowContentRegionMax().x - 70.0f);
            ImGui::SetCursorPosY(cursorY);

            ImGui::PushID(i);
            if (ImGui::Button("Remove", ImVec2(70.0f, 0.0f)))
            {
                m_Files.erase(m_Files.begin() + i);
                CombineFiles();
            }
            ImGui::PopID();
        }
        ImGui::EndChild();
    }

    void Combine::DetailsBar()
    {
        int num_colors = m_Palette.size();

        ImGui::InputInt("No. of Colors", &num_colors, 0, 0, ImGuiInputTextFlags_ReadOnly);

        ImGui::Spacing();

        auto windowWidth = ImGui::GetContentRegionAvail().x;

        ImGui::BeginGroup();
        ImGui::Dummy(ImVec2(windowWidth, ImGui::GetFrameHeight() * 0.25f));

        ImGui::Dummy(ImVec2(ImGui::GetFrameHeight() * 0.25f, 0.0f));
        ImGui::SameLine();
        ImGui::Text("This will load %i file(s) into the editor.", (int)std::ceilf(num_colors / 256.0f));
        ImGui::SameLine();
        ImGui::Dummy(ImVec2(ImGui::GetFrameHeight() * 0.25f, 0.0f));

        ImGui::Dummy(ImVec2(0.0f, ImGui::GetFrameHeight() * 0.25f));
        ImGui::EndGroup();

        ImGui::GetWindowDrawList()->AddRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), IM_COL32_WHITE, ImGui::GetFrameHeight() * 0.25f);

        ImGui::Spacing();
    }

    void Combine::PaletteEditor()
    {            
        ImGui::BeginChild("Colors", ImVec2(0.0f, 0.0f), true, ImGuiWindowFlags_AlwaysAutoResize);

        for (int i = 0; i < m_Palette.size(); ++i)
        {
            char label[20];
            snprintf(label, 20, "Color #%i", i);
            auto color = m_Palette[i];
            ImVec4 color_vec4 = ImVec4(color[0], color[1], color[2], 1.0f);
            ImGui::ColorButton(label, color_vec4, ImGuiColorEditFlags_NoDragDrop | ImGuiColorEditFlags_NoInputs, ImVec2(30.0f, 30.0f));

            if (i % 8 != 7)
                ImGui::SameLine();
        }

        ImGui::EndChild();
    }
}