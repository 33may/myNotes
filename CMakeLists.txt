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

# build GLAD manually
add_library(glad STATIC external/glad/src/glad.c)
target_include_directories(glad PUBLIC ${CMAKE_SOURCE_DIR}/external/glad/include)

# bring in GLFW and ImGui
add_subdirectory(external/glfw)
add_subdirectory(external/imgui)

# now descend into src where the executable is defined
add_subdirectory(src)
