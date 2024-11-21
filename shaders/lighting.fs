#version 330 core
out vec4 FragColor;

in VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
} fs_in;

uniform sampler2D floorTexture;
uniform vec3 lightPos;
uniform vec3 viewPos;
uniform bool blinn;

// Define the ambient light color and intensity
uniform vec3 ambientLightColor; // e.g., vec3(1.0, 1.0, 1.0) for white light
uniform float ambientIntensity; // e.g., 0.5 for moderate intensity

void main()
{           
    vec3 color = texture(floorTexture, fs_in.TexCoords).rgb;

    // Ambient lighting
    vec3 ambient = ambientLightColor * ambientIntensity * color;

    // Diffuse lighting
    vec3 lightDir = normalize(lightPos - fs_in.FragPos);
    vec3 normal = normalize(fs_in.Normal);
    float diff = max(dot(lightDir, normal), 0.0);
    vec3 diffuse = diff * color;

    // Specular lighting
    vec3 viewDir = normalize(viewPos - fs_in.FragPos);
    float spec = 0.0;
    if(blinn)
    {
        vec3 halfwayDir = normalize(lightDir + viewDir);  
        spec = pow(max(dot(normal, halfwayDir), 0.0), 32.0);
    }
    else
    {
        vec3 reflectDir = reflect(-lightDir, normal);
        spec = pow(max(dot(viewDir, reflectDir), 0.0), 8.0);
    }
    vec3 specular = vec3(0.3) * spec; // Specular highlight intensity

    // Combine lighting components
    vec3 lighting = ambient + diffuse + specular;

    // Output final color
    FragColor = vec4(lighting, 1.0);
}
