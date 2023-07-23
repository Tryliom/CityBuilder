#include "GUI.h"
#include "Graphics.h"

#include "sokol_app.h"
#include "sokol_gfx.h"
#include "imgui.h"
#include "util\sokol_imgui.h"

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
}