#ifndef MAPREADER_H
#define MAPREADER_H

#include <vector>
#include <string>
#include <memory>
#include <utility>
#include <fstream>
#include <algorithm>
#include "../../UserCommon/header/BoardSatelliteView.h"

using std::string, std::vector, std::shared_ptr, std::pair, std::ifstream, std::ofstream, std::move, std::to_string, std::exception, std::getline, std::invalid_argument, std::runtime_error, std::stoi, std::count_if;

namespace UserCommon__212535058_324022904 {
  class BoardSatelliteView;  // Forward declaration
}

struct GameMapInfo {
  std::unique_ptr<SatelliteView> map;
  string name;
  size_t width;
  size_t height;
  size_t max_steps;
  size_t num_shells;

  GameMapInfo() : map(nullptr), width(0), height(0), max_steps(0), num_shells(0) {}

  // Disable copying
  GameMapInfo(const GameMapInfo&) = delete;
  GameMapInfo& operator=(const GameMapInfo&) = delete;

  GameMapInfo(GameMapInfo&&) = default;
    GameMapInfo& operator=(GameMapInfo&&) = default;
  ~GameMapInfo() = default;
};

class MapReader{
private:
size_t rows_=0;
size_t cols_=0;
size_t max_steps_=0;

  void parseMetadata(ifstream &file, bool &hasErrors, ofstream &errorLog,GameMapInfo&);
  bool tryParseMetadata(const string &line, const string &key, size_t &value, bool &hasErrors, ofstream &errorLog);
  bool hasAllMetadata(GameMapInfo&) const;
  void processMapRows(ifstream &file, bool &hasErrors, ofstream &errorLog,GameMapInfo&);
  void checkExcessColumns(const string &line, size_t row, bool &hasErrors, ofstream &errorLog);
  void checkExcessRows(ifstream &file, bool &hasErrors, ofstream &errorLog);
  void processRowCells(const string &line, size_t row, vector<vector<char>> &map, bool &hasErrors, ofstream &errorLog);
  public:
  size_t getMaxSteps() const { return max_steps_; }
  GameMapInfo readBoard(const string &filePath);
  string gameStateToString(const SatelliteView &);
  };
#endif //MAPREADER_H
