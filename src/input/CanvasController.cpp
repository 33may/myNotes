#include "input/CanvasController.hpp"
#include <algorithm>
#include <cmath>

#include <util/ImVecUtil.hpp>
#include <iostream>

static bool point_near(const ImVec2& a, const ImVec2& b, float r) {
    float dx = a.x - b.x;
    float dy = a.y - b.y;
    return dx*dx + dy*dy <= r * r;
}

ImVec2 last_mouse;

void CanvasController::update(CanvasState& canvas, History& history, bool& is_drawing, ImGuiIO& io, ToolSettings& tool) {
    ImVec2 mouse_pos_screen = io.MousePos;

    ImVec2 mouse_pos_world = mouse_pos_screen + canvas.pan;
    std::cout << mouse_pos_world[0] << " " << mouse_pos_world[1] << std::endl;

    
    // zoom
    float wheel = io.MouseWheel;
    if (wheel != 0){
        canvas.zoom += wheel * 3e-2;
    }

    // pan
    if (ImGui::IsKeyDown(ImGuiKey_LeftAlt)) {

        ImVec2 d_mouse = ImVec2(mouse_pos_screen[0] - last_mouse[0], mouse_pos_screen[1] - last_mouse[1]);

        canvas.pan.x += d_mouse.x;
        canvas.pan.y += d_mouse.y;
    }
    // Brush drawing
    else if (tool.type == ToolType::Brush) {
        if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
            history.push(canvas);
            is_drawing = true;
            canvas.strokes.emplace_back();
            Stroke& s = canvas.strokes.back();
            s.color = tool.color;
            s.thickness = tool.radius;
            s.points.push_back(mouse_pos_world);
        }
        if (is_drawing) {
            if (ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
                canvas.strokes.back().points.push_back(mouse_pos_world);
            } else {
                is_drawing = false;
            }
        }
    }
    // Eraser: remove entire stroke if any point is within radius on click
    else if (tool.type == ToolType::Eraser) {
        if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
            history.push(canvas);
            auto& strokes = canvas.strokes;
            strokes.erase(std::remove_if(strokes.begin(), strokes.end(),
                [&](const Stroke& s) {
                    for (const ImVec2& p : s.points) {
                        if (point_near(p, io.MousePos, tool.radius)) return true;
                    }
                    return false;
                }), strokes.end());
        }
    }

    // update last mouse pos
    last_mouse = io.MousePos;


    // Undo / Redo shortcuts
    if (ImGui::IsKeyDown(ImGuiKey_LeftCtrl) && ImGui::IsKeyPressed(ImGuiKey_Z, false)) {
        if (auto prev = history.undo(canvas)) canvas = *prev;
    }
    if (ImGui::IsKeyDown(ImGuiKey_LeftCtrl) && ImGui::IsKeyPressed(ImGuiKey_Y, false)) {
        if (auto next = history.redo(canvas)) canvas = *next;
    }
}
