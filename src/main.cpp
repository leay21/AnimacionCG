#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Shader.h"
#include "Model.h"
#include "Sphere.h"

#include <iostream>

// --- Configuraciones ---
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// Variables de Cámara y Jugador
glm::vec3 cameraPos   = glm::vec3(0.0f, 2.0f, 6.0f);
glm::vec3 cameraUp    = glm::vec3(0.0f, 1.0f, 0.0f);

glm::vec3 gokuPos = glm::vec3(0.0f, 0.0f, 0.0f); 
float gokuAngle = 0.0f;       // Hacia donde mira Goku (controlado por A/D)
float cameraAngleAround = 0.0f; // Hacia donde mira la cámara (controlado por Mouse)

// Variables del Ataque
bool isAttacking = false;
float attackTime = 0.0f; 
glm::vec3 spherePos;     

// Variables de mouse
bool firstMouse = true;
float lastX = SCR_WIDTH / 2.0;

// Temporizador
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// Funciones
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

// Bezier
glm::vec3 calculateBezier(float t, glm::vec3 p0, glm::vec3 p1, glm::vec3 p2, glm::vec3 p3) {
    float u = 1.0f - t;
    float tt = t * t;
    float uu = u * u;
    float uuu = uu * u;
    float ttt = tt * t;
    glm::vec3 p = uuu * p0; 
    p += 3 * uu * t * p1;   
    p += 3 * u * tt * p2;   
    p += ttt * p3;          
    return p;
}

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Proyecto Final - Goku 3D Ultimate", NULL, NULL);
    if (window == NULL) { glfwTerminate(); return -1; }
    
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback); // HABILITAMOS EL MOUSE
    glfwSetScrollCallback(window, scroll_callback);
    
    // Capturamos el mouse (desaparece el cursor) para mover la cámara cómodo
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) { return -1; }

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Shaders
    Shader ourShader("src/basic.vert", "src/basic.frag");
    Shader outlineShader("src/outline.vert", "src/outline.frag");

    // Modelos
    Model idleModel("assets/goku/GokuIdle.fbx");
    Model runModel("assets/goku/GokuRun.fbx");
    
    // Esferas (Energía y Cielo)
    Sphere energyBall(0.3f, 24, 24);
    Sphere skyDome(50.0f, 32, 32); // Esfera gigante (Radio 50) para el cielo

    // Suelo
    float planeVertices[] = {
         50.0f, -0.0f,  50.0f,  0.0f, 1.0f, 0.0f,  50.0f,  0.0f,
        -50.0f, -0.0f,  50.0f,  0.0f, 1.0f, 0.0f,   0.0f,  0.0f,
        -50.0f, -0.0f, -50.0f,  0.0f, 1.0f, 0.0f,   0.0f, 50.0f,

         50.0f, -0.0f,  50.0f,  0.0f, 1.0f, 0.0f,  50.0f,  0.0f,
        -50.0f, -0.0f, -50.0f,  0.0f, 1.0f, 0.0f,   0.0f, 50.0f,
         50.0f, -0.0f, -50.0f,  0.0f, 1.0f, 0.0f,  50.0f, 50.0f
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

    // Texturas
    unsigned int floorTexture = TextureFromFile("grass.jpg", "assets/textures");
    unsigned int poderTexture = TextureFromFile("rayo.jpg", "assets/textures");
    // IMPORTANTE: Asegúrate de tener sky.jpg, si no, usa grass.jpg para probar
    unsigned int skyTexture   = TextureFromFile("sky.jpg", "assets/textures"); 

    while (!glfwWindowShouldClose(window))
    {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window);

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // --- CÁMARA ORBITAL ---
        // La cámara ahora depende del mouse (cameraAngleAround) en lugar de Goku
        float distanceFromPlayer = 7.0f;
        float heightFromPlayer = 3.0f;

        // Calculamos posición de la cámara rotando alrededor de Goku
        glm::vec3 targetCameraPos;
        targetCameraPos.x = gokuPos.x + sin(glm::radians(cameraAngleAround)) * distanceFromPlayer;
        targetCameraPos.z = gokuPos.z + cos(glm::radians(cameraAngleAround)) * distanceFromPlayer;
        targetCameraPos.y = gokuPos.y + heightFromPlayer;

        // Suavizado
        cameraPos = glm::mix(cameraPos, targetCameraPos, 10.0f * deltaTime);

        glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = glm::lookAt(cameraPos, gokuPos + glm::vec3(0.0f, 1.5f, 0.0f), cameraUp);

        // --- RENDERIZADO DEL CIELO (SKYBOX/DOME) ---
        
        // CAMBIO: En lugar de glCullFace(GL_FRONT), usamos Disable.
        // Esto obliga a dibujar la esfera por ambos lados.
        glDisable(GL_CULL_FACE); 

        ourShader.use();
        ourShader.setMat4("projection", projection);
        ourShader.setMat4("view", view);
        
        glm::mat4 modelSky = glm::mat4(1.0f);
        modelSky = glm::translate(modelSky, gokuPos); 
        ourShader.setMat4("model", modelSky);
        
        // Poner la luz muy alta
        ourShader.setVec3("lightPos", glm::vec3(0.0f, 200.0f, 0.0f)); 

        glActiveTexture(GL_TEXTURE0);
        // Asegúrate que skyTexture se cargó bien (si no, prueba con floorTexture aquí para testear)
        glBindTexture(GL_TEXTURE_2D, skyTexture);
        ourShader.setInt("texture_diffuse1", 0);
        
        skyDome.Draw();

        // CAMBIO: Volver a activar el Culling normal para Goku y el resto
        glEnable(GL_CULL_FACE); 
        glCullFace(GL_BACK); // Aseguramos que vuelva al estándar

        // --- RENDERIZADO DE GOKU ---
        bool isMoving = glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS || 
                        glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS;
        Model* currentModel = isMoving ? &runModel : &idleModel;

        // Matriz de Goku
        glm::mat4 modelBase = glm::mat4(1.0f);
        modelBase = glm::translate(modelBase, gokuPos); 
        modelBase = glm::rotate(modelBase, glm::radians(gokuAngle), glm::vec3(0.0f, 1.0f, 0.0f)); 
        modelBase = glm::rotate(modelBase, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f)); 
        modelBase = glm::translate(modelBase, glm::vec3(0.0f, -1.0f, 0.0f));

        // Outline
        glCullFace(GL_FRONT); 
        outlineShader.use();
        outlineShader.setMat4("projection", projection);
        outlineShader.setMat4("view", view);
        glm::mat4 modelOutline = glm::scale(modelBase, glm::vec3(1.02f, 1.02f, 1.02f)); 
        outlineShader.setMat4("model", modelOutline);
        currentModel->Draw(outlineShader);

        // Normal
        glCullFace(GL_BACK); 
        ourShader.use();
        ourShader.setMat4("projection", projection);
        ourShader.setMat4("view", view);
        // Luz tipo SOL (Dirección fija desde arriba a la derecha)
        ourShader.setVec3("lightPos", glm::vec3(50.0f, 100.0f, 50.0f)); 
        ourShader.setVec3("viewPos", cameraPos);
        
        glm::mat4 modelNormal = glm::scale(modelBase, glm::vec3(1.0f, 1.0f, 1.0f));
        ourShader.setMat4("model", modelNormal);
        currentModel->Draw(ourShader);

        // --- ATAQUE ---
        if (isAttacking) {
            attackTime += deltaTime * 1.5f;
            glm::vec3 p0 = gokuPos + glm::vec3(0.0f, 1.5f, 0.0f);
            float dist = 10.0f;
            glm::vec3 p3;
            // Dispara hacia donde mira GOKU, no la cámara
            p3.x = gokuPos.x + sin(glm::radians(gokuAngle)) * dist;
            p3.z = gokuPos.z + cos(glm::radians(gokuAngle)) * dist;
            p3.y = gokuPos.y + 0.5f;
            glm::vec3 p1 = p0 + glm::vec3(0.0f, 3.0f, 0.0f);
            glm::vec3 p2 = p3 + glm::vec3(0.0f, 2.0f, 0.0f);

            if (attackTime <= 1.0f) {
                spherePos = calculateBezier(attackTime, p0, p1, p2, p3);
                glm::mat4 modelBall = glm::mat4(1.0f);
                modelBall = glm::translate(modelBall, spherePos);
                modelBall = glm::scale(modelBall, glm::vec3(0.5f, 0.5f, 0.5f)); 
                ourShader.setMat4("model", modelBall);
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, poderTexture); 
                energyBall.Draw();
            } else {
                isAttacking = false;
            }
        }

        // --- SUELO ---
        glDisable(GL_CULL_FACE); 
        glm::mat4 modelPlane = glm::mat4(1.0f);
        modelPlane = glm::translate(modelPlane, glm::vec3(0.0f, -0.01f, 0.0f)); 
        ourShader.setMat4("model", modelPlane);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, floorTexture); 
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

