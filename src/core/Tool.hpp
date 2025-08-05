#pragma once
#include <imgui.h>

enum class ToolType
{
    Brush,
    Eraser,
    Text,
    Select
};

struct ToolSettings
{
    ToolType type = ToolType::Brush;
    ImVec4 color = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
    float radius = 4.0f;
};
