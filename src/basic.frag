#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

uniform sampler2D texture_diffuse1;
uniform vec3 lightPos;   // Posición de la luz
uniform vec3 viewPos;    // Posición de la cámara (para brillo especular, opcional)

void main()
{
    vec3 color = texture(texture_diffuse1, TexCoords).rgb;
    vec3 norm = normalize(Normal);
    
    // --- CAMBIO CLAVE: LUZ TIPO SOL (DIRECTIONAL LIGHT) ---
    // En lugar de (lightPos - FragPos), usamos directamente lightPos como dirección
    // asumiendo que lightPos es un vector que apunta HACIA la luz (ej: arriba).
    vec3 lightDir = normalize(lightPos); 

    // Iluminación Difusa
    float diff = max(dot(norm, lightDir), 0.0);

    // --- CEL SHADING ---
    float levels = 3.0;
    float level = floor(diff * levels);
    float toonDiff = level / levels;

    // Luz Ambiental un poco más fuerte para exteriores (0.6)
    vec3 ambient = 0.6 * color; 
    vec3 diffuse = color * toonDiff; 
    
    vec3 result = ambient + diffuse;
    FragColor = vec4(result, 1.0);
}