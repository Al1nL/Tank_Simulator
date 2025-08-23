#include "../header/BoardManager.h"
namespace GameManager_212535058_324022904
{

    /**
     * @brief Constructor: Initializes board dimensions and takes ownership of the game map.
     */
    BoardManager::BoardManager(vector<vector<vector<unique_ptr<GameObject>>>> gameMap, int rows, int cols) : height(rows), width(cols), game_map(std::move(gameMap)) {} // printBoard();}

    /**
     * @brief Returns the top object at a given position, prioritizing the shell if multiple objects exist.
     *
     * @param x Row index.
     * @param y Column index.
     * @return Pointer to the GameObject at the specified position, or nullptr if out of bounds or empty.
     */
    GameObject *BoardManager::getObjectAt(int x, int y) const
    {
        if (x >= height || y >= width)
            return nullptr;
        return !game_map[x][y].empty() ? game_map[x][y].back().get() : nullptr;
    }

    /**
     * @brief Stores a textual representation of the current board state.
     */
    void BoardManager::printBoard()
    {
        stringstream boardState;
        for (int i = 0; i < height; ++i)
        {
            for (int j = 0; j < width; ++j)
            {
                GameObject *obj = getObjectAt(i, j);
                obj != nullptr && !obj->isDestroyed() ? boardState << obj->getSymbol() : boardState << ' ';
            }
            boardState << "\n";
        }
        // Store the complete board state
        boardStates.push_back(boardState.str());
    }

    /**
     * @brief Writes all stored board states to a file.
     *
     * @param fileName Base file name for output file (prefixed with "gameSteps_").
     */
    void BoardManager::writeBoardStates(string fileName)
    {
        // Get directory from GameManager output_file (assuming you have access to it)
        std::string dir = fs::path(fileName).parent_path().string();
        fileName = fs::path(fileName).filename().string();

        // Build full path
        std::string fullPath = dir + "/gameSteps_" + fileName;

        ofstream outFile(fullPath);
        if (!outFile.is_open())
        {
            cerr << "Error: Could not open file for writing!" << endl;
            return;
        }

        // Write header
        outFile << "=== GAME BOARD STATES ===\n\n";

        // Write each board state with turn number
        for (size_t i = 0; i < boardStates.size(); ++i)
        {
            outFile << "Turn " << (i + 1) << ":\n";
            outFile << boardStates[i] << "\n";
        }

        outFile.close();
        boardStates.clear(); // Clear logs after writing
    }

    /**
     * @brief Inserts a GameObject into the map at the given position.
     *
     * @param obj The object to std::move.
     * @param new_pos The target position, or (-1,-1) to indicate deferred removal.
     */
    void BoardManager::updateMap(unique_ptr<GameObject> obj, pair<int, int> new_pos)
    {
        if (!obj)
            return;

        if (new_pos.first == -1 && new_pos.second == -1)
        {
            // deletion handled in cleanupDestroyedObjects
            return;
        }

        obj->setPos(new_pos);
        if (!game_map[new_pos.first][new_pos.second].empty() && game_map[new_pos.first][new_pos.second][0] == nullptr)
            game_map[new_pos.first][new_pos.second][0] = std::move(obj);
        else
            game_map[new_pos.first][new_pos.second].push_back(std::move(obj));
    }

    /**
     * @brief std::moves all shells that have been fired up to 2 steps and handles collisions.
     */
    void BoardManager::moveFiredShells()
    {

        for (int step = 0; step < 2; step++)
        {
            for (size_t i = 0; i < fired_shells.size(); i++)
            {
                auto &shell = fired_shells[i];
                if (shell == nullptr || shell->isDestroyed())
                {
                    continue;
                }
                pair<int, int> oldPos = shell->getPos();
                shell->setLastPos(oldPos);
                pair<int, int> newPos = calculateNewPosition(oldPos, shell->getDirection());

                // Find the shell's unique_ptr in game_map
                unique_ptr<GameObject> shellPtr = extractObjectFromMap(shell); // restd::move shell from map

                // std::move to new position using updateMap
                shellPtr->setPos(newPos);
                updateMap(std::move(shellPtr), newPos);
                handleAllCollisions();
            }
        }
        auto new_end = std::remove_if(fired_shells.begin(), fired_shells.end(),
                                      [](Shell *shell)
                                      {
                                          return shell == nullptr || shell->isDestroyed();
                                      });
        fired_shells.erase(new_end, fired_shells.end());
    }

