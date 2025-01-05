#version 330 core

layout (location = 0) in vec4 vertex;
// vertex.xy   = posição do vértice (no espaço de tela ou NDC)
// vertex.zw   = coordenadas de textura (UV)

out vec2 TexCoords;

uniform mat4 projection;

void main()
{
    // Projeta e envia ao pipeline
    gl_Position = projection * vec4(vertex.xy, 0.0, 1.0);
    // Passa as coordenadas de textura ao fragment shader
    TexCoords = vertex.zw;
}
