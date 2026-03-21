#include "common/gl_includes.hpp"
#include "app/Config.hpp"
#include "core/Window.hpp"
#include "core/InputCallbacks.hpp"
#include <iostream>

float Window::deltaTime = 0.0f;
static float lastFrame = 0.0f;

int Window::InitializeOpenGL(GLFWwindow*& window)
{
    if (!glfwInit()) return -1;

    window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "OpenGL - Room", nullptr, nullptr);
    if (!window) {
        std::cout << "Failed to create GLFW window\n";
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    if (glewInit() != GLEW_OK) {
        std::cout << "cant load glew\n";
    }
    
    glfwSwapInterval(0); // VSync OFF
    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
    glfwSetFramebufferSizeCallback(window, Window::framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    return 1;
}

void Window::framebuffer_size_callback(GLFWwindow*, int width, int height)
{
    glViewport(0, 0, width, height);
}

void Window::UpdateDeltaTime() {
    float currentFrame = glfwGetTime();
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;
}
