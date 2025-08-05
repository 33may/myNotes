#pragma once
#include <vector>
#include <memory>
#include <string>
#include <algorithm>
#include <imgui.h>
#include <util/ImVecUtil.hpp>

// Базовый абстрактный объект на холсте
struct CanvasElement
{
    virtual ~CanvasElement() = default;

    // Функция копирования через клонирование для корректной работы undo/redo
    virtual std::unique_ptr<CanvasElement> clone() const = 0;

    // Отрисовка объекта. origin — левый-верхний угол холста на экране
    virtual void render(ImDrawList *draw_list, const ImVec2 &origin, const ImVec2 &pan, float zoom) const = 0;

    // Проверка попадания точки в элемент (для выбора)
    virtual bool contains(const ImVec2 &point, const ImVec2 &pan, float zoom) const = 0;

    // Получение типа элемента
    virtual const char *get_type() const = 0;
};

// ---------- Stroke ----------
struct Stroke : public CanvasElement
{
    std::vector<ImVec2> points;
    ImVec4 color = ImVec4(1, 1, 1, 1);
    float thickness = 2.0f;

    std::unique_ptr<CanvasElement> clone() const override
    {
        return std::make_unique<Stroke>(*this);
    }

    void render(ImDrawList *draw_list, const ImVec2 &origin, const ImVec2 &pan, float zoom) const override
    {
        if (points.size() < 2)
            return;
        std::vector<ImVec2> transformed;
        transformed.reserve(points.size());
        for (const ImVec2 &p : points)
        {
            transformed.push_back(origin + pan + p * zoom);
        }
        float thick = thickness * zoom;
        draw_list->AddPolyline(
            transformed.data(),
            static_cast<int>(transformed.size()),
            ImColor(color),
            ImDrawFlags_None,
            thick);
    }

    bool contains(const ImVec2 &point, const ImVec2 &pan, float zoom) const override
    {
        for (const ImVec2 &p : points)
        {
            ImVec2 screen_p = p * zoom + pan;
            float dx = point.x - screen_p.x;
            float dy = point.y - screen_p.y;
            if (dx * dx + dy * dy <= (thickness * zoom) * (thickness * zoom))
            {
                return true;
            }
        }
        return false;
    }

    const char *get_type() const override { return "Stroke"; }
};

// ---------- Text label (Markdown/LaTeX) ----------
struct TextLabel : public CanvasElement
{
    std::string text;                     // исходный markdown / LaTeX-текст
    ImVec2 position = ImVec2(0.0f, 0.0f); // позиция в координатах холста
    ImVec4 color = ImVec4(1, 1, 1, 1);
    float size = 16.0f; // базовый размер шрифта в px при zoom = 1

    // Редактирование
    int cursor_pos = 0;       // позиция курсора
    int selection_start = -1; // начало выделения (-1 = нет выделения)
    int selection_end = -1;   // конец выделения
    bool is_focused = false;  // фокус на этом элементе

    std::unique_ptr<CanvasElement> clone() const override
    {
        return std::make_unique<TextLabel>(*this);
    }

    void render(ImDrawList *draw_list, const ImVec2 &origin, const ImVec2 &pan, float zoom) const override
    {
        ImVec2 screen_pos = origin + pan + position * zoom;

        // Рендерим текст
        ImGui::SetCursorScreenPos(screen_pos);
        ImGui::PushStyleColor(ImGuiCol_Text, color);
        ImGui::SetWindowFontScale(zoom);

        // Рендерим выделение если есть
        if (selection_start >= 0 && selection_end >= 0 && is_focused)
        {
            int start = std::min(selection_start, selection_end);
            int end = std::max(selection_start, selection_end);

            if (start < end)
            {
                std::string before_selection = text.substr(0, start);
                std::string selection = text.substr(start, end - start);

                ImVec2 before_size = ImGui::CalcTextSize(before_selection.c_str());
                ImVec2 selection_size = ImGui::CalcTextSize(selection.c_str());

                // Рисуем фон выделения
                ImVec2 selection_pos = screen_pos;
                selection_pos.x += before_size.x * zoom;
                draw_list->AddRectFilled(
                    selection_pos,
                    ImVec2(selection_pos.x + selection_size.x * zoom, selection_pos.y + selection_size.y * zoom),
                    IM_COL32(100, 150, 255, 100));
            }
        }

        ImGui::Text("%s", text.c_str());

        // Рисуем курсор если элемент в фокусе
        if (is_focused && cursor_pos >= 0 && cursor_pos <= (int)text.length())
        {
            std::string before_cursor = text.substr(0, cursor_pos);
            ImVec2 cursor_size = ImGui::CalcTextSize(before_cursor.c_str());
            ImVec2 cursor_pos_screen = screen_pos;
            cursor_pos_screen.x += cursor_size.x * zoom;

            draw_list->AddLine(
                ImVec2(cursor_pos_screen.x, cursor_pos_screen.y),
                ImVec2(cursor_pos_screen.x, cursor_pos_screen.y + cursor_size.y * zoom),
                IM_COL32(255, 255, 255, 255),
                2.0f);
        }

        ImGui::PopStyleColor();
        ImGui::SetWindowFontScale(1.0f);
    }

    bool contains(const ImVec2 &point, const ImVec2 &pan, float zoom) const override
    {
        ImVec2 screen_pos = position * zoom + pan;
        ImVec2 text_size = ImGui::CalcTextSize(text.c_str());
        text_size.x *= zoom;
        text_size.y *= zoom;

        return point.x >= screen_pos.x && point.x <= screen_pos.x + text_size.x &&
               point.y >= screen_pos.y && point.y <= screen_pos.y + text_size.y;
    }

    const char *get_type() const override { return "TextLabel"; }
};
