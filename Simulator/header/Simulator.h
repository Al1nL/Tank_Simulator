#ifndef SIMULATOR_H
#define SIMULATOR_H
#include <iostream>
#include <dirent.h>
#include <dlfcn.h>
#include <vector>
#include <string>


class Simulator{

  Simulator() = default;
  Simulator(Simulator const&) = delete;
  Simulator& operator=(const Simulator&) = delete;
  ~Simulator() {}

  typedef void (*plugin_init_func)();
  void loadSharedObjectsFromFolder(const std::string& folderPath);
};



#endif //SIMULATOR_H
