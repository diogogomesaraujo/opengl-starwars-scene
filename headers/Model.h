#pragma once

#include "header.h"
#include "Mesh.h"

#include <unordered_map>
#include <tuple>
#include <string>
#include <iostream>
#include <vector>
#include <cmath>

// Include Assimp
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

// Include stb_image if you haven't already
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

using std::vector;
using std::tuple;
using std::string;
using std::make_tuple;
using std::abs;
using std::get;
using std::cout;
using std::endl;

class Model
{
public:
    // Position of the model in the scene
    tuple<float, float, float> position = make_tuple(0.0f, 0.0f, 0.0f);

    // A static texture cache to avoid loading the same texture multiple times
    static std::unordered_map<std::string, Texture> textureCache;

    // Constructor: automatically loads a model from a file
    Model(const char *path)
    {
        loadModel(path);
    }

    // Render the model using a given shader
    void Draw(Shader &shader)
    {
        for (unsigned int i = 0; i < meshes.size(); i++)
        {
            meshes[i].Draw(shader);
        }
    }

    /**
     *  The new lerp function:
     *    - Moves 'a' towards 'b' by step 's'
     *    - If the distance is less than 's', it returns 'b'
     */
    float lerp(float a, float b, float s)
    {
        float distance = std::fabs(a - b);
        if (distance < s)
            return b;
        else if (a < b)
            return a + s;
        else
            return a - s;
    }

    // Moves the model's position gradually toward finalPos
    void modelMove(tuple<float, float, float> finalPos)
    {
        // Use 'speed' as a step, not as a fraction
        float speed = 0.5f; 
        position = make_tuple(
            lerp(get<0>(position), get<0>(finalPos), speed),
            lerp(get<1>(position), get<1>(finalPos), speed),
            lerp(get<2>(position), get<2>(finalPos), speed)
        );
    }

    // Returns true if the model is "close enough" to the target position
    bool hasReachedTarget(tuple<float, float, float> finalPos)
    {
        float threshold = 3.0f; // Acceptable distance to the target
        return (std::fabs(get<0>(position) - get<0>(finalPos)) < threshold &&
                std::fabs(get<1>(position) - get<1>(finalPos)) < threshold &&
                std::fabs(get<2>(position) - get<2>(finalPos)) < threshold);
    }

private:
    // The collection of meshes that compose this model
    vector<Mesh> meshes;
    // The directory (folder) where the model file is located
    string directory;

    // Loads a model from file using Assimp
    void loadModel(const string &path)
    {
        Assimp::Importer import;
        // Add extra optimization flags
        const aiScene *scene = import.ReadFile(
            path,
            aiProcess_Triangulate         |
            aiProcess_FlipUVs             |
            aiProcess_OptimizeMeshes      |
            aiProcess_OptimizeGraph       |
            aiProcess_JoinIdenticalVertices
        );

        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
        {
            cout << "ERROR::ASSIMP::" << import.GetErrorString() << endl;
            return;
        }
        directory = path.substr(0, path.find_last_of('/'));

        // Process the root node recursively
        processNode(scene->mRootNode, scene);
    }

    // Recursively processes a node in the Assimp scene
    void processNode(aiNode *node, const aiScene *scene)
    {
        // Process each mesh at this node
        for (unsigned int i = 0; i < node->mNumMeshes; i++)
        {
            aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
            meshes.push_back(processMesh(mesh, scene));
        }
        // Recursively process child nodes
        for (unsigned int i = 0; i < node->mNumChildren; i++)
        {
            processNode(node->mChildren[i], scene);
        }
    }

    // Extract mesh data from an Assimp mesh
    Mesh processMesh(aiMesh *mesh, const aiScene *scene)
    {
        vector<Vertex> vertices;
        vector<unsigned int> indices;
        vector<Texture> textures;

        // Walk through each vertex
        for (unsigned int i = 0; i < mesh->mNumVertices; i++)
        {
            Vertex vertex;
            glm::vec3 vector3; 

            // Positions
            vector3.x = mesh->mVertices[i].x;
            vector3.y = mesh->mVertices[i].y;
            vector3.z = mesh->mVertices[i].z;
            vertex.Position = vector3;

            // Normals
            if (mesh->HasNormals())
            {
                vector3.x = mesh->mNormals[i].x;
                vector3.y = mesh->mNormals[i].y;
                vector3.z = mesh->mNormals[i].z;
                vertex.Normal = vector3;
            }
            else
            {
                vertex.Normal = glm::vec3(0.0f, 0.0f, 0.0f);
            }

            // Texture Coordinates
            if (mesh->mTextureCoords[0])
            {
                glm::vec2 vec2;
                vec2.x = mesh->mTextureCoords[0][i].x;
                vec2.y = mesh->mTextureCoords[0][i].y;
                vertex.TexCoords = vec2;
            }
            else
            {
                vertex.TexCoords = glm::vec2(0.0f, 0.0f);
            }

            vertices.push_back(vertex);
        }

        // Walk through each face (triangle) and get indices
        for (unsigned int i = 0; i < mesh->mNumFaces; i++)
        {
            aiFace face = mesh->mFaces[i];
            for (unsigned int j = 0; j < face.mNumIndices; j++)
            {
                indices.push_back(face.mIndices[j]);
            }
        }

        // Process materials
        aiMaterial *material = scene->mMaterials[mesh->mMaterialIndex];

        // 1. Diffuse maps
        vector<Texture> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
        textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());

        // 2. Specular maps
        vector<Texture> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
        textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());

        // 3. Normal maps
        vector<Texture> normalMaps = loadMaterialTextures(material, aiTextureType_HEIGHT, "texture_normal");
        textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());

        // 4. Height maps
        vector<Texture> heightMaps = loadMaterialTextures(material, aiTextureType_AMBIENT, "texture_height");
        textures.insert(textures.end(), heightMaps.begin(), heightMaps.end());

        // Return a Mesh object
        return Mesh(vertices, indices, textures);
    }

    // Loads textures of a specific type for a given material
    vector<Texture> loadMaterialTextures(aiMaterial *mat, aiTextureType type, const string &typeName)
    {
        vector<Texture> textures;
        for (unsigned int i = 0; i < mat->GetTextureCount(type); i++)
        {
            aiString str;
            mat->GetTexture(type, i, &str);

            // Check if we have already loaded this texture
            if (textureCache.find(str.C_Str()) != textureCache.end())
            {
                // Already loaded
                textures.push_back(textureCache[str.C_Str()]);
            }
            else
            {
                // Load a new texture
                Texture texture;
                texture.id   = TextureFromFile(str.C_Str(), directory);
                texture.type = typeName;
                texture.path = str.C_Str();

                textures.push_back(texture);
                textureCache[str.C_Str()] = texture;
            }
        }
        return textures;
    }

    // Helper to load texture from file using stb_image
    unsigned int TextureFromFile(const char *path, const string &directory)
    {
        string filename = string(path);
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
            else
                format = GL_RGB; // fallback

            glBindTexture(GL_TEXTURE_2D, textureID);
            glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
            glGenerateMipmap(GL_TEXTURE_2D);

            // Set texture wrapping/filtering options
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            stbi_image_free(data);
        }
        else
        {
            std::cout << "Texture failed to load at path: " << filename << std::endl;
            stbi_image_free(data);
        }

        return textureID;
    }
};

// Initialize the static texture cache
std::unordered_map<std::string, Texture> Model::textureCache = {};
