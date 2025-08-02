#pragma once
#include "core/CanvasState.hpp"
#include <optional>
#include <vector>

// Простой менеджер undo/redo через снимки
class History {
public:
    void push(const CanvasState& state);
    std::optional<CanvasState> undo(const CanvasState& current);
    std::optional<CanvasState> redo(const CanvasState& current);

private:
    std::vector<CanvasState> undo_stack;
    std::vector<CanvasState> redo_stack;
};
