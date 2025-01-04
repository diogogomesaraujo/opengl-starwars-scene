#ifndef ENEMY_H
#define ENEMY_H

#include "Model.h"
#include <tuple>

class Enemy : public Model
{
public:
    // Constructor
    Enemy(const std::string &path, std::tuple<float, float, float> initialPos)
        : Model(const_cast<char*>(path.c_str())), initialPosition(initialPos)
    {
        position = initialPosition;
    }

    // Move horizontally by deltaX
    void moveHorizontally(float deltaX)
    {
        std::tuple<float, float, float> newPos = std::make_tuple(
            std::get<0>(position),
            std::get<1>(position),
            std::get<2>(position) + deltaX
        );
        modelMove(newPos);
    }

    // Move down by deltaY
    void moveDown(float deltaY)
    {
        std::tuple<float, float, float> newPos = std::make_tuple(
            std::get<0>(position) - deltaY,
            std::get<1>(position),
            std::get<2>(position)
        );
        modelMove(newPos);
    }

    // Get current x position (now returns z coordinate)
    float getX() const { return std::get<2>(position); }

    // Get current y position (now returns y coordinate)
    float getY() const { return std::get<0>(position); }

private:
    std::tuple<float, float, float> initialPosition;
};

#endif // ENEMY_H