    /**
     * @brief Extracts and restd::moves a specific object from the map, returning ownership.
     *
     * @tparam T Type derived from GameObject.
     * @param object Pointer to the object to extract.
     * @return A unique_ptr to the restd::moved object.
     */
    template <typename T>
    unique_ptr<GameObject> BoardManager::extractObjectFromMap(T *object)
    {
        if (!object)
            return nullptr;

        pair<int, int> pos = object->getPos();
        if (pos.first < 0 || pos.first >= height || pos.second < 0 || pos.second >= width)
        {
            return nullptr;
        }

        auto &cell = game_map[pos.first][pos.second];
        for (auto it = cell.begin(); it != cell.end(); ++it)
        {
            if (it->get() == object)
            { // std::move ownership and restd::move from map, later gets back to map in updateMap
                unique_ptr<GameObject> objPtr = std::move(*it);
                cell.erase(it);
                return objPtr;
            }
        }
        return nullptr;
    }

    /**
     * @brief Processes all cells with multiple objects and resolves collisions.
     */
    void BoardManager::handleAllCollisions()
    {
        // Find all cells with multiple objects
        for (int x = 0; x < height; x++)
        {
            for (int y = 0; y < width; y++)
            {
                auto &cell = game_map[x][y];
                if (count_if(cell.begin(), cell.end(),
                             [](const auto &obj)
                             { return obj && !obj->isDestroyed(); }) > 1)
                {
                    vector<GameObject *> objects;
                    for (auto &obj : cell)
                    {
                        if (obj && !obj->isDestroyed()) // only consider non-destroyed objects
                            objects.push_back(obj.get());
                    }

                    processCollision(objects);
                }
            }
        }
    }

    /**
     * @brief Calculates a new position from a given direction, with wrap-around.
     */
    pair<int, int> BoardManager::calculateNewPosition(pair<int, int> pos, Direction dir) const
    {
        const auto &offset = offsets[static_cast<int>(dir)];
        pos.first = (pos.first + offset.first + height) % height;
        pos.second = (pos.second + offset.second + width) % width;
        return pos;
    }

    /**
     * @brief Processes collision for a group of objects in a single cell.
     *
     * @param objects All objects occupying the same cell.
     */
    void BoardManager::processCollision(vector<GameObject *> &objects)
    {
        bool containsMine = false;
        bool containsTank = false;
        bool containsShell = false;
        bool containsWall = false;
        bool contains2Shells = count_if(objects.begin(), objects.end(),
                                        [](GameObject *obj)
                                        { return dynamic_cast<Shell *>(obj); }) > 1;
        // Analyze collision group
        for (auto obj : objects)
        {
            if (dynamic_cast<Mine *>(obj))
                containsMine = true;
            else if (dynamic_cast<Tank *>(obj))
                containsTank = true;
            else if (dynamic_cast<Shell *>(obj))
                containsShell = true;
            else if (dynamic_cast<Wall *>(obj))
                containsWall = true;
        }

        // Handle special cases
        if (containsMine && containsTank)
        {
            // Mine-Tank explosion destroys everything in the cell
            for (auto obj : objects)
            {
                if (!obj->isDestroyed()) // ignore destroyed objects
                    obj->destroy();
            }
            return;
        }

        // Tank-Tank collision (all tanks die)
        if (containsTank && count_if(objects.begin(), objects.end(),
                                     [](GameObject *obj)
                                     { return dynamic_cast<Tank *>(obj); }) > 1)
        {
            for (auto obj : objects)
            {
                if (auto tank = dynamic_cast<Tank *>(obj))
                {
                    if (!tank->isDestroyed()) // ignore destroyed tanks
                        tank->destroy();
                }
            }
            return;
        }

        // Shell-specific collisions
        for (auto obj : objects)
        {
            if (auto shell = dynamic_cast<Shell *>(obj))
            {
                if (shell->isDestroyed())
                    continue; // ignore destroyed shells
                bool destroyShell = false;

                // Shell-Shell collision
                if (contains2Shells)
                {
                    for (auto other : objects)
                    {
                        if (!dynamic_cast<Mine *>(other))
                        {
                            other->destroy(); // Destroy all object but mine
                            destroyShell = true;
                        }
                    }
                    // continue;
                }
                else if (containsShell)
                {
                    // Shell-Wall collision
                    if (containsWall && shell->getLastPos() != shell->getPos())
                    {
                        for (auto other : objects)
                        {
                            if (auto wall = dynamic_cast<Wall *>(other))
                            {
                                if (wall->isDestroyed())
                                    continue; // ignore destroyed walls
                                wall->damage();
                            }
                        }
                    }
                    // Shell-Tank collision
                    if (containsTank)
                    {
                        for (auto other : objects)
                        {
                            if (auto tank = dynamic_cast<Tank *>(other))
                            {
                                if (tank->isDestroyed())
                                    continue;
                                tank->destroy();
                                destroyShell = true;
                            }
                        }
                    }
                    shell->setLastPos(shell->getPos()); // reset last position to current position
                }
                if (destroyShell)
                {
                    shell->destroy(); // Destroy shell
                }
            }
        }
    }

