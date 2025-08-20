#include "./header/Simulator.h"

int main(int argc, char **argv)
{
    const std::string soPath = argv[1],
                      gameManagerPath = argv[2];
    Simulator sim;
    if (sim.init(argc, argv))
        sim.run();
    return 0;
}