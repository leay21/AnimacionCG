#pragma once

#include <vector>
#include <string>
#include <glm/glm.hpp>
#include "Vertex.h" // Asegúrate de incluir tu struct Vertex

// Un struct simple para guardar la info de la textura
struct Texture {
    unsigned int id;
    std::string type; // ej. "textura_difusa" o "textura_especular"
};

class Mesh {
public:
    // Datos de la malla
    std::vector<Vertex>       vertices;
    std::vector<unsigned int> indices;
    std::vector<Texture>      textures;
    unsigned int VAO; // ID del Vertex Array Object

    // Constructor
    Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures) {
        this->vertices = vertices;
        this->indices = indices;
        this->textures = textures;

        // Ahora configuramos los búferes de OpenGL
        setupMesh();
    }

    // Función para dibujar la malla
    void Draw(/* Shader& shader */) { // Eventualmente pasarás tu shader aquí
        // ... (código para vincular texturas) ...

        // Dibujar malla
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, static_cast<unsigned int>(indices.size()), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }

private:
    // IDs de los búferes de OpenGL
    unsigned int VBO, EBO;

    // Función de configuración
    void setupMesh() {
        // 1. Crear búferes
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);

        // 2. Vincular VAO
        glBindVertexArray(VAO);

        // 3. Cargar datos en el VBO (Vertex Buffer Object)
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

        // 4. Cargar datos en el EBO (Element Buffer Object)
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

        // 5. Configurar los punteros de atributos de vértice (Vertex Attrib Pointers)
        // Le dice a OpenGL cómo leer el VBO
        
        // Posición
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
        
        // Normales
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));
        
        // Coordenadas de Textura
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));

        // Desvincular VAO
        glBindVertexArray(0);
    }
};