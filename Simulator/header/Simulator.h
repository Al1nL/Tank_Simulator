#ifndef SIMULATOR_H
#define SIMULATOR_H
#include <iostream>
#include <dirent.h>
#include <dlfcn.h>
#include <vector>
#include <string>
#include <filesystem>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>
#include <fstream>
#include <future>
#include <cmath>
#include "../header/AlgorithmRegistrar.h"
#include "../header/GameManagerRegistrar.h"
#include "../header/MapReader.h"

namespace fs = std::filesystem;
class GameResult;

enum Mode
{
  Comparative,
  Competition
};

struct Config
{
  Mode mode; // "-comparative" or "-competition"
  std::unordered_map<std::string, std::string> arguments;
  int num_threads = 1;
  bool verbose = false;

  ~Config()
  {
    arguments.clear(); // Ensure map is cleared
  }
};

class Simulator
{
  typedef void (*plugin_init_func)();

public:
  Simulator() {};
  Simulator(Simulator const &) = delete;
  Simulator &operator=(const Simulator &) = delete;
  ~Simulator();
  bool init(int argc, char *argv[]);
  void run();

private:
  Config config_;
  vector<GameMapInfo> mapInfo_;
  MapReader reader = MapReader();

  bool loadAllFromDirectory(const std::string &dirPath, bool isGameManager);

  // Initialization helpers
  bool initialize(const Config &config);
  bool initializeComparativeMode();
  bool initializeCompetitionMode();
  int computeThreadCount() const;

  // Dynamic loading functions
  bool loadAlgorithm(const std::string &path);
  bool loadGameManager(const std::string &path);
  bool loadAllMaps(const std::string &path);

  // Mode-specific execution
  void runComparative();
  void runCompetition();

  void printUsage(const std::string &error_msg = "", const std::vector<std::string> &invalid_args = {});
  Config parseArguments(int argc, char *argv[]);
  std::string generateOutputFilename(const std::string &folder, const std::string &prefix);
  std::string getBaseName(const std::string &filename);
  void logResults(std::unordered_map<std::string, std::vector<std::string>>, std::ofstream &);
  std::string getGameResultMsg(const GameResult &result, size_t max_steps);
  // mode helper functions
  GameResult runSingleComparativeGame(int manager_num);
  GameResult runSingleCompetitionGame(size_t map_idx, size_t algo1_idx, size_t algo2_idx);
};

#endif // SIMULATOR_H
