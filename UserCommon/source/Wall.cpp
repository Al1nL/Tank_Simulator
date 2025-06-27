#include "../header/Wall.h"

/**
 * @brief Constructs a Wall object at a specified position.
 *
 * @param pos The (row, col) position of the wall on the map.
 */
Wall::Wall(pair<int, int> pos) : GameObject(pos) {}

/**
 * @brief Gets the number of times this wall has been hit.
 *
 * @return Number of registered hits.
 */
int Wall::getHitCount() const
{
    return hitCount;
}

/**
 * @brief Registers a hit to the wall, incrementing the hit counter.
 */
void Wall::registerHit()
{
    hitCount++;
}

/**
 * @brief Marks the wall as destroyed manually.
 */
void Wall::destroy()
{
    destroyed = true;
}

/**
 * @brief Checks if the wall has been destroyed based on its hit count.
 *
 * @return True if hit count has reached the maximum allowed; false otherwise.
 */
bool Wall::isDestroyed() const
{
    return hitCount >= WALL_MAX_HITS;
}

/**
 * @brief Returns the symbol representing the wall.
 * @return Character symbol for the wall - '#'.
 */
char Wall::getSymbol() const
{
    return WALL_SYMBOL;
}
