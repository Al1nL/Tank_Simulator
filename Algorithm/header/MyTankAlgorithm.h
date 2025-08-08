#ifndef MYTANKALGORITHM_H
#define MYTANKALGORITHM_H
#include "../../common/TankAlgorithm.h"
#include "../../common/ActionRequest.h"
#include "../../UserCommon/header/GameObject.h"
#include "TankBattleInfo.h"

#include <memory>

using std::pair, std::vector, std::abs, std::make_unique, std::unique_ptr;
namespace Algorithm_212535058_324022904
{
    class MyTankAlgorithm : public TankAlgorithm
    {

    protected:
        int player_index;
        int tank_index;

        unique_ptr<TankBattleInfo> battle_info;
        pair<int, int> nextStep(bool forward, const pair<int, int> pos, const Direction dir);
        ActionRequest determineRotation(Direction currentDir, Direction desiredDir);

        // Safety check for backward movement
        bool canSafelyBack(int backR, int backC);
        bool willBeHitIn(int row, int col, int t);
        bool isAlignedWithOpponent(pair<int, int> opponentPos);
        ActionRequest checkForEscape();

        // Common decision utilities
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

    public:
        MyTankAlgorithm(int player_index, int tank_index);
        ~MyTankAlgorithm() override = default;
        ActionRequest getAction() override;
        virtual ActionRequest decideAction() { return ActionRequest::DoNothing; }

        void updateBattleInfo(BattleInfo &info) override;
        int getTankId() const { return tank_index; }
        int getOwnerId() const { return player_index; }
    };
}
#endif // MYTANKALGORITHM_H
