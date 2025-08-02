#pragma once

// Prevent GLFW from including the old OpenGL headers (so glad can control loading)
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

GLFWwindow* InitWindow(int width = 1280, int height = 720, const char* title = "myNotes");
void ShutdownWindow(GLFWwindow* window);
