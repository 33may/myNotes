#include "core/History.hpp"

void History::push(const CanvasState& state) {
    undo_stack.push_back(state);
    redo_stack.clear();
}

std::optional<CanvasState> History::undo(const CanvasState& current) {
    if (undo_stack.empty()) return std::nullopt;
    redo_stack.push_back(current);
    CanvasState prev = undo_stack.back();
    undo_stack.pop_back();
    return prev;
}

std::optional<CanvasState> History::redo(const CanvasState& current) {
    if (redo_stack.empty()) return std::nullopt;
    undo_stack.push_back(current);
    CanvasState next = redo_stack.back();
    redo_stack.pop_back();
    return next;
}
