#include "input/CanvasController.hpp"
#include <algorithm>
#include <cmath>
#include <imgui.h>

#include <util/ImVecUtil.hpp>
#include <iostream>

static bool point_near(const ImVec2& a, const ImVec2& b, float r) {
    float dx = a.x - b.x;
    float dy = a.y - b.y;
    return dx*dx + dy*dy <= r * r;
}

static ImVec2 last_mouse;
static bool was_alt = false;

static float clamp_float(float v, float lo, float hi) {
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}

void CanvasController::update(CanvasState& canvas, History& history, bool& is_drawing, ImGuiIO& io, ToolSettings& tool) {
    ImVec2 mouse_screen = io.MousePos;
    ImVec2 canvas_origin = ImGui::GetMainViewport()->Pos;

    // --- Zoom around the cursor position ---
    float wheel = io.MouseWheel;
    if (wheel != 0.0f) {
        float old_zoom = canvas.zoom;
        // Compute the canvas-space point under the cursor before zoom change
        ImVec2 canvas_point = (mouse_screen - canvas_origin - canvas.pan) / old_zoom;

        // Adjust zoom exponentionally with clamping
        float factor = std::pow(1.1f, wheel);
        float new_zoom = clamp_float(old_zoom * factor, 0.1f, 10.0f);

        // Recompute pan so that the same canvas point stays under the cursor
        canvas.pan = mouse_screen - canvas_origin - canvas_point * new_zoom;
        canvas.zoom = new_zoom;
    }

    // --- Pan when Alt is held and mouse moves ---
    bool alt = ImGui::IsKeyDown(ImGuiKey_LeftAlt);
    if (alt) {
        if (!was_alt) {
            last_mouse = mouse_screen;
            was_alt = true;
        } else {
            ImVec2 delta = ImVec2(mouse_screen.x - last_mouse.x, mouse_screen.y - last_mouse.y);
            canvas.pan += delta;
            last_mouse = mouse_screen;
        }
    } else {
        was_alt = false;
    }

    // --- Convert screen mouse position to canvas space ---
    ImVec2 mouse_world = (mouse_screen - canvas_origin - canvas.pan) / canvas.zoom;

    // --- Brush drawing or erasing depending on tool ---
    if (!alt && tool.type == ToolType::Brush) {
        if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
            history.push(canvas);
            is_drawing = true;
            canvas.strokes.emplace_back();
            Stroke& s = canvas.strokes.back();
            s.color = tool.color;
            s.thickness = tool.radius;
            s.points.push_back(mouse_world);
        }
        if (is_drawing) {
            if (ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
                canvas.strokes.back().points.push_back(mouse_world);
            } else {
                is_drawing = false;
            }
        }
    } else if (!alt && tool.type == ToolType::Eraser) {
        if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
            history.push(canvas);
            auto& strokes = canvas.strokes;
            strokes.erase(std::remove_if(strokes.begin(), strokes.end(),
                [&](const Stroke& s) {
                    for (const ImVec2& p : s.points) {
                        if (point_near(p, mouse_world, tool.radius)) return true;
                    }
                    return false;
                }), strokes.end());
        }
    }

    // --- Undo / Redo handling ---
    if (ImGui::IsKeyDown(ImGuiKey_LeftCtrl) && ImGui::IsKeyPressed(ImGuiKey_Z, false)) {
        if (auto prev = history.undo(canvas)) canvas = *prev;
    }
    if (ImGui::IsKeyDown(ImGuiKey_LeftCtrl) && ImGui::IsKeyPressed(ImGuiKey_Y, false)) {
        if (auto next = history.redo(canvas)) canvas = *next;
    }
}
