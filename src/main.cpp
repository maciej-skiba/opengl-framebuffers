#include "common/gl_includes.hpp"
#include <iostream>
#include <memory>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "vertices.hpp" 
#include "camera.hpp"

#include "app/Config.hpp"
#include "core/Window.hpp"
#include "core/InputCallbacks.hpp"
#include "gfx/Input.hpp"
#include "gfx/MeshUtils.hpp"
#include "gfx/Model.hpp"
#include "gfx/Attenuation.hpp"
#include "gfx/Gui.hpp"
#include "io/FileLoader.hpp"
#include "Shader.hpp"

const glm::mat4 identityMatrix = glm::mat4(1.0f);
extern bool flashlightOn;

int main(void)
{    
    GLFWwindow* window;
    int initSuccess = 1;

    if (Window::InitializeOpenGL(window) != initSuccess)
    {
        return -1;
    }

    Gui::ImGuiInit(window);

    stbi_set_flip_vertically_on_load(true);

    unsigned int boxVao;
    unsigned int planeVao;
    
    const char* cubeVertShaderPath = "../shaders/cube_vert.glsl";
    const char* cubeFragShaderPath = "../shaders/cube_frag.glsl";

    Shader cubeShader(cubeVertShaderPath, cubeFragShaderPath);
        
    int numOfVerticesInBox = 36;
    int amountOfCubes = 2;

    glm::vec3 cubePositions[] = {
        glm::vec3(-3.0f, 1.0f, 0.0f),
        glm::vec3(3.0f, 1.0f, 0.0f),
    };

    glm::vec3 cubeColors[] = {
        glm::vec3(0.4, 1, 0.4), //green
        glm::vec3(0.4, 1, 0.4), //green
       
    };

    glm::vec3 planePosition = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 planeColor =  glm::vec3(0.7, 0.7, 0.7);  //gray
    
    int boxBufferSize = numOfVerticesInBox * 6;
    CreateBoxVao(boxVao, boxVertices, boxBufferSize);
    CreateBoxVao(planeVao, planeVertices, boxBufferSize);

    glEnable(GL_DEPTH_TEST);

    std::unique_ptr<Camera> mainCamera = std::make_unique<Camera>(
        glm::vec3(-4.0f, 1.4f,  4.0f),
        glm::vec3(0.0f, 1.0f, 0.0f));

    glfwSetWindowUserPointer(window, mainCamera.get());

    float lastFrame = 0.0f;
    float aspectRatio = static_cast<float>(WINDOW_WIDTH) / WINDOW_HEIGHT;
    float nearClippingPlane = 0.1f;
    float farClippingPlane = 100.0f;

    glm::mat4 cubeModelMatrix = identityMatrix;
    glm::mat4 projectionMatrix = 
        glm::perspective(
            glm::radians(mainCamera->Zoom),
            aspectRatio, 
            nearClippingPlane,
            farClippingPlane);


    cubeShader.UseProgram();
    cubeShader.SetUniformMat4("projection", projectionMatrix);
    
    while (!glfwWindowShouldClose(window))
    {
        Window::UpdateDeltaTime();

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        cubeShader.UseProgram();
        cubeShader.SetUniformMat4("view", mainCamera->GetViewMatrix());
        
        // -cubes-
        glBindVertexArray(boxVao);

        for (int cube = 0; cube < amountOfCubes; cube++)
        {
            cubeModelMatrix = glm::translate(identityMatrix, cubePositions[cube]);
            cubeModelMatrix = glm::scale(cubeModelMatrix, glm::vec3(0.5f));
            cubeShader.SetUniformMat4("model", cubeModelMatrix);
            cubeShader.SetUniformVec3("lightColor", cubeColors[cube]);

            glDrawArrays(GL_TRIANGLES, 0, numOfVerticesInBox);
        }
        // -------

        // -plane-
        glBindVertexArray(planeVao);

        cubeModelMatrix = glm::translate(identityMatrix, planePosition);
        cubeModelMatrix = glm::scale(cubeModelMatrix, glm::vec3(0.5f));
        cubeShader.SetUniformMat4("model", cubeModelMatrix);
        cubeShader.SetUniformVec3("lightColor", planeColor);

        glDrawArrays(GL_TRIANGLES, 0, numOfVerticesInBox);
        // -------

        Gui::ImGuiFrame(window);
        glfwSwapBuffers(window);
        ProcessInput(window, mainCamera.get());
        mainCamera->updateCameraVectors();
        glfwPollEvents();
    }

    glfwTerminate();
    Gui::ImGuiShutdown();
    return 0;
}