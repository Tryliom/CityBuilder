#include "GUI.h"
#include "Graphics.h"
#include "Tile.h"
#include "Grid.h"

#include "sokol_app.h"
#include "sokol_gfx.h"
#include "imgui.h"
#include "util\sokol_imgui.h"

bool constrMenuNeverOpened = true;

Texture buildings[] =
{
	Texture(Buildings::Sawmill),
	Texture(Buildings::BuilderHut),
	Texture(Buildings::Quarry),
	Texture(Buildings::Storage),
	Texture(Buildings::House),
	Texture(Road::Single),
	Texture(Buildings::LogisticsCenter),
	Texture(Buildings::InactiveFurnace)
};

bool isButtonSelected[ARR_LEN(buildings)] =
{
	true,
	false,
	false,
	false,
	false,
	false,
	false,
	false
};

namespace GUI
{
    void DrawStartMenu(bool* gameStarted)
    {
        Graphics::CalculTransformationMatrix(Vector2F::One);

        bool isOpen = false;
        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->WorkPos);
        ImGui::SetNextWindowSize(viewport->WorkSize);
        ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | 
                                 ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;
        ImGui::SetNextWindowBgAlpha(0.6); // Transparent background
        ImGui::Begin("Start Menu", &isOpen, flags);
        ImGui::SetWindowFontScale(2.f);

        // Center the window content using ImGui layout features
        ImVec2 windowContentRegion = ImGui::GetContentRegionAvail();
        ImVec2 buttonSize(250, 75); // Adjust button size as needed
        ImVec2 windowCenter(ImGui::GetCursorPos().x + windowContentRegion.x * 0.5f - buttonSize.x * 0.5f,
                            ImGui::GetCursorPos().y + windowContentRegion.y * 0.5f - buttonSize.y * 0.5f);

        ImGui::SetCursorPos(ImVec2(windowCenter.x, windowCenter.y / 2.f));
        if (ImGui::Button("Start", buttonSize)) 
        {
            *gameStarted = true;
        }

        ImGui::SetCursorPos(windowCenter);
        if (ImGui::Button("Reset", buttonSize)) 
        {
            // TODO reset the save.
        }

        ImGui::SetCursorPos(ImVec2(windowCenter.x, windowCenter.y + windowCenter.y / 2.f));
        if (ImGui::Button("Exit", buttonSize)) 
        {
            exit(1);
        }

        ImGui::End();
    }

    void DrawTileInventory(Tile& tile)
    {
        ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | 
											ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;
        ImGui::SetNextWindowBgAlpha(0.5); // Transparent background
        ImGui::Begin("Inventory", NULL, window_flags);
        ImGui::SetWindowPos(ImVec2(5, 120), ImGuiCond_Always);
        ImGui::SetWindowFontScale(1.35f);
        //ImGui::SetWindowSize(ImVec2(200, 400), ImGuiCond_Always);

        std::string title = "Inventory of " + TileTypeToString(tile.Type);
        ImGui::Text("%s", title.c_str());
        ImGui::Separator();
        
        for (auto pair: *tile.Inventory)
        {
            //TODO: Olive, the texture of the item
            auto texture = Texture((Icons) pair.first);

            std::string text = std::to_string(pair.second) + " of " + Texture::ItemToString[(int) pair.first];

            if (!tile.IsBuilt)
            {
                text += " / " + std::to_string(Grid::GetNeededItemsToBuild(tile.Type, pair.first)) + "\n";
            }

            ImGui::Text("%s", text.c_str());
            ImGui::Separator();
        }

        ImGui::End();
    }

    void DrawConstructionMenu(int* buildingSelected, Vector2F* screenSize, ImTextureID* imTilemapTextureID)
    {
        ImGuiWindowFlags constrMenuFlags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_None;

        ImGui::Begin("Construction Menu", NULL, constrMenuFlags);

        if (constrMenuNeverOpened) 
        {
            ImGui::SetWindowCollapsed(true);
            constrMenuNeverOpened = false;
        }

        if (ImGui::IsWindowCollapsed() && *buildingSelected != -1)
        {
            isButtonSelected[*buildingSelected] = false;
            *buildingSelected = -1;
        }

        ImGui::SetWindowSize(ImVec2(200, screenSize->Y - 10));
        ImVec2 windowSize = ImGui::GetWindowSize();
        ImGui::SetWindowPos(ImVec2(screenSize->X - 5 - windowSize.x, 5), ImGuiCond_Always);

        int i = 0;

        for (auto& building : buildings)
        {
            auto uvs = Graphics::GetUvs(building);

            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.7f, 0.8f, 1.f, 1.00f));

            // Temporarily modify the button background color to indicate the selection
            if (isButtonSelected[i])
            {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.7f, 0.8f, 1.f, 1.00f));
            }
            else
            {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.35f, 0.40f, 0.61f, 0.62f));
            }

            ImGui::PushID(i);

            // Center the window content using ImGui layout features
            ImVec2 windowContentRegion = ImGui::GetContentRegionAvail();
            ImVec2 buttonSize(75, 75); 
            ImVec2 windowCenter(ImGui::GetCursorPos().x + windowContentRegion.x * 0.5f - buttonSize.x * 0.5f,
                                ImGui::GetCursorPos().y + windowContentRegion.y * 0.5f - buttonSize.y * 0.5f);

            ImGui::SetCursorPosX(windowCenter.x);

            // Use the sg_image handle (converted to ImTextureID) for the image button
            if (ImGui::ImageButton(*imTilemapTextureID, buttonSize, ImVec2(uvs[0].X, uvs[0].Y), ImVec2(uvs[2].X, uvs[2].Y)))
            {
                isButtonSelected[*buildingSelected] = false;
                *buildingSelected = i;
                isButtonSelected[*buildingSelected] = true;
            }

            ImGui::PopID();

            i++;
        }

        ImGui::PopStyleColor(i * 2);

        ImGui::End();
    }
}