#ifndef SIMULATOR_H
#define SIMULATOR_H
#include <iostream>
#include <dirent.h>
#include <dlfcn.h>
#include <vector>
#include <string>
#include <filesystem>
#include <unordered_map>
#include <algorithm>
struct ProgramArguments {
        std::string mode;  // "-comparative" or "-competition"
        std::unordered_map<std::string, std::string> arguments;
        int num_threads = 1;
        bool verbose = false;
    };

class Simulator{
    ProgramArguments args;
  typedef void (*plugin_init_func)();
  bool loadAlgorithm(const std::string& folderPath);




    void printUsage(const std::string& error_msg = "", const std::vector<std::string>& invalid_args = {});
    int runComparativeMode();
    int runCompetitionMode();
    ProgramArguments parseArguments(int argc, char* argv[]);

  public:
    Simulator(int argc, char* argv[])
    {
        args = parseArguments(argc, argv);
    };

    Simulator(Simulator const&) = delete;
    Simulator& operator=(const Simulator&) = delete;
    ~Simulator() {}
    bool initGame(const std::string& folderPath);

    int run();

};



#endif //SIMULATOR_H
