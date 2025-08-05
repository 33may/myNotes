#include "render/CanvasRenderer.hpp"
#include <imgui.h>
#include "core/CanvasElement.hpp"
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

    // Render each element (strokes, text, etc.)
    for (const auto& element : canvas.elements) {
        element->render(draw_list, canvas_origin, canvas.pan, canvas.zoom);
        
        // Highlight selected element
        if (element.get() == canvas.selected_element) {
            // Draw selection rectangle
            if (auto text = dynamic_cast<TextLabel*>(element.get())) {
                ImVec2 screen_pos = canvas_origin + canvas.pan + text->position * canvas.zoom;
                ImVec2 text_size = ImGui::CalcTextSize(text->text.c_str());
                text_size.x *= canvas.zoom;
                text_size.y *= canvas.zoom;
                
                draw_list->AddRect(
                    screen_pos,
                    ImVec2(screen_pos.x + text_size.x, screen_pos.y + text_size.y),
                    IM_COL32(255, 255, 0, 255),
                    0.0f, 0, 2.0f
                );
            }
        }
    }

    // Capture mouse interaction region over entire canvas
    ImGui::InvisibleButton("canvas_full", canvas_size, ImGuiButtonFlags_MouseButtonLeft);
    ImGui::End();
}
