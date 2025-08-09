#include "../header/Simulator.h"
#include "../header/AlgorithmRegistrar.h"
#include "../header/ThreadPool.h"

namespace fs = std::filesystem;

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

ProgramArguments Simulator::parseArguments(int argc, char* argv[]) {
    ProgramArguments args;
    std::vector<std::string> invalid_args;
    std::vector<std::string> required_args;

    if (argc < 2) {
        printUsage("Not enough arguments");
    }

    // Check mode
    args.mode = argv[1];
    if (args.mode != "-comparative" && args.mode != "-competition") {
        printUsage("Invalid mode specified", {args.mode});
    }

    // Set required arguments based on mode
    if (args.mode == "-comparative") {
        required_args = {"game_map", "game_managers_folder", "algorithm1", "algorithm2"};
    } else {
        required_args = {"game_maps_folder", "game_manager", "algorithms_folder"};
    }

    // Parse arguments
    for (int i = 2; i < argc; ++i) {
        std::string arg = argv[i];

        // Handle flags
        if (arg == "-verbose") {
            args.verbose = true;
            continue;
        }

        // Handle key=value pairs
        size_t eq_pos = arg.find('=');
        if (eq_pos == std::string::npos) {
            invalid_args.push_back(arg);
            continue;
        }

        std::string key = arg.substr(0, eq_pos);
        std::string value = arg.substr(eq_pos + 1);

        if (key == "num_threads") {
            try {
                args.num_threads = std::stoi(value);
                if (args.num_threads < 1) {
                    printUsage("num_threads must be positive");
                }
            } catch (...) {
                printUsage("Invalid num_threads value");
            }
        } else {
            args.arguments[key] = value;
        }
    }

    // Check for missing required arguments
    std::vector<std::string> missing_args;
    for (const auto& req : required_args) {
        if (args.arguments.find(req) == args.arguments.end()) {
            missing_args.push_back(req);
        }
    }

    if (!missing_args.empty()) {
        printUsage("Missing required arguments", missing_args);
    }

    if (!invalid_args.empty()) {
        printUsage("Invalid arguments provided", invalid_args);
    }

    // Validate file paths
    try {
        if (args.mode == "-comparative") {
            if (!fs::exists(args.arguments["game_map"])) {
                printUsage("Game map file does not exist");
            }
            if (!fs::is_directory(args.arguments["game_managers_folder"])) {
                printUsage("Game managers folder is invalid");
            }
            if (!fs::exists(args.arguments["algorithm1"])) {
                printUsage("Algorithm1 file does not exist");
            }
            if (!fs::exists(args.arguments["algorithm2"])) {
                printUsage("Algorithm2 file does not exist");
            }
        } else {
            if (!fs::is_directory(args.arguments["game_maps_folder"])) {
                printUsage("Game maps folder is invalid");
            }
            if (!fs::exists(args.arguments["game_manager"])) {
                printUsage("Game manager file does not exist");
            }
            if (!fs::is_directory(args.arguments["algorithms_folder"])) {
                printUsage("Algorithms folder is invalid");
            }
        }
    } catch (const fs::filesystem_error&) {
        printUsage("Filesystem error while validating paths");
    }

    return args;
}

void Simulator::printUsage(const std::string& error_msg,
               const std::vector<std::string>& invalid_args) {
    std::cerr << "Usage:\n";
    std::cerr << "Comparative mode:\n";
    std::cerr << "  ./simulator_<submitter_ids> -comparative game_map=<file> game_managers_folder=<dir> "
              << "algorithm1=<file> algorithm2=<file> [num_threads=<num>] [-verbose]\n";
    std::cerr << "Competition mode:\n";
    std::cerr << "  ./simulator_<submitter_ids> -competition game_maps_folder=<dir> game_manager=<file> "
              << "algorithms_folder=<dir> [num_threads=<num>] [-verbose]\n\n";

    if (!error_msg.empty()) {
        std::cerr << "Error: " << error_msg << "\n";
    }
    if (!invalid_args.empty()) {
        std::cerr << "Invalid arguments:";
        for (const auto& arg : invalid_args) {
            std::cerr << " " << arg;
        }
        std::cerr << "\n";
    }
    exit(1);
}

