#include <iostream>
#include <string>
#include "./header/AlgorithmRegistrar.h"
#include "./header/GameManagerRegistrar.h"
#include "./header/Simulator.h"

int main(int argc, char **argv)
{
    const std::string soPath = argv[1],
                      gameManagerPath = argv[2];
    Simulator sim;
    if (sim.initGame(argc, argv))
        sim.run();
    return 0;
}