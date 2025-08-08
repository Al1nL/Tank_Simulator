#include "../header/Simulator.h"
#include "../header/AlgorithmRegistrar.h"

namespace fs = std::filesystem;

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

void Simulator::printUsage(const std::string& error_msg = "",
               const std::vector<std::string>& invalid_args = {}) {
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