int Simulator::run()
{
    if (args.mode == "-comparative") {
        return runComparativeMode();
    }
    return runCompetitionMode();
}

int Simulator::runComparativeMode(){
    // Get all game manager files
    std::vector<fs::path> game_managers;
    for (const auto& entry : fs::directory_iterator(args.arguments["game_managers_folder"])) {
        if (entry.path().extension() == ".so") {
            game_managers.push_back(entry.path());
        }
    }

    if (game_managers.empty()) {
        throw std::runtime_error("No game manager files found in directory");
    }

    // Prepare output filename with timestamp
    auto now = std::chrono::system_clock::now();
    auto time_str = std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()).count());
    fs::path output_path = fs::path(args.arguments["game_managers_folder"]) /
                          ("comparative_results_" + time_str + ".txt");

    // Try to open output file
    std::ofstream out_file;
    out_file.open(output_path);
    bool write_to_file = out_file.is_open();

    if (!write_to_file) {
        std::cerr << "Error: Could not create output file at " << output_path
                  << ". Results will be printed to screen instead.\n\n";
    }

    // Write header information
    auto& output = write_to_file ? out_file : std::cout;
    output << "game_map=" << args.arguments["game_map"] << "\n";
    output << "algorithm1=" << args.arguments["algorithm1"] << "\n";
    output << "algorithm2=" << args.arguments["algorithm2"] << "\n\n";

    //todo: Load algorithms --change
    auto algo1 = loadAlgorithm(args.arguments["algorithm1"]);
    auto algo2 = loadAlgorithm(args.arguments["algorithm2"]);

    // Process game managers in parallel
    std::vector<GameResult> results;
    std::mutex results_mutex;
    auto worker = [&](const fs::path& manager_path) {
        GameManager manager(manager_path.string());
        GameResult result = manager.runGame(args.arguments["game_map"], algo1, algo2);
        std::lock_guard<std::mutex> lock(results_mutex);
        results.push_back(result);
    };

    // Create thread pool
    ThreadPool pool(args.num_threads);
    for (const auto& manager : game_managers) {
        pool.enqueue(worker, manager);
    }
    pool.waitAll();

    // Group results by outcome
    std::unordered_map<std::string, std::vector<std::string>> result_groups;
    for (const auto& result : results) {
        std::string key = result.final_state + "|" +
                         std::to_string(result.final_round) + "|" +
                         result.game_result;
        result_groups[key].push_back(result.manager_name);
    }

    // Output grouped results
    std::unordered_set<std::string> processed_managers;
    for (const auto& [key, managers] : result_groups) {
        // Skip if all managers already processed
        bool all_processed = true;
        for (const auto& mgr : managers) {
            if (!processed_managers.count(mgr)) {
                all_processed = false;
                break;
            }
        }
        if (all_processed) continue;

        // Output manager names with same result
        std::ostringstream manager_names;
        for (size_t i = 0; i < managers.size(); ++i) {
            if (i != 0) manager_names << ", ";
            manager_names << managers[i];
        }
        output << manager_names.str() << "\n";

        // Parse and output result details
        size_t pos1 = key.find('|');
        size_t pos2 = key.find('|', pos1 + 1);
        output << key.substr(pos2 + 1) << "\n";  // Game result message
        output << key.substr(pos1 + 1, pos2 - pos1 - 1) << "\n";  // Round number

        // Output final game state (from first manager in group)
        auto sample_result = std::find_if(results.begin(), results.end(),
            [&](const GameResult& r) { return r.manager_name == managers[0]; });
        if (sample_result != results.end()) {
            output << sample_result->final_state << "\n";
        }

        // Mark managers as processed
        for (const auto& mgr : managers) {
            processed_managers.insert(mgr);
        }

        // Add separator if more groups remain
        if (processed_managers.size() < game_managers.size()) {
            output << "\n";
        }
    }

    if (write_to_file) {
        out_file.close();
        std::cout << "Results written to: " << output_path << "\n";
    }
}

int Simulator::runCompetitionMode(){ return 0; }