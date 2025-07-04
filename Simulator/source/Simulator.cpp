#include "../header/Simulator.h"
#include "../header/AlgorithmRegistrar.h"

bool Simulator::initGame(const std::string& folderPath){
    return loadAlgorithm(folderPath);
}

bool Simulator::loadAlgorithm(const std::string& folderPath) {
     void* handle = dlopen(folderPath.c_str(), RTLD_NOW);
        if (!handle) {
            std::cerr << "Failed to load " << folderPath << ": " << dlerror() << std::endl;
            return false;
        }

        // Get the registrar instance
        auto& registrar = AlgorithmRegistrar::getAlgorithmRegistrar();
        
        // Create a new entry for this algorithm
        registrar.createAlgorithmFactoryEntry(folderPath);

        // Load factory functions directly
        typedef PlayerFactory (*get_player_factory)();
        typedef TankAlgorithmFactory (*get_tank_factory)();

        // Get Player factory
        get_player_factory player_factory_func = (get_player_factory)dlsym(handle, "PlayerRegistration");
        if (const char* error = dlerror()) {
            std::cerr << "Failed to find Player factory: " << error << std::endl;
            registrar.removeLast();
            dlclose(handle);
            return false;
        }

        // Get TankAlgorithm factory
        dlerror(); // Clear previous error
        get_tank_factory tank_factory_func = (get_tank_factory)dlsym(handle, "TankAlgorithmRegistration");
        if (const char* error = dlerror()) {
            std::cerr << "Failed to find TankAlgorithm factory: " << error << std::endl;
            registrar.removeLast();
            dlclose(handle);
            return false;
        }

        // Register the factories
        registrar.addPlayerFactoryToLastEntry(player_factory_func());
        registrar.addTankAlgorithmFactoryToLastEntry(tank_factory_func());

        // Validate the registration
        try {
            registrar.validateLastRegistration();
            std::cout << "Successfully loaded algorithm from " << folderPath << std::endl;
            return true;
        } catch (const AlgorithmRegistrar::BadRegistrationException& e) {
            std::cerr << "Bad registration for " << folderPath << ":\n"
                      << "Has name: " << e.hasName << "\n"
                      << "Has Player factory: " << e.hasPlayerFactory << "\n"
                      << "Has TankAlgorithm factory: " << e.hasTankAlgorithmFactory << std::endl;
            registrar.removeLast();
            dlclose(handle);
            return false;
        }
    }