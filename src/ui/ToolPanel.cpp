#include "ui/ToolPanel.hpp"
#include <imgui.h>
#include <cstring>
#include "core/CanvasElement.hpp"

void RenderToolPanel(CanvasState &canvas, History &history, ToolSettings &tool)
{
    ImGui::Begin("Tools");

    // Select Tool
    if (ImGui::RadioButton("Brush", tool.type == ToolType::Brush))
        tool.type = ToolType::Brush;
    ImGui::SameLine();
    if (ImGui::RadioButton("Eraser", tool.type == ToolType::Eraser))
        tool.type = ToolType::Eraser;
    ImGui::SameLine();
    if (ImGui::RadioButton("Text", tool.type == ToolType::Text))
        tool.type = ToolType::Text;
    ImGui::SameLine();
    if (ImGui::RadioButton("Select", tool.type == ToolType::Select))
        tool.type = ToolType::Select;

    // Настройки в зависимости от текущего инструмента
    if (tool.type == ToolType::Brush)
    {
        ImGui::ColorEdit4("Color", (float *)&tool.color);
        ImGui::SliderFloat("Size", &tool.radius, 1.0f, 50.0f, "%.1f");
    }
    else if (tool.type == ToolType::Eraser)
    {
        ImGui::SliderFloat("Eraser Radius", &tool.radius, 5.0f, 100.0f, "%.1f");
    }
    else if (tool.type == ToolType::Text)
    {
        ImGui::ColorEdit4("Text Color", (float *)&tool.color);
        ImGui::SliderFloat("Font Size", &tool.radius, 8.0f, 72.0f, "%.1f");
    }

    ImGui::Separator();

    // Undo/Redo
    if (ImGui::Button("Undo"))
    {
        if (auto prev = history.undo(canvas))
            canvas = *prev;
    }
    ImGui::SameLine();
    if (ImGui::Button("Redo"))
    {
        if (auto next = history.redo(canvas))
            canvas = *next;
    }

    size_t stroke_count = 0;
    size_t text_count = 0;
    for (const auto &el : canvas.elements)
    {
        if (dynamic_cast<Stroke *>(el.get()))
            ++stroke_count;
        if (dynamic_cast<TextLabel *>(el.get()))
            ++text_count;
    }
    ImGui::Text("Strokes: %zu, Texts: %zu", stroke_count, text_count);
    const char *tool_names[] = {"Brush", "Eraser", "Text"};
    ImGui::Text("Current tool: %s", tool_names[static_cast<int>(tool.type)]);

    // Text editing info
    if (canvas.selected_element && canvas.is_editing_text)
    {
        if (auto text = dynamic_cast<TextLabel *>(canvas.selected_element))
        {
            ImGui::Separator();
            ImGui::Text("Editing text (press Escape to finish)");
            ImGui::Text("Text: %s", text->text.c_str());
        }
    }

    ImGui::End();
}
