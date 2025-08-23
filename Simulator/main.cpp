#include "./header/Simulator.h"

int main(int argc, char **argv)
{
    Simulator sim;
    if (sim.init(argc, argv))
        sim.run();
    return 0;
}