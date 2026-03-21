#include <gfx/Model.hpp>

void Model::loadModel(std::string path)
{
    Assimp::Importer import;
    const aiScene *scene = import.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);	
	
    if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) 
    {
        std::cout << "ERROR::ASSIMP::" << import.GetErrorString() << std::endl;
        return;
    }
    directory = path.substr(0, path.find_last_of('/'));

    processNode(scene->mRootNode, scene);
}

void Model::Draw(Shader &shader)
{
    for(unsigned int meshIndex = 0; meshIndex < meshes.size(); meshIndex++)
        meshes[meshIndex].Draw(shader);
}

void Model::processNode(aiNode *node, const aiScene *scene)
{
    // process all the node's meshes (if any)
    for(unsigned int i = 0; i < node->mNumMeshes; i++)
    {
        aiMesh *mesh = scene->mMeshes[node->mMeshes[i]]; 
        meshes.push_back(processMesh(mesh, scene));			
    }
    // then do the same for each of its children
    for(unsigned int i = 0; i < node->mNumChildren; i++)
    {
        processNode(node->mChildren[i], scene);
    }
}
Mesh Model::processMesh(aiMesh *mesh, const aiScene *scene)
{
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    std::vector<Texture> textures;

    loadVertices(mesh, &vertices);
    loadIndices(mesh, &indices);
    loadTextures(mesh, scene, &textures);
    
    return Mesh(vertices, indices, textures);
}

void Model::loadVertices(aiMesh *mesh, std::vector<Vertex> *vertices)
{
    for(unsigned int vertexIndex = 0; vertexIndex < mesh->mNumVertices; vertexIndex++)
    {
        Vertex vertex;
        // process vertex positions, normals and texture coordinates
        glm::vec3 position, normal; 
        position.x = mesh->mVertices[vertexIndex].x;
        position.y = mesh->mVertices[vertexIndex].y;
        position.z = mesh->mVertices[vertexIndex].z; 
        vertex.Position = position;

        normal.x = mesh->mNormals[vertexIndex].x;
        normal.y = mesh->mNormals[vertexIndex].y;
        normal.z = mesh->mNormals[vertexIndex].z;
        vertex.Normal = normal;

        if(mesh->mTextureCoords[0]) // does the mesh contain texture coordinates?
        {
            glm::vec2 meshCoords;
            meshCoords.x = mesh->mTextureCoords[0][vertexIndex].x; 
            meshCoords.y = mesh->mTextureCoords[0][vertexIndex].y;
            vertex.TexCoords = meshCoords;
        }
        else vertex.TexCoords = glm::vec2(0.0f, 0.0f);  

        vertices->push_back(vertex);
    }
}

void Model::loadIndices(aiMesh *mesh, std::vector<unsigned int> *indices)
{
    for(unsigned int faceIndex = 0; faceIndex < mesh->mNumFaces; faceIndex++)
    {
        aiFace face = mesh->mFaces[faceIndex];
        
        for(unsigned int index = 0; index < face.mNumIndices; index++)
        {
            indices->push_back(face.mIndices[index]);
        }
    }  
}

void Model::loadTextures(aiMesh *mesh, const aiScene *scene, std::vector<Texture> *textures)
{
    if(mesh->mMaterialIndex >= 0)
    {
        aiMaterial *material = scene->mMaterials[mesh->mMaterialIndex];

        std::vector<Texture> diffuseMaps = 
            loadTexturesOfSingleMaterial(material, aiTextureType_DIFFUSE, "texture_diffuse");
        textures->insert(textures->end(), diffuseMaps.begin(), diffuseMaps.end());

        std::vector<Texture> specularMaps = 
            loadTexturesOfSingleMaterial(material, aiTextureType_SPECULAR, "texture_specular");
        textures->insert(textures->end(), specularMaps.begin(), specularMaps.end());
    }  
}

std::vector<Texture> Model::loadTexturesOfSingleMaterial(aiMaterial *mat, aiTextureType type, std::string typeName)
{
    std::vector<Texture> textures;
    for(unsigned int i = 0; i < mat->GetTextureCount(type); i++)
    {
        aiString str;
        mat->GetTexture(type, i, &str);
        bool skip = false;

        for (Texture tex : textures_loaded)
        {
            if (std::strcmp(tex.path.data(), str.C_Str()) == 0)
            {
                textures.push_back(tex);
                skip = true;
                break;
            }
        }

        if (!skip)
        {
            Texture texture;
            texture.id = loadTextureFromFile(str.C_Str(), directory);
            texture.type = typeName;
            texture.path = str.C_Str();
            textures.push_back(texture);
            textures_loaded.push_back(texture);
        }
    }
    
    return textures;
}

unsigned int Model::loadTextureFromFile(const char *path, const std::string &directory)
{
    std::string filename = std::string(path);
    filename = directory + '/' + filename;

    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char *data = stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);
    if (data)
    {
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
    }
    else
    {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}
