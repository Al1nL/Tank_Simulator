#ifndef PLAYER_212535058_324022904_H
#define PLAYER_212535058_324022904_H

#include "../../common/Player.h"
#include <vector>
#include <memory>

class BattleInfo;
namespace UserCommon_212535058_324022904 {
    class Shell;  // Forward declaration
}

using Shell = UserCommon_212535058_324022904::Shell;
using std::unique_ptr, std::vector,std::make_unique;

namespace Algorithm_212535058_324022904 {

    class Player_212535058_324022904: public Player {
        int player_index;
        size_t map_width = 0;
        size_t map_height = 0;
        int steps_left;
        int shells_per_tank;
        unique_ptr<BattleInfo> battle_info;

        int last_battleInfo_step = steps_left;
        void calcShellsDirection(vector<Shell *> knownShells);

    public:
        Player_212535058_324022904(int, size_t, size_t, size_t, size_t);

        // Delete copy operations
        Player_212535058_324022904(const Player_212535058_324022904 &) = delete;
        Player_212535058_324022904 &operator=(const Player_212535058_324022904 &) = delete;

        ~Player_212535058_324022904() override = default;

        void updateTankWithBattleInfo(TankAlgorithm&, SatelliteView&) override;

        int getPlayerIndex() const;
        void updateStepsLeft();

        void getBattleInfoFromSatelliteView(SatelliteView &view);
        vector<Shell *> getShellsFromKnownObjects();
    };

}

#endif //PLAYER_212535058_324022904_H
