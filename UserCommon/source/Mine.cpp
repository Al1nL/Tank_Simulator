#include "../header/Mine.h"

/**
 * @brief Constructs a Mine object at the given position.
 * @param pos Position coordinates of the mine.
 */
Mine::Mine(pair<int, int> pos) : GameObject(pos) {}

/**
 * @brief Marks the mine as triggered.
 */
void Mine::trigger()
{
    triggered = true;
}

/**
 * @brief Checks if the mine has been triggered.
 * @return True if triggered, false otherwise.
 */
bool Mine::isTriggered() const
{
    return triggered;
}

/**
 * @brief Marks the mine as destroyed.
 */
void Mine::destroy()
{
    destroyed = true;
}

/**
 * @brief Checks if the mine has been destroyed.
 * @return True if destroyed, false otherwise.
 */
bool Mine::isDestroyed() const
{
    return destroyed;
}

/**
 * @brief Returns the symbol representing the mine.
 * @return Character symbol for the mine - '@'.
 */
char Mine::getSymbol() const
{
    return MINE_SYMBOL;
}
