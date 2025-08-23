#include "../header/Simulator.h"

/**
 *	@brief Initializes the simulator with command line arguments.
 *	@details Parses command line arguments and sets up the configuration.
 *	@param argc The number of command line arguments.
 *	@param argv The array of command line arguments.
 *	@returns true if initialization is successful, false otherwise.
 */
bool Simulator::init(int argc, char *argv[])
{
	// Parse command line arguments
	config_ = parseArguments(argc, argv);

	// Initialize the game based on the mode
	return initialize(config_);
}
/**
 * @brief Initializes the simulator in comparative mode.
 * @details Sets up the necessary components for running a comparative simulation.
 * @return true if initialization is successful, false otherwise.
 */
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
	config_.num_threads = computeThreadCount();
	return true;
}

/**
 * @brief Initializes the simulator in competition mode.
 * @details Sets up the necessary components for running a competition simulation.
 * @return true if initialization is successful, false otherwise.
 */
bool Simulator::initializeCompetitionMode()
{
	// Load the game manager
	if (!loadGameManager(config_.arguments["game_manager"]))
	{
		return false;
	}

	if (!loadAllMaps(config_.arguments["game_maps_folder"]))
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
	config_.num_threads = computeThreadCount();
	return true;
}

/**
 * @brief Initializes the simulator with the given configuration.
 * @param config The configuration to initialize the simulator with.
 * @return true if initialization is successful, false otherwise.
 */
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

/**
 * @brief Loads a game manager from the specified path.
 * @param path The path to the game manager shared library.
 * @return true if loading is successful, false otherwise.
 */
bool Simulator::loadGameManager(const std::string &path)
{
	auto &registrar = GameManagerRegistrar::getGameManagerRegistrar();
	registrar.createGameManagerEntry(fs::path(path).stem().string());

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
		return false;
	}
}

/**
 * @brief Loads an algorithm from the specified path.
 * @param path The path to the algorithm shared library.
 * @return true if loading is successful, false otherwise.
 */
bool Simulator::loadAlgorithm(const std::string &path)
{
	auto &registrar = AlgorithmRegistrar::getAlgorithmRegistrar();
	std::string baseName = fs::path(path).stem().string();

	// Check if we already loaded this exact file
	bool isRegistred = false;
	int size = registrar.count();
	for (int i = 0; i < size; i++)
	{
		auto algo = registrar.getAlgorithmAndPlayerFactory(i);
		if (algo.name() == baseName)
		{
			// Create new entry with same factories
			registrar.createAlgorithmFactoryEntry(baseName);
			registrar.addTankAlgorithmFactoryToLastEntry(algo.getTankAlgorithmFactory());
			registrar.addPlayerFactoryToLastEntry(algo.getPlayerFactory());
			isRegistred = true;
			break;
		}
	}

	// Otherwise, load it fresh
	if (!isRegistred)
	{
		registrar.createAlgorithmFactoryEntry(baseName);

		void *handle = dlopen(path.c_str(), RTLD_LAZY | RTLD_LOCAL);

		if (!handle)
		{
			std::cerr << "Failed to load Algorithm " << path << ": " << dlerror() << std::endl;
			registrar.removeLast();
			return false;
		}
		registrar.setHandleToLastEntry(handle);
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
		return false;
	}
}

/**
 * @brief Loads all maps from the specified path.
 * @param path The path to the directory containing map files / to the map file.
 * @return true if loading is successful, false otherwise.
 */
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
}

/**
 * @brief Parses command-line arguments and returns a Config object.
 *
 * This function orchestrates the parsing, validation, and verification of
 * command-line arguments for the simulator. It checks the mode, flags,
 * required key=value arguments, invalid arguments, and validates file paths.
 *
 * @param argc Argument count.
 * @param argv Argument values.
 * @return Config object containing parsed values.
 */
