#include "../header/GameManager.h"
#include "../../common/GameManagerRegistration.h"

namespace GameManager_212535058_324022904
{

    REGISTER_GAME_MANAGER(GameManager);

    GameManager::GameManager(bool verbose) : verbose(verbose) {}

    GameResult GameManager::run(
        size_t map_width, size_t map_height,
        const SatelliteView &map, // <= assume it is a snapshot, NOT updated
        string map_name,
        size_t maxSteps, size_t numShells,
        Player &player1, string name1, Player &player2, string name2,
        TankAlgorithmFactory player1_tank_algo_factory,
        TankAlgorithmFactory player2_tank_algo_factory)
    {
        // Initialize game state
        rows = map_height;
        cols = map_width;
        max_steps = maxSteps;
        current_step = 0;
        num_shells = numShells;
        steps_since_no_shells = 0;
        // Initialize board and tanks
        players.push_back(&player1);
        players.push_back(&player2);
        initializeBoard(map);
        auto res = prepareResult();
        if (res.winner != -1)
        {
            return res;
        }
        initializeTanks(player1_tank_algo_factory, player2_tank_algo_factory);
        // Main game loop
        while (!isGameOver())
        {
            processRound();
            board_view->update(board->objMapToCharMap());

            if (verbose)
            {
                board->printBoard();
            }

            ++current_step;
            if (player_shell_count.at(1) == 0 && player_shell_count.at(2) == 0)
            {
                ++steps_since_no_shells;
            }
        }
        if (verbose)
        {
            setupOutputFile(map_name, name1, name2);
            logGameResult();
            writeOutput();
            board->writeBoardStates(output_file);
        }
        auto result = prepareResult();
        return result;
    }

    void GameManager::initializeBoard(const SatelliteView &map)
    {
        vector<vector<vector<unique_ptr<GameObject>>>> game_map(rows);

        for (size_t i = 0; i < rows; ++i)
        {
            game_map[i].resize(cols);
            for (size_t j = 0; j < cols; ++j)
            {
                char cell = map.getObjectAt(i, j);
                pair<int, int> pos = {static_cast<int>(i), static_cast<int>(j)};

                switch (cell)
                {
                case '1':
                case '2':
                {
                    int player_id = cell - '0';
                    player_tank_count[player_id]++;
                    player_tanks_pos[player_id].emplace_back(pos);
                    auto tank = make_unique<Tank>(
                        pos,
                        player_tank_count[player_id] - 1,
                        player_id == 1 ? Direction::L : Direction::R,
                        player_id,
                        num_shells);
                    game_map[i][j].push_back(std::move(tank));
                    player_shell_count[player_id] += num_shells;
                    break;
                }
                case '@':
                    game_map[i][j].push_back(make_unique<Mine>(pos));
                    break;
                case '#':
                    game_map[i][j].push_back(make_unique<Wall>(pos));
                    break;
                default:
                    game_map[i][j].push_back(nullptr);
                }
            }
        }

        board = make_unique<BoardManager>(std::move(game_map), rows, cols);
        board_view = make_unique<BoardSatelliteView>(rows, cols, board->objMapToCharMap());
    }

    void GameManager::initializeTanks(
        TankAlgorithmFactory &player1_factory,
        TankAlgorithmFactory &player2_factory)
    {
        for (int player_id = 1; player_id <= 2; ++player_id)
        {
            auto &factory = (player_id == 1) ? player1_factory : player2_factory;
            for (int tank_idx = 0; tank_idx < player_tank_count[player_id]; ++tank_idx)
            {
                // Store with tank_idx as key
                player_tanks_algo[player_id][tank_idx] = factory(player_id, tank_idx);
            }
        }
    }