    /**
     * @brief Converts the object map into a character representation.
     *
     * @return 2D vector of characters for display/debugging.
     */
    vector<vector<char>> BoardManager::objMapToCharMap()
    {
        vector<vector<char>> charMap;
        for (int x = 0; x < height; x++)
        {
            charMap.push_back(vector<char>());
            for (int y = 0; y < width; y++)
            {
                if (game_map[x][y].size() > 0)
                {
                    auto obj = game_map[x][y].back().get();
                    charMap[x].push_back(obj && !obj->isDestroyed() ? obj->getSymbol() : ' ');
                }
                else // shouldn't reach here
                    charMap[x].push_back(' ');
            }
        }
        return charMap;
    }

    /**
     * @brief Iterates over the board and restd::moves destroyed objects.
     */
    void BoardManager::boardCleanup()
    {
        for (int x = 0; x < height; x++)
        {
            for (int y = 0; y < width; y++)
            {
                cleanupDestroyedObjects({x, y});
            }
        }
    }

    /**
     * @brief Restd::moves destroyed objects from a specific cell.
     *
     * @param pos Coordinates of the cell to clean.
     */
    void BoardManager::cleanupDestroyedObjects(pair<int, int> pos)
    {
        auto &cell = game_map[pos.first][pos.second];
        cell.erase(remove_if(cell.begin(), cell.end(), [](const unique_ptr<GameObject> &obj)
                             { return obj && obj->isDestroyed() && !(obj->getSymbol() != '2' || obj->getSymbol() != '1'); }),
                   cell.end()); // releases ptr automaticly when erasing.
    }

    /**
     * @brief Checks if a std::move is valid for a given tank.
     *
     * @param tank Pointer to the tank attempting the std::move.
     * @param std::move The intended direction of std::movement.
     * @return true if the std::move is valid (e.g., not a wall); false otherwise.
     */
    bool BoardManager::isValidMove(Tank *tank, ActionRequest action)
    {
        Direction dir = tank->getDirection();
        if (!tank || tank->isDestroyed())
        {
            return false;
        }
        if (tank->isWaitingToBackward() && action != ActionRequest::MoveForward) // only fwd std::move can cancel while waiting for the back std::move
            return false;

        if (action == ActionRequest::Shoot && tank->isWaitingToShoot())
        {
            return false;
        }

        // For std::movement actions, check the target position
        if (action == ActionRequest::MoveForward || action == ActionRequest::MoveBackward)
        {
            if (action == ActionRequest::MoveBackward)
            {
                dir = static_cast<Direction>((static_cast<int>(tank->getDirection()) + 4) % 8);
            }
            auto [newX, newY] = calculateNewPosition(tank->getPos(), dir);

            // Check if the new position is occupied
            GameObject *obj = !game_map[newX][newY].empty() ? game_map[newX][newY][0].get() : nullptr;
            if (obj && !obj->isDestroyed() && obj->getSymbol() == '#')
            {                 // not valid only if it's a wall
                return false; // Position is blocked
            }
        }
        return true; // all other actions are allways valid
    }

