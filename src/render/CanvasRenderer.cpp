#include "render/CanvasRenderer.hpp"
#include <imgui.h>

void RenderCanvas(const CanvasState& canvas) {
    ImGui::SetNextWindowPos(ImGui::GetMainViewport()->Pos);
    ImGui::SetNextWindowSize(ImGui::GetMainViewport()->Size);
    ImGuiWindowFlags canvas_flags = ImGuiWindowFlags_NoDecoration
        | ImGuiWindowFlags_NoMove
        | ImGuiWindowFlags_NoScrollbar
        | ImGuiWindowFlags_NoScrollWithMouse
        | ImGuiWindowFlags_NoBringToFrontOnFocus
        | ImGuiWindowFlags_NoSavedSettings;
    ImGui::Begin("##full_canvas", nullptr, canvas_flags);

    ImVec2 canvas_pos = ImGui::GetWindowPos();
    ImVec2 canvas_size = ImGui::GetWindowSize();
    ImDrawList* draw_list = ImGui::GetWindowDrawList();

    // bg
    draw_list->AddRectFilled(canvas_pos,
        ImVec2(canvas_pos.x + canvas_size.x, canvas_pos.y + canvas_size.y),

        // light
        // IM_COL32(255, 255, 255, 255));
        
        // dark
        IM_COL32(40, 40, 50, 255));

    // strokes
    for (const auto& s : canvas.strokes) {
        if (s.points.size() < 2) continue;
        draw_list->AddPolyline(s.points.data(),
            static_cast<int>(s.points.size()),
            ImColor(s.color),
            ImDrawFlags_None,
            s.thickness);
    }

    ImGui::InvisibleButton("canvas_full", canvas_size, ImGuiButtonFlags_MouseButtonLeft);
    ImGui::End();
}
