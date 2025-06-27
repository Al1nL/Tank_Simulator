#ifndef TANKBATTLEINFO_H
#define TANKBATTLEINFO_H
#include "../common/BattleInfo.h"
#include "../../UserCommon/header/GameObject.h"
#include <utility>
#include <vector>
#include <memory>
#include <map>

using std::map, std::vector, std::pair, std::unique_ptr, std::make_pair;
namespace Algorithm_212535058_324022904
{
    struct OppData
    {
        pair<int, int> opponentPos;
        Direction opponentDir = None;
    };

    class TankBattleInfo : public BattleInfo
    {
        int id = -1;
        int player_id = -1;
        Direction direction = None;
        pair<int, int> position = {-1, -1};
        int remaining_shells = 0;
        int shoot_cooldown = 0;
        bool is_waiting_for_backward = false;
        int backward_cooldown = 0;        // Number of turns left until backward move happens
        bool cancel_backward = false;     // Whether the backward move was canceled
        bool moved_backward_last = false; // Indicates immediate next back is allowed
        vector<OppData> opponents;
        map<pair<int, int>, vector<GameObject *>> knownObjects;

        bool set_shells = false;
        pair<int, int> map_size = {-1, -1};

    public:
        TankBattleInfo(int tank_index, int player_id);
        ~TankBattleInfo() override = default;

        void setBackwardCooldown(int cooldown = 2);

        void setWaitingForBackward(bool);
        bool getWaitingForBackward();

        void setMovedBackwardLast(bool);
        bool getMovedBackwardLast();

        void setShootCooldown(int cooldown = 4);

        bool isWaitingToReverse() const;
        bool isWaitingToShoot() const;

        void decreaseRemainingShells();
        void setRemainingShells(int num_of_shells);
        int getRemainingShells() const;
        bool isShellsSet() const;

        void setDirection(Direction direction);
        Direction getDirection() const;

        void setPosition(int x, int y);
        pair<int, int> getPosition() const;

        vector<OppData> getOpponents() const;

        void setOpponents(vector<OppData> opps);
        void addOpponent(pair<int, int> position, Direction dir = None);

        GameObject *getObjectByPosition(pair<int, int> pos) const;

        map<pair<int, int>, vector<GameObject *>> getKnownObjects() const;
        void setKnownObjects(map<pair<int, int>, vector<GameObject *>> knownObjects);
        void updateObjectDirByPosition(pair<int, int> pos, Direction dir);

        void setMapSize(int h, int w);
        pair<int, int> getMapSize() const;

        Direction calculateRealDirection(int currRow, int currCol, int targetRow, int targetCol);
    };
}
#endif // TANKBATTLEINFO_H