Config Simulator::parseArguments(int argc, char *argv[])
{
	if (argc < 2)
		printUsage("Not enough arguments");

	Config args;
	std::vector<std::string> required_args;
	std::vector<std::string> invalid_args;

	// Determine mode
	std::string mode = argv[1];
	parseMode(mode, args, required_args);

	// Parse key=value and flags
	parseKeyValueArguments(argc, argv, args, invalid_args);

	// Check required and invalid arguments
	checkRequiredArguments(args, required_args);
	checkInvalidArguments(invalid_args);

	// Validate file paths
	validatePaths(args);

	return args;
}

/**
 * @brief Determines the simulator mode and sets required arguments for that mode.
 *
 * @param mode The mode string from command-line arguments (e.g., "-comparative").
 * @param args Config object to store the parsed mode.
 * @param required_args Vector to store the names of required arguments for the mode.
 */
void Simulator::parseMode(const std::string &mode, Config &args, std::vector<std::string> &required_args)
{
	if (mode == "-comparative")
	{
		args.mode = Comparative;
		required_args = {"game_map", "game_managers_folder", "algorithm1", "algorithm2"};
	}
	else if (mode == "-competition")
	{
		args.mode = Competition;
		required_args = {"game_maps_folder", "game_manager", "algorithms_folder"};
	}
	else
	{
		printUsage("Invalid mode specified", {mode});
	}
}

/**
 * @brief Parses command-line key=value pairs and flags like -verbose.
 *
 * @param argc Argument count.
 * @param argv Argument values.
 * @param args Config object to store parsed values.
 * @param invalid_args Vector to store unrecognized or invalid arguments.
 */
void Simulator::parseKeyValueArguments(int argc, char *argv[], Config &args, std::vector<std::string> &invalid_args)
{
	for (int i = 2; i < argc; ++i)
	{
		std::string arg = argv[i];

		// Verbose flag
		if (arg == "-verbose")
		{
			args.verbose = true;
			continue;
		}

		// Key=value pair
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
			parseNumThreads(value, args);
		}
		else
		{
			args.arguments[key] = value;
		}
	}
}

/**
 * @brief Parses and validates the num_threads argument.
 *
 * @param value String representing the number of threads.
 * @param args Config object where the parsed number of threads will be stored.
 */
void Simulator::parseNumThreads(const std::string &value, Config &args)
{
	try
	{
		args.num_threads = std::stoi(value);
		if (args.num_threads < 1)
			printUsage("num_threads must be positive");
	}
	catch (...)
	{
		printUsage("Invalid num_threads value");
	}
}

/**
 * @brief Checks if all required arguments are present in the parsed Config.
 *
 * @param args Config object containing parsed arguments.
 * @param required_args Vector of argument names that are required.
 */
void Simulator::checkRequiredArguments(const Config &args, const std::vector<std::string> &required_args)
{
	std::vector<std::string> missing_args;
	for (const auto &req : required_args)
	{
		if (args.arguments.find(req) == args.arguments.end())
			missing_args.push_back(req);
	}

	if (!missing_args.empty())
		printUsage("Missing required arguments", missing_args);
}

/**
 * @brief Checks for invalid/unrecognized arguments collected during parsing.
 *
 * @param invalid_args Vector of argument strings that were not recognized.
 */
void Simulator::checkInvalidArguments(const std::vector<std::string> &invalid_args)
{
	if (!invalid_args.empty())
		printUsage("Invalid arguments provided", invalid_args);
}

/**
 * @brief Validates filesystem paths for maps, algorithms, and game managers.
 *
 * Checks that files exist or folders are valid depending on the mode.
 * Throws usage error if validation fails.
 *
 * @param args Config object containing parsed arguments and mode.
 */
void Simulator::validatePaths(const Config &args)
{
	try
	{
		if (args.mode == Comparative)
		{
			if (!fs::exists(args.arguments.at("game_map")))
				printUsage("Game map file does not exist");
			if (!fs::is_directory(args.arguments.at("game_managers_folder")))
				printUsage("Game managers folder is invalid");
			if (!fs::exists(args.arguments.at("algorithm1")))
				printUsage("Algorithm1 file does not exist");
			if (!fs::exists(args.arguments.at("algorithm2")))
				printUsage("Algorithm2 file does not exist");
		}
		else
		{
			if (!fs::is_directory(args.arguments.at("game_maps_folder")))
				printUsage("Game maps folder is invalid");
			if (!fs::exists(args.arguments.at("game_manager")))
				printUsage("Game manager file does not exist");
			if (!fs::is_directory(args.arguments.at("algorithms_folder")))
				printUsage("Algorithms folder is invalid");
		}
	}
	catch (const fs::filesystem_error &)
	{
		printUsage("Filesystem error while validating paths");
	}
}

