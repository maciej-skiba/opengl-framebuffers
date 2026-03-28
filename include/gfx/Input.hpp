#pragma once
struct GLFWwindow;
#include "camera.hpp"

void ProcessInput(GLFWwindow* window, Camera* camera, short &postProcShaderIndex, bool &antialiasingOn);

extern bool flashlightOn;