#include "../header/TankBattleInfo.h"
#include "../../UserCommon/header/Shell.h"
#include <iostream>
using namespace UserCommon__212535058_324022904;
using namespace Algorithm_212535058_324022904;

/**
 * @brief Constructor initializing the tank's ID and player ID.
 *
 * Also sets the initial facing direction: L for player 1, R otherwise.
 */

 TankBattleInfo::TankBattleInfo(int tank_index, int player_index) : id(tank_index), player_id(player_index)
{
    direction = player_id == 1 ? L : R;
}

vector<std::unique_ptr<GameObject>> TankBattleInfo::takeObjectStorage() {
    vector<std::unique_ptr<GameObject>> newStorage;
    for (auto& obj : objectStorage) {
        if (obj) {
            newStorage.push_back(obj->clone());  // Safe: deep copy
        }
    }
    return newStorage;
}
void TankBattleInfo::setFrameObjects(
    std::map<std::pair<int,int>, std::vector<GameObject*>>&& newKnownObjects,
    std::vector<std::unique_ptr<GameObject>>&& newStorage
) {
    knownObjects.clear(); // Clear existing known objects
    objectStorage.clear(); // Clear existing storage
    objectStorage = std::move(newStorage);
    knownObjects = std::move(newKnownObjects);
}

/**
 * @brief Set the cooldown until backward movement can occur.
 */
void TankBattleInfo::setBackwardCooldown(int cooldown) { backward_cooldown = cooldown; }

/**
 * @brief Set whether the tank is waiting for a delayed backward movement.
 */
void TankBattleInfo::setWaitingForBackward(bool is_waiting) { is_waiting_for_backward = is_waiting; }

/**
 * @brief Get whether the tank is waiting for a delayed backward movement.
 */
bool TankBattleInfo::getWaitingForBackward() { return is_waiting_for_backward; }

/**
 * @brief Mark if the tank moved backward on its last action.
 */
void TankBattleInfo::setMovedBackwardLast(bool moved) { moved_backward_last = moved; }

/**
 * @brief Check if the tank moved backward last turn.
 */
bool TankBattleInfo::getMovedBackwardLast() { return moved_backward_last; }

/**
 * @brief Set the cooldown until the tank can shoot again.
 */
void TankBattleInfo::setShootCooldown(int cooldown) { shoot_cooldown = cooldown; }

/**
 * @brief Returns whether the tank is currently in a shooting cooldown.
 */
bool TankBattleInfo::isWaitingToShoot() const { return shoot_cooldown > 0; }

/**
 * @brief Returns whether the tank is waiting to complete a delayed backward move.
 */
bool TankBattleInfo::isWaitingToReverse() const { return is_waiting_for_backward && backward_cooldown > 0; }

/**
 * @brief Decrease the remaining number of shells by 1.
 */
void TankBattleInfo::decreaseRemainingShells() { --remaining_shells; }

/**
 * @brief Set the number of remaining shells.
 */
void TankBattleInfo::setRemainingShells(int num_of_shells)
{
    remaining_shells = num_of_shells;
    set_shells = true;
}

/**
 * @brief Get the current number of remaining shells.
 */
int TankBattleInfo::getRemainingShells() const { return remaining_shells; }

/**
 * @brief Check if the number of shells has been initialized.
 */
bool TankBattleInfo::isShellsSet() const { return set_shells; }

/**
 * @brief Set the tank's facing direction.
 */
void TankBattleInfo::setDirection(Direction dir) { direction = dir; }

/**
 * @brief Get the current direction the tank is facing.
 */
Direction TankBattleInfo::getDirection() const { return direction; }

/**
 * @brief Set the tank's current position.
 */
void TankBattleInfo::setPosition(int x, int y)
{
    position.first = x;
    position.second = y;
}

/**
 * @brief Get the tank's current position.
 */
pair<int, int> TankBattleInfo::getPosition() const { return position; }

/**
 * @brief Get the list of known opponents.
 */
vector<OppData> TankBattleInfo::getOpponents() const { return opponents; }

