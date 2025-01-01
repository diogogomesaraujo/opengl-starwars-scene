#pragma once

#include "Model.h"   // Include your base Model class
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

/**
 * Enemy class that inherits from Model
 */
class Enemy : public Model
{
public:
    // Constructor: pass the model path to the base Model constructor
    Enemy(const char* path, 
          float initialHealth    = 1.0f, 
          int   initialScore     = 100,
          float boundingRadius   = 2.0f)
        : Model(path),
          health(initialHealth),
          maxHealth(initialHealth),
          scoreValue(initialScore),
          collisionRadius(boundingRadius),
          alive(true)
    {
        // Initialize velocity to zero by default
        velocity = std::make_tuple(0.0f, 0.0f, 0.0f);
    }

    /**
     * Update the enemy each frame:
     *   - Move it based on velocity
     *   - Possibly check for out-of-bounds or other conditions
     *   - You could add animations, etc.
     */
    void Update(float deltaTime)
    {
        if (!alive) return;

        // Current position
        float x = std::get<0>(position);
        float y = std::get<1>(position);
        float z = std::get<2>(position);

        // Velocity
        float vx = std::get<0>(velocity);
        float vy = std::get<1>(velocity);
        float vz = std::get<2>(velocity);

        // Update position using basic Euler integration
        x += vx * deltaTime;
        y += vy * deltaTime;
        z += vz * deltaTime;

        // Store new position
        position = std::make_tuple(x, y, z);
    }

    /**
     * Draw the enemy using the base Model's Draw method
     * (Optionally apply transformations or set uniforms before drawing)
     */
    void Draw(Shader &shader)
    {
        if (!alive) return;
        // You could apply a translation based on 'position' here
        // e.g., build a model matrix with glm and pass it as a uniform
        // For simplicity, we'll just call the base Draw:
        Model::Draw(shader);
    }

    /**
     * Checks collision with another object (e.g., player bullet) using
     * bounding-sphere collision.
     *
     * \param otherPos  - The (x, y, z) position of the other object
     * \param otherRadius - The collision radius of the other object
     */
    bool checkCollision(const std::tuple<float, float, float>& otherPos, float otherRadius)
    {
        if (!alive) return false;

        // Distance between centers
        float dx = std::get<0>(position) - std::get<0>(otherPos);
        float dy = std::get<1>(position) - std::get<1>(otherPos);
        float dz = std::get<2>(position) - std::get<2>(otherPos);

        float distSq = dx*dx + dy*dy + dz*dz;
        float combinedRadius = collisionRadius + otherRadius;
        float combinedSq = combinedRadius * combinedRadius;

        return (distSq <= combinedSq);
    }

    /**
     * Apply damage to the enemy. If health <= 0, mark as dead.
     */
    void takeDamage(float amount)
    {
        if (!alive) return;
        health -= amount;
        if (health <= 0.0f)
        {
            health = 0.0f;
            alive = false;
        }
    }

    /**
     * Get whether the enemy is alive
     */
    bool isAlive() const
    {
        return alive;
    }

    /**
     * Set the velocity for this enemy
     */
    void setVelocity(float vx, float vy, float vz)
    {
        velocity = std::make_tuple(vx, vy, vz);
    }

    /**
     * Get the velocity
     */
    std::tuple<float, float, float> getVelocity() const
    {
        return velocity;
    }

    /**
     * Optional: Move toward a target using your Model's custom lerp-based approach.
     *   - You can override or adapt the modelMove() if you need a special speed.
     */
    void moveToward(const std::tuple<float, float, float>& finalPos, float step)
    {
        if (!alive) return;
        position = std::make_tuple(
            lerp(std::get<0>(position), std::get<0>(finalPos), step),
            lerp(std::get<1>(position), std::get<1>(finalPos), step),
            lerp(std::get<2>(position), std::get<2>(finalPos), step)
        );
    }

    /**
     * Get the current score value for this enemy
     */
    int getScoreValue() const
    {
        return scoreValue;
    }

    /**
     * Reset the enemy (e.g., if you want to reuse it in a pool)
     */
    void reset(const std::tuple<float, float, float>& startPos,
               float newHealth,
               int newScoreValue)
    {
        position   = startPos;
        health     = newHealth;
        maxHealth  = newHealth;
        scoreValue = newScoreValue;
        alive      = true;
    }

private:
    float health;
    float maxHealth;
    int   scoreValue;        // Points awarded to the player for destroying this enemy
    float collisionRadius;   // For simple bounding-sphere collision
    bool  alive;

    // Velocity for movement
    std::tuple<float, float, float> velocity;
};
