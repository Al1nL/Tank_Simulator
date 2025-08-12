// needed for player and game manager

#ifndef BOARDSATELLITEVIEW_H
#define BOARDSATELLITEVIEW_H
#include <vector>
#include <string>

#include "../../common/SatelliteView.h"

using std::pair, std::vector, std::move;
namespace UserCommon__212535058_324022904
{
    class BoardSatelliteView : public SatelliteView
    {
        int cols;
        int rows;
        vector<vector<char>> map;
        pair<int, int> requestingTankPos={-1,-1}; // Position of tank making the request

    public:
        BoardSatelliteView(int r, int c, vector<vector<char>> m) : cols(c), rows(r), map(move(m)) {}
        void update(vector<vector<char>> m)
        {
            std::copy(m.begin(), m.end(), map.begin()); // copy
        }
        void setObjectAt(int row, int col, char c) { map[row][col] = c; }
        char getObjectAt(size_t x, size_t y) const override
        {

            // Convert size_t to int for comparison with tank positions
            int posX = static_cast<int>(x);
            int posY = static_cast<int>(y);

            // Check if location is outside the battlefield
            if (rows < posX || cols < posY)
            {
                return '&';
            }

            // Check if this is the requesting tank's position
            if (posX == requestingTankPos.first && posY == requestingTankPos.second)
            {
                return '%';
            }

            // Empty space
            return map[posX][posY];
        }
        void setRequestingTankPos(pair<int, int> pos) { requestingTankPos = pos; }
    };
}
#endif // BOARDSATELLITEVIEW_H
