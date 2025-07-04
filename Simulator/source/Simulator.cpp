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

        // Try to find and call registration functions
        typedef void (*register_func)();
        
        // Try to register TankAlgorithm
        register_func registerTank = (register_func)dlsym(handle, "TankAlgorithm_212535058_324022904");
        if (const char* error = dlerror()) {
            std::cerr << "Failed to find TankAlgorithm registration: " << error << std::endl;
            registrar.removeLast();
            dlclose(handle);
            return false;
        }
        registerTank();

        // Try to register Player
        register_func registerPlayer = (register_func)dlsym(handle, "register_me_Player_212535058_324022904");
        if (const char* error = dlerror()) {
            std::cerr << "Failed to find Player registration: " << error << std::endl;
            registrar.removeLast();
            dlclose(handle);
            return false;
        }
        registerPlayer();

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