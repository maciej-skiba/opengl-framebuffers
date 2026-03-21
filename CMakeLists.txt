cmake_minimum_required(VERSION 3.10)
project(opengl-room)

find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(assimp CONFIG REQUIRED)

# ===== LIBRARIES =====

add_library(core
    src/core/InputCallbacks.cpp
    src/core/Window.cpp
)

add_library(gfx
    src/gfx/Input.cpp
    src/gfx/Mesh.cpp
    src/gfx/MeshUtils.cpp
    src/gfx/Model.cpp
    src/gfx/Gui.cpp
    src/gfx/Attenuation.cpp
)

add_library(io
    src/io/FileLoader.cpp
)

add_library(external
    src/external/imgui/imgui.cpp
    src/external/imgui/imgui_draw.cpp
    src/external/imgui/imgui_tables.cpp
    src/external/imgui/imgui_widgets.cpp
    src/external/imgui/backends/imgui_impl_glfw.cpp
    src/external/imgui/backends/imgui_impl_opengl3.cpp
)

# ===== EXECUTABLE =====

add_executable(app
    src/main.cpp
)

# ===== INCLUDE DIRECTORIES =====

target_include_directories(core PUBLIC include include/external)
target_include_directories(gfx  PUBLIC include include/external include/external/imgui)
target_include_directories(io   PUBLIC include include/external)
target_include_directories(external
    PUBLIC
        include
        include/external
        include/external/imgui
        include/external/imgui/backends
)


# ===== LINK LIBRARIES (KLUCZOWE!) =====

target_link_libraries(core
    PUBLIC
        OpenGL::GL
        glfw
        GLEW::GLEW
        glm::glm
)

target_link_libraries(gfx
    PUBLIC
        OpenGL::GL
        glfw
        GLEW::GLEW
        glm::glm
        assimp
        external
)

target_link_libraries(io
    PUBLIC
        OpenGL::GL
        GLEW::GLEW
)

target_link_libraries(app
    PRIVATE
        core
        gfx
        io
        external
)

# ===== C++ STANDARD =====

target_compile_features(app PRIVATE cxx_std_17)
