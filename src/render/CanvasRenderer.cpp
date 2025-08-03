#include "render/CanvasRenderer.hpp"
#include <imgui.h>
#include <util/ImVecUtil.hpp>

#include <iostream>

void RenderCanvas(const CanvasState& canvas) {
    // Setup a full-viewport invisible ImGui window for the canvas background and strokes
    ImGui::SetNextWindowPos(ImGui::GetMainViewport()->Pos);
    ImGui::SetNextWindowSize(ImGui::GetMainViewport()->Size);
    ImGuiWindowFlags canvas_flags = ImGuiWindowFlags_NoDecoration
        | ImGuiWindowFlags_NoMove
        | ImGuiWindowFlags_NoScrollbar
        | ImGuiWindowFlags_NoScrollWithMouse
        | ImGuiWindowFlags_NoBringToFrontOnFocus
        | ImGuiWindowFlags_NoSavedSettings;
    ImGui::Begin("##full_canvas", nullptr, canvas_flags);

    ImVec2 canvas_origin = ImGui::GetWindowPos();
    ImVec2 canvas_size = ImGui::GetWindowSize();
    ImDrawList* draw_list = ImGui::GetWindowDrawList();

    // Draw background rectangle
    draw_list->AddRectFilled(
        canvas_origin,
        ImVec2(canvas_origin.x + canvas_size.x, canvas_origin.y + canvas_size.y),
        IM_COL32(40, 40, 50, 255)
    );

    // Draw each stroke with pan/zoom applied
    for (const auto& s : canvas.strokes) {
        if (s.points.size() < 2) continue;

        std::vector<ImVec2> transformed;
        transformed.reserve(s.points.size());

        // Transform canvas-space points into screen space
        for (const ImVec2& p : s.points) {
            ImVec2 t = canvas_origin + canvas.pan + p * canvas.zoom;
            transformed.push_back(t);
        }

        float thickness = s.thickness * canvas.zoom; // scale thickness with zoom
        draw_list->AddPolyline(
            transformed.data(),
            static_cast<int>(transformed.size()),
            ImColor(s.color),
            ImDrawFlags_None,
            thickness
        );
    }

    // Capture mouse interaction region over entire canvas
    ImGui::InvisibleButton("canvas_full", canvas_size, ImGuiButtonFlags_MouseButtonLeft);
    ImGui::End();
}
