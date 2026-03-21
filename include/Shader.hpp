#pragma once

#include <GL/glew.h>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <unordered_map>

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "io/FileLoader.hpp"

class Shader
{
public:
    unsigned int ID;

    mutable std::unordered_map<std::string, GLint> uniformLocationCache;

    Shader(const char* vertexPath, const char* fragmentPath)
    {
        std::string vertexCode;
        std::string fragmentCode;

        try {
            vertexCode   = LoadShaderSource(vertexPath);
            fragmentCode = LoadShaderSource(fragmentPath);
        }
        catch (const std::exception& e) {
            std::cerr << "ERROR::SHADER::FILE_NOT_SUCCESSFULLY_READ: " << e.what() << std::endl;
        }

        const char* vShaderCode = vertexCode.c_str();
        const char* fShaderCode = fragmentCode.c_str();

        unsigned int vertex, fragment;

        vertex = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertex, 1, &vShaderCode, NULL);
        glCompileShader(vertex);
        CheckShaderCompilation(vertex, vertexPath);

        fragment = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragment, 1, &fShaderCode, NULL);
        glCompileShader(fragment);
        CheckShaderCompilation(fragment, fragmentPath);

        ID = glCreateProgram();
        glAttachShader(ID, vertex);
        glAttachShader(ID, fragment);
        glLinkProgram(ID);
        CheckShaderLink(ID);

        glDeleteShader(vertex);
        glDeleteShader(fragment);
    }

    void UseProgram() 
    { 
        glUseProgram(ID); 
    }

    void SetUniformBool(const std::string &name, bool value) const {
        glUniform1i(GetUniformLocation(name), (int)value);
    }

    void SetUniformInt(const std::string &name, int value) const {
        glUniform1i(GetUniformLocation(name), value);
    }

    void SetUniformFloat(const std::string &name, float value) const {
        glUniform1f(GetUniformLocation(name), value);
    }

    void SetUniformMat4(const std::string &name, const glm::mat4& value) const {
        glUniformMatrix4fv(GetUniformLocation(name), 1, GL_FALSE, glm::value_ptr(value));
    }

    void SetUniformVec3(const std::string &name, const glm::vec3& value) const {
        glUniform3fv(GetUniformLocation(name), 1, glm::value_ptr(value));
    }

private:
    void CheckShaderCompilation(unsigned int shader, const char* shaderPath)
    {
        int success;
        char infoLog[512];
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

        if(!success)
        {
            glGetShaderInfoLog(shader, 512, NULL, infoLog);
            std::cout << "ERROR::SHADER::COMPILATION_FAILED\n" << shaderPath << std::endl << infoLog << std::endl;
        }
    }

    void CheckShaderLink(unsigned int shaderProgram)
    {
        int success;
        char infoLog[512];
        glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);

        if(!success)
        {
            glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
            std::cout << "ERROR::SHADER::PROGRAM::LINK_FAILED\n" << infoLog << std::endl;
        }
    }

    GLint GetUniformLocation(const std::string& name) const {
        if (uniformLocationCache.find(name) != uniformLocationCache.end())
            return uniformLocationCache[name];

        GLint location = glGetUniformLocation(ID, name.c_str());
        uniformLocationCache[name] = location;
        return location;
    }
};