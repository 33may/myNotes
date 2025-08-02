#include "platform/Window.hpp"
#include <iostream>
#include <glad/glad.h>

static void glfw_error_callback(int error, const char* description) {
    std::cerr << "[GLFW Error " << error << "] " << description << "\n";
}

GLFWwindow* InitWindow(int width, int height, const char* title) {
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW\n";
        return nullptr;
    }

    glfwWindowHint(GLFW_CONTEXT_CREATION_API, GLFW_EGL_CONTEXT_API);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(width, height, title, nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        return nullptr;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    if (!gladLoadGL()) {
        std::cerr << "Failed to initialize GLAD\n";
        return nullptr;
    }

    std::cout << "GL Renderer: " << glGetString(GL_RENDERER) << "\n";
    std::cout << "GL Version:  " << glGetString(GL_VERSION) << "\n";

    return window;
}

void ShutdownWindow(GLFWwindow* window) {
    if (window) glfwDestroyWindow(window);
    glfwTerminate();
}
