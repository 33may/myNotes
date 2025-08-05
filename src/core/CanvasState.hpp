#pragma once
#include <vector>
#include <memory>
#include "core/CanvasElement.hpp"

struct CanvasState {
    std::vector<std::unique_ptr<CanvasElement>> elements;
    
    // Выбранный элемент для редактирования
    CanvasElement* selected_element = nullptr;
    bool is_editing_text = false;

    ImVec2 pan = ImVec2(0.0f, 0.0f);
    float  zoom = 1.0f;

    CanvasState() = default;

    // Копирование с глубоким клонированием объектов для корректного undo/redo
    CanvasState(const CanvasState& other) : pan(other.pan), zoom(other.zoom) {
        elements.reserve(other.elements.size());
        for (const auto& el : other.elements) {
            elements.push_back(el->clone());
        }
        // selected_element не копируем, так как это указатель на элемент в векторе
    }

    CanvasState& operator=(const CanvasState& other) {
        if (this == &other) return *this;
        pan  = other.pan;
        zoom = other.zoom;
        elements.clear();
        elements.reserve(other.elements.size());
        for (const auto& el : other.elements) {
            elements.push_back(el->clone());
        }
        selected_element = nullptr; // Сбрасываем выбор при копировании
        is_editing_text = false;
        return *this;
    }
};
