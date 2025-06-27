#ifndef TANKALGORITHM_212535058_324022904_H
#define TANKALGORITHM_212535058_324022904_H

#include "../../common/TankAlgorithm.h"
#include "../../common/ActionRequest.h"
#include "../../UserCommon/header/GameObject.h"
#include "TankBattleInfo.h"

#include <memory>
#include <utility>
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

    class TankAlgorithm_212535058_324022904 : public TankAlgorithm {
        int player_index;
        int tank_index;
        int current_turn = 0;
        int last_info_update = 0;

        unique_ptr<TankBattleInfo> battle_info;
        pair<int, int> nextStep(bool forward, const pair<int, int> pos, const Direction dir);
        ActionRequest determineRotation(Direction currentDir, Direction desiredDir);

        // Safety check for backward movement
        bool canSafelyBack(int backR, int backC);
        bool willBeHitIn(int row, int col, int t);
        bool isAlignedWithOpponent(pair<int, int> opponentPos);
        ActionRequest checkForEscape();

        // decision utilities
        bool isOccupierFree(pair<int, int> pos);
        bool shouldShootOpponent(const pair<int, int> &opponentPos);
        bool canMoveFwd();
        bool canMoveBack();
        void rotate(ActionRequest action);
        bool isValidMove(ActionRequest action);
        Direction calculateDirection(int currRow, int currCol, int targetRow, int targetCol);
        OppData getClosestOpponent();
        int calculateActionsToOpponent(const OppData &oppPos);
        void updateInnerInfoAfterAction(ActionRequest action);
        int wrap(int value, int size) { return size != -1 ? (value % size + size) % size : value; }; // wrap-around edges

        vector<ActionRequest> rotations = {ActionRequest::RotateLeft45, ActionRequest::RotateLeft90, ActionRequest::RotateRight45, ActionRequest::RotateRight90};
        int countOpenSpaceInDirection(pair<int, int> pos);
        RotationOption rotationOption(ActionRequest rotation, Direction newDir, Direction oldDir);
        ActionRequest calculateBestEscapeRotation();
        bool canShootAfterRotate(Direction targetDir, OppData &opp);
        bool shouldShootOpponent(OppData &opp);
        Direction simulateRotation(ActionRequest act);
        ActionRequest decideAction();
        void moveKnownShells();

    public:
        TankAlgorithm_212535058_324022904(int, int);
        ~TankAlgorithm_212535058_324022904() override = default;

        TankAlgorithm_212535058_324022904(const TankAlgorithm_212535058_324022904 &) = delete;
        TankAlgorithm_212535058_324022904 &operator=(const TankAlgorithm_212535058_324022904 &) = delete;

        ActionRequest getAction() override;
        void updateBattleInfo(BattleInfo&) override;

        int getTankId() const { return tank_index; }
        int getOwnerId() const { return player_index; }
    };

}

#endif //TANKALGORITHM_212535058_324022904_H
