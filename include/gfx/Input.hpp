#pragma once
struct GLFWwindow;
#include "camera.hpp"

void ProcessInput(GLFWwindow* window, Camera* camera, ushort &postProcShaderIndex);

extern bool flashlightOn;