    /**
     * @brief Applies a tank's std::move actions to the game board.
     *
     * @param std::moves A map from Tank pointer to the action it wants to do.
     */
    void BoardManager::applyMoves(map<Tank *, ActionRequest> moves)
    {
        for (auto &[tank, action] : moves)
        {
            if (!tank || tank->isDestroyed())
                continue;

            bool isValid = isValidMove(tank, action);
            tank->setActionSuccess(isValid);
            tank->setLastAction(action);
            if (!isValid)
                continue;

            pair<int, int> newPos = {-1, -1};
            if (tank->isWaitingToBackward() && tank->getBackwardCooldown() == 0)
                action = ActionRequest::MoveBackward; // back after 3 rounds, no matter what the other action now is, it is ignored?
            switch (action)
            {
            case ActionRequest::MoveForward:
                if (tank->isWaitingToBackward())
                {
                    tank->setBackwardCooldown(0);
                    tank->setWaitingForBackward(false);
                    // Tank stays in place
                }
                else
                {
                    newPos = calculateNewPosition(tank->getPos(), tank->getDirection());
                    unique_ptr<GameObject> tankPtr = extractObjectFromMap(tank);

                    tank->setPos(newPos);
                    updateMap(std::move(tankPtr), newPos);
                }
                break;

            case ActionRequest::MoveBackward:
                if (tank->isBackLastMove() || (tank->isWaitingToBackward() && tank->getBackwardCooldown() == 0))
                { // Instant backward std::move
                    newPos = calculateNewPosition(tank->getPos(), static_cast<Direction>((static_cast<int>(tank->getDirection()) + 4) % 8));
                    tank->setWaitingForBackward(false);
                    unique_ptr<GameObject> tankPtr = extractObjectFromMap(tank);
                    tank->setPos(newPos);

                    updateMap(std::move(tankPtr), newPos);
                }
                else if (!tank->isWaitingToBackward())
                {
                    // Start backward cooldown (waiting for 2 steps)
                    tank->setBackwardCooldown(2);
                    tank->setWaitingForBackward(true);
                }
                break;

            case ActionRequest::Shoot:
                if (!tank->isWaitingToShoot() && tank->getNumOfRemainingShells() > 0)
                {
                    tank->setNumOfShells(tank->getNumOfRemainingShells() - 1);
                    tank->setShootCooldown(4);
                    pair shell_pos = calculateNewPosition(tank->getPos(), tank->getDirection());
                    auto shellPtr = std::make_unique<Shell>(shell_pos, tank->getDirection(), tank->getOwnerId());
                    Shell *rawShellPtr = shellPtr.get();       // keep non-owning pointer

                    fired_shells.push_back(rawShellPtr);       // track shell
                    updateMap(std::move(shellPtr), shell_pos); // transfer ownership into map
                    if (game_map[shell_pos.first][shell_pos.second].size() > 1)
                    {
                        if (auto wall = dynamic_cast<Wall *>(game_map[shell_pos.first][shell_pos.second][0].get()))
                        {
                            if (!wall->isDestroyed()) // ignore destroyed walls
                                wall->damage();
                        }
                    }
                }
                else if (tank->isWaitingToShoot())
                {
                    tank->setShootCooldown(tank->getShootCooldown() - 1);
                }
                break;

            case ActionRequest::RotateLeft45:
            case ActionRequest::RotateRight45:
            case ActionRequest::RotateLeft90:
            case ActionRequest::RotateRight90:
                tank->rotate(action);
                break;

            case ActionRequest::DoNothing:
            case ActionRequest::GetBattleInfo: // handled in GameMmanager
                break;
            }
            tank->setLastAction(action);

            // Handle cooldowns
            if (action != ActionRequest::Shoot && tank->isWaitingToShoot())
            {
                tank->setShootCooldown(tank->getShootCooldown() - 1);
            }
            if (action != ActionRequest::MoveBackward && tank->isWaitingToBackward())
                tank->setBackwardCooldown(tank->getBackwardCooldown() - 1);
        }

        handleAllCollisions();
    }
}