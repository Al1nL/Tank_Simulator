#ifndef GAMEMANAGER
#define GAMEMANAGER

#include "../../common/AbstractGameManager.h"
#include "../../common/GameResult.h"

#include "../../common/Player.h"
#include "../../common/TankAlgorithm.h"
#include <memory>
#include <map>
#include <vector>
#include <string>
#include <iostream>
#include "./BoardManager.h"
using namespace UserCommon__212535058_324022904;

#include "../../UserCommon/header/BoardSatelliteView.h"
#include "../../UserCommon/header/Tank.h"

namespace GameManager_212535058_324022904
{
    using std::unique_ptr, std::vector, std::map, std::pair, std::move, std::make_unique;

    class GameManager : public AbstractGameManager
    {
    public:
        explicit GameManager(bool verbose);
        ~GameManager() {}

        GameResult run(
        size_t map_width, size_t map_height,
   const SatelliteView& map, // <= assume it is a snapshot, NOT updated
string map_name,
size_t max_steps, size_t num_shells,
Player& player1, string name1, Player& player2, string name2,
TankAlgorithmFactory player1_tank_algo_factory,
TankAlgorithmFactory player2_tank_algo_factory) override;

    private:
        // Game state
        size_t rows = 0;
        size_t cols = 0;
        size_t current_step = 0;
        size_t max_steps = 0;
        size_t num_shells = 0;
        size_t steps_since_no_shells = 0;
        bool verbose;
        vector<string> logs;
        string output_file;

        // Game components
        std::unique_ptr<BoardManager> board;
        std::unique_ptr<BoardSatelliteView> board_view;

        // Player data (using abstract interfaces)
        std::map<int, int> player_tank_count = {{1, 0}, {2, 0}};
        std::map<int, int> player_shell_count = {{1, 0}, {2, 0}};
        std::map<int, std::map<int, std::unique_ptr<TankAlgorithm>>> player_tanks_algo;
        std::map<int, std::vector<std::pair<int, int>>> player_tanks_pos;
        std::vector<Player*> players;

        // Private methods


        void initializeBoard(const SatelliteView &map);
        void initializeTanks(
            TankAlgorithmFactory &player1_factory,
            TankAlgorithmFactory &player2_factory);
        void processRound();
        bool isGameOver();
        int countAliveTanks(int player_id);
        TankAlgorithm *findTankAlgorithmById(Tank *tank);
        void updateTanksInfo(std::vector<Tank *> tanks);
        GameResult prepareResult();
        //logs

        string actionToString(ActionRequest action);
        string generateRoundOutput(map<Tank *, ActionRequest>);
        string joinActions(const vector<string> &actions);
        void writeOutput();
            void logGameResult();
                void setupOutputFile(const string &filePath);


    };
}

#endif // GAMEMANAGER