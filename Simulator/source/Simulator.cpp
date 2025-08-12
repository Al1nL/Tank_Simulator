#include "../header/Simulator.h"
#include "../header/AlgorithmRegistrar.h"
#include "../header/ThreadPool.h"
#include "../../common/GameResult.h"
#include "../header/MapReader.h"

namespace fs = std::filesystem;
bool Simulator::initGame_(int argc, char *argv[])
{
    // Parse command line arguments
    config_ = parseArguments(argc, argv);

    // Initialize the game based on the mode
    return initialize(config_);
}
bool Simulator::initGame(const std::vector<std::string> &folderPath)
{

    return loadGameManager(folderPath[1]) && loadAlgorithm(folderPath[0]);
    // TODO:need to use initialize() to set up the config from the args given.
}

bool Simulator::initializeComparativeMode()
{

    // Load the two algorithms
    if (!loadAlgorithm(config_.arguments["algorithm1"]) || !loadAlgorithm(config_.arguments["algorithm2"]))
    {
        return false;
    }

    if (!loadAllMaps(config_.arguments["game_map"]))
    {
        return false;
    }
    // Load all game managers from the folder
    bool found_valid_gm = false;
    for (const auto &entry : fs::directory_iterator(config_.arguments["game_managers_folder"]))
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
                  << config_.arguments["game_managers_folder"] << std::endl;
        return false;
    }

    return true;
}

bool Simulator::initializeCompetitionMode()
{
    // Load the game manager
    if (!loadGameManager(config_.arguments["game_manager"]))
    {
        return false;
    }

    // Load all algorithms from the folder
    size_t algorithm_count = 0;
    for (const auto &entry : fs::directory_iterator(config_.arguments["algorithms_folder"]))
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

bool Simulator::loadGameManager(const std::string &path)
{
    auto &registrar = GameManagerRegistrar::getGameManagerRegistrar();
    registrar.createGameManagerEntry(getBaseName(path));

    void *handle = dlopen(path.c_str(), RTLD_LAZY | RTLD_GLOBAL);
    if (!handle)
    {
        std::cerr << "Failed to load GameManager " << path << ": " << dlerror() << std::endl;
        registrar.removeLast();
        return false;
    }
    registrar.setHandleToLastEntry(handle);
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
        //dlclose(handle);
        return false;
    }
}

bool Simulator::loadAlgorithm(const std::string &path)
{
    auto &registrar = AlgorithmRegistrar::getAlgorithmRegistrar();
    std::string baseName = getBaseName(path);

    // Check if we already loaded this exact file
    bool isRegistred = false;
    for (const auto& entry : registrar) {
        if (entry.name() == baseName) {
            // Create new entry with same factories
            registrar.createAlgorithmFactoryEntry(baseName);
            TankAlgorithmFactory tank_dup = [factory = entry.getTankAlgorithmFactory()]
                                          (int p, int t) { return factory(p, t); };
            registrar.addTankAlgorithmFactoryToLastEntry(std::move(tank_dup));
            registrar.addPlayerFactoryToLastEntry(entry.getPlayerFactory());
            isRegistred = true;
            break;
        }
    }

    // Otherwise, load it fresh
    if(!isRegistred){
        registrar.createAlgorithmFactoryEntry(baseName);
        void *handle = dlopen(path.c_str(), RTLD_LAZY | RTLD_LOCAL);
        if (!handle)
        {
            std::cerr << "Failed to load Algorithm " << path << ": " << dlerror() << std::endl;
            registrar.removeLast();
            return false;
        }
    }
    //registrar.setHandleToLastEntry(handle);
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
        // dlclose(handle);
        return false;
    }
}

