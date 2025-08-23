#ifndef TANKALGORITHM_212535058_324022904_H
#define TANKALGORITHM_212535058_324022904_H
#include "../../common/TankAlgorithmRegistration.h"
#include "MyTankAlgorithm.h"
#include <queue>
#include <algorithm>

using std::pair, std::vector, std::abs, std::make_unique, std::unique_ptr;

namespace Algorithm_212535058_324022904 {

    struct RotationOption
    {
        ActionRequest action;
        int safetyScore;
        bool canMove;
        Direction newDir;
    };

    class TankAlgorithm_212535058_324022904 : public MyTankAlgorithm {
        int current_turn = 0;
        int last_info_update = 0;

        vector<ActionRequest> rotations = {ActionRequest::RotateLeft45, ActionRequest::RotateLeft90, ActionRequest::RotateRight45, ActionRequest::RotateRight90};
        int countOpenSpaceInDirection(pair<int, int> pos);
        RotationOption rotationOption(ActionRequest rotation, Direction newDir, Direction oldDir);
        ActionRequest calculateBestEscapeRotation();
        bool canShootAfterRotate(Direction targetDir, OppData &opp);
        bool shouldShootOpponent(OppData &opp);
        Direction simulateRotation(ActionRequest act);
        ActionRequest decideAction() override;
        void moveKnownShells();

    public:
        TankAlgorithm_212535058_324022904(int, int);
        ~TankAlgorithm_212535058_324022904() override = default;

        TankAlgorithm_212535058_324022904(const TankAlgorithm_212535058_324022904 &) = delete;
        TankAlgorithm_212535058_324022904 &operator=(const TankAlgorithm_212535058_324022904 &) = delete;

        ActionRequest getAction() override;
    };

}

#endif //TANKALGORITHM_212535058_324022904_H
