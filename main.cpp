#include <iostream>
#include <string>
#include "./Simulator/header/AlgorithmRegistrar.h"
#include "./Simulator/header/GameManagerRegistrar.h"
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
    const std::string soPath = argv[1],
        gameManagerPath = argv[2];
    Simulator* sim = new Simulator();
    std::vector<std::string> so_args;
    so_args.push_back(soPath);
    so_args.push_back(gameManagerPath);
    if (sim->initGame(so_args)) {
        auto& registrar = AlgorithmRegistrar::getAlgorithmRegistrar();
        
        // Example usage of the loaded algorithm
        if (registrar.count() > 0) {
            const auto& algorithm = *registrar.begin();
            
            // Create a player instance
            auto player = algorithm.createPlayer(1, 10, 10, 100, 50);
            
            // Create a tank algorithm instance
            auto tankAlgorithm = algorithm.createTankAlgorithm(1, 1);
            
            std::cout << "Successfully created Player and TankAlgorithm instances" << std::endl;
        }
    } else {
        std::cerr << "Failed to load algorithm from " << soPath << std::to_string(argc) << std::endl;
        return 1;
    }

    return 0;
}