    void GameManager::processRound()
    {
        board->moveFiredShells();

        if (isGameOver())
        {
            return;
        }

        vector<Tank *> tanks = board->getSortedTanks();
        map<Tank *, ActionRequest> actionRequests;

        // Collect actions from all tanks
        for (auto tank : tanks)
        {
            // Get action from algorithm
            TankAlgorithm *tankPtr = findTankAlgorithmById(tank);
            ActionRequest action = tankPtr->getAction();
            actionRequests[tank] = action;
        }
        board->applyMoves(actionRequests);
        updateTanksInfo(tanks);
        if (verbose)
            logs.push_back(generateRoundOutput(actionRequests));

        board->boardCleanup();
    }

    GameResult GameManager::prepareResult()
    {
        GameResult result;
        int p1_tanks = countAliveTanks(1);
        int p2_tanks = countAliveTanks(2);
        result.remaining_tanks.push_back(p1_tanks);
        result.remaining_tanks.push_back(p2_tanks);
        result.winner = -1;

        if (p1_tanks == 0 && p2_tanks == 0)
        {
            result.winner = 0;
            result.reason = GameResult::ALL_TANKS_DEAD;
        }
        else if (p1_tanks == 0)
        {
            result.winner = 2;
            result.reason = GameResult::ALL_TANKS_DEAD;
        }
        else if (p2_tanks == 0)
        {
            result.winner = 1;
            result.reason = GameResult::ALL_TANKS_DEAD;
        }
        else if (current_step >= max_steps)
        {
            result.winner = 0;
            result.reason = GameResult::MAX_STEPS;
        }
        else if (steps_since_no_shells >= 40)
        {
            result.winner = 0;
            result.reason = GameResult::ZERO_SHELLS;
        }
        result.rounds = current_step;
        result.gameState = make_unique<BoardSatelliteView>(rows, cols, board->objMapToCharMap());
        return result;
    }

    bool GameManager::isGameOver()
    {
        int p1_tanks = countAliveTanks(1);
        int p2_tanks = countAliveTanks(2);

        return p1_tanks == 0 ||             // Player 1 eliminated
               p2_tanks == 0 ||             // Player 2 eliminated
               current_step >= max_steps || // Timeout
               steps_since_no_shells >= 40; // No shells left for 40 steps
    }

    int GameManager::countAliveTanks(int player_id)
    {
        return player_tank_count[player_id];
    }

    TankAlgorithm *GameManager::findTankAlgorithmById(Tank *tank)
    {
        // Find the player's algorithm map
        auto player_it = player_tanks_algo.find(tank->getOwnerId());

        // If player exists, find the specific tank algorithm
        if (player_it != player_tanks_algo.end())
        {
            auto tank_it = player_it->second.find(tank->getId());

            // If tank exists, return its algorithm
            if (tank_it != player_it->second.end())
            {
                return tank_it->second.get();
            }
        }

        // 4. Return nullptr if not found
        return nullptr;
    }

    void GameManager::updateTanksInfo(vector<Tank *> tanks)
    {
        for (Tank *tank : tanks)
        {
            int player_id = tank->getOwnerId();
            int tank_idx = tank->getId();

            if (!player_tanks_algo[player_id].count(tank_idx))
                continue;
            TankAlgorithm *algo = findTankAlgorithmById(tank);
            if (!algo)
                continue;
            if (tank->isKilledThisRound())
            {
                --player_tank_count[player_id];
                if (!verbose)
                    tank->setKilledThisRound(false);
            }
            // Update position
            player_tanks_pos[player_id][tank_idx] = tank->getPos();

            // Update shell count if tank shot
            if (tank->getLastAction() == ActionRequest::Shoot)
            {
                --player_shell_count[player_id];
            }
            // Handle battle info requests
            if (tank->getLastAction() == ActionRequest::GetBattleInfo)
            {
                dynamic_cast<BoardSatelliteView *>(board_view.get())->setRequestingTankPos(tank->getPos());
                Player &player = (player_id == 1) ? *players[0] : *players[1];
                player.updateTankWithBattleInfo(*algo, *board_view);
            }
        }
    }
    // Logging funcs