bool Simulator::loadAllMaps(const std::string &path)
{
    if (!fs::is_directory(path))
    {
        auto map = reader.readBoard(path);
        if (!map.map)
            return false;
        map.name = path;
        mapInfo_.push_back(std::move(map));
        return true;
    }
    for (const auto &entry : fs::directory_iterator(path))
    {
        if (entry.path().extension() == ".txt")
        {
            auto map = reader.readBoard(entry.path().string());
            if (!map.map)
                return false;
            map.name = entry.path().filename().string();
            mapInfo_.push_back(std::move(map));
        }
    }
    return !mapInfo_.empty();
    // return true;
}

Config Simulator::parseArguments(int argc, char *argv[])
{
    Config args;
    std::vector<std::string> invalid_args;
    std::vector<std::string> required_args;

    if (argc < 2)
    {
        printUsage("Not enough arguments");
    }

    // Check mode
    string mode = argv[1];
    if (mode != "-comparative" && mode != "-competition")
    {
        printUsage("Invalid mode specified", {mode});
    }

    // Set required arguments based on mode
    if (mode == "-comparative")
    {
        args.mode = Comparative;
        required_args = {"game_map", "game_managers_folder", "algorithm1", "algorithm2"};
    }
    else
    {
        args.mode = Competition;
        required_args = {"game_maps_folder", "game_manager", "algorithms_folder"};
    }

    // Parse arguments
    for (int i = 2; i < argc; ++i)
    {
        std::string arg = argv[i];

        // Handle flags
        if (arg == "-verbose")
        {
            args.verbose = true;
            continue;
        }

        // Handle key=value pairs
        size_t eq_pos = arg.find('=');
        if (eq_pos == std::string::npos)
        {
            invalid_args.push_back(arg);
            continue;
        }

        std::string key = arg.substr(0, eq_pos);
        std::string value = arg.substr(eq_pos + 1);

        if (key == "num_threads")
        {
            try
            {
                args.num_threads = std::stoi(value);
                if (args.num_threads < 1)
                {
                    printUsage("num_threads must be positive");
                }
            }
            catch (...)
            {
                printUsage("Invalid num_threads value");
            }
        }
        else
        {
            args.arguments[key] = value;
        }
    }

    // Check for missing required arguments
    std::vector<std::string> missing_args;
    for (const auto &req : required_args)
    {
        if (args.arguments.find(req) == args.arguments.end())
        {
            missing_args.push_back(req);
        }
    }

    if (!missing_args.empty())
    {
        printUsage("Missing required arguments", missing_args);
    }

    if (!invalid_args.empty())
    {
        printUsage("Invalid arguments provided", invalid_args);
    }

    // Validate file paths
    try
    {
        if (args.mode == Comparative)
        {
            if (!fs::exists(args.arguments["game_map"]))
            {
                printUsage("Game map file does not exist");
            }
            if (!fs::is_directory(args.arguments["game_managers_folder"]))
            {
                printUsage("Game managers folder is invalid");
            }
            if (!fs::exists(args.arguments["algorithm1"]))
            {
                printUsage("Algorithm1 file does not exist");
            }
            if (!fs::exists(args.arguments["algorithm2"]))
            {
                printUsage("Algorithm2 file does not exist");
            }
        }
        else
        {
            if (!fs::is_directory(args.arguments["game_maps_folder"]))
            {
                printUsage("Game maps folder is invalid");
            }
            if (!fs::exists(args.arguments["game_manager"]))
            {
                printUsage("Game manager file does not exist");
            }
            if (!fs::is_directory(args.arguments["algorithms_folder"]))
            {
                printUsage("Algorithms folder is invalid");
            }
        }
    }
    catch (const fs::filesystem_error &)
    {
        printUsage("Filesystem error while validating paths");
    }

    return args;
}