/**
 * @brief Prints the usage information for the simulator.
 * @param error_msg An optional error message to display.
 * @param invalid_args A list of invalid arguments provided by the user.
 */
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

/**
 * @brief Runs the simulator.
 * @details Depending on the mode, either the comparative or competition simulation is executed.
 */
void Simulator::run()
{
	if (config_.mode == Comparative)
	{
		runComparative();
	}
	else
	{
		runCompetition();
	}
}

/**
 * @brief Runs the comparative simulation.
 * @details Executes the comparative simulation using the specified game managers and algorithms.
 */
void Simulator::runComparative()
{

	// Prepare output filename with timestamp
	auto now = std::chrono::system_clock::now();
	auto time_str = std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count());
	fs::path output_path = fs::path(config_.arguments["game_managers_folder"]) / ("comparative_results_" + time_str + ".txt");

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
		std::string key = getGameResultMsg(result, reader.getMaxSteps()) + "|" +
						  std::to_string(result.rounds) + "|" + reader.gameStateToString(*result.gameState); // Use gameStateToString to get a string representation of the game state
		result_groups[key].push_back(gm_registrar.getGameManager(manager_idx).getName());
	}

	logResults(result_groups, out_file);
	if (write_to_file)
	{
		out_file.close();
		std::cout << "Results written to: " << output_path << "\n";
	}
}

/**
 * @brief Runs the competition simulation.
 * @details Executes the competition simulation using all combinations of algorithms and maps.
 */
void Simulator::runCompetition()
{

	// Check we have enough algorithms and maps
	auto &algo_registrar = AlgorithmRegistrar::getAlgorithmRegistrar();
	size_t num_algorithms = algo_registrar.count();

	// Prepare output filename with timestamp
	auto now = std::chrono::system_clock::now();
	auto time_str = std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(
									   now.time_since_epoch())
									   .count());
	fs::path output_path = fs::path(config_.arguments["algorithms_folder"]) /
						   ("competition_results_" + time_str + ".txt");

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
	output << "game_maps_folder=" << config_.arguments["game_maps_folder"] << "\n";
	output << "game_manager=" << config_.arguments["game_manager"] << "\n\n";

	// Prepare results storage
	std::vector<int> algorithm_scores(num_algorithms, 0);
	std::mutex scores_mutex;

	// Create thread pool
	ThreadPool pool(config_.num_threads);
	// Process each map
	for (size_t map_idx = 0; map_idx < mapInfo_.size(); ++map_idx)
	{
		std::cerr << "the map: " << map_idx << std::endl;
		size_t k = map_idx % (num_algorithms - 1);

		// Create all pairs for this map
		for (size_t i = 0; i < num_algorithms; ++i)
		{
			size_t j = (i + 1 + k) % num_algorithms;

			// Skip duplicate pairs when N is even and k = N/2 - 1
			if (num_algorithms % 2 == 0 && k == (num_algorithms / 2 - 1) && i >= j)
			{
				continue;
			}

			// Enqueue the game
			pool.enqueue([this, map_idx, i, j, &algorithm_scores, &scores_mutex]()
						 {
				GameResult result = runSingleCompetitionGame(map_idx, i, j);

				if (result.gameState == nullptr) {
					std::cerr << "Warning: Game returned null result!" << std::endl;
					return; // Skip invalid results
				}

				// Update scores
				std::lock_guard<std::mutex> lock(scores_mutex);
				if (result.winner == 0) { // Tie
					algorithm_scores[i] += 1;
					algorithm_scores[j] += 1;
				} else if (result.winner == 1) {
					algorithm_scores[i] += 3;
				} else if (result.winner == 2) {
					algorithm_scores[j] += 3;
				} });
		}
	}

	// Wait for all games to complete
	pool.waitAll();

	// Prepare final results sorted by score
	std::vector<std::pair<std::string, int>> final_results;
	for (size_t i = 0; i < num_algorithms; ++i)
	{
		final_results.emplace_back(algo_registrar.getAlgorithmAndPlayerFactory(i).name(), algorithm_scores[i]);
	}

	// Sort results by score (descending)
	std::sort(final_results.begin(), final_results.end(),
			  [](const auto &a, const auto &b)
			  {
				  return b.second < a.second;
			  });

	// Output final results
	for (const auto &[name, score] : final_results)
	{
		output << name << " " << score << "\n";
	}

	if (write_to_file)
	{
		out_file.close();
		std::cout << "Results written to: " << output_path << "\n";
	}
}

