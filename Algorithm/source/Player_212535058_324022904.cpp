#include "../header/Player_212535058_324022904.h"
#include <iostream>
using namespace Algorithm_212535058_324022904;
REGISTER_PLAYER(Player_212535058_324022904);

/**
 * Constructor for AdvancedPlayer.
 * Initializes the player with map size, max steps, and number of shells.
 */
Player_212535058_324022904::Player_212535058_324022904(int player_index, size_t map_height, size_t map_width, size_t max_steps, size_t num_shells)
    : player_index(player_index), map_width(map_width), map_height(map_height), steps_left(static_cast<int>(max_steps)),
      shells_per_tank(num_shells), battle_info(make_unique<TankBattleInfo>(-1, player_index)) {}

/**
 * @brief Returns the player index.
 * @return Player's index.
 */
int Player_212535058_324022904::getPlayerIndex() const { return player_index; }

/**
 * @brief Updates the provided tank's battle info using the current satellite view.
 *
 * @param tank The tank algorithm instance to update.
 * @param view The satellite view containing the latest battlefield info.
 */
void Player_212535058_324022904::updateTankWithBattleInfo(TankAlgorithm &tank, SatelliteView &view)
{
    auto knownShells = getShellsFromKnownObjects();
    getBattleInfoFromSatelliteView(view);
    calcShellsDirection(knownShells);
    tank.updateBattleInfo(*battle_info);
    last_battleInfo_step = steps_left;
}

/**
 * @brief Calculates the direction of shells that currently have unknown directions.
 * This method attempts to infer direction by checking where
 *
 * @param knownShells Vector of pointers to known Shell objects.
 */
void Player_212535058_324022904::calcShellsDirection(vector<Shell *> knownShells)
{
    int steps_passed = 2 * (last_battleInfo_step - steps_left);
    vector<pair<int, int>> candidates;
    auto *tank_info = dynamic_cast<TankBattleInfo *>(battle_info.get());
    pair<int, int> new_pos = {-1, -1};
    auto calc_pos = [h = this->map_height, w = this->map_width](const pair<int, int> &pos,
                                                                const pair<int, int> &dir_offset,
                                                                int steps)
    {
        return make_pair(
            ((pos.first + dir_offset.first * steps) % h + h) % h,
            ((pos.second + dir_offset.second * steps) % w + w) % w);
    };

    for (auto shell : knownShells)
    {
        if (shell->getDirection() == Direction::None)
        {
            for (int dir = U; dir != None; dir++)
            {
                new_pos = calc_pos(shell->getPos(), offsets[dir], steps_passed);
                auto candidate = tank_info->getObjectByPosition(new_pos);
                if (candidate != nullptr && candidate->getSymbol() == '*')
                {
                    candidates.push_back(candidate->getPos());
                }
            }
        }
        else
        {
            //if(shell->getDirection() > None || shell->getDirection() < U) continue; // Invalid direction
            new_pos = calc_pos(shell->getPos(), offsets[shell->getDirection()], steps_passed);
            auto candidate = tank_info->getObjectByPosition(new_pos);
            if (candidate == nullptr || (candidate != nullptr && candidate->getSymbol() != '*'))
            {
                for (int dir = U; dir != None; dir++)
                {
                    new_pos = calc_pos(shell->getPos(), offsets[dir], steps_passed);
                    auto candidate = tank_info->getObjectByPosition(new_pos);
                    if (candidate != nullptr && candidate->getSymbol() == '*')
                    {
                        candidates.push_back(candidate->getPos());
                    }
                }
            }
        }
        if (!candidates.empty())
        {
            auto [r, c] = shell->getPos();
            auto [nr, nc] = candidates[0];
            tank_info->updateObjectDirByPosition(new_pos, tank_info->calculateRealDirection(r, c, nr, nc));
            candidates.clear();
        }
    }
}

/**
 * @brief Retrieves a vector of pointers to all known Shell objects in the current battle info.
 *
 * @return Vector of Shell pointers representing all known shells.
 */
vector<Shell *> Player_212535058_324022904::getShellsFromKnownObjects()
{
    const auto &knownObj = dynamic_cast<TankBattleInfo *>(battle_info.get())->getKnownObjectsView();
    vector<Shell *> shells;
    for (auto &[pos, objs] : knownObj)
    {
        for (auto &obj : objs)
        {
            if (obj && obj->getSymbol() == '*')
            {
                auto shell = dynamic_cast<Shell *>(obj);
                if (shell)
                {
                    shells.push_back(dynamic_cast<Shell*>(shell->clone().get()));
                }
            }
        }
    }
    return shells;
}

/**
 * @brief Extracts and updates battle info from the satellite view.
 * @param view Current satellite view of the board.
 */
void Player_212535058_324022904::getBattleInfoFromSatelliteView(SatelliteView &view)
{
    int i = 0, j = 0;

    char symbol = ' ';
    int tanki_1 = 0, tanki_2 = 0;
    auto *tank_info = dynamic_cast<TankBattleInfo *>(battle_info.get());

    vector<OppData> opponents;
    std::map<std::pair<int, int>, std::vector<GameObject *>> newKnownObjects;

    // This will own all the game objects
    vector<unique_ptr<GameObject>> objectStorage;

    if (!tank_info->isShellsSet())
        tank_info->setRemainingShells(shells_per_tank);
    if (tank_info->getMapSize().first == -1)
        tank_info->setMapSize(map_height, map_width);

    for (i = 0; i < static_cast<int>(map_height); i++)
    {
        for (j = 0; j < static_cast<int>(map_width); j++)
        {
            symbol = view.getObjectAt(i, j);
            if (symbol == '&')
            {
                break;
            }
            if (symbol == '%')
            {
                tank_info->setPosition(i, j);
                continue;
            }
            pair pos = {i, j};

            if (symbol == ' ')
            {
                continue;
            }
            vector<GameObject *> vec;

            unique_ptr<GameObject> obj;
            if (symbol == '@')
            {
                obj = make_unique<Mine>(pos);
            }
            else if (symbol == '#')
            {
                obj = make_unique<Wall>(pos);
            }
            else if (symbol == '*')
            {
                obj = make_unique<Shell>(pos, None, -1);
            }
            else if (isdigit(symbol))
            {
                int player = symbol - '0';
                obj = make_unique<Tank>(pos, player_index == 1 ? tanki_1++ : tanki_2++, Direction::None, player, shells_per_tank);
                if (player != player_index)
                {
                    opponents.push_back(OppData(pos));
                }
            }
            if (obj)
            {
                objectStorage.push_back(move(obj));
                newKnownObjects[pos].push_back(objectStorage.back().get());
            }
        }
    }

    tank_info->setFrameObjects(move(newKnownObjects), move(objectStorage));
    tank_info->setOpponents(opponents);
}

/**
 * @brief Decrements the number of steps left for the player.
 */
void Player_212535058_324022904::updateStepsLeft()
{
    --steps_left;
}