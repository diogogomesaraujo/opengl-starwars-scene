#ifndef CYLINDER_H
#define CYLINDER_H

#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glad/glad.h>
#include "header.h"

// Function to generate a unit cylinder aligned along the Y-axis
inline void generateCylinder(float height, float radius, int segments, std::vector<Vertex>& vertices, std::vector<unsigned int>& indices) {
    vertices.clear();
    indices.clear();

    float halfHeight = height / 2.0f;
    float deltaAngle = 2.0f * glm::pi<float>() / segments;

    // Generate side vertices
    for (int i = 0; i <= segments; ++i) {
        float angle = i * deltaAngle;
        float x = radius * std::cos(angle);
        float z = radius * std::sin(angle);

        // Bottom vertex
        vertices.push_back(Vertex{ glm::vec3(x, -halfHeight, z), glm::normalize(glm::vec3(x, 0.0f, z)) });

        // Top vertex
        vertices.push_back(Vertex{ glm::vec3(x, halfHeight, z), glm::normalize(glm::vec3(x, 0.0f, z)) });
    }

    // Generate side indices
    for (int i = 0; i < segments; ++i) {
        int start = i * 2;
        indices.push_back(start);
        indices.push_back(start + 1);
        indices.push_back(start + 2);

        indices.push_back(start + 1);
        indices.push_back(start + 3);
        indices.push_back(start + 2);
    }

    // Optional: Generate top and bottom caps
    // Top center
    Vertex topCenter{ glm::vec3(0.0f, halfHeight, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f) };
    vertices.push_back(topCenter);
    unsigned int topCenterIndex = vertices.size() - 1;

    // Bottom center
    Vertex bottomCenter{ glm::vec3(0.0f, -halfHeight, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f) };
    vertices.push_back(bottomCenter);
    unsigned int bottomCenterIndex = vertices.size() - 1;

    // Top and bottom cap indices
    for (int i = 0; i < segments; ++i) {
        int start = i * 2;

        // Top cap
        indices.push_back(topCenterIndex);
        indices.push_back(start + 1);
        indices.push_back(((i + 1) % segments) * 2 + 1);

        // Bottom cap
        indices.push_back(bottomCenterIndex);
        indices.push_back(((i + 1) % segments) * 2);
        indices.push_back(start);
    }
}

#endif // CYLINDER_H
