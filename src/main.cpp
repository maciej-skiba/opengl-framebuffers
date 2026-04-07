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

unsigned int loadCubemap(std::vector<std::string> faces)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int width, height, nrChannels;
    for (unsigned int i = 0; i < faces.size(); i++)
    {
        unsigned char *data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
        if (data)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 
                         0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data
            );
            stbi_image_free(data);
        }
        else
        {
            std::cout << "Cubemap tex failed to load at path: " << faces[i] << std::endl;
            stbi_image_free(data);
        }
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return textureID;
}

int main(void)
{    
    GLFWwindow* window;
    int initSuccess = 1;

    if (Window::InitializeOpenGL(window) != initSuccess)
    {
        return -1;
    }

    Gui::ImGuiInit(window);

    unsigned int boxVao, boxVbo, skyboxVao, skyboxVbo;
    
    const char* cubeVertShaderPath = "../shaders/plane/plane_vert.glsl";
    const char* cubeFragShaderPath = "../shaders/plane/plane_frag.glsl";

    Shader planeShader(cubeVertShaderPath, cubeFragShaderPath);

    const char* houseVertShaderPath = "../shaders/house/house_vert.glsl";
    const char* houseFragShaderPath = "../shaders/house/house_frag.glsl";
        
    Shader houseShader(houseVertShaderPath, houseFragShaderPath);

    const char* skyboxVertShaderPath = "../shaders/skybox/skybox_vert.glsl";
    const char* skyboxFragShaderPath = "../shaders/skybox/skybox_frag.glsl";
        
    Shader skyboxShader(skyboxVertShaderPath, skyboxFragShaderPath);

    int numOfVerticesInBox = 36;

    glm::vec3 planePosition = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 planeColor =  glm::vec3(0.15, 0.5, 0.1);  //green
    glm::vec3 polygonColor =  glm::vec3(0.9, 1, 0.6);; //orange
    
    int boxBufferSize = numOfVerticesInBox * 6;
    CreateBoxVao(boxVao, boxVbo, boxVertices, boxBufferSize);
    CreateBoxVao(skyboxVao, skyboxVbo, skyboxVertices, boxBufferSize);

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

    glm::mat4 planeModelMatrix = identityMatrix;
    glm::mat4 houseModelMatrix = identityMatrix;
    glm::mat4 skyboxModelMatrix = identityMatrix;

    glm::mat4 projectionMatrix = 
        glm::perspective(
            glm::radians(mainCamera->Zoom),
            aspectRatio, 
            nearClippingPlane,
            farClippingPlane);

    planeShader.UseProgram();
    planeShader.SetUniformMat4("projection", projectionMatrix);

    houseShader.UseProgram();
    houseShader.SetUniformMat4("projection", projectionMatrix);

    skyboxShader.UseProgram();
    skyboxShader.SetUniformMat4("projection", projectionMatrix);

    // ---CUSTOM FRAMEBUFFER OBJECTS---

    unsigned int multisampleFbo;
    glGenFramebuffers(1, &multisampleFbo);
    glBindFramebuffer(GL_FRAMEBUFFER, multisampleFbo);

    unsigned int msTextureColorBuffer;
    glGenTextures(1, &msTextureColorBuffer);
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, msTextureColorBuffer);
    int numOfAntialiasingSamples = 4;
    glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, numOfAntialiasingSamples, GL_RGB, WINDOW_WIDTH, WINDOW_HEIGHT, GL_TRUE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, msTextureColorBuffer, 0);

    unsigned int msRbo;
    glGenRenderbuffers(1, &msRbo);
    glBindRenderbuffer(GL_RENDERBUFFER, msRbo);
    glRenderbufferStorageMultisample(GL_RENDERBUFFER, numOfAntialiasingSamples, GL_DEPTH24_STENCIL8, WINDOW_WIDTH, WINDOW_HEIGHT);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, msRbo);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
    }

    unsigned int intermediateFbo;
    glGenFramebuffers(1, &intermediateFbo);
    glBindFramebuffer(GL_FRAMEBUFFER, intermediateFbo);

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

    const char* postProcVertShaderPath_1 = "../shaders/postprocessing/postProc_vert.glsl";
    const char* postProcFragShaderPath_1 = "../shaders/postprocessing/postProc_frag1.glsl";

    Shader postProcShader1(postProcVertShaderPath_1, postProcFragShaderPath_1);
    postProcShader1.UseProgram();
    postProcShader1.SetUniformInt("screenTexture", 0);

    const char* postProcVertShaderPath_2 = "../shaders/postprocessing/postProc_vert.glsl";
    const char* postProcFragShaderPath_2 = "../shaders/postprocessing/postProc_frag2.glsl";

    Shader postProcShader2(postProcVertShaderPath_2, postProcFragShaderPath_2);
    postProcShader2.UseProgram();
    postProcShader2.SetUniformInt("screenTexture", 0);
    
    const char* postProcVertShaderPath_3 = "../shaders/postprocessing/postProc_vert.glsl";
    const char* postProcFragShaderPath_3 = "../shaders/postprocessing/postProc_frag3.glsl";

    Shader postProcShader3(postProcVertShaderPath_3, postProcFragShaderPath_3);
    postProcShader3.UseProgram();
    postProcShader3.SetUniformInt("screenTexture", 0);

    const char* postProcVertShaderPath_4 = "../shaders/postprocessing/postProc_vert.glsl";
    const char* postProcFragShaderPath_4 = "../shaders/postprocessing/postProc_frag4.glsl";

    Shader postProcShader4(postProcVertShaderPath_4, postProcFragShaderPath_4);
    postProcShader4.UseProgram();
    postProcShader4.SetUniformInt("screenTexture", 0);

    const char* postProcVertShaderPath_5 = "../shaders/postprocessing/postProc_vert.glsl";
    const char* postProcFragShaderPath_5 = "../shaders/postprocessing/postProc_frag5.glsl";

    Shader postProcShader5(postProcVertShaderPath_5, postProcFragShaderPath_5);
    postProcShader5.UseProgram();
    postProcShader5.SetUniformInt("screenTexture", 0);

    std::unordered_map<short, Shader> postprocessingShaders =
    {
        { 1, postProcShader1 },
        { 2, postProcShader2 },
        { 3, postProcShader3 },
        { 4, postProcShader4 },
        { 5, postProcShader5 },
    };
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // ---------------------------------

    const char* houseModelPath = "../assets/models/house/HouseSuburban.obj";
    Model houseModel(houseModelPath);

    std::cout << "Entering main loop\n";

    short currentPostProcShaderIndex = 1;

    glm::vec3 dirLightPosition = glm::vec3(-20.0f, 50.0f, 0.0f);
    glm::vec3 dirLightColor = glm::vec3(1.0f, 1.0f, 1.0f);

    bool antialiasingOn = true;
    std::vector<std::string> faces
    {
        "../assets/textures/skybox/right.png",
        "../assets/textures/skybox/left.png",
        "../assets/textures/skybox/top.png",
        "../assets/textures/skybox/bottom.png",
        "../assets/textures/skybox/front.png",
        "../assets/textures/skybox/back.png"
    };
    
    unsigned int cubemapTexture = loadCubemap(faces); 

    // load height map texture
    int width, height, nChannels;
    unsigned char *data = stbi_load(
        "../assets/heightmaps/iceland_heightmap.png",
        &width, &height, &nChannels,
        0);

    // vertex generation
    std::vector<float> vertices;
    float yScale = 64.0f / 256.0f, yShift = 25.0f;  // apply a scale+shift to the height data
    for(unsigned int i = 0; i < height; i++)
    {
        for(unsigned int j = 0; j < width; j++)
        {
            // retrieve texel for (i,j) tex coord
            unsigned char* texel = data + (j + width * i) * nChannels;
            // raw height at coordinate
            unsigned char y = texel[0];

            // vertex
            vertices.push_back( -height/2.0f + i );        // v.x
            vertices.push_back( (int)y * yScale - yShift); // v.y
            vertices.push_back( -width/2.0f + j );        // v.z
        }
    }

    // index generation
    std::vector<unsigned int> indices;
    for(unsigned int i = 0; i < height-1; i++)       // for each row a.k.a. each strip
    {
        for(unsigned int j = 0; j < width; j++)      // for each column
        {
            for(unsigned int k = 0; k < 2; k++)      // for each side of the strip
            {
                indices.push_back(j + width * (i + k));
            }
        }
    }

    const unsigned int NUM_STRIPS = height-1;
    const unsigned int NUM_VERTS_PER_STRIP = width*2;

    // register VAO
    GLuint terrainVAO, terrainVBO, terrainEBO;
    glGenVertexArrays(1, &terrainVAO);
    glBindVertexArray(terrainVAO);

    glGenBuffers(1, &terrainVBO);
    glBindBuffer(GL_ARRAY_BUFFER, terrainVBO);
    glBufferData(GL_ARRAY_BUFFER,
                vertices.size() * sizeof(float),       // size of vertices buffer
                &vertices[0],                          // pointer to first element
                GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glEnableVertexAttribArray(0);

    glGenBuffers(1, &terrainEBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, terrainEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                indices.size() * sizeof(unsigned int), // size of indices buffer
                &indices[0],                           // pointer to first element
                GL_STATIC_DRAW);



    while (!glfwWindowShouldClose(window))
    {
        Window::UpdateDeltaTime();

        if (antialiasingOn)
        {
            glBindFramebuffer(GL_FRAMEBUFFER, multisampleFbo);
        }
        else
        {
            glBindFramebuffer(GL_FRAMEBUFFER, intermediateFbo);
        }

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);
        
        // -house-
        houseShader.UseProgram();
        houseModelMatrix = glm::translate(identityMatrix, planePosition);
        houseModelMatrix = glm::scale(houseModelMatrix, glm::vec3(0.002f));
        houseShader.SetUniformMat4("model", houseModelMatrix);
        houseShader.SetUniformMat4("view", mainCamera->GetViewMatrix());

        houseShader.SetUniformVec3("cameraPos", mainCamera->Position);
        houseShader.SetUniformVec3("dirLight[0].position", dirLightPosition);
        houseShader.SetUniformVec3("dirLight[0].ambient", dirLightColor * 0.1f);
        houseShader.SetUniformVec3("dirLight[0].diffuse", dirLightColor);
        houseShader.SetUniformVec3("dirLight[0].specular", dirLightColor);

        houseModel.Draw(houseShader);

        // -------

        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

        // -plane-
        glBindVertexArray(boxVao);

        planeShader.UseProgram();
        planeShader.SetUniformMat4("view", mainCamera->GetViewMatrix());
        planeModelMatrix = glm::translate(identityMatrix, planePosition);
        planeModelMatrix = glm::scale(planeModelMatrix, glm::vec3(0.5f));
        planeShader.SetUniformMat4("model", planeModelMatrix);
        planeShader.SetUniformVec3("lightColor", polygonColor);
        
        // draw mesh
        glBindVertexArray(terrainVAO);
        // render the mesh triangle strip by triangle strip - each row at a time
        for(unsigned int strip = 0; strip < NUM_STRIPS; ++strip)
        {
            glDrawElements(GL_TRIANGLE_STRIP,   // primitive type
                        NUM_VERTS_PER_STRIP, // number of indices to render
                        GL_UNSIGNED_INT,     // index data type
                        (void*)(sizeof(unsigned int)
                                    * NUM_VERTS_PER_STRIP
                                    * strip)); // offset to starting index
        }

        planeShader.SetUniformVec3("lightColor", planeColor);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        for(unsigned int strip = 0; strip < NUM_STRIPS; ++strip)
        {
            glDrawElements(GL_TRIANGLE_STRIP,   // primitive type
                        NUM_VERTS_PER_STRIP, // number of indices to render
                        GL_UNSIGNED_INT,     // index data type
                        (void*)(sizeof(unsigned int)
                                    * NUM_VERTS_PER_STRIP
                                    * strip)); // offset to starting index
        }

        // -------

        // --skybox--

        glDepthFunc(GL_LEQUAL);
        skyboxShader.UseProgram();
        skyboxShader.SetUniformMat4("view", 
            glm::mat4(glm::mat3(mainCamera->GetViewMatrix())));
        glBindVertexArray(skyboxVao);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
        glDrawArrays(GL_TRIANGLES, 0, numOfVerticesInBox);
        glDepthFunc(GL_LESS);
        
        // -------

        // --postprocessing--
        
        postprocessingShaders.at(currentPostProcShaderIndex).UseProgram();
        
        if (antialiasingOn)
        {
            glBindFramebuffer(GL_READ_FRAMEBUFFER, multisampleFbo);
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, intermediateFbo);
            glBlitFramebuffer(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, GL_COLOR_BUFFER_BIT, GL_NEAREST);
        }
        else
        {
            glBindFramebuffer(GL_FRAMEBUFFER, intermediateFbo);
        }

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
        ProcessInput(window, mainCamera.get(), currentPostProcShaderIndex, antialiasingOn);
        mainCamera->updateCameraVectors();
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &boxVao);
    glDeleteBuffers(1, &boxVbo);
    glDeleteFramebuffers(1, &multisampleFbo);

    glfwTerminate();
    Gui::ImGuiShutdown();
    return 0;
}