/**
 * @brief Runs a single comparative game.
 * @param manager_number The game manager number to use.
 * @return The result of the game.
 */
GameResult Simulator::runSingleComparativeGame(int manager_number)
{
	try
	{
		auto &algo_registrar = AlgorithmRegistrar::getAlgorithmRegistrar();
		auto algo1_player_factory = algo_registrar.getAlgorithmAndPlayerFactory(0);
		auto algo2_player_factory = algo_registrar.getAlgorithmAndPlayerFactory(1);

		auto &gm_registrar = GameManagerRegistrar::getGameManagerRegistrar();

		auto game_manager_factory = gm_registrar.getGameManager(manager_number).getFactory();
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
		auto player1 = algo1_player_factory.createPlayer(1, map.height, map.width, map.max_steps, map.num_shells);
		auto player2 = algo2_player_factory.createPlayer(2, map.height, map.width, map.max_steps, map.num_shells);

		// Run the game
		string map_name = fs::path(map.name).stem().string();
		GameResult result = game_manager->run(map.width, map.height,
											  *map.map, map_name, map.max_steps, map.num_shells,
											  *player1, algo1_player_factory.name(), *player2, algo2_player_factory.name(),
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

/**
 * @brief Generates a message summarizing the game result.
 * @param result The result of the game.
 * @param max_steps The maximum number of steps allowed in the game.
 * @return A string message summarizing the game result.
 */
std::string Simulator::getGameResultMsg(const GameResult &result, size_t max_steps)
{
	std::string msg;

	if (result.winner == 0)
	{ // tie
		switch (result.reason)
		{
		case GameResult::ALL_TANKS_DEAD:
			msg = "Tie, both players have zero tanks";
			break;
		case GameResult::MAX_STEPS:
			msg = "Tie, reached max steps = " + std::to_string(max_steps) + ", player 1 has " + std::to_string(result.remaining_tanks[0]) + " tanks, player 2 has " + std::to_string(result.remaining_tanks[1]) + " tanks";
			break;
		case GameResult::ZERO_SHELLS:
			msg = "Tie, both players have zero shells for 40 steps";
			break;
		}
	}
	else
	{ // someone won
		size_t alive = result.remaining_tanks[result.winner - 1];
		msg = "Player " + std::to_string(result.winner) + " won with " + std::to_string(alive) + " tank";
		if (alive != 1)
			msg += "s";
		msg += " still alive";
	}

	return msg;
}

/**
 * @brief Logs the results of the games to the specified output stream.
 * @param results The results to log.
 * @param output The output stream to log to.
 */
void Simulator::logResults(std::unordered_map<std::string, std::vector<std::string>> results, std::ofstream &output)
{
	// Output results
	for (const auto &[key, managers] : results)
	{
		// Parse key components
		size_t pos1 = key.find('|');
		size_t pos2 = key.find('|', pos1 + 1);

		string winner = key.substr(0, pos1);
		size_t rounds = std::stoi(key.substr(pos1 + 1, pos2 - pos1 - 1));
		string game_state = key.substr(pos2 + 1);
		// Output manager names
		for (size_t i = 0; i < managers.size(); ++i)
		{
			if (i != 0)
				output << ", ";
			output << managers[i];
		}
		output << "\n";

		// Output result details
		output << winner << "\n"
			   << rounds << "\n"
			   << game_state << "\n";
	}
}

/**
 * @brief Runs a single competition game.
 * @param map_idx The index of the map to use.
 * @param algo1_idx The index of the first algorithm to use.
 * @param algo2_idx The index of the second algorithm to use.
 * @return The result of the game.
 */
GameResult Simulator::runSingleCompetitionGame(size_t map_idx, size_t algo1_idx, size_t algo2_idx)
{
	auto &algo_registrar = AlgorithmRegistrar::getAlgorithmRegistrar();

	try
	{
		// Get algorithm factories
		auto algo1_factory = algo_registrar.getAlgorithmAndPlayerFactory(algo1_idx);
		auto algo2_factory = algo_registrar.getAlgorithmAndPlayerFactory(algo2_idx);

		// Get game manager factory
		auto &gm_registrar = GameManagerRegistrar::getGameManagerRegistrar();
		auto game_manager_factory = gm_registrar.getGameManager(0).getFactory();
		if (!game_manager_factory)
		{
			std::cerr << "Failed to create game manager factory" << std::endl;
			return {};
		}

		// Create game manager instance
		auto game_manager = game_manager_factory(config_.verbose);
		if (!game_manager)
		{
			std::cerr << "Failed to create game manager instance" << std::endl;
			return {};
		}

		// Create players
		const auto &map = mapInfo_[map_idx];
		auto player1 = algo1_factory.createPlayer(1, map.height, map.width, map.max_steps, map.num_shells);
		auto player2 = algo2_factory.createPlayer(2, map.height, map.width, map.max_steps, map.num_shells);

		// Run the game

		GameResult result = game_manager->run(
			map.width, map.height, *map.map, map.name, map.max_steps, map.num_shells,
			*player1, algo_registrar.getAlgorithmAndPlayerFactory(algo1_idx).name(),
			*player2, algo_registrar.getAlgorithmAndPlayerFactory(algo2_idx).name(),
			algo1_factory.getTankAlgorithmFactory(),
			algo2_factory.getTankAlgorithmFactory());

		if (result.gameState == nullptr)
		{
			std::cerr << "Warning: Game returned null result for map "
					  << map.name << " and algorithms "
					  << algo_registrar.getAlgorithmAndPlayerFactory(algo1_idx).name() << " vs "
					  << algo_registrar.getAlgorithmAndPlayerFactory(algo2_idx).name() << std::endl;
			return {};
		}
		return result;
	}
	catch (const std::exception &e)
	{
		std::cerr << "Error running game: " << e.what() << std::endl;
		return {};
	}
}

/**
 * @brief Computes the number of threads to use for the simulation.
 * @return The number of threads to use.
 */
int Simulator::computeThreadCount() const
{
	size_t requested_threads = config_.num_threads;

	// Calculate total number of games
	size_t total_games = 0;
	if (config_.mode == Competition)
	{
		auto &algo_registrar = AlgorithmRegistrar::getAlgorithmRegistrar();
		size_t num_algorithms = algo_registrar.count();

		for (size_t map_idx = 0; map_idx < mapInfo_.size(); ++map_idx)
		{
			size_t k = map_idx % (num_algorithms - 1);
			for (size_t i = 0; i < num_algorithms; ++i)
			{
				size_t j = (i + 1 + k) % num_algorithms;
				if (!(num_algorithms % 2 == 0 && k == (num_algorithms / 2 - 1) && i >= j))
				{
					total_games++;
				}
			}
		}
	}
	else
	{
		auto &gm_registrar = GameManagerRegistrar::getGameManagerRegistrar();
		total_games = gm_registrar.count();
	}

	size_t worker_threads = std::min(requested_threads - 1, total_games - 1);
	if (worker_threads == 1)
	{
		return 1;
	}
	return worker_threads + 1;
}

/**
 * @brief Destroys the simulator and cleans up resources.
 */
Simulator::~Simulator()
{
	// Clean up loaded libraries
	GameManagerRegistrar::getGameManagerRegistrar().cleanup();
	AlgorithmRegistrar::getAlgorithmRegistrar().cleanup();
	mapInfo_.clear();
}