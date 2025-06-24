#include <iostream>
#include "./simulator/header/AlgorithmRegistrar.h"
#include "./common/TankAlgorithmRegistration.h"
#include "./common/PlayerRegistration.h"
#include "./common/GameManagerRegistration.h"
#include "./algorithm/header/Player_212535058_324022904.h"
#include "./algorithm/header/TankAlgorithm_212535058_324022904.h"

void fake_dlopen_algo_so(const std::string& name) {
    using namespace Algorithm_212535058_324022904;
    REGISTER_TANK_ALGORITHM(TankAlgorithm_212535058_324022904);
    if(name != "Bad") {
        REGISTER_PLAYER(Player_212535058_324022904);
    }
}

int main() {
    //-----
    // NOTE:
    // This should NOT be in your main!!!
    // It should be somewhere in your Simulator class
    //-----
    // dlopen - we have the name of the so
    // let's assume that the so name is "Algorithm_212535058_324022904.so"
    // strip the so suffix, so name is "Algorithm_212535058_324022904"
    std::vector<std::string> names = {"Algorithm_212535058_324022904", "Bad", "Algorithm_123456781"};
    auto& registrar = AlgorithmRegistrar::getAlgorithmRegistrar();
    for(const auto& name: names) {
        registrar.createAlgorithmFactoryEntry(name);
        // TODO - actual dlopen, also: check dlopen for failure
        fake_dlopen_algo_so(name);
        try {
            registrar.validateLastRegistration();
        }
        catch(AlgorithmRegistrar::BadRegistrationException& e) {
            // TODO: report according to requirements
            std::cout << "---------------------------------" << std::endl;
            std::cout << "BadRegistrationException for: " << name << std::endl;
            std::cout << "Name as registered: " << e.name << std::endl;
            std::cout << "Has tank algorithm factory? " << std::boolalpha << e.hasTankAlgorithmFactory << std::endl;
            std::cout << "Has Player factory? " << std::boolalpha << e.hasPlayerFactory << std::endl;
            std::cout << "---------------------------------" << std::endl;
            registrar.removeLast();
        }
    }
    // loop over all factories to do something
    for(const auto& algo: AlgorithmRegistrar::getAlgorithmRegistrar()) {
        auto algorithm = algo.createTankAlgorithm(1, 0);
        std::cout << algo.name() << ": " << static_cast<int>(algorithm->getAction()) << std::endl;
    }
    AlgorithmRegistrar::getAlgorithmRegistrar().clear();
    // dlclose
    // TODO handle dlclose
}