#include <iostream>
#include <vector>

#include <glad/glad.h>           // glad2 loader
#include <GLFW/glfw3.h>

#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>

// simple freehand stroke
struct Stroke {
    std::vector<ImVec2> points;
}

static void glfw_error_callback(int error, const char* description) {
    std::cerr << "[GLFW Error " << error << "] " << description << "\n";
}

int main() {
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit()) return -1;

    glfwWindowHint(GLFW_CONTEXT_CREATION_API, GLFW_EGL_CONTEXT_API);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(1280, 720, "myNotes", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create window\n";
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    if (!gladLoadGL()) {
        std::cerr << "Failed to initialize GLAD\n";
        return -1;
    }

    std::cout << "GL Renderer: " << glGetString(GL_RENDERER) << "\n";
    std::cout << "GL Version:  " << glGetString(GL_VERSION) << "\n";

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330 core");

    std::vector<Stroke> strokes;
    bool is_drawing = false;

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("Canvas");
        ImVec2 canvas_pos = ImGui::GetCursorScreenPos();
        ImVec2 canvas_size = ImGui::GetContentRegionAvail();
        ImGui::InvisibleButton("canvas", canvas_size, ImGuiButtonFlags_MouseButtonLeft);
        bool hovered = ImGui::IsItemHovered();
        ImDrawList* draw_list = ImGui::GetWindowDrawList();

        draw_list->AddRectFilled(canvas_pos,
                                 ImVec2(canvas_pos.x + canvas_size.x, canvas_pos.y + canvas_size.y),
                                 IM_COL32(40, 40, 50, 255));

        if (hovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
            is_drawing = true;
            strokes.emplace_back();
            strokes.back().points.push_back(io.MousePos);
        }
        if (is_drawing) {
            if (ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
                strokes.back().points.push_back(io.MousePos);
            } else {
                is_drawing = false;
            }
        }

        draw_list->PushClipRect(canvas_pos,
                                ImVec2(canvas_pos.x + canvas_size.x, canvas_pos.y + canvas_size.y),
                                true);
        for (const auto& s : strokes) {
            if (s.points.size() < 2) continue;
            draw_list->AddPolyline(s.points.data(),
                                   static_cast<int>(s.points.size()),
                                   IM_COL32(220, 220, 220, 255),
                                   ImDrawFlags_None,
                                   2.0f);
        }
        draw_list->PopClipRect();
        ImGui::End();

        ImGui::Render();
        int w, h;
        glfwGetFramebufferSize(window, &w, &h);
        glViewport(0, 0, w, h);
        glClearColor(0.1f, 0.1f, 0.12f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
