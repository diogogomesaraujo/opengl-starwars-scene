#ifndef ENEMY_H
#define ENEMY_H

#include "Model.h"
#include <tuple>

class Enemy : public Model
{
public:
    // Constructor
    Enemy(char* path, std::tuple<float, float, float> initialPos, float moveDistance, float moveSpeed)
        : Model(path), moveDistance(moveDistance), moveSpeed(moveSpeed), direction(1)
    {
        position = initialPos;
        initialPosition = initialPos;
        targetPosition = initialPos;
    }

    // Update enemy position
    void updatePosition()
    {
        if (hasReachedTarget())
        {
            // Change direction if at horizontal bounds
            if (std::abs(std::get<0>(position) - std::get<0>(initialPosition)) >= moveDistance)
            {
                direction = -direction;
                // Move down
                std::get<1>(targetPosition) -= moveDistance;
            }
            else
            {
                // Move horizontally
                std::get<0>(targetPosition) += direction * moveDistance;
            }
        }
        // Move towards target position
        modelMove(targetPosition);
    }

private:
    std::tuple<float, float, float> initialPosition;
    std::tuple<float, float, float> targetPosition;
    float moveDistance; // Distance to move in each step
    float moveSpeed;    // Speed of movement
    int direction;      // Current horizontal direction: 1 for right, -1 for left

    // Check if the enemy has reached its target position
    bool hasReachedTarget()
    {
        float threshold = 0.1f;
        return (std::abs(std::get<0>(position) - std::get<0>(targetPosition)) < threshold &&
                std::abs(std::get<1>(position) - std::get<1>(targetPosition)) < threshold);
    }
};

#endif // ENEMY_H
