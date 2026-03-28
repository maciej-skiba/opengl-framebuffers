#include "common/gl_includes.hpp"
#include "gfx/Input.hpp"
#include "core/Window.hpp"

bool spacePressedLastFrame = false;

void ProcessInput(GLFWwindow* window, Camera* camera, short &postProcShaderIndex, bool &antialiasingOn)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) camera->ProcessKeyboardWithDepthLimit(FORWARD);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) camera->ProcessKeyboardWithDepthLimit(BACKWARD);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) camera->ProcessKeyboardWithDepthLimit(LEFT);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) camera->ProcessKeyboardWithDepthLimit(RIGHT);    
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) camera->ProcessKeyboardWithDepthLimit(DOWN);
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) camera->ProcessKeyboardWithDepthLimit(UP);

    // Choosing Framebuffers
    if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) postProcShaderIndex = 1;
    if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS) postProcShaderIndex = 2;
    if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS) postProcShaderIndex = 3;
    if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS) postProcShaderIndex = 4;
    if (glfwGetKey(window, GLFW_KEY_5) == GLFW_PRESS) postProcShaderIndex = 5;

    if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS) {
        if (!spacePressedLastFrame) { antialiasingOn = !antialiasingOn; spacePressedLastFrame = true; }
    } else {
        spacePressedLastFrame = false;
    }

    camera->Position = glm::mix(camera->Position, camera->targetPosition, camera->moveSmoothing);
}