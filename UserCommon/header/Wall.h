#ifndef WALL_H
#define WALL_H
#include "GameObject.h"
using std::pair;
namespace UserCommon_212535058_324022904 {
    class Wall : public GameObject
    {
        int hitCount = 0;
        static const int WALL_MAX_HITS = 2;  // Assuming this is the intended max hits
        static const char WALL_SYMBOL = '#'; // Assuming this is the intended symbol
        bool destroyed;

    public:
        Wall(pair<int, int> pos);
        ~Wall() override = default;

        Wall(Wall const &) = delete;
        Wall &operator=(const Wall &) = delete;
        void damage()
        {
            ++hitCount;
            if (hitCount == WALL_MAX_HITS)
                destroy();
        }
        std::unique_ptr<GameObject> clone() const override {
            return std::make_unique<Wall>(getPos());
        }
        int getHitCount() const;
        void registerHit();
        void destroy() override;
        bool isDestroyed() const override;
        char getSymbol() const override;
    };
}
#endif // WALL_H
