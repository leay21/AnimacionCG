#pragma once

#include <glm/glm.hpp>

struct Vertex {
    glm::vec3 Position;  // Posición (x, y, z)
    glm::vec3 Normal;    // Vector Normal (para la iluminación)
    glm::vec2 TexCoords; // Coordenadas de Textura (para "pegar" las imágenes)
};