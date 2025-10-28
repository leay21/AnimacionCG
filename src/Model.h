#pragma once

#include <vector>
#include <string>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "Mesh.h"

class Model {
public:
    // Datos del modelo
    std::vector<Mesh> meshes; // Un modelo se compone de una o más mallas
    std::string directory;

    // Constructor - Carga el modelo desde la ruta
    Model(std::string const& path) {
        loadModel(path);
    }

    // Dibuja el modelo (dibuja todas sus mallas)
    void Draw(/* Shader& shader */) {
        for (unsigned int i = 0; i < meshes.size(); i++)
            meshes[i].Draw(/* shader */);
    }

private:
    // Carga el archivo .fbx usando Assimp
    void loadModel(std::string const& path) {
        Assimp::Importer importer;
        // aiProcess_Triangulate: Asegura que todo esté en triángulos
        // aiProcess_FlipUVs: Voltea las texturas (común en OpenGL)
        const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);

        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
            // Manejo de error
            return;
        }
        directory = path.substr(0, path.find_last_of('/'));

        // Inicia el procesamiento recursivo de los nodos de Assimp
        processNode(scene->mRootNode, scene);
    }

    // Procesa un nodo (recursivo)
    void processNode(aiNode* node, const aiScene* scene) {
        // Procesa todas las mallas del nodo
        for (unsigned int i = 0; i < node->mNumMeshes; i++) {
            aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
            meshes.push_back(processMesh(mesh, scene));
        }
        // Repite para todos los hijos del nodo
        for (unsigned int i = 0; i < node->mNumChildren; i++) {
            processNode(node->mChildren[i], scene);
        }
    }

    // Convierte una 'aiMesh' de Assimp a nuestra clase 'Mesh'
    Mesh processMesh(aiMesh* mesh, const aiScene* scene) {
        std::vector<Vertex> vertices;
        std::vector<unsigned int> indices;
        std::vector<Texture> textures; // Lógica de texturas iría aquí

        // 1. Procesar Vértices
        for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
            Vertex vertex;
            // Posición
            vertex.Position = glm::vec3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);
            // Normales
            if (mesh->HasNormals()) {
                vertex.Normal = glm::vec3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);
            }
            // Coordenadas de Textura
            if (mesh->mTextureCoords[0]) {
                vertex.TexCoords = glm::vec2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y);
            } else {
                vertex.TexCoords = glm::vec2(0.0f, 0.0f);
            }
            vertices.push_back(vertex);
        }

        // 2. Procesar Índices
        for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
            aiFace face = mesh->mFaces[i];
            for (unsigned int j = 0; j < face.mNumIndices; j++) {
                indices.push_back(face.mIndices[j]);
            }
        }

        // 3. Procesar Materiales/Texturas (Esta parte es más avanzada)
        // ... (Aquí iría el código para cargar las texturas desde 'scene->mMaterials')

        // Crear y devolver la malla
        return Mesh(vertices, indices, textures);
    }
};