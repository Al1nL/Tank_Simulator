#ifndef PLAYER_212535058_324022904_H
#define PLAYER_212535058_324022904_H

#include "../../common/Player.h"

namespace Algorithm_212535058_324022904 {

    class Player_212535058_324022904: public Player {
    public:
        Player_212535058_324022904(int, size_t, size_t, size_t, size_t) {}
        void updateTankWithBattleInfo
            (TankAlgorithm&, SatelliteView&) override {
        }
    };

}

#endif //PLAYER_212535058_324022904_H
