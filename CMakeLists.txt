cmake_minimum_required(VERSION 3.20)
project(myNotes)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# sanity checks
if(NOT EXISTS ${CMAKE_SOURCE_DIR}/external/imgui/imgui.h)
    message(FATAL_ERROR "ImGui submodule missing; run git submodule update --init --recursive")
endif()
if(NOT EXISTS ${CMAKE_SOURCE_DIR}/external/glad/include/glad/glad.h)
    message(FATAL_ERROR "GLAD header missing")
endif()
if(NOT EXISTS ${CMAKE_SOURCE_DIR}/external/glad/src/glad.c)
    message(FATAL_ERROR "GLAD source missing")
endif()

# build GLAD manually (external/glad is generator, not a CMake project)
add_library(glad STATIC external/glad/src/glad.c)
target_include_directories(glad PUBLIC ${CMAKE_SOURCE_DIR}/external/glad/include)

# bring in GLFW and ImGui from submodules
add_subdirectory(external/glfw)
add_subdirectory(external/imgui)

# main binary
add_executable(myNotes src/main.cpp)
target_include_directories(myNotes PRIVATE external/imgui external/glad/include)

# tell ImGui to use GLAD loader
if(TARGET imgui::imgui)
    target_compile_definitions(imgui::imgui PUBLIC IMGUI_IMPL_OPENGL_LOADER_GLAD)
elseif(TARGET imgui)
    target_compile_definitions(imgui PUBLIC IMGUI_IMPL_OPENGL_LOADER_GLAD)
endif()

# linking order
if(TARGET imgui::imgui)
    target_link_libraries(myNotes PRIVATE glfw glad imgui::imgui dl m)
elseif(TARGET imgui)
    target_link_libraries(myNotes PRIVATE glfw glad imgui dl m)
else()
    message(FATAL_ERROR "ImGui target not found")
endif()

# optional EGL (Wayland)
find_library(EGL_LIB EGL)
if(EGL_LIB)
    target_link_libraries(myNotes PRIVATE ${EGL_LIB})
endif()
