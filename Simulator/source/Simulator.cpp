#include "../header/Simulator.h"
#include "../header/AlgorithmRegistrar.h"

bool Simulator::initGame(const std::string& folderPath){
    return loadAlgorithm(folderPath);
}

bool Simulator::loadAlgorithm(const std::string& folderPath) {

    auto& registrar = AlgorithmRegistrar::getAlgorithmRegistrar();

    registrar.createAlgorithmFactoryEntry(folderPath);

    void* handle = dlopen(folderPath.c_str(), RTLD_LAZY | RTLD_GLOBAL);
    if (!handle) {
        std::cerr << "Failed to load " << folderPath << ": " << dlerror() << std::endl;
        return false;
    }
   
    // Clear any existing errors
    dlerror();

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