#include <iostream>
#include <string>
#include "./Simulator/header/AlgorithmRegistrar.h"
#include "./Simulator/header/GameManagerRegistrar.h"
#include "./Simulator/header/Simulator.h"

int main(int argc, char **argv)
{
    const std::string soPath = argv[1],
                      gameManagerPath = argv[2];
    Simulator sim;
    if (sim.initGame(argc, argv))
        sim.run();
    return 0;
}