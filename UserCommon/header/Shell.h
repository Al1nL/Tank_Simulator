#ifndef SHELL_H
#define SHELL_H
#include "GameObject.h"
using std::pair;
namespace UserCommon_212535058_324022904
{
    class Shell : public GameObject
    {
        Direction direction;
        int ownerId;
        bool destroyed;
        static const char SHELL_SYMBOL = '*';
        pair<int, int> lastpos;

    public:
        // Constructor
        Shell(pair<int, int> pos, Direction dir, int owner);
        ~Shell() override = default;
        pair<int, int> getLastPos() const { return lastpos; }
        void setLastPos(pair<int, int> pos) { lastpos = pos; }
        Shell(Shell const &) = delete;
        Shell &operator=(const Shell &) = delete;
        std::unique_ptr<GameObject> clone() const override {
            return std::make_unique<Shell>(getPos(), direction, ownerId);
        }
        // Accessors
        Direction getDirection() const;
        void setDirection(Direction dir);

        int getOwnerId() const;
        bool isDestroyed() const override;

        // Mutators
        void destroy() override;

        // Symbol for representation on the board
        char getSymbol() const override;
    };
}
#endif // SHELL_H
