#include <iostream>
#include <string>
#include "./Simulator/header/AlgorithmRegistrar.h"
// #include "./common/TankAlgorithmRegistration.h"
// #include "./common/PlayerRegistration.h"
// #include "./common/GameManagerRegistration.h"
// #include "./Algorithm/header/Player_212535058_324022904.h"
// #include "./Algorithm/header/TankAlgorithm_212535058_324022904.h"
#include "./Simulator/header/Simulator.h"
// void fake_dlopen_algo_so(const std::string& name) {
//     using namespace Algorithm_212535058_324022904;
//     REGISTER_TANK_ALGORITHM(TankAlgorithm_212535058_324022904);
//     if(name != "Bad") {
//         REGISTER_PLAYER(Player_212535058_324022904);
//     }
// }

int main(int argc, char** argv) {

    try {
        Simulator simulator(argc,argv);  // Create instance with parsed args

        simulator.run();

        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }


    return 0;
}