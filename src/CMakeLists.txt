# collect module sources
set(SRCS
    platform/Window.cpp
    ui/ImGuiLayer.cpp
    core/History.cpp
    input/CanvasController.cpp
    render/CanvasRenderer.cpp
    ui/ToolPanel.cpp
    main.cpp
)

add_executable(myNotes ${SRCS})

# includes for our headers and externals
target_include_directories(myNotes PRIVATE
    ${CMAKE_SOURCE_DIR}/external/imgui
    ${CMAKE_SOURCE_DIR}/external/glad/include
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# ensure ImGui uses GLAD loader
if(TARGET imgui::imgui)
    target_compile_definitions(imgui::imgui PUBLIC IMGUI_IMPL_OPENGL_LOADER_GLAD)
endif()

# link dependencies
if(TARGET imgui::imgui)
    target_link_libraries(myNotes PRIVATE glfw glad imgui::imgui dl m)
elseif(TARGET imgui)
    target_link_libraries(myNotes PRIVATE glfw glad imgui dl m)
else()
    message(FATAL_ERROR "ImGui target not found")
endif()

# optional EGL for Wayland
find_library(EGL_LIB EGL)
if(EGL_LIB)
    target_link_libraries(myNotes PRIVATE ${EGL_LIB})
endif()