void Simulator::printUsage(const std::string &error_msg,
                           const std::vector<std::string> &invalid_args)
{
    std::cerr << "Usage:\n";
    std::cerr << "Comparative mode:\n";
    std::cerr << "  ./simulator_<submitter_ids> -comparative game_map=<file> game_managers_folder=<dir> "
              << "algorithm1=<file> algorithm2=<file> [num_threads=<num>] [-verbose]\n";
    std::cerr << "Competition mode:\n";
    std::cerr << "  ./simulator_<submitter_ids> -competition game_maps_folder=<dir> game_manager=<file> "
              << "algorithms_folder=<dir> [num_threads=<num>] [-verbose]\n\n";

    if (!error_msg.empty())
    {
        std::cerr << "Error: " << error_msg << "\n";
    }
    if (!invalid_args.empty())
    {
        std::cerr << "Invalid arguments:";
        for (const auto &arg : invalid_args)
        {
            std::cerr << " " << arg;
        }
        std::cerr << "\n";
    }
    exit(1);
}

void Simulator::run()
{
    if (config_.mode == Comparative)
    {
        runComparative();
    }
    runCompetition();
}

void Simulator::runComparative()
{

    // Prepare output filename with timestamp
    auto now = std::chrono::system_clock::now();
    auto time_str = std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(
                                       now.time_since_epoch())
                                       .count());
    fs::path output_path = fs::path(config_.arguments["game_managers_folder"]) /
                           ("comparative_results_" + time_str + ".txt");

    // Try to open output file
    std::ofstream out_file;
    out_file.open(output_path);
    bool write_to_file = out_file.is_open();

    if (!write_to_file)
    {
        std::cerr << "Error: Could not create output file at " << output_path
                  << ". Results will be printed to screen instead.\n\n";
    }

    // Write header information
    auto &output = write_to_file ? out_file : std::cout;
    output << "game_map=" << config_.arguments["game_map"] << "\n";
    output << "algorithm1=" << config_.arguments["algorithm1"] << "\n";
    output << "algorithm2=" << config_.arguments["algorithm2"] << "\n\n";

    // Process game managers in parallel
    std::vector<std::pair<size_t, GameResult>> results;
    std::mutex results_mutex;

    // Create thread pool
    ThreadPool pool(config_.num_threads);

    // Get the number of loaded game managers
    auto &gm_registrar = GameManagerRegistrar::getGameManagerRegistrar();
    size_t num_managers = gm_registrar.count();

    for (size_t i = 0; i < num_managers; ++i)
    {
        pool.enqueue([this, i, &results, &results_mutex]()
                     {
            GameResult result = runSingleComparativeGame(i);
            if (result.winner!=-1) {  // Only add valid results
    std::cout << "Game " << i << " completed - Winner: " 
              << result.winner << std::endl;
    std::lock_guard<std::mutex> lock(results_mutex);
    results.emplace_back(std::make_pair(i, std::move(result)));
} else {
    std::cerr << "Game " << i << " returned invalid result" << std::endl;
}
            if (result.gameState) {  // Only add valid results
                std::lock_guard<std::mutex> lock(results_mutex);
                results.emplace_back(std::make_pair(i, std::move(result)));  // Store manager index with result
            } });
    }
    pool.waitAll();
    std::cout << "Completed " << results.size() << " games out of " << num_managers << std::endl;

    // Group results by outcome
    std::unordered_map<std::string, std::vector<std::string>> result_groups;
    for (const auto &[manager_idx, result] : results)
    {
        std::string key = std::to_string(result.winner) + "|" +
                          std::to_string(static_cast<int>(result.reason)) + "|" +
                          std::to_string(result.rounds);
                          //TODO: add key for game state
        result_groups[key].push_back(gm_registrar.getGameManager(manager_idx).getName());
    }

    logResults(result_groups, out_file); //::move(out_file));
    if (write_to_file)
    {
        out_file.close();
        std::cout << "Results written to: " << output_path << "\n";
    }
}

void Simulator::runCompetition() { return; }

