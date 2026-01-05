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

// --- Variables de la Cámara ---
glm::vec3 cameraPos   = glm::vec3(0.0f, 0.0f, 3.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp    = glm::vec3(0.0f, 1.0f, 0.0f);

bool firstMouse = true;
float yaw   = -90.0f;
float pitch =  0.0f;
float lastX =  800.0f / 2.0;
float lastY =  600.0 / 2.0;
float fov   =  45.0f;

// --- Temporizador (para movimiento suave) ---
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// Declaración de funciones callback
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);

int main()
{
    // 1. Inicialización de GLFW y OpenGL
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
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // Capturar el mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Fallo al inicializar GLAD" << std::endl;
        return -1;
    }

    // Configuración global de OpenGL
    glEnable(GL_DEPTH_TEST); // Z-Buffer (Ya lo tenías)
    glEnable(GL_CULL_FACE);  // Activar la eliminación de caras ocultas
    glEnable(GL_BLEND);      // (Opcional) Para transparencia si la necesitaras
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // 2. Cargar Shaders
    // Asegúrate de que las rutas sean correctas respecto a tu carpeta de proyecto
    Shader ourShader("src/basic.vert", "src/basic.frag");
    Shader outlineShader("src/outline.vert", "src/outline.frag"); // <--- NUEVO

    // 3. Cargar Modelo
    // IMPORTANTE: Ruta al archivo FBX. Si tu carpeta se llama 'assets', esto debe coincidir.
    // Usamos '/' en lugar de '\' para evitar problemas.
    std::cout << "Cargando modelo... (esto puede tardar unos segundos)" << std::endl;
    Model ourModel("assets/goku/GokuFinal.fbx");

    // --- Configuración del Suelo (Plane) ---
    float planeVertices[] = {
        // posiciones          // normales         // texturas
         25.0f, -0.0f,  25.0f,  0.0f, 1.0f, 0.0f,  25.0f,  0.0f,
        -25.0f, -0.0f,  25.0f,  0.0f, 1.0f, 0.0f,   0.0f,  0.0f,
        -25.0f, -0.0f, -25.0f,  0.0f, 1.0f, 0.0f,   0.0f, 25.0f,

         25.0f, -0.0f,  25.0f,  0.0f, 1.0f, 0.0f,  25.0f,  0.0f,
        -25.0f, -0.0f, -25.0f,  0.0f, 1.0f, 0.0f,   0.0f, 25.0f,
         25.0f, -0.0f, -25.0f,  0.0f, 1.0f, 0.0f,  25.0f, 25.0f
    };
    
    // VAO y VBO del suelo
    unsigned int planeVAO, planeVBO;
    glGenVertexArrays(1, &planeVAO);
    glGenBuffers(1, &planeVBO);
    
    glBindVertexArray(planeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), planeVertices, GL_STATIC_DRAW);
    
    // Atributos (coinciden con layout location en basic.vert)
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0); // Posición
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float))); // Normal
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float))); // Textura
    glBindVertexArray(0);

    // Cargar textura del suelo (Necesitas una imagen, ej: "grass.jpg" o "tile.jpg")
    // Usamos la función auxiliar que ya tienes en Model.h, pero como es estática/global, 
    // quizás necesites copiarla al main o hacerla accesible. 
    // Por ahora, asumamos que tienes una textura o usamos una de Goku temporalmente.
    unsigned int floorTexture = TextureFromFile("grass.jpg", "assets/textures");

    // 4. Bucle de Renderizado
    while (!glfwWindowShouldClose(window))
    {
        // --- 1. Lógica de Tiempo y Entrada ---
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window);

        // --- 2. Limpiar Pantalla ---
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // --- 3. Calcular Matrices Comunes (View & Projection) ---
        // Se calculan una sola vez por frame porque son iguales para el contorno y el modelo
        glm::mat4 projection = glm::perspective(glm::radians(fov), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);

        // Matriz Base del Modelo (Posición, Rotación original)
        glm::mat4 modelBase = glm::mat4(1.0f);
        modelBase = glm::translate(modelBase, glm::vec3(0.0f, -1.0f, 0.0f)); 
        modelBase = glm::rotate(modelBase, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        // NOTA: No aplicamos la escala aquí todavía, lo haremos en cada pase

        // ==========================================================
        // PASE 1: DIBUJAR EL CONTORNO NEGRO (El "Casco")
        // ==========================================================
        
        // A. Cull Front: Le decimos a OpenGL que oculte las caras delanteras 
        // y muestre las traseras (el interior del modelo).
        glCullFace(GL_FRONT); 

        // B. Usar Shader de Contorno
        outlineShader.use();
        outlineShader.setMat4("projection", projection);
        outlineShader.setMat4("view", view);

        // C. Escalar el modelo (hacerlo "gordo")
        // Multiplicamos por 1.03f (3% más grande). Si el borde es muy grueso, baja a 1.02f
        glm::mat4 modelOutline = glm::scale(modelBase, glm::vec3(1.01f, 1.01f, 1.01f)); 
        outlineShader.setMat4("model", modelOutline);

        // D. Dibujar
        ourModel.Draw(outlineShader);


        // ==========================================================
        // PASE 2: DIBUJAR EL MODELO NORMAL (Cel Shading)
        // ==========================================================
        
        // A. Cull Back: Volvemos a la normalidad (ocultar caras traseras)
        glCullFace(GL_BACK); 

        // B. Usar Shader Principal
        ourShader.use();
        ourShader.setMat4("projection", projection);
        ourShader.setMat4("view", view);
        
        // C. Escalar el modelo (tamaño normal 1.0)
        glm::mat4 modelNormal = glm::scale(modelBase, glm::vec3(1.0f, 1.0f, 1.0f));
        ourShader.setMat4("model", modelNormal);

        // D. Configuración de Luz (necesaria para el Cel Shading)
        ourShader.setVec3("lightPos", glm::vec3(2.0f, 4.0f, 3.0f));
        ourShader.setVec3("viewPos", cameraPos);

        // E. Dibujar
        ourModel.Draw(ourShader);

        // ==========================================================
        // --- DIBUJAR SUELO ---
        
        // 1. Desactivamos el Face Culling temporalmente para que el suelo se vea siempre
        glDisable(GL_CULL_FACE); 

        glm::mat4 modelPlane = glm::mat4(1.0f);
        modelPlane = glm::translate(modelPlane, glm::vec3(0.0f, -1.01f, 0.0f)); 
        
        ourShader.setMat4("model", modelPlane);
        
        // Activar textura
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, floorTexture); 
        ourShader.setInt("texture_diffuse1", 0);

        glBindVertexArray(planeVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);

        // 2. Reactivamos el Culling para el siguiente frame (importante para el outline)
        glEnable(GL_CULL_FACE);

        // Swap buffers y eventos
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}

// --- Funciones de control (Teclado y Mouse) ---

void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    float velocity = 2.5f * deltaTime;
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        cameraPos += velocity * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        cameraPos -= velocity * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * velocity;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * velocity;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // Invertido porque coordenadas Y van de abajo a arriba en OpenGL (a veces) pero glfw da top-bottom
    lastX = xpos;
    lastY = ypos;

    float sensitivity = 0.1f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw += xoffset;
    pitch += yoffset;

    if (pitch > 89.0f)
        pitch = 89.0f;
    if (pitch < -89.0f)
        pitch = -89.0f;

    glm::vec3 front;
    front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    front.y = sin(glm::radians(pitch));
    front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraFront = glm::normalize(front);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    fov -= (float)yoffset;
    if (fov < 1.0f)
        fov = 1.0f;
    if (fov > 45.0f)
        fov = 45.0f;
}