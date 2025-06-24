#ifndef TANKALGORITHM_212535058_324022904_H
#define TANKALGORITHM_212535058_324022904_H

#include "../../common/TankAlgorithm.h"

namespace Algorithm_212535058_324022904 {

    class TankAlgorithm_212535058_324022904 : public TankAlgorithm {
    public:
        TankAlgorithm_212535058_324022904(int, int) {}
        ActionRequest getAction() override;
        void updateBattleInfo(BattleInfo&) override {}
    };

}

#endif //TANKALGORITHM_212535058_324022904_H
