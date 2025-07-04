#ifndef SIMULATOR_H
#define SIMULATOR_H
#include <iostream>
#include <dirent.h>
#include <dlfcn.h>
#include <vector>
#include <string>


class Simulator{
  typedef void (*plugin_init_func)();
  bool loadAlgorithm(const std::string& folderPath);

  public:
    Simulator() {};
    Simulator(Simulator const&) = delete;
    Simulator& operator=(const Simulator&) = delete;
    ~Simulator() {}
    bool initGame(const std::string& folderPath);
};



#endif //SIMULATOR_H