std::string Simulator::getBaseName(const std::string &filename)
{
    // Find the last '.' in the string
    size_t last_dot = filename.find_last_of('.');

    // If no extension found, return the whole string
    if (last_dot == std::string::npos)
    {
        return filename;
    }

    // Return substring up to (but not including) the last '.'
    return filename.substr(0, last_dot);
}

GameResult Simulator::runSingleComparativeGame(int manager_number)
{
    try
    {
        auto &algo_registrar = AlgorithmRegistrar::getAlgorithmRegistrar();
        auto algo1_player_factory = algo_registrar.getAlgorithmAndPlayerFactory(0);
        auto algo2_player_factory = algo_registrar.getAlgorithmAndPlayerFactory(1);

        auto &gm_registrar = GameManagerRegistrar::getGameManagerRegistrar();

        auto game_manager_factory = gm_registrar.getGameManager(manager_number).getFactory();
        // if (!game_manager) {
        //     throw std::runtime_error("Failed to create game manager");
        // }
        if (!game_manager_factory)
        {
            return {};
        }

        auto game_manager = game_manager_factory(config_.verbose);
        if (!game_manager)
        {
            std::cerr << "Failed to create game manager instance for manager number "
                      << manager_number << std::endl;
            return {};
        }
        else
        {
            std::cout << "Using GameManager: " << gm_registrar.getGameManager(manager_number).getName() << std::endl;
        }

        auto &map = mapInfo_.at(0);
        auto player1 = algo1_player_factory.createPlayer(1, map.width, map.height, map.max_steps, map.num_shells);
        auto player2 = algo2_player_factory.createPlayer(2, map.width, map.height, map.max_steps, map.num_shells);
        
        bool check = (algo1_player_factory.getTankAlgorithmFactory().target_type() == algo2_player_factory.getTankAlgorithmFactory().target_type());
        // Run the game
        GameResult result = game_manager->run(map.width, map.height,
                                              *map.map, map.name, map.max_steps, map.num_shells,
                                              *player1, "", *player2, "",
                                              algo1_player_factory.getTankAlgorithmFactory(), algo2_player_factory.getTankAlgorithmFactory());

        if (result.gameState == nullptr)
        {
            std::cerr << "Warning: Game returned null result!" << std::endl;
        }
        return result;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error running game for manager number " << manager_number
                  << ": " << e.what() << std::endl;
        return {};
    }
}

void Simulator::logResults(std::unordered_map<std::string, std::vector<std::string>> results, std::ofstream &output)
{
    // Output results
    for (const auto &[key, managers] : results)
    {
        // Parse key components
        size_t pos1 = key.find('|');
        size_t pos2 = key.find('|', pos1 + 1);

        int winner = std::stoi(key.substr(0, pos1));
        GameResult::Reason reason = static_cast<GameResult::Reason>(std::stoi(key.substr(pos1 + 1, pos2 - pos1 - 1)));
        size_t rounds = std::stoi(key.substr(pos2 + 1));

        // Output manager names
        for (size_t i = 0; i < managers.size(); ++i)
        {
            if (i != 0)
                output << ", ";
            output << managers[i];
        }
        output << "\n";

        // Output result details
        output << "Winner: " << (winner == 0 ? "Tie" : ("Player " + std::to_string(winner))) << "\n";
        output << "Reason: ";
        switch (reason)
        {
        case GameResult::ALL_TANKS_DEAD:
            output << "All tanks destroyed";
            break;
        case GameResult::MAX_STEPS:
            output << "Maximum steps reached";
            break;
        case GameResult::ZERO_SHELLS:
            output << "No shells remaining";
            break;
        }
        output << "\n";
        output << "Rounds: " << rounds << "\n\n";
        output << "Game State:\n";
        // TODO: Add game state output
    }
}


Simulator::~Simulator()
{
    // Clean up loaded libraries
    GameManagerRegistrar::getGameManagerRegistrar().cleanup();
    AlgorithmRegistrar::getAlgorithmRegistrar().cleanup();
    mapInfo_.clear();
}