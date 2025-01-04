#version 330 core
out vec4 FragColor;

in vec3 FragPos;  
in vec3 Normal;  
in vec2 TexCoords; // From vertex shader

uniform vec3 viewPos;

// Material properties
uniform vec3 materialColor;
uniform float materialShininess;

// Light properties
uniform vec3 lightPos;
uniform vec3 lightAmbient;
uniform vec3 lightDiffuse;
uniform vec3 lightSpecular;

// Emission properties
uniform vec3 emissionColor; // Base emission color
uniform float time; // Time for animation

void main()
{
    // Ambient
    vec3 ambient = lightAmbient * materialColor;

    // Diffuse 
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = lightDiffuse * diff * materialColor;

    // Specular
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);  
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), materialShininess);
    vec3 specular = lightSpecular * spec; 

    // Emission with pulsing effect
    float pulse = 0.5 + 0.5 * sin(time * 10.0); // Pulses between 0.5 and 1.0
    vec3 emission = emissionColor * pulse;

    // Combine all components
    vec3 result = ambient + diffuse + specular + emission;
    FragColor = vec4(result, 1.0);
}
