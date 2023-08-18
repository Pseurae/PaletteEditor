#include "popups/combine.hpp"
#include "context.hpp"

#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>
#include <fstream>
#include <filesystem>
#include <nfd.h>

namespace Popups
{
    // TODO: Currently ignores the 256 color limit.
    Combine::Combine() :
        Popup("combine", true, true, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoDecoration) 
    {
    }

    void Combine::CombineFiles()
    {
        m_Palette.clear();
        Palette temp;

        for (auto &path : m_Files)
        {
            std::ifstream stream(path);
            if (stream.is_open())
            {
                temp.LoadFromFile(stream);
                m_Palette += temp;
            }
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
        ImGui::BeginTabBar("LoggerActions");

        if (ImGui::BeginTabItem("Files"))
        {
            FileDetails();
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Palette"))
        {  
            auto size = ImGui::GetWindowContentRegionMax();
            
            DetailsBar();
            PaletteEditor();
            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }

    void Combine::Load()
    {
        Context::GetContext().palette = m_Palette;
    }

    static const nfdfilteritem_t sFilterPatterns[] = { {"Palette Files", "pal"} };

    void Combine::FileDetails()
    {
        if (ImGui::Button("Add Palette"))
        {
            const nfdpathset_t *paths;
            nfdresult_t result = NFD_OpenDialogMultipleU8(&paths, sFilterPatterns, 1, nullptr);

            if (result == NFD_OKAY)
            {
                nfdpathsetsize_t numPaths;
                NFD_PathSet_GetCount(paths, &numPaths);

                for (nfdpathsetsize_t i = 0; i < numPaths; ++i)
                {
                    nfdchar_t* path;
                    NFD_PathSet_GetPath(paths, i, &path);
                    m_Files.push_back(path);
                    NFD_PathSet_FreePath(path);
                }

                NFD_PathSet_Free(paths);
                CombineFiles();
            }
        }

        ImGui::SameLine();

        if (ImGui::Button("Cancel"))
            ImGui::CloseCurrentPopup();

        ImGui::BeginChild("##Filenames", ImVec2(0, 0), true);

        for (size_t i = 0; i < m_Files.size(); ++i)
        {
            auto &fpath = m_Files[i];
            auto fname = std::filesystem::path(fpath).filename().string();
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
        }
        ImGui::EndChild();
    }

    void Combine::DetailsBar()
    {
        int num_colors = m_Palette.size();

        ImGui::InputInt("No. of Colors", &num_colors, 1, 100, ImGuiInputTextFlags_ReadOnly);
        if (ImGui::Button("Load"))
        {
            Load();
            ImGui::CloseCurrentPopup();
        }

        ImGui::SameLine();
        
        if (ImGui::Button("Cancel"))
        {
            ImGui::CloseCurrentPopup();
        }
    }

    void Combine::PaletteEditor()
    {
        ImGui::BeginChild("Colors", ImVec2(0.0f, 0.0f), true, ImGuiWindowFlags_AlwaysAutoResize);
        for (int i = 0; i < m_Palette.size(); i++)
        {
            char label[10];
            snprintf(label, 10, "Color #%i", i);

            auto color = m_Palette[i];
            ImGui::ColorEdit3(label, (float *)&color, ImGuiColorEditFlags_NoDragDrop | ImGuiColorEditFlags_NoInputs);
        }

        ImGui::EndChild();
    }
}