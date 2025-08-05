#include "input/CanvasController.hpp"
#include <algorithm>
#include <cmath>
#include <imgui.h>
#include "core/CanvasElement.hpp"
#include <memory>
#include <algorithm>

#include <util/ImVecUtil.hpp>
#include <iostream>

static bool point_near(const ImVec2 &a, const ImVec2 &b, float r)
{
    float dx = a.x - b.x;
    float dy = a.y - b.y;
    return dx * dx + dy * dy <= r * r;
}

static ImVec2 last_mouse;
static bool was_alt = false;

static float clamp_float(float v, float lo, float hi)
{
    if (v < lo)
        return lo;
    if (v > hi)
        return hi;
    return v;
}

void CanvasController::update(CanvasState &canvas, History &history, bool &is_drawing, ImGuiIO &io, ToolSettings &tool)
{
    ImVec2 mouse_screen = io.MousePos;
    ImVec2 canvas_origin = ImGui::GetMainViewport()->Pos;

    // --- Zoom around the cursor position ---
    float wheel = io.MouseWheel;
    if (wheel != 0.0f)
    {
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
    if (alt)
    {
        if (!was_alt)
        {
            last_mouse = mouse_screen;
            was_alt = true;
        }
        else
        {
            ImVec2 delta = ImVec2(mouse_screen.x - last_mouse.x, mouse_screen.y - last_mouse.y);
            canvas.pan += delta;
            last_mouse = mouse_screen;
        }
    }
    else
    {
        was_alt = false;
    }

    // --- Convert screen mouse position to canvas space ---
    ImVec2 mouse_world = (mouse_screen - canvas_origin - canvas.pan) / canvas.zoom;

    // --- Element selection ---
    if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !alt && tool.type == ToolType::Select)
    {
        std::cout << "tool selected" << std::endl;

        canvas.selected_element = nullptr;
        canvas.is_editing_text = false;

        // Проверяем попадание в элементы (в обратном порядке для выбора верхнего)
        for (auto it = canvas.elements.rbegin(); it != canvas.elements.rend(); ++it)
        {
            if ((*it)->contains(mouse_screen - canvas_origin, canvas.pan, canvas.zoom))
            {
                canvas.selected_element = it->get();
                if (auto text = dynamic_cast<TextLabel *>(canvas.selected_element))
                {
                    // Двойной клик для редактирования
                    static float last_click_time = 0.0f;
                    static CanvasElement *last_clicked = nullptr;
                    float current_time = ImGui::GetTime();

                    if (last_clicked == canvas.selected_element &&
                        current_time - last_click_time < 0.3f)
                    {
                        // Двойной клик - начинаем редактирование
                        canvas.is_editing_text = true;
                        text->cursor_pos = (int)text->text.length(); // Курсор в конец
                        text->selection_start = -1;
                        text->selection_end = -1;
                    }
                    else
                    {
                        // Одинарный клик - просто выделяем
                        canvas.is_editing_text = false;
                    }

                    last_clicked = canvas.selected_element;
                    last_click_time = current_time;
                }
                break;
            }
        }
    }

    // --- Brush drawing or erasing depending on tool ---
    static Stroke *active_stroke = nullptr;
    if (!alt && tool.type == ToolType::Brush)
    {
        if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
        {
            // Сбрасываем выбор перед созданием нового элемента
            canvas.selected_element = nullptr;
            canvas.is_editing_text = false;
            history.push(canvas);
            is_drawing = true;
            auto stroke = std::make_unique<Stroke>();
            stroke->color = tool.color;
            stroke->thickness = tool.radius;
            stroke->points.push_back(mouse_world);
            active_stroke = stroke.get();
            canvas.elements.push_back(std::move(stroke));
        }
        if (is_drawing)
        {
            if (ImGui::IsMouseDown(ImGuiMouseButton_Left) && active_stroke)
            {
                active_stroke->points.push_back(mouse_world);
            }
            else
            {
                is_drawing = false;
                active_stroke = nullptr;
            }
        }
    }
    else if (!alt && tool.type == ToolType::Eraser)
    {
        if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
        {
            history.push(canvas);
            auto &elems = canvas.elements;
            elems.erase(std::remove_if(elems.begin(), elems.end(),
                                       [&](const std::unique_ptr<CanvasElement> &el)
                                       {
                                           auto s = dynamic_cast<Stroke *>(el.get());
                                           if (!s)
                                               return false;
                                           for (const ImVec2 &p : s->points)
                                           {
                                               if (point_near(p, mouse_world, tool.radius))
                                                   return true;
                                           }
                                           return false;
                                       }),
                        elems.end());
        }
    }
    else if (!alt && tool.type == ToolType::Text)
    {
        if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
        {
            // Сбрасываем выбор перед созданием нового элемента
            canvas.selected_element = nullptr;
            canvas.is_editing_text = false;
            history.push(canvas);
            auto text = std::make_unique<TextLabel>();
            text->position = mouse_world;
            text->color = tool.color;
            text->size = tool.radius;
            text->text = "Sample Text"; // Пока простой текст, позже можно добавить диалог ввода
            canvas.elements.push_back(std::move(text));
        }
    }

    // --- Text editing with keyboard ---
    if (canvas.selected_element && canvas.is_editing_text)
    {
        auto text = dynamic_cast<TextLabel *>(canvas.selected_element);
        if (text)
        {
            text->is_focused = true;

            // Обработка ввода текста
            for (int i = 0; i < io.InputQueueCharacters.Size; i++)
            {
                unsigned short c = io.InputQueueCharacters[i];
                if (c >= 32 && c < 128)
                { // Печатаемые символы
                    if (text->selection_start >= 0)
                    {
                        // Удаляем выделенный текст
                        int start = std::min(text->selection_start, text->selection_end);
                        int end = std::max(text->selection_start, text->selection_end);
                        text->text.erase(start, end - start);
                        text->cursor_pos = start;
                        text->selection_start = -1;
                        text->selection_end = -1;
                    }

                    text->text.insert(text->cursor_pos, 1, (char)c);
                    text->cursor_pos++;
                }
            }

            // Обработка специальных клавиш
            if (ImGui::IsKeyPressed(ImGuiKey_Backspace))
            {
                if (text->selection_start >= 0)
                {
                    // Удаляем выделенный текст
                    int start = std::min(text->selection_start, text->selection_end);
                    int end = std::max(text->selection_start, text->selection_end);
                    text->text.erase(start, end - start);
                    text->cursor_pos = start;
                    text->selection_start = -1;
                    text->selection_end = -1;
                }
                else if (text->cursor_pos > 0)
                {
                    text->text.erase(text->cursor_pos - 1, 1);
                    text->cursor_pos--;
                }
            }

            if (ImGui::IsKeyPressed(ImGuiKey_Delete))
            {
                if (text->selection_start >= 0)
                {
                    // Удаляем выделенный текст
                    int start = std::min(text->selection_start, text->selection_end);
                    int end = std::max(text->selection_start, text->selection_end);
                    text->text.erase(start, end - start);
                    text->cursor_pos = start;
                    text->selection_start = -1;
                    text->selection_end = -1;
                }
                else if (text->cursor_pos < (int)text->text.length())
                {
                    text->text.erase(text->cursor_pos, 1);
                }
            }

            if (ImGui::IsKeyPressed(ImGuiKey_LeftArrow))
            {
                if (text->cursor_pos > 0)
                {
                    text->cursor_pos--;
                    if (!ImGui::IsKeyDown(ImGuiKey_LeftShift))
                    {
                        text->selection_start = -1;
                        text->selection_end = -1;
                    }
                }
            }

            if (ImGui::IsKeyPressed(ImGuiKey_RightArrow))
            {
                if (text->cursor_pos < (int)text->text.length())
                {
                    text->cursor_pos++;
                    if (!ImGui::IsKeyDown(ImGuiKey_LeftShift))
                    {
                        text->selection_start = -1;
                        text->selection_end = -1;
                    }
                }
            }

            if (ImGui::IsKeyPressed(ImGuiKey_Home))
            {
                text->cursor_pos = 0;
                if (!ImGui::IsKeyDown(ImGuiKey_LeftShift))
                {
                    text->selection_start = -1;
                    text->selection_end = -1;
                }
            }

            if (ImGui::IsKeyPressed(ImGuiKey_End))
            {
                text->cursor_pos = (int)text->text.length();
                if (!ImGui::IsKeyDown(ImGuiKey_LeftShift))
                {
                    text->selection_start = -1;
                    text->selection_end = -1;
                }
            }

            // Выделение с Shift
            if (ImGui::IsKeyDown(ImGuiKey_LeftShift))
            {
                if (text->selection_start == -1)
                {
                    text->selection_start = text->cursor_pos;
                }
                text->selection_end = text->cursor_pos;
            }

            // Ctrl+A для выделения всего
            if (ImGui::IsKeyDown(ImGuiKey_LeftCtrl) && ImGui::IsKeyPressed(ImGuiKey_A))
            {
                text->selection_start = 0;
                text->selection_end = (int)text->text.length();
                text->cursor_pos = (int)text->text.length();
            }

            // Escape для выхода из редактирования
            if (ImGui::IsKeyPressed(ImGuiKey_Escape))
            {
                text->is_focused = false;
                canvas.selected_element = nullptr;
                canvas.is_editing_text = false;
            }
        }
    }
    else
    {
        // Сбрасываем фокус если не редактируем
        for (auto &el : canvas.elements)
        {
            if (auto text = dynamic_cast<TextLabel *>(el.get()))
            {
                text->is_focused = false;
            }
        }
    }

    // --- Undo / Redo handling ---
    if (ImGui::IsKeyDown(ImGuiKey_LeftCtrl) && ImGui::IsKeyPressed(ImGuiKey_Z, false))
    {
        if (auto prev = history.undo(canvas))
        {
            canvas = *prev;
            // Сбрасываем выбор после undo
            canvas.selected_element = nullptr;
            canvas.is_editing_text = false;
        }
    }
    if (ImGui::IsKeyDown(ImGuiKey_LeftCtrl) && ImGui::IsKeyPressed(ImGuiKey_Y, false))
    {
        if (auto next = history.redo(canvas))
        {
            canvas = *next;
            // Сбрасываем выбор после redo
            canvas.selected_element = nullptr;
            canvas.is_editing_text = false;
        }
    }
}
