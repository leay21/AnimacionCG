#ifndef SPHERE_H
#define SPHERE_H

#include <vector>
#include <glad/glad.h>
#include <glm/glm.hpp>

class Sphere {
public:
    // Configuración de la esfera
    std::vector<float> vertices;
    std::vector<unsigned int> indices;
    unsigned int VAO;

    // Constructor: Radio, sectores (cortes verticales), stacks (cortes horizontales)
    Sphere(float radius = 1.0f, int sectorCount = 36, int stackCount = 18) {
        buildVerticesSmooth(radius, sectorCount, stackCount);
        setupSphere();
    }

    void Draw() {
        glBindVertexArray(VAO);
        // Dibujamos usando índices (Elements)
        glDrawElements(GL_TRIANGLES, (unsigned int)indices.size(), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }

private:
    unsigned int VBO, EBO;

    void buildVerticesSmooth(float radius, int sectorCount, int stackCount) {
        float x, y, z, xy;                              // Posición del vértice
        float nx, ny, nz, lengthInv = 1.0f / radius;    // Normales
        float s, t;                                     // Coordenadas de textura (UV)

        float sectorStep = 2 * 3.14159f / sectorCount;
        float stackStep = 3.14159f / stackCount;
        float sectorAngle, stackAngle;

        for (int i = 0; i <= stackCount; ++i) {
            stackAngle = 3.14159f / 2 - i * stackStep;  // de pi/2 a -pi/2
            xy = radius * cosf(stackAngle);             // r * cos(u)
            z = radius * sinf(stackAngle);              // r * sin(u)

            for (int j = 0; j <= sectorCount; ++j) {
                sectorAngle = j * sectorStep;           // de 0 a 2pi

                // Posición (x, y, z)
                x = xy * cosf(sectorAngle);             // r * cos(u) * cos(v)
                y = xy * sinf(sectorAngle);             // r * cos(u) * sin(v)
                
                
                // Agregamos vértice: Posición (X, Y, Z) - Intercambiamos Y/Z para que los polos queden verticales
                vertices.push_back(x);
                vertices.push_back(z); // Y matemático pasa a ser Z OpenGL
                vertices.push_back(y); // Z matemático pasa a ser Y OpenGL (Arriba)

                // Normales (nx, ny, nz)
                nx = x * lengthInv;
                ny = z * lengthInv;
                nz = y * lengthInv;
                vertices.push_back(nx);
                vertices.push_back(ny);
                vertices.push_back(nz);

                // Coordenadas de Textura (s, t)
                s = (float)j / sectorCount;
                t = (float)i / stackCount;
                vertices.push_back(s);
                vertices.push_back(t);
            }
        }

        // Generar índices para los triángulos
        int k1, k2;
        for (int i = 0; i < stackCount; ++i) {
            k1 = i * (sectorCount + 1);     // inicio del stack actual
            k2 = k1 + sectorCount + 1;      // inicio del siguiente stack

            for (int j = 0; j < sectorCount; ++j, ++k1, ++k2) {
                // 2 triángulos por sector
                if (i != 0) {
                    indices.push_back(k1);
                    indices.push_back(k2);
                    indices.push_back(k1 + 1);
                }
                if (i != (stackCount - 1)) {
                    indices.push_back(k1 + 1);
                    indices.push_back(k2);
                    indices.push_back(k2 + 1);
                }
            }
        }
    }

    void setupSphere() {
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);

        glBindVertexArray(VAO);

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), &vertices[0], GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

        // Stride: 3 (Pos) + 3 (Norm) + 2 (Tex) = 8 floats
        long stride = 8 * sizeof(float);

        // 1. Posición
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
        // 2. Normales
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
        // 3. Texturas
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void*)(6 * sizeof(float)));

        glBindVertexArray(0);
    }
};

#endif