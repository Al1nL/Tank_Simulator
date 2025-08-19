#include "../header/MapReader.h"
#include <iostream>
using namespace UserCommon_212535058_324022904;
/**
 * @brief Reads and parses the game board from the input file.
 * @details Parses metadata, validates input, initializes players and sets up output file.
 * @param filePath Path to the input file.
 * @throws invalid_argument if the input file cannot be opened.
 */
GameMapInfo MapReader::readBoard(const string &filePath)
{
	ifstream file(filePath);
	if (!file)
	{
		throw invalid_argument("File could not be opened");
	}

	ofstream errorLog("input_errors.txt");
	bool hasErrors = false;
	GameMapInfo map_info;
	string line;

	// Parse metadata
	parseMetadata(file, hasErrors, errorLog, map_info);

	// Process map rows
	processMapRows(file, hasErrors, errorLog, map_info);

	// Clean up error log if no errors
	if (!hasErrors)
	{
		errorLog.close();
		remove("input_errors.txt");
	}
	rows_ = map_info.height;
	cols_ = map_info.width;
	max_steps_ = map_info.max_steps;
	return map_info;
}

/**
 * @brief Parses the first five lines of metadata from the input file.
 * @param file Input file stream.
 * @param hasErrors Reference to a boolean flag indicating if errors occurred.
 * @param errorLog Reference to the error log output stream.
 */
void MapReader::parseMetadata(ifstream &file, bool &hasErrors, ofstream &errorLog, GameMapInfo &map_info)
{
	string line;
	for (int i = 0; i < 5; i++)
	{
		getline(file, line);
		// size_t temp;
		if (line.empty() || line[0] == ';')
			continue;
		if (tryParseMetadata(line, "MaxSteps", map_info.max_steps, hasErrors, errorLog))
			continue;
		if (tryParseMetadata(line, "NumShells", map_info.num_shells, hasErrors, errorLog))
			continue;

		// For Rows/Cols, parse into temp int first to avoid size_t->int conversion issues
		if (tryParseMetadata(line, "Rows", map_info.height, hasErrors, errorLog))
		{
			// map_info.height = static_cast<size_t>(temp);
			continue;
		}
		if (tryParseMetadata(line, "Cols", map_info.width, hasErrors, errorLog))
		{
			// map_info.width = static_cast<size_t>(temp);
			continue;
		}

		if (hasAllMetadata(map_info))
			break;
	}

	if (!hasAllMetadata(map_info))
	{
		throw invalid_argument("Missing required metadata in file");
	}
}

/**
 * @brief Checks if all required metadata values have been set properly.
 * @return true if max_steps, num_shells, rows, and cols are all greater than zero.
 */
bool MapReader::hasAllMetadata(GameMapInfo &info) const
{
	return info.max_steps > 0 && info.num_shells > 0 && info.width > 0 && info.height > 0;
}

/**
 * @brief Attempts to parse a single metadata key-value pair from a line.
 * @param line Input line to parse.
 * @param key Metadata key to search for.
 * @param value Reference to store parsed integer value.
 * @param hasErrors Reference flag to mark error occurrence.
 * @param errorLog Output stream to log errors.
 * @return true if the key is found and value parsed successfully, false otherwise.
 */
bool MapReader::tryParseMetadata(const string &line, const string &key, size_t &value, bool &hasErrors, ofstream &errorLog)
{
	if (line.find(key) != string::npos)
	{
		size_t equalPos = line.find('=');
		if (equalPos != string::npos)
		{
			try
			{
				// stoi ignores whitespaces, so it'll get the number after '=' if the row exists
				value = stoi(line.substr(equalPos + 1));
				return true;
			}
			catch (...)
			{
				hasErrors = true;
				errorLog << "Invalid " << key << " value\n";
			}
		}
	}
	return false;
}

/**
 * @brief Processes the map rows after metadata parsing.
 *        Constructs the internal map representation with game objects.
 * @param file Input file stream positioned at map rows.
 * @param hasErrors Reference flag for error detection.
 * @param errorLog Stream to log any parsing errors.
 */
void MapReader::processMapRows(ifstream &file, bool &hasErrors, ofstream &errorLog, GameMapInfo &map_info)
{
	string line;
	vector<vector<char>> map(map_info.height);

	for (size_t i = 0; i < map_info.height; ++i)
	{
		for (size_t j = 0; j < map_info.width; ++j)
		{
			map[i].emplace_back(); // empty vec
		}
	}

	for (size_t row = 0; row < map_info.height; ++row)
	{
		bool rowHadErrors = false;

		if (!getline(file, line))
		{
			hasErrors = true;
			errorLog << "Row " << row << " is missing. Filling with empty cells.\n";
			continue;
		}
		for (size_t col = 0; col < map_info.width; ++col)
		{
			char symbol = (col < line.size()) ? line[col] : ' ';
			map[row][col] = symbol;
		}
		if (line.size() > map_info.width)
		{
			hasErrors = true;
			errorLog << line.size() - map_info.width << " excess columns in row " << row << ". Ignoring them.\n";
		}

		hasErrors |= rowHadErrors;
	}

	checkExcessRows(file, hasErrors, errorLog);
	map_info.map = std::make_unique<BoardSatelliteView>(map_info.height, map_info.width, map);
}

/**
 * @brief Checks for extra rows beyond expected count and logs any found.
 * @param file Input file stream at end of reading map rows.
 * @param hasErrors Reference flag to indicate errors.
 * @param errorLog Stream to log errors.
 */
void MapReader::checkExcessRows(ifstream &file, bool &hasErrors, ofstream &errorLog)
{
	size_t extraRows = 0;
	string line;
	while (getline(file, line))
	{
		extraRows++;
	}
	if (extraRows > 0)
	{
		hasErrors = true;
		errorLog << extraRows << " excess rows found. Ignoring them.\n";
	}
}
string MapReader::gameStateToString(const SatelliteView &view)
{
	string result;
	for (size_t i = 0; i < rows_; ++i)
	{
		for (size_t j = 0; j < cols_; ++j)
		{
			result += view.getObjectAt(i, j);
		}
		result += '\n';
	}
	return result;
}