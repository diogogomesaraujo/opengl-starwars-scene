#version 330 core
out vec4 FragColor; // Final fragment color

// Input data from the vertex shader
in VS_OUT {
    vec3 FragPos;   // Fragment position in world space
    vec3 Normal;    // Normal vector
    vec2 TexCoords; // Texture coordinates
} fs_in;

// Uniforms for textures, lighting, and camera
uniform sampler2D floorTexture; // Texture sampler
uniform vec3 lightPos;          // Light position in world space
uniform vec3 viewPos;           // Camera position in world space
uniform bool blinn;             // Use Blinn-Phong lighting

void main()
{
    // Sample the texture color
    vec3 color = texture(floorTexture, fs_in.TexCoords).rgb;

    // Ambient lighting
    vec3 ambient = 0.05 * color;

    // Diffuse lighting
    vec3 lightDir = normalize(lightPos - fs_in.FragPos);
    vec3 normal = normalize(fs_in.Normal);
    float diff = max(dot(lightDir, normal), 0.0);
    vec3 diffuse = diff * color;

    // Specular lighting
    vec3 viewDir = normalize(viewPos - fs_in.FragPos);
    float spec = 0.0;
    if (blinn)
    {
        // Blinn-Phong specular
        vec3 halfwayDir = normalize(lightDir + viewDir);
        spec = pow(max(dot(normal, halfwayDir), 0.0), 32.0);
    }
    else
    {
        // Phong specular
        vec3 reflectDir = reflect(-lightDir, normal);
        spec = pow(max(dot(viewDir, reflectDir), 0.0), 8.0);
    }
    vec3 specular = vec3(0.3) * spec; // Assuming a white specular light

    // Combine results
    FragColor = vec4(ambient + diffuse + specular, 1.0);
}
