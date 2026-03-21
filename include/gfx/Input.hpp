#pragma once
struct GLFWwindow;
#include "camera.hpp"

void ProcessInput(GLFWwindow* window, Camera* camera);

extern bool flashlightOn;