#pragma once

#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include "GLFW/glfw3.h"

class Gui
{
private:
    static bool g_VSync;
    static bool g_Wireframe;
    static float g_Gamma;
public:
    static void ImGuiInit(GLFWwindow* window);
    static void ImGuiFrame(GLFWwindow* window);
    static void ImGuiShutdown();
    static void ShowFrameInfo();
    static void ShowControlsInfo();
};