#ifndef GAMEOBJECT_H
#define GAMEOBJECT_H
#include <utility>
#include <vector>

namespace UserCommon__212535058_324022904
{
    using std::vector, std::pair;
    enum Direction
    {
        U,
        UR,
        R,
        DR,
        D,
        DL,
        L,
        UL,
        None
    };
    static const vector<pair<int, int>> offsets = {
        {-1, 0}, // U
        {-1, 1}, // UR
        {0, 1},  // R
        {1, 1},  // DR
        {1, 0},  // D
        {1, -1}, // DL
        {0, -1}, // L
        {-1, -1} // UL
    };
    using std::pair;
    class GameObject
    {
        pair<int, int> pos;

    public:
        GameObject() = default;
        GameObject(pair<int, int> pos) : pos(pos) {}
        virtual ~GameObject() {};
        GameObject(GameObject const &) = delete;
        GameObject &operator=(const GameObject &) = delete;
        virtual bool isDestroyed() const = 0;
        virtual void destroy() = 0;
        virtual pair<int, int> getPos() { return pos; }
        virtual void setPos(pair<int, int> position) { pos = position; }
        virtual char getSymbol() const { return ' '; }
    };
}
#endif // GAMEOBJECT_H