    /**
     * @brief Converts the actions of tanks for a round into a formatted string output.
     * @param tankActions Map of Tank pointers to their respective ActionRequests.
     * @return String summarizing the actions performed by each tank.
     */
    string GameManager::generateRoundOutput(map<Tank *, ActionRequest> tankActions)
    {
        vector<string> actions;
        for (const auto &[tank, acts] : tankActions)
        {
            string move = actionToString(acts);
            if (tank->isDestroyed())
            {
                if (tank->isKilledThisRound())
                {
                    actions.push_back(tank->getActionSuccess() ? move + " (killed)" : move + " (ignored) (killed)");
                    tank->setKilledThisRound(false);
                }
                else
                {
                    actions.push_back("killed");
                }
                continue;
            }
            actions.push_back(tank->getActionSuccess() ? move : move + " (ignored)");
        }
        return joinActions(actions);
    }

    /**
     * @brief Joins a vector of action strings into a single comma-separated string.
     * @param actions Vector of action strings.
     * @return Single string with actions separated by commas.
     */
    string GameManager::joinActions(const vector<string> &actions)
    {
        string result;
        for (size_t i = 0; i < actions.size(); ++i)
        {
            if (i != 0)
                result += ", ";
            result += actions[i];
        }
        return result;
    }

    /**
     * @brief Logs the game result based on remaining alive tanks.
     *        Adds a summary message to the logs vector.
     */
    void GameManager::logGameResult()
    {
        int p1 = countAliveTanks(1);
        int p2 = countAliveTanks(2);

        if (p1 > 0 && p2 == 0)
        {
            logs.push_back("Player 1 won with " + std::to_string(p1) + " tanks still alive");
        }
        else if (p2 > 0 && p1 == 0)
        {
            logs.push_back("Player 2 won with " + std::to_string(p2) + " tanks still alive");
        }
        else if (current_step >= max_steps)
        {
            logs.push_back("Tie, reached max steps = " + std::to_string(max_steps) +
                           ", player 1 has " + std::to_string(p1) +
                           " tanks, player 2 has " + std::to_string(p2) + " tanks");
        }
        else
        {
            logs.push_back("Tie, both players have zero tanks");
        }

        cerr << logs[logs.size() - 1] << endl;
    }

    /**
     * @brief Converts an ActionRequest enum to its string representation.
     * @param action The ActionRequest to convert.
     * @return The string corresponding to the action.
     */
    string GameManager::actionToString(ActionRequest action)
    {
        switch (action)
        {
        case ActionRequest::MoveForward:
            return "MoveForward";
        case ActionRequest::MoveBackward:
            return "MoveBackward";
        case ActionRequest::RotateLeft90:
            return "RotateLeft90";
        case ActionRequest::RotateRight90:
            return "RotateRight90";
        case ActionRequest::RotateLeft45:
            return "RotateLeft45";
        case ActionRequest::RotateRight45:
            return "RotateRight45";
        case ActionRequest::Shoot:
            return "Shoot";
        case ActionRequest::GetBattleInfo:
            return "GetBattleInfo";
        case ActionRequest::DoNothing:
            return "DoNothing";
        default:
            return "Unknown";
        }
    }

    /**
     * @brief Writes the logs collected during the game to the output file.
     *        Appends each log entry on a new line.
     */
    void GameManager::writeOutput()
    {
        ofstream out(output_file);
        for (const auto &line : logs)
        {
            out << line << "\n";
        }
    }
    void GameManager::setupOutputFile(const string &filePath, const string &name1, const string &name2)
    {
        size_t last_slash = filePath.find_last_of("/\\");
        output_file = "output_GM_212535058_324022904_P1_" + name1 + "_P2_" + name2 + "_" + (last_slash == string::npos ? filePath : filePath.substr(last_slash + 1)); // string::npos = “not found”
    }
}