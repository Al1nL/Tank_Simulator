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
#include "../header/AlgorithmRegistrar.h"
#include "../header/GameManagerRegistrar.h"
namespace fs = std::filesystem;

struct GameResult {
    std::string manager_name;
    std::string final_state;
    int final_round;
    std::string game_result;  // e.g., "Player 1 won by elimination"
};

class Simulator
{
  typedef void (*plugin_init_func)();
public:
  enum Mode
  {
    Comparative,
    Competition
  };

  struct Config
  {
    Mode mode;
    std::string game_map;
    std::string game_maps_folder;
    std::string game_managers_folder;
    std::string algorithms_folder;
    std::string algorithm1;
    std::string algorithm2;
    std::string game_manager;
    int num_threads = 1;
    bool verbose = false;
  };

  Simulator() {};
  Simulator(Simulator const &) = delete;
  Simulator &operator=(const Simulator &) = delete;
  ~Simulator() {}
  bool initGame(const std::vector<std::string> &folderPath);
  void run();

private:
  Config config_;

  bool loadAllFromDirectory(const std::string &dirPath, bool isGameManager);

  // Initialization helpers
  bool validate_paths() const;
  bool initialize(const Config &config);
  bool initializeComparativeMode();
  bool initializeCompetitionMode();
  // Dynamic loading functions
  bool loadAlgorithm(const std::string &path);
  bool loadGameManager(const std::string &path);

  // Mode-specific execution
  void runComparative();
  void runCompetition();
  std::string generateOutputFilename(const std::string &folder, const std::string &prefix);
};

#endif // SIMULATOR_H
