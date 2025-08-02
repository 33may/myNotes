#pragma once
#include <vector>
#include <imgui.h>

struct Stroke {
    std::vector<ImVec2> points;
    ImVec4 color = ImVec4(1, 1, 1, 1);
    float thickness = 2.0f;
};

struct CanvasState {
    std::vector<Stroke> strokes;

    ImVec2 pan = ImVec2(0.0f, 0.0f);
    float zoom = 1.0f;
};
