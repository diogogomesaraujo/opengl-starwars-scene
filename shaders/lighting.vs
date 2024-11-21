#version 330 core
layout (location = 0) in vec3 aPos;       // Vertex position
layout (location = 1) in vec3 aNormal;    // Vertex normal
layout (location = 2) in vec2 aTexCoords; // Texture coordinates

// Interface block to pass data to the fragment shader
out VS_OUT {
    vec3 FragPos;   // Fragment position in world space
    vec3 Normal;    // Normal vector
    vec2 TexCoords; // Texture coordinates
} vs_out;

uniform mat4 projection; // Projection matrix
uniform mat4 view;       // View matrix
uniform mat4 model;      // Model matrix

void main()
{
    // Transform vertex position to world space
    vec4 fragPosWorld = model * vec4(aPos, 1.0);
    vs_out.FragPos = vec3(fragPosWorld);

    // Transform normal to world space and normalize
    vs_out.Normal = mat3(transpose(inverse(model))) * aNormal;

    // Pass texture coordinates unchanged
    vs_out.TexCoords = aTexCoords;

    // Calculate final vertex position in clip space
    gl_Position = projection * view * fragPosWorld;
}
