#ifndef SHELL_H
#define SHELL_H
#include "GameObject.h"
using std::pair;
namespace UserCommon__212535058_324022904
{
    class Shell : public GameObject
    {
        Direction direction;
        int ownerId;
        bool destroyed;
        static const char SHELL_SYMBOL = '*';

    public:
        // Constructor
        Shell(pair<int, int> pos, Direction dir, int owner);
        ~Shell() override = default;

        Shell(Shell const &) = delete;
        Shell &operator=(const Shell &) = delete;

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