// Control del Mouse para Rotar Cámara
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    float xpos = static_cast<float>(xposIn);

    if (firstMouse) {
        lastX = xpos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    lastX = xpos;

    // Sensibilidad del mouse
    float sensitivity = 0.5f; 
    cameraAngleAround -= xoffset * sensitivity;
}

void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    float moveSpeed = 4.0f * deltaTime;
    float rotSpeed  = 90.0f * deltaTime;

    // Movimiento: Se mueve relativo a GOKU, no a la cámara (Estilo Resident Evil clásico)
    // Si quisieras que se mueva relativo a la cámara, tendrías que usar cameraAngleAround aquí.
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        gokuPos.x += sin(glm::radians(gokuAngle)) * moveSpeed;
        gokuPos.z += cos(glm::radians(gokuAngle)) * moveSpeed;
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        gokuPos.x -= sin(glm::radians(gokuAngle)) * moveSpeed;
        gokuPos.z -= cos(glm::radians(gokuAngle)) * moveSpeed;
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        gokuAngle += rotSpeed;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        gokuAngle -= rotSpeed;

    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && !isAttacking) {
        isAttacking = true;
        attackTime = 0.0f;
    }
}
void framebuffer_size_callback(GLFWwindow* window, int width, int height) { glViewport(0, 0, width, height); }
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {}