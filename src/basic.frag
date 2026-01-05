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
    // 1. Configuración básica
    vec3 color = texture(texture_diffuse1, TexCoords).rgb;
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);

    // 2. Iluminación Difusa (Lambert)
    float diff = max(dot(norm, lightDir), 0.0);

    // --- CEL SHADING (TOON) ---
    // En lugar de usar 'diff' tal cual, lo "escalonamos".
    // Esto crea 3 niveles de luz: Sombra, Tono medio, Luz plena.
    float levels = 3.0;
    float level = floor(diff * levels);
    float toonDiff = level / levels;
    // --------------------------

    // 3. Luz Ambiental (para que la sombra no sea negra pura)
    vec3 ambient = 0.4 * color; 
    
    // 4. Resultado final
    // Multiplicamos el color de la textura por la intensidad toon
    vec3 diffuse = color * toonDiff; 
    
    vec3 result = ambient + diffuse;
    
    FragColor = vec4(result, 1.0);
}