#pragma once
#include "core/CanvasState.hpp"
#include "core/History.hpp"
#include "core/Tool.hpp"
#include <imgui.h>

class CanvasController {
public:
    void update(CanvasState& canvas, History& history, bool& is_drawing, ImGuiIO& io, ToolSettings& tool);
};
