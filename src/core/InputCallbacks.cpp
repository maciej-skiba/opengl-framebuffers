#include "common/gl_includes.hpp"
#include "core/InputCallbacks.hpp"
#include "camera.hpp"

// stan myszy ukryty lokalnie:
static bool  firstMouse = true;
static float lastX = 800.0f / 2.0f;
static float lastY = 600.0f / 2.0f;

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    Camera* camera = static_cast<Camera*>(glfwGetWindowUserPointer(window));
    if (!camera) return;

    if (firstMouse) {
        lastX = xpos; lastY = ypos; firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed
    lastX = xpos; lastY = ypos;

    camera->AddMouseDelta(xoffset, yoffset);
}

void scroll_callback(GLFWwindow* window, double, double yoffset)
{
    Camera* camera = static_cast<Camera*>(glfwGetWindowUserPointer(window));
    if (camera) camera->ProcessMouseScroll(static_cast<float>(yoffset));
}