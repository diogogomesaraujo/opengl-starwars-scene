// Projectile.h
#ifndef PROJECTILE_H
#define PROJECTILE_H

#include "header.h"

#include <glm/glm.hpp>
#include "Cylinder.h"

class Projectile
{
public:
    glm::vec3 position;
    glm::vec3 velocity;
    bool active;

    // Static members for cylinder geometry
    static unsigned int VAO;
    static unsigned int VBO, EBO;
    static unsigned int indexCount;

    Projectile(glm::vec3 startPos, glm::vec3 vel)
        : position(startPos), velocity(vel), active(true), lifetime(0.0f) {}

    // Initialize the cylinder geometry (call once)
    static void initializeCylinder()
    {
        std::vector<Vertex> vertices;
        std::vector<unsigned int> indices;
        generateCylinder(1.0f, 0.1f, 36, vertices, indices);
        indexCount = indices.size();

        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);

        glBindVertexArray(VAO);

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

        // Vertex Positions
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)0);

        // Vertex Normals
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, Normal));

        glBindVertexArray(0);
    }

    // Update projectile position
    void update(float deltaTime)
    {
        position += velocity * deltaTime;
        lifetime += deltaTime;
        // Deactivate if out of bounds or exceeds lifetime
        if (glm::length(position) > 100.0f || lifetime > maxLifetime)
        {
            active = false;
        }
    }

    // Render the projectile
    void Draw(Shader &shader) const
    {
        shader.use();

        glm::mat4 modelMatrix = glm::mat4(1.0f);
        modelMatrix = glm::translate(modelMatrix, position);

        // Rotate to align with velocity
        if (glm::length(velocity) > 0.0f)
        {
            glm::vec3 dir = glm::normalize(velocity);
            float angle = glm::acos(glm::dot(dir, glm::vec3(0.0f, 1.0f, 0.0f)));
            glm::vec3 axis = glm::cross(glm::vec3(0.0f, 1.0f, 0.0f), dir);
            if (glm::length(axis) > 0.001f)
                modelMatrix = glm::rotate(modelMatrix, angle, glm::normalize(axis));
        }

        modelMatrix = glm::scale(modelMatrix, glm::vec3(1.0f)); // Adjust size as needed

        shader.setMat4("model", modelMatrix);

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }

    glm::vec3 getBoundingBoxMin() const
    {
        // Based on cylinder dimensions (height=1.0f, radius=0.1f)
        return position - glm::vec3(0.1f, 0.5f, 0.1f);
    }

    glm::vec3 getBoundingBoxMax() const
    {
        // Based on cylinder dimensions (height=1.0f, radius=0.1f)
        return position + glm::vec3(0.1f, 0.5f, 0.1f);
    }

private:
    float lifetime;
    float maxLifetime = 2.5f; // seconds
};

// Initialize static members
unsigned int Projectile::VAO = 0;
unsigned int Projectile::VBO = 0;
unsigned int Projectile::EBO = 0;
unsigned int Projectile::indexCount = 0;

#endif // PROJECTILE_H
