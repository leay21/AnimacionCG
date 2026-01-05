#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Shader.h"
#include "Model.h"

#include <iostream>

// --- Configuraciones ---
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// --- Variables de la Cámara y Jugador ---
glm::vec3 cameraPos   = glm::vec3(0.0f, 2.0f, 6.0f);
glm::vec3 cameraUp    = glm::vec3(0.0f, 1.0f, 0.0f);

// Variables de Goku
glm::vec3 gokuPos = glm::vec3(0.0f, 0.0f, 0.0f); 
float gokuAngle = 0.0f;                          

// Variables de mouse
float lastX =  SCR_WIDTH / 2.0;
float lastY =  SCR_HEIGHT / 2.0;
float fov   =  45.0f;

// --- Temporizador ---
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// Declaración de funciones
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

int main()
{
    // 1. Inicialización
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Proyecto Final - Goku 3D", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Fallo al crear la ventana GLFW" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetScrollCallback(window, scroll_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Fallo al inicializar GLAD" << std::endl;
        return -1;
    }

    // Configuración Global OpenGL
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // 2. Shaders
    Shader ourShader("src/basic.vert", "src/basic.frag");
    Shader outlineShader("src/outline.vert", "src/outline.frag");

    // 3. Cargar Modelos (CAMBIO AQUÍ: Cargamos los dos estados)
    // Asegúrate de que los archivos existan en esa ruta
    std::cout << "Cargando modelo Idle..." << std::endl;
    Model idleModel("assets/goku/GokuIdle.fbx");

    std::cout << "Cargando modelo Run..." << std::endl;
    Model runModel("assets/goku/GokuRun.fbx");

    // --- Configuración del Suelo ---
    float planeVertices[] = {
         25.0f, -0.0f,  25.0f,  0.0f, 1.0f, 0.0f,  25.0f,  0.0f,
        -25.0f, -0.0f,  25.0f,  0.0f, 1.0f, 0.0f,   0.0f,  0.0f,
        -25.0f, -0.0f, -25.0f,  0.0f, 1.0f, 0.0f,   0.0f, 25.0f,

         25.0f, -0.0f,  25.0f,  0.0f, 1.0f, 0.0f,  25.0f,  0.0f,
        -25.0f, -0.0f, -25.0f,  0.0f, 1.0f, 0.0f,   0.0f, 25.0f,
         25.0f, -0.0f, -25.0f,  0.0f, 1.0f, 0.0f,  25.0f, 25.0f
    };
    unsigned int planeVAO, planeVBO;
    glGenVertexArrays(1, &planeVAO);
    glGenBuffers(1, &planeVBO);
    glBindVertexArray(planeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), planeVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glBindVertexArray(0);

    unsigned int floorTexture = TextureFromFile("grass.jpg", "assets/textures");

    // 4. Bucle Principal
    while (!glfwWindowShouldClose(window))
    {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window);

        // Color del cielo
        glClearColor(0.53f, 0.81f, 0.92f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // --- LÓGICA DE SELECCIÓN DE MODELO ---
        // Si W o S están presionados, usamos el modelo corriendo
        bool isMoving = glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS || 
                        glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS;
        
        Model* currentModel; // Puntero al modelo actual
        if (isMoving) {
            currentModel = &runModel;
        } else {
            currentModel = &idleModel;
        }

        // --- CÁMARA TERCERA PERSONA ---
        float distanceFromPlayer = 5.0f;
        float heightFromPlayer = 2.5f;

        glm::vec3 targetCameraPos;
        targetCameraPos.x = gokuPos.x - sin(glm::radians(gokuAngle)) * distanceFromPlayer;
        targetCameraPos.z = gokuPos.z - cos(glm::radians(gokuAngle)) * distanceFromPlayer;
        targetCameraPos.y = gokuPos.y + heightFromPlayer;

        cameraPos = glm::mix(cameraPos, targetCameraPos, 5.0f * deltaTime);

        glm::mat4 projection = glm::perspective(glm::radians(fov), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = glm::lookAt(cameraPos, gokuPos + glm::vec3(0.0f, 1.5f, 0.0f), cameraUp);

        // --- MATRIZ BASE DEL PERSONAJE ---
        glm::mat4 modelBase = glm::mat4(1.0f);
        
        // 1. Posición base
        modelBase = glm::translate(modelBase, gokuPos); 
        
        // 2. Rotación de dirección (A/D)
        modelBase = glm::rotate(modelBase, glm::radians(gokuAngle), glm::vec3(0.0f, 1.0f, 0.0f)); 

        // 3. Ajustes de orientación (Rotación -90 en X es estándar para Mixamo)
        modelBase = glm::rotate(modelBase, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f)); 
        modelBase = glm::translate(modelBase, glm::vec3(0.0f, -1.0f, 0.0f));

        // PASE 1: OUTLINE
        glCullFace(GL_FRONT); 
        outlineShader.use();
        outlineShader.setMat4("projection", projection);
        outlineShader.setMat4("view", view);
        glm::mat4 modelOutline = glm::scale(modelBase, glm::vec3(1.02f, 1.02f, 1.02f)); 
        outlineShader.setMat4("model", modelOutline);
        
        currentModel->Draw(outlineShader); // Dibujamos el modelo seleccionado

        // PASE 2: GOKU NORMAL
        glCullFace(GL_BACK); 
        ourShader.use();
        ourShader.setMat4("projection", projection);
        ourShader.setMat4("view", view);
        
        ourShader.setVec3("lightPos", gokuPos + glm::vec3(2.0f, 4.0f, 2.0f));
        ourShader.setVec3("viewPos", cameraPos);

        glm::mat4 modelNormal = glm::scale(modelBase, glm::vec3(1.0f, 1.0f, 1.0f));
        ourShader.setMat4("model", modelNormal);
        
        currentModel->Draw(ourShader); // Dibujamos el modelo seleccionado

        // SUELO
        glDisable(GL_CULL_FACE); 
        glm::mat4 modelPlane = glm::mat4(1.0f);
        modelPlane = glm::translate(modelPlane, glm::vec3(0.0f, 0.0f, 0.0f)); 
        
        ourShader.setMat4("model", modelPlane);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, floorTexture); 
        ourShader.setInt("texture_diffuse1", 0);

        glBindVertexArray(planeVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);
        
        glEnable(GL_CULL_FACE);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}

// --- Inputs: Control de Personaje ---
void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    float moveSpeed = 4.0f * deltaTime;
    float rotSpeed  = 90.0f * deltaTime;

    // W: Avanzar
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        gokuPos.x += sin(glm::radians(gokuAngle)) * moveSpeed;
        gokuPos.z += cos(glm::radians(gokuAngle)) * moveSpeed;
    }
    // S: Retroceder
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        gokuPos.x -= sin(glm::radians(gokuAngle)) * moveSpeed;
        gokuPos.z -= cos(glm::radians(gokuAngle)) * moveSpeed;
    }
    // A: Girar Izquierda
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        gokuAngle += rotSpeed;
    // D: Girar Derecha
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        gokuAngle -= rotSpeed;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    fov -= (float)yoffset;
    if (fov < 1.0f) fov = 1.0f;
    if (fov > 45.0f) fov = 45.0f;
}