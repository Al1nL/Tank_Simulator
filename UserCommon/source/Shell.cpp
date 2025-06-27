#include "../header/Shell.h"

/**
 * @brief Constructs a Shell object.
 *
 * @param pos The initial position of the shell (x, y).
 * @param dir The direction in which the shell is moving.
 * @param owner The ID of the tank/player who fired the shell.
 */
Shell::Shell(pair<int, int> pos, Direction dir, int owner)
    : GameObject(pos), direction(dir), ownerId(owner), destroyed(false) {}

/**
 * @brief Gets the current direction of the shell.
 * @return The direction the shell is moving.
 */
Direction Shell::getDirection() const
{
    return direction;
}

/**
 * @brief Sets the direction of the shell.
 * @param dir The new direction to set.
 */
void Shell::setDirection(Direction dir)
{
    direction = dir;
}

/**
 * @brief Gets the owner ID of the shell.
 * @return int The ID of the tank/player who owns the shell.
 */
int Shell::getOwnerId() const
{
    return ownerId;
}

/**
 * @brief Marks the shell as destroyed.
 */
void Shell::destroy()
{
    destroyed = true;
}

/**
 * @brief Checks if the shell has been destroyed.
 * @return true if destroyed, false otherwise.
 */
bool Shell::isDestroyed() const
{
    return destroyed;
}

/**
 * @brief Returns the symbol representing the shell.
 * @return Character symbol for the shell - '*'.
 */
char Shell::getSymbol() const
{
    return SHELL_SYMBOL; // Using
}