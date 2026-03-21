#pragma once

#include <string>
#include <vector>
#include <gfx/Mesh.hpp>
#include <stb_image.h>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

class Model 
{
    public:
        Model(const char *path)
        {
            loadModel(path);
        }
        void Draw(Shader &shader);	
    private:
        // model data
        std::vector<Mesh> meshes;
        std::string directory;
        std::vector<Texture> textures_loaded; 

        void loadModel(std::string path);
        void loadVertices(aiMesh *mesh, std::vector<Vertex> *vertices);
        void loadIndices(aiMesh *mesh, std::vector<unsigned int> *indices);
        void loadTextures(aiMesh *mesh, const aiScene *scene, std::vector<Texture> *textures);
        std::vector<Texture> loadTexturesOfSingleMaterial(aiMaterial *mat, aiTextureType type, std::string typeName);
        unsigned int loadTextureFromFile(const char *path, const std::string &directory);

        void processNode(aiNode *node, const aiScene *scene);
        Mesh processMesh(aiMesh *mesh, const aiScene *scene);
};