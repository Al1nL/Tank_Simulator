#ifndef BOARDMANAGER_H
#define BOARDMANAGER_H
#include <filesystem>
namespace fs = std::filesystem;

#include "../../UserCommon/header/GameObject.h"
#include "../../UserCommon/header/Tank.h"
#include "../../UserCommon/header/Shell.h"
#include "../../UserCommon/header/Mine.h"
#include "../../UserCommon/header/Wall.h"
#include <vector>
#include <string>
#include <memory>
#include <sstream>
#include <fstream>
#include <iostream>
#include <map>
#include <algorithm>

using std::vector, std::unique_ptr, std::string, std::stringstream, std::map, std::cerr, std::endl, std::ofstream;
using namespace UserCommon__212535058_324022904;
namespace GameManager_212535058_324022904
{

    class BoardManager
    {
        int height;
        int width;
        vector<vector<vector<unique_ptr<GameObject>>>> game_map;
        vector<unique_ptr<Shell>> fired_shells;
        vector<string> boardStates;

    public:
        BoardManager(const BoardManager &) = delete;
        BoardManager(vector<vector<vector<unique_ptr<GameObject>>>> map, int rows, int cols);
        ~BoardManager() = default;
        GameObject *getObjectAt(int x, int y) const;
        vector<vector<char>> objMapToCharMap();
        void updateBoard(size_t x, size_t y, char value);

        void printBoard();
        void writeBoardStates(string);

        void updateMap(unique_ptr<GameObject> obj, pair<int, int> new_pos);
        void moveFiredShells();
        bool isValidMove(Tank *, ActionRequest);
        void applyMoves(map<Tank *, ActionRequest>);

        pair<int, int> calculateNewPosition(pair<int, int> pos, Direction dir) const; // replaces wrap and returns new pos
        std::pair<int, int> getTankPosition(int player_id, int tank_idx) const;

        void processCollision(vector<GameObject *> &objects);
        void cleanupDestroyedObjects(pair<int, int>);
        void boardCleanup();
        void handleAllCollisions();
        template <typename T>
        unique_ptr<GameObject> extractObjectFromMap(T *object);
    };
}
#endif // BOARDMANAGER_H
