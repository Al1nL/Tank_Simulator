#ifndef TANKBATTLEINFO_H
#define TANKBATTLEINFO_H
#include "../../common/BattleInfo.h"
#include "../../UserCommon/header/GameObject.h"
#include <utility>
#include <vector>
#include <memory>
#include <map>

using GameObject = UserCommon__212535058_324022904::GameObject;
using UserCommon__212535058_324022904::Direction;

using std::map, std::vector, std::pair, std::unique_ptr, std::make_pair;
namespace Algorithm_212535058_324022904
{
    struct OppData
    {
        pair<int, int> opponentPos;
        Direction opponentDir = Direction::None;
    };

    class TankBattleInfo : public BattleInfo
    {
        int id = -1;
        int player_id = -1;
        Direction direction = Direction::None;
        pair<int, int> position = {-1, -1};
        int remaining_shells = 0;
        int shoot_cooldown = 0;
        bool is_waiting_for_backward = false;
        int backward_cooldown = 0;        // Number of turns left until backward move happens
        bool cancel_backward = false;     // Whether the backward move was canceled
        bool moved_backward_last = false; // Indicates immediate next back is allowed
        vector<OppData> opponents;
        map<pair<int, int>, vector<GameObject *>> knownObjects;
        std::vector<std::unique_ptr<GameObject>> objectStorage;

        bool set_shells = false;
        pair<int, int> map_size = {-1, -1};

    public:
        TankBattleInfo(int tank_index, int player_id);
        ~TankBattleInfo() override {
            // Clear containers (though not strictly necessary as they'll auto-clean)
            knownObjects.clear();
            objectStorage.clear();
            opponents.clear();
        }
        // Block copy operations
        TankBattleInfo(const TankBattleInfo&) = delete;
        TankBattleInfo& operator=(const TankBattleInfo&) = delete;
    
        void setBackwardCooldown(int cooldown = 2);

        void setWaitingForBackward(bool);
        bool getWaitingForBackward();

        void setMovedBackwardLast(bool);
        bool getMovedBackwardLast();
        void decreaseShootCooldown(){ shoot_cooldown>0?shoot_cooldown--:shoot_cooldown;}
        void setShootCooldown(int cooldown = 4);        bool isWaitingToReverse() const;
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
        void addOpponent(pair<int, int> position, Direction dir = Direction::None);
  std::vector<std::unique_ptr<GameObject>> takeObjectStorage();
    
        // Modified to accept rvalue references
        void setFrameObjects(
            std::map<std::pair<int,int>, std::vector<GameObject*>>&& newKnownObjects,
            std::vector<std::unique_ptr<GameObject>>&& newStorage
        );
        GameObject *getObjectByPosition(pair<int, int> pos) const;

        map<pair<int, int>, vector<GameObject *>> getKnownObjects() const;
        void setKnownObjects(map<pair<int, int>, vector<GameObject *>> knownObjects);
        void updateObjectDirByPosition(pair<int, int> pos, Direction dir);

        void setMapSize(int h, int w);
        pair<int, int> getMapSize() const;

        Direction calculateRealDirection(int currRow, int currCol, int targetRow, int targetCol);
        const std::map<std::pair<int,int>, std::vector<GameObject*>>& getKnownObjectsView() const {
            return knownObjects;
        }

        std::map<std::pair<int,int>, std::vector<GameObject*>>& getKnownObjectsForUpdate() {
            return knownObjects;
        }
    };
}
#endif // TANKBATTLEINFO_H
