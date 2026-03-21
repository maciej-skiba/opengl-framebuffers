#pragma once
struct GLFWwindow;

class Window
{
public:
    static float deltaTime;
    
    static int InitializeOpenGL(GLFWwindow*& window);
    static void framebuffer_size_callback(GLFWwindow* window, int width, int height);
    static void UpdateDeltaTime();
};