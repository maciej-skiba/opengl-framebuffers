#include "common/gl_includes.hpp"
#include "io/FileLoader.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <iostream>
#include <fstream> 
#include <sstream>  
#include <string>  
#include <algorithm>

unsigned int LoadTexture(char const* path)
{
    unsigned int textureID = 0;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data) {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    } else {
        std::cout << "Texture failed to load at path: " << path << "\n";
        stbi_image_free(data);
    }

    return textureID;
}

std::string LoadShaderSource(const std::string& path)
{
    std::ifstream in(path);
    if (!in.is_open())
        throw std::runtime_error("Cannot open shader file: " + path);

    std::stringstream out;
    std::string line;
    std::string baseDir = path.substr(0, path.find_last_of("/\\") + 1);

    while (std::getline(in, line)) {
        if (line.find("#include") == 0) {
            size_t l = line.find_first_of("\"");
            size_t r = line.find_last_of("\"");
            if (l != std::string::npos && r != std::string::npos && r > l) {
                std::string incName = line.substr(l + 1, r - l - 1);
                std::string incPath = baseDir + incName;
                out << LoadShaderSource(incPath) << "\n";
            } else {
                throw std::runtime_error("Malformed #include in: " + path + " -> " + line);
            }
        } else {
            out << line << "\n";
        }
    }

    return out.str();
}