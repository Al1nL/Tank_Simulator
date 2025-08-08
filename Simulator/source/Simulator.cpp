#include "../header/Simulator.h"

bool Simulator::initGame(const std::vector<std::string> &folderPath)
{
    return loadGameManager(folderPath[1]) && loadAlgorithm(folderPath[0]);
    // TODO:need to use initialize() to set up the config from the args given.
}

bool Simulator::initialize(const Config &config)
{
    config_ = config;

    // Validate all paths first
    if (!validate_paths())
    {
        return false;
    }
    try
    {
        // Load required libraries based on mode
        if (config_.mode == Comparative)
        {
            return initializeComparativeMode();
        }
        else
        {
            return initializeCompetitionMode();
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << "Initialization failed: " << e.what() << std::endl;
        return false;
    }
}

bool Simulator::initializeComparativeMode()
{
    // Load the two algorithms
    if (!loadAlgorithm(config_.algorithm1) || !loadAlgorithm(config_.algorithm2))
    {
        return false;
    }

    // Load all game managers from the folder
    bool found_valid_gm = false;
    for (const auto &entry : fs::directory_iterator(config_.game_managers_folder))
    {
        if (entry.path().extension() == ".so")
        {
            if (loadGameManager(entry.path().string()))
            {
                found_valid_gm = true;
            }
        }
    }

    if (!found_valid_gm)
    {
        std::cerr << "Error: No valid game managers found in "
                  << config_.game_managers_folder << std::endl;
        return false;
    }

    return true;
}

bool Simulator::initializeCompetitionMode()
{
    // Load the game manager
    if (!loadGameManager(config_.game_manager))
    {
        return false;
    }

    // Load all algorithms from the folder
    size_t algorithm_count = 0;
    for (const auto &entry : fs::directory_iterator(config_.algorithms_folder))
    {
        if (entry.path().extension() == ".so")
        {
            if (loadAlgorithm(entry.path().string()))
            {
                algorithm_count++;
            }
        }
    }

    if (algorithm_count < 2)
    {
        std::cerr << "Error: Need at least 2 valid algorithms for competition, found "
                  << algorithm_count << std::endl;
        return false;
    }

    return true;
}

bool Simulator::initialize(const Config &config)
{
    config_ = config;

    if (!validate_paths())
    {
        return false;
    }

    try
    {
        return (config_.mode == Comparative) ? initializeComparativeMode()
                                             : initializeCompetitionMode();
    }
    catch (const std::exception &e)
    {
        std::cerr << "Initialization failed: " << e.what() << std::endl;
        return false;
    }
}

bool Simulator::initializeComparativeMode()
{
    if (!loadAlgorithm(config_.algorithm1) || !loadAlgorithm(config_.algorithm2))
    {
        return false;
    }

    bool found_valid_gm = false;
    for (const auto &entry : fs::directory_iterator(config_.game_managers_folder))
    {
        if (entry.path().extension() == ".so" && loadGameManager(entry.path().string()))
        {
            found_valid_gm = true;
        }
    }

    if (!found_valid_gm)
    {
        std::cerr << "Error: No valid game managers found in "
                  << config_.game_managers_folder << std::endl;
        return false;
    }
    return true;
}

bool Simulator::initializeCompetitionMode()
{
    if (!loadGameManager(config_.game_manager))
    {
        return false;
    }

    size_t algorithm_count = 0;
    for (const auto &entry : fs::directory_iterator(config_.algorithms_folder))
    {
        if (entry.path().extension() == ".so" && loadAlgorithm(entry.path().string()))
        {
            algorithm_count++;
        }
    }

    if (algorithm_count < 2)
    {
        std::cerr << "Error: Need at least 2 valid algorithms for competition, found "
                  << algorithm_count << std::endl;
        return false;
    }
    return true;
}

bool Simulator::loadGameManager(const std::string &path)
{
    auto &registrar = GameManagerRegistrar::getGameManagerRegistrar();
    registrar.createGameManagerEntry(path);

    void *handle = dlopen(path.c_str(), RTLD_LAZY | RTLD_GLOBAL);
    if (!handle)
    {
        std::cerr << "Failed to load GameManager " << path << ": " << dlerror() << std::endl;
        registrar.removeLast();
        return false;
    }

    dlerror(); // Clear errors

    try
    {
        registrar.validateLastRegistration();
        std::cout << "Successfully loaded GameManager from " << path << std::endl;
        return true;
    }
    catch (const GameManagerRegistrar::BadRegistrationException &e)
    {
        std::cerr << "Bad GameManager registration for " << path << ":\n"
                  << "Has name: " << e.hasName << "\n"
                  << "Has factory: " << e.hasFactory << std::endl;
        registrar.removeLast();
        dlclose(handle);
        return false;
    }
}

bool Simulator::loadAlgorithm(const std::string &path)
{
    auto &registrar = AlgorithmRegistrar::getAlgorithmRegistrar();
    registrar.createAlgorithmFactoryEntry(path);

    void *handle = dlopen(path.c_str(), RTLD_LAZY | RTLD_GLOBAL);
    if (!handle)
    {
        std::cerr << "Failed to load Algorithm " << path << ": " << dlerror() << std::endl;
        registrar.removeLast();
        return false;
    }

    dlerror(); // Clear errors

    try
    {
        registrar.validateLastRegistration();
        std::cout << "Successfully loaded Algorithm from " << path << std::endl;
        return true;
    }
    catch (const AlgorithmRegistrar::BadRegistrationException &e)
    {
        std::cerr << "Bad Algorithm registration for " << path << ":\n"
                  << "Has name: " << e.hasName << "\n"
                  << "Has Player factory: " << e.hasPlayerFactory << "\n"
                  << "Has TankAlgorithm factory: " << e.hasTankAlgorithmFactory << std::endl;
        registrar.removeLast();
        dlclose(handle);
        return false;
    }
}

bool Simulator::validate_paths() const
{
    auto check_file = [](const std::string &path, const std::string &desc)
    {
        if (!fs::exists(path))
        {
            std::cerr << "Error: " << desc << " file not found: " << path << std::endl;
            return false;
        }
        return true;
    };

    auto check_dir = [](const std::string &path, const std::string &desc)
    {
        if (!fs::exists(path) || !fs::is_directory(path))
        {
            std::cerr << "Error: " << desc << " directory not found: " << path << std::endl;
            return false;
        }
        return true;
    };

    if (config_.mode == Comparative)
    {
        return check_file(config_.game_map, "Game map") &&
               check_file(config_.algorithm1, "Algorithm 1") &&
               check_file(config_.algorithm2, "Algorithm 2") &&
               check_dir(config_.game_managers_folder, "Game managers");
    }
    else
    {
        return check_file(config_.game_manager, "Game manager") &&
               check_dir(config_.game_maps_folder, "Game maps") &&
               check_dir(config_.algorithms_folder, "Algorithms");
    }
}

void Simulator::print_usage(const std::string &error) const
{
    if (!error.empty())
    {
        std::cerr << "Error: " << error << "\n\n";
    }

    std::cerr << "Usage:\n"
              << "Comparative mode:\n"
              << "  ./simulator -comparative game_map=<file> game_managers_folder=<dir> "
              << "algorithm1=<file> algorithm2=<file> [num_threads=<n>] [-verbose]\n"
              << "Competition mode:\n"
              << "  ./simulator -competition game_maps_folder=<dir> algorithms_folder=<dir> "
              << "game_manager=<file> [num_threads=<n>] [-verbose]\n";
}