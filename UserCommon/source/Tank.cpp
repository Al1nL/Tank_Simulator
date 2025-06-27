#include "../header/Tank.h"

/**
 * @brief Constructs a Tank object.
 *
 * @param pos Initial position (x, y) of the tank.
 * @param tank_index Index of the tank
 * @param dir Initial direction of the tank.
 * @param owner The player ID that owns this tank.
 * @param num_of_shells The number of shells the tank has at the beginning.
 */
Tank::Tank(pair<int, int> pos, int tank_index, Direction dir, int owner, int num_of_shells)
    : GameObject(pos), tank_index(tank_index), direction(dir), ownerId(owner), destroyed(false), remaining_shells(num_of_shells) {}

/**
 * @brief Returns the current direction the tank is facing.
 *
 * @return Direction The direction enum value.
 */
Direction Tank::getDirection() const
{
    return direction;
}

/**
 * @brief Sets a new direction for the tank.
 *
 * @param dir The new direction.
 */
void Tank::setDirection(Direction dir)
{
    direction = dir;
}

/**
 * @brief Returns the ID of the tank's owner.
 *
 * @return int The owner's ID.
 */
int Tank::getOwnerId() const
{
    return ownerId;
}

/**
 * @brief Rotates the tank based on the given action.
 *
 * This method only responds to rotation actions. Other actions are ignored.
 *
 * @param action The ActionRequest enum (e.g., RotateLeft45, RotateRight90, etc.).
 */
void Tank::rotate(ActionRequest action)
{

    switch (action)
    {
    case ActionRequest::RotateLeft45:
        direction = static_cast<Direction>((direction + 7) % 8); // move 1 counter-clockwise
        break;
    case ActionRequest::RotateRight45:
        direction = static_cast<Direction>((direction + 1) % 8); // move 1 clockwise
        break;
    case ActionRequest::RotateLeft90:
        direction = static_cast<Direction>((direction + 6) % 8); // move 2 clockwise
        break;
    case ActionRequest::RotateRight90:
        direction = static_cast<Direction>((direction + 2) % 8); // move 2 counter-clockwise
        break;
    case ActionRequest::MoveForward:
    case ActionRequest::MoveBackward:
    case ActionRequest::Shoot:
    case ActionRequest::DoNothing:
    case ActionRequest::GetBattleInfo:
        break;
    }
}

/**
 * @brief Marks the tank as destroyed.
 *
 * Also sets `killed_this_round` to true.
 */
void Tank::destroy()
{
    destroyed = true;
    killed_this_round = true;
}

/**
 * @brief Checks whether the tank has been destroyed.
 *
 * @return true if destroyed, false otherwise.
 */
bool Tank::isDestroyed() const
{
    return destroyed;
}

/**
 * @brief Returns the symbol representing the tank.
 * @return Character symbol for the tank - '1' or '2'.
 */
char Tank::getSymbol() const
{
    return ownerId + '0';
}

/**
 * @brief Sets the number of remaining shells.
 *
 * @param shells Number of shells to set.
 */
void Tank::setNumOfShells(int shells)
{
    remaining_shells = shells;
}

/**
 * @brief Returns the number of remaining shells the tank can shoot.
 *
 * @return int Number of remaining shells.
 */
int Tank::getNumOfRemainingShells() const
{
    return remaining_shells;
}

/**
 * @brief Sets the cooldown time before the tank can shoot again.
 *
 * @param cooldown Number of turns to wait before next shot.
 */
void Tank::setShootCooldown(int cooldown)
{
    shoot_cooldown = cooldown;
}

/**
 * @brief Gets the current shoot cooldown.
 *
 * @return int Number of turns remaining until the tank can shoot.
 */
int Tank::getShootCooldown() const
{
    return shoot_cooldown;
}

/**
 * @brief Sets the cooldown before backward movement can take effect again.
 *
 * @param cooldown Number of steps before another backward action is allowed.
 */
void Tank::setBackwardCooldown(int cooldown)
{
    backward_cooldown = cooldown;
}

/**
 * @brief Gets the current cooldown for backward movement.
 *
 * @return int Number of turns remaining before backward can be used.
 */
int Tank::getBackwardCooldown() const
{
    return backward_cooldown;
}