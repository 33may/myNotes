#include "platform/Window.hpp"
#include "ui/ImGuiLayer.hpp"
#include "core/CanvasState.hpp"
#include "core/History.hpp"
#include "core/Tool.hpp"
#include "input/CanvasController.hpp"
#include "render/CanvasRenderer.hpp"
#include "ui/ToolPanel.hpp"

#include <imgui.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <chrono>
#include <iomanip>
#include <sstream>

// Global focus flag
static bool g_window_focused = true;

// Return current system time as string for logging prefix.
static std::string now_str() {
    using namespace std::chrono;
    auto now = system_clock::now();
    auto itt = system_clock::to_time_t(now);
    std::tm tm = *std::localtime(&itt);
    auto ms = duration_cast<milliseconds>(now.time_since_epoch()) % 1000;
    std::ostringstream oss;
    oss << std::put_time(&tm, "%H:%M:%S") << "." << std::setfill('0') << std::setw(3) << ms.count();
    return oss.str();
}

// Focus callback to track active/inactive state.
void focus_callback(GLFWwindow* window, int focused) {
    g_window_focused = (focused != 0);
    std::cerr << "[" << now_str() << "] focus changed: " << (g_window_focused ? "FOCUSED" : "UNFOCUSED") << "\n";

    // Optionally toggle vsync: disable when unfocused to avoid SwapBuffers blocking
    glfwMakeContextCurrent(window);
    if (g_window_focused) {
        glfwSwapInterval(1);
    } else {
        glfwSwapInterval(0);
    }
}

int main() {
    GLFWwindow* window = InitWindow();
    if (!window) return -1;

    // Set focus callback and initial vsync
    glfwSetWindowFocusCallback(window, focus_callback);
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // enable vsync initially

    InitImGui(window);
    ImGuiIO& io = ImGui::GetIO();

    CanvasState canvas;
    History history;
    CanvasController controller;
    ToolSettings tool;
    bool is_drawing = false;

    // Timing helpers
    auto last_loop_time = std::chrono::steady_clock::now();
    auto last_unfocused_full = std::chrono::steady_clock::now();
    const std::chrono::milliseconds unfocused_full_interval(500); // full canvas update every 500ms when unfocused

    std::cerr << "[" << now_str() << "] Application started\n";

    while (!glfwWindowShouldClose(window)) {
        // Poll events always to stay responsive.
        glfwPollEvents();

        // Watchdog: measure time since last loop iteration.
        auto loop_start = std::chrono::steady_clock::now();
        auto gap = std::chrono::duration_cast<std::chrono::milliseconds>(loop_start - last_loop_time);
        if (gap.count() > 200) { // threshold for long gap
            std::cerr << "[" << now_str() << "] Warning: long loop gap " << gap.count()
                      << "ms focused=" << (g_window_focused ? "yes" : "no") << "\n";
        }
        last_loop_time = loop_start;

        // Start ImGui frame.
        NewFrame();

        // Measure controller/update time.
        auto before_update = std::chrono::steady_clock::now();
        controller.update(canvas, history, is_drawing, io, tool);
        auto after_update = std::chrono::steady_clock::now();
        auto update_dur = std::chrono::duration_cast<std::chrono::milliseconds>(after_update - before_update);
        if (update_dur.count() > 50) {
            std::cerr << "[" << now_str() << "] Notice: controller.update took " << update_dur.count() << "ms\n";
        }

        // Submit UI (tool panel always, canvas drawing is gated below)
        RenderToolPanel(canvas, history, tool);

        // Determine if full canvas render should happen.
        bool do_full_canvas = g_window_focused;
        auto now = std::chrono::steady_clock::now();
        if (!g_window_focused) {
            if (now - last_unfocused_full >= unfocused_full_interval) {
                do_full_canvas = true;
                last_unfocused_full = now;
                std::cerr << "[" << now_str() << "] Unfocused full render triggered\n";
            }
        }

        if (do_full_canvas) {
            RenderCanvas(canvas);
        }

        // Finalize ImGui frame.
        ImGui::Render();

        // Framebuffer size check.
        int display_w = 0, display_h = 0;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        if (display_w == 0 || display_h == 0) {
            // minimized or zero-size, still render ImGui internals
            RenderImGui();
            continue;
        }

        // Clear and render.
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.1f, 0.1f, 0.12f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // Draw ImGui (includes tool panel and any overlays)
        RenderImGui();

        // Swap buffers with timing log.
        auto swap_before = std::chrono::steady_clock::now();
        glfwSwapBuffers(window);
        auto swap_after = std::chrono::steady_clock::now();
        auto swap_dur = std::chrono::duration_cast<std::chrono::milliseconds>(swap_after - swap_before);
        if (swap_dur.count() > 100) {
            std::cerr << "[" << now_str() << "] Warning: SwapBuffers took " << swap_dur.count() << "ms\n";
        }
    }

    std::cerr << "[" << now_str() << "] Application exiting\n";
    ShutdownImGui();
    ShutdownWindow(window);
    return 0;
}
