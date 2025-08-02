#include "ui/ToolPanel.hpp"
#include <imgui.h>

void RenderToolPanel(CanvasState& canvas, History& history, ToolSettings& tool) {
    ImGui::Begin("Tools");

    // Select Tool
    if (ImGui::RadioButton("Brush", tool.type == ToolType::Brush)) tool.type = ToolType::Brush;
    ImGui::SameLine();
    if (ImGui::RadioButton("Eraser", tool.type == ToolType::Eraser)) tool.type = ToolType::Eraser;

    // Настройки в зависимости от текущего инструмента
    if (tool.type == ToolType::Brush) {
        ImGui::ColorEdit4("Color", (float*)&tool.color);
        ImGui::SliderFloat("Size", &tool.radius, 1.0f, 50.0f, "%.1f");
    } else { // Eraser
        ImGui::SliderFloat("Eraser Radius", &tool.radius, 5.0f, 100.0f, "%.1f");
    }

    ImGui::Separator();

    // Undo/Redo
    if (ImGui::Button("Undo")) {
        if (auto prev = history.undo(canvas)) canvas = *prev;
    }
    ImGui::SameLine();
    if (ImGui::Button("Redo")) {
        if (auto next = history.redo(canvas)) canvas = *next;
    }

    ImGui::Text("Strokes: %zu", canvas.strokes.size());
    ImGui::Text("Current tool: %s", tool.type == ToolType::Brush ? "Brush" : "Eraser");

    ImGui::End();
}
