#include "common/gl_includes.hpp"
#include <iostream>
#include <memory>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <unordered_map>

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

    unsigned int boxVao, boxVbo;
    unsigned int planeVao, planeVbo;
    
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
    CreateBoxVao(boxVao, boxVbo, boxVertices, boxBufferSize);
    CreateBoxVao(planeVao, planeVbo, planeVertices, boxBufferSize);

    //screen quad for postprocessing
    unsigned int quadVAO, quadVBO;
    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);
    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

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

    // ---CUSTOM FRAMEBUFFER OBJECTS---

    unsigned int fbo;
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    unsigned int textureColorBuffer;
    glGenTextures(1, &textureColorBuffer);
    glBindTexture(GL_TEXTURE_2D, textureColorBuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, WINDOW_WIDTH, WINDOW_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColorBuffer, 0);

    unsigned int rbo;
    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, WINDOW_WIDTH, WINDOW_HEIGHT);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
    }

    const char* postProcVertShaderPath_1 = "../shaders/postProc_vert.glsl";
    const char* postProcFragShaderPath_1 = "../shaders/postProc_frag1.glsl";

    Shader postProcShader1(postProcVertShaderPath_1, postProcFragShaderPath_1);
    postProcShader1.UseProgram();
    postProcShader1.SetUniformInt("screenTexture", 0);

    const char* postProcVertShaderPath_2 = "../shaders/postProc_vert.glsl";
    const char* postProcFragShaderPath_2 = "../shaders/postProc_frag2.glsl";

    Shader postProcShader2(postProcVertShaderPath_2, postProcFragShaderPath_2);
    postProcShader2.UseProgram();
    postProcShader2.SetUniformInt("screenTexture", 0);
    
    const char* postProcVertShaderPath_3 = "../shaders/postProc_vert.glsl";
    const char* postProcFragShaderPath_3 = "../shaders/postProc_frag3.glsl";

    Shader postProcShader3(postProcVertShaderPath_3, postProcFragShaderPath_3);
    postProcShader3.UseProgram();
    postProcShader3.SetUniformInt("screenTexture", 0);

    const char* postProcVertShaderPath_4 = "../shaders/postProc_vert.glsl";
    const char* postProcFragShaderPath_4 = "../shaders/postProc_frag4.glsl";

    Shader postProcShader4(postProcVertShaderPath_4, postProcFragShaderPath_4);
    postProcShader4.UseProgram();
    postProcShader4.SetUniformInt("screenTexture", 0);

    const char* postProcVertShaderPath_5 = "../shaders/postProc_vert.glsl";
    const char* postProcFragShaderPath_5 = "../shaders/postProc_frag5.glsl";

    Shader postProcShader5(postProcVertShaderPath_5, postProcFragShaderPath_5);
    postProcShader5.UseProgram();
    postProcShader5.SetUniformInt("screenTexture", 0);

    
    std::unordered_map<ushort, Shader> postprocessingShaders =
    {
        { 1, postProcShader1 },
        { 2, postProcShader2 },
        { 3, postProcShader3 },
        { 4, postProcShader4 },
        { 5, postProcShader5 },
    };
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // ---------------------------------

    std::cout << "Entering main loop\n";

    ushort currentPostProcShaderIndex = 1;

    while (!glfwWindowShouldClose(window))
    {
        Window::UpdateDeltaTime();

        glBindFramebuffer(GL_FRAMEBUFFER, fbo);

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);

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

        // --postprocessing--
        
        postprocessingShaders.at(currentPostProcShaderIndex).UseProgram();
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glBindVertexArray(quadVAO);
        glDisable(GL_DEPTH_TEST);
        glBindTexture(GL_TEXTURE_2D, textureColorBuffer);	// use the color attachment texture as the texture of the quad plane
        glDrawArrays(GL_TRIANGLES, 0, 6);

        // ------------------

        Gui::ImGuiFrame(window);
        glfwSwapBuffers(window);
        ProcessInput(window, mainCamera.get(), currentPostProcShaderIndex);
        mainCamera->updateCameraVectors();
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &boxVao);
    glDeleteVertexArrays(1, &planeVao);
    glDeleteBuffers(1, &boxVbo);
    glDeleteBuffers(1, &planeVbo);
    glDeleteFramebuffers(1, &fbo);

    glfwTerminate();
    Gui::ImGuiShutdown();
    return 0;
}