/**
 * @brief Set the list of known opponents.
 */
void TankBattleInfo::setOpponents(vector<OppData> opps) { opponents = opps; }

/**
 * @brief Add a single opponent's information.
 *
 * @param position Opponent's position.
 * @param dir Opponent's direction.
 */
void TankBattleInfo::addOpponent(pair<int, int> position, Direction dir)
{
    opponents.emplace_back(OppData(make_pair(position.first, position.second), dir));
}

/**
 * @brief Return the GameObject pointer at a given position.
 *
 * If two objects exist at that position, returns the second. Otherwise, the first.
 */
GameObject *TankBattleInfo::getObjectByPosition(pair<int, int> pos) const
{
    return knownObjects.contains(pos) && !knownObjects.at(pos).empty() ? knownObjects.at(pos).size() > 1 ? knownObjects.at(pos)[1] : knownObjects.at(pos)[0] : nullptr;
}

/**
 * @brief Get a map of all known objects indexed by their position.
 */
map<pair<int, int>, vector<GameObject *>> TankBattleInfo::getKnownObjects() const
{
     std::map<std::pair<int,int>, std::vector<GameObject*>> copy;
    for (const auto& [pos, objects] : knownObjects) {
        for (auto* obj : objects) {
            if (obj) copy[pos].push_back(obj->clone().release());  // Requires GameObject::clone()
        }
    }
    return copy;
}

/**
 * @brief Replace the current known objects map with a new one.
 */
void TankBattleInfo::setKnownObjects(map<pair<int, int>, vector<GameObject *>> newKnownObjects)
{
    knownObjects = move(newKnownObjects);
}

/**
 * @brief Set the map size (height, width).
 */
void TankBattleInfo::setMapSize(int h, int w)
{
    map_size = make_pair(h, w);
}

/**
 * @brief Get the current map size.
 */
pair<int, int> TankBattleInfo::getMapSize() const
{
    return map_size;
}

/**
 * @brief Updates the direction of a Shell object at a given position, if any.
 *
 * @param pos The position to check.
 * @param dir The direction to set on the Shell object.
 */
void TankBattleInfo::updateObjectDirByPosition(pair<int, int> pos, Direction dir)
{
    if (!knownObjects[pos].empty())
    {
        if (knownObjects[pos].size() > 1 && knownObjects[pos][1]->getSymbol() == '*')
        {
            dynamic_cast<Shell *>(knownObjects[pos][1])->setDirection(dir);
        }
        else if (knownObjects[pos][0]->getSymbol() == '*')
        {
            dynamic_cast<Shell *>(knownObjects[pos][0])->setDirection(dir);
        }
    }
}

/**
 * @brief Calculates the directional alignment from one cell to another.
 *
 * @param currRow Starting row.
 * @param currCol Starting column.
 * @param targetRow Target row.
 * @param targetCol Target column.
 * @return Direction representing the alignment (strict cardinal or diagonal), or None if unaligned.
 */
Direction TankBattleInfo::calculateRealDirection(int currRow, int currCol, int targetRow, int targetCol)
{
    int rowDiff = targetRow - currRow;
    int colDiff = targetCol - currCol;

    // Normalize differences to identify direction only if strictly aligned
    if (rowDiff > 0 && colDiff == 0)
        return Direction::D; // Down
    if (rowDiff < 0 && colDiff == 0)
        return Direction::U; // Up
    if (rowDiff == 0 && colDiff > 0)
        return Direction::L; // Right
    if (rowDiff == 0 && colDiff < 0)
        return Direction::R; // Left
    if (rowDiff == colDiff && rowDiff > 0)
        return Direction::DR; // Down-Right (strict diagonal)
    if (rowDiff == -colDiff && rowDiff > 0)
        return Direction::DL; // Down-Left (strict diagonal)
    if (rowDiff == colDiff && rowDiff < 0)
        return Direction::UL; // Up-Right (strict diagonal)
    if (rowDiff == -colDiff && rowDiff < 0)
        return Direction::UR; // Up-Left (strict diagonal)

    return Direction::None; // None for non-aligned movement
}