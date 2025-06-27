#include "../header/TankAlgorithm_212535058_324022904.h"
#include "../../common/TankAlgorithmRegistration.h"
#include "../header/TankBattleInfo.h"
#include "../../UserCommon/header/Shell.h"

using namespace Algorithm_212535058_324022904;
// REGISTER_TANK_ALGORITHM(TankAlgorithm_212535058_324022904);



/**
 * @brief Constructs a TankAlgorithm_212535058_324022904 instance with game parameters.
 * @param player_index Index of the player that the tank will belong to.
 * @param tank_index Index of this tank.
 */
TankAlgorithm_212535058_324022904::TankAlgorithm_212535058_324022904(int player_index, int tank_index) : player_index(player_index), tank_index(tank_index), battle_info(make_unique<TankBattleInfo>(tank_index, player_index)) {}

/**
 * @brief Updates the internal battle info object with the latest game state.
 *
 * Copies opponent data, position, direction, known objects, and shell count from the provided BattleInfo.
 *
 * @param info The BattleInfo object containing the current state.
 */
void TankAlgorithm_212535058_324022904::updateBattleInfo(BattleInfo &info)
{
	auto tank_info = dynamic_cast<TankBattleInfo &>(info);
	battle_info->setOpponents(tank_info.getOpponents());
	battle_info->setPosition(tank_info.getPosition().first, tank_info.getPosition().second);
	battle_info->setDirection(tank_info.getDirection());
	battle_info->setKnownObjects(tank_info.getKnownObjects());
	battle_info->setRemainingShells(tank_info.getRemainingShells());
}

/**
 * @brief Determines and returns the next action.
 *
 * @return The decided ActionRequest.
 */
ActionRequest TankAlgorithm_212535058_324022904::getAction()
{
	ActionRequest action = decideAction();
	updateInnerInfoAfterAction(action);
	moveKnownShells();
	return action;
}

/**
 * @brief Determines the next move for the tank based on current gathered info.
 *
 * @return The next decided Action.
 */
ActionRequest TankAlgorithm_212535058_324022904::decideAction() {
	auto [currentRow, currentCol] = battle_info->getPosition();

	++current_turn;

	OppData opp = getClosestOpponent();
	if (opp.opponentPos == make_pair(-1,-1)) {
          last_info_update = current_turn;
          return ActionRequest::GetBattleInfo;
    }

	auto [targetRow, targetCol] = opp.opponentPos;
    opp.opponentDir = opp.opponentDir == None ? battle_info->calculateRealDirection(currentRow, currentCol, targetRow, targetCol) :  opp.opponentDir;

	Direction currentDir = battle_info->getDirection();
	Direction desiredDir = calculateDirection(currentRow, currentCol, targetRow, targetCol);

	// Try to dodge danger
	if (willBeHitIn(battle_info->getPosition().first,battle_info->getPosition().second,1)) {
		ActionRequest res= checkForEscape();
		if(res!=ActionRequest::DoNothing)
			return res;
		if(battle_info->getRemainingShells() > 0 && !battle_info->isWaitingToShoot())
			return ActionRequest::Shoot;
	}

	if(willBeHitIn(battle_info->getPosition().first,battle_info->getPosition().second,2)) {
		ActionRequest res;
		res =calculateBestEscapeRotation();
		if(res!=ActionRequest::DoNothing)
			return res;
		else {
			res=checkForEscape();
			if(res!=ActionRequest::DoNothing)
				return res;
		}
	}


    // Shoot if aligned and safe
    if (shouldShootOpponent(opp) && !battle_info->isWaitingToShoot()) {
        return  ActionRequest::Shoot;
    }

	if(current_turn - last_info_update > tank_index + 4){
		last_info_update = current_turn;
		return ActionRequest::GetBattleInfo;
	}

    if (currentDir != desiredDir) {
         ActionRequest rotate = determineRotation(currentDir, desiredDir);
         RotationOption option = rotationOption(rotate, desiredDir, currentDir);
         if((option.canMove || canShootAfterRotate(desiredDir, opp)) && option.safetyScore > 0)
         	return rotate;
    }

    // Move forward if not aligned/blocked
    pair<int,int> next = nextStep(true, battle_info->getPosition(), battle_info->getDirection());
    if ((currentRow != targetRow || currentCol != targetCol) && isOccupierFree(next)) {
        return ActionRequest::MoveForward;
    }

    // Default fallback ActionRequest
	ActionRequest escapeRotation = calculateBestEscapeRotation();
	if (escapeRotation != ActionRequest::DoNothing) {
		return escapeRotation;
	}
	return ActionRequest::GetBattleInfo;
}

/** ----------------------------- Basic Actions -------------------------------- **/

/**
 * @brief Computes the next position on the board after moving forward or backward.
 *
 * Applies wraparound for map edges.
 *
 * @param forward True to move forward, false to move backward.
 * @param pos Current position as (row, col).
 * @param dir Current direction.
 * @return The new position after the step, wrapped on map edges.
 */
pair<int, int> TankAlgorithm_212535058_324022904::nextStep(bool forward, const pair<int, int> pos, const Direction dir)
{
	int side = forward ? 1 : -1;
	auto [h, w] = battle_info->getMapSize();

	int newRow = wrap(pos.first + offsets[dir].first * side, h);
	int newCol = wrap(pos.second + offsets[dir].second * side, w);

	return {newRow, newCol};
}

/**
 * @brief Performs rotation of the tank's direction based on an ActionRequest.
 *
 * Updates internal direction state accordingly.
 *
 * @param action The rotation action requested.
 */
void TankAlgorithm_212535058_324022904::rotate(ActionRequest action)
{
	Direction dir = battle_info->getDirection();
	switch (action)
	{
	case ActionRequest::RotateLeft45:
		dir = static_cast<Direction>((dir + 7) % 8); // move 1 counter-clockwise
		break;
	case ActionRequest::RotateRight45:
		dir = static_cast<Direction>((dir + 1) % 8); // move 1 clockwise
		break;
	case ActionRequest::RotateLeft90:
		dir = static_cast<Direction>((dir + 6) % 8); // move 2 clockwise
		break;
	case ActionRequest::RotateRight90:
		dir = static_cast<Direction>((dir + 2) % 8); // move 2 counter-clockwise
		break;
	case ActionRequest::MoveForward:
	case ActionRequest::MoveBackward:
	case ActionRequest::Shoot:
	case ActionRequest::DoNothing:
	case ActionRequest::GetBattleInfo:
		break;
	}
	battle_info->setDirection(dir);
}

/**
 * @brief Checks if the requested action is valid given the current game state.
 *
 * Considers backward cooldowns, occupier presence, and shell availability.
 *
 * @param action The action to validate.
 * @return True if action is valid, false otherwise.
 */
bool TankAlgorithm_212535058_324022904::isValidMove(ActionRequest action)
{
	if (battle_info->isWaitingToReverse() && action != ActionRequest::MoveForward)
		return false;

	pair<int, int> newPos;
	pair<int, int> currPos = battle_info->getPosition();
	Direction currDir = battle_info->getDirection();

	switch (action)
	{
	case ActionRequest::Shoot:
		return !battle_info->isWaitingToShoot() && battle_info->getRemainingShells() > 0;
	case ActionRequest::MoveForward:
		newPos = nextStep(true, currPos, currDir);
		return isOccupierFree(newPos);
	case ActionRequest::MoveBackward:
		newPos = nextStep(false, currPos, currDir);
		return !battle_info->isWaitingToReverse() && isOccupierFree(newPos);
	case ActionRequest::RotateLeft45:
	case ActionRequest::RotateRight45:
	case ActionRequest::RotateLeft90:
	case ActionRequest::RotateRight90:
		return !battle_info->isWaitingToReverse();
	default:
		return false;
	}
}

/**
 * @brief Checks if a given board position is free of occupiers(Wall, Tank, Mine).
 *
 * @param pos Position to check.
 * @return True if the cell is free, false otherwise.
 */
bool TankAlgorithm_212535058_324022904::isOccupierFree(pair<int, int> pos)
{
	return battle_info->getObjectByPosition(pos) == nullptr;
}

/**
 * @brief Determines if the player should shoot at the opponent based on alignment and available shells.
 *
 * @param opponentPos The opponent's position.
 * @return True if the player should shoot, false otherwise.
 */
bool TankAlgorithm_212535058_324022904::shouldShootOpponent(const pair<int, int> &opponentPos)
{
	return battle_info->getRemainingShells() > 0 &&
		   isAlignedWithOpponent(opponentPos) &&
		   !battle_info->isWaitingToShoot();
}

/**
 * @brief Checks if the player can move forward safely.
 *
 * @return True if moving forward is possible and safe, false otherwise.
 */
bool TankAlgorithm_212535058_324022904::canMoveFwd()
{
	auto [r, c] = nextStep(true, battle_info->getPosition(), battle_info->getDirection());
	return isOccupierFree({r, c}) && !willBeHitIn(r, c, 1);
}

/**
 * @brief Checks if the player can move backward safely.
 *
 * @return True if moving backward is possible and safe, false otherwise.
 */
bool TankAlgorithm_212535058_324022904::canMoveBack()
{
	if (!isValidMove(ActionRequest::MoveBackward))
		return false;
	pair<int, int> next = nextStep(false, battle_info->getPosition(), battle_info->getDirection());
	return canSafelyBack(next.first, next.second) && isOccupierFree(next);
}

/**
 * @brief Determines if moving backward would be safe for the player.
 *
 * @param backR Row after moving backward.
 * @param backC Column after moving backward.
 * @return True if the move is safe, false otherwise.
 */
bool TankAlgorithm_212535058_324022904::canSafelyBack(int backR, int backC)
{
	auto [r, c] = battle_info->getPosition();
	// current position and backward cell are safe
	if (!battle_info->getMovedBackwardLast())
	{
		if (willBeHitIn(r, c, 1) || willBeHitIn(r, c, 2) || willBeHitIn(backR, backC, 3))
		{				  //
			return false; // Danger in current or backward position
		}
	}
	else
	{
		// If just moved back, only check immediate danger in next cell
		if (willBeHitIn(backR, backC, 1))
		{
			return false;
		}
	}
	return true; // Safe to move backward
}

/**
 * @brief Chooses an escape action if the player is in danger.
 *
 * @param board The current game board.
 * @return Action::MoveFwd if forward is safe, Action::MoveBack if backward is safe, or Action::None if stuck.
 */
ActionRequest TankAlgorithm_212535058_324022904::checkForEscape()
{
	if (canMoveFwd())
	{
		return ActionRequest::MoveForward;
	}
	if (canMoveBack())
	{
		return ActionRequest::MoveBackward;
	}
	return ActionRequest::DoNothing;
}

/**
 * @brief Predicts if a shell will hit a specific cell within a given number of game steps.
 *
 * @param row Row of the cell to check.
 * @param col Column of the cell to check.
 * @param t Number of game steps ahead.
 * @return True if a shell will hit, false otherwise.
 */
bool TankAlgorithm_212535058_324022904::willBeHitIn(int row, int col, int t)
{
	auto knownObjects = battle_info->getKnownObjects();
	for (auto &[pos, objects] : knownObjects)
	{
		if (objects.empty())
			continue;

		auto object = objects.size() > 1 ? objects[1] : objects[0];
		char symbol = object->getSymbol();
		if (symbol != '*')
			continue;

		auto [sr, sc] = pos;
		Direction dir = dynamic_cast<Shell *>(object)->getDirection();

		for (int i = 0; i <= 2 && dir != None; i++)
		{

			auto [dr, dc] = offsets[static_cast<int>(dir)];

			// Calc forward `t` steps:
			int stepsAhead = (t - 1) * 2 + i;
			int pr = sr + dr * stepsAhead;
			int pc = sc + dc * stepsAhead;

			// Wraparound edges
			auto [h, w] = battle_info->getMapSize();
			pr = wrap(pr, h);
			pc = wrap(pc, w);

			if (pr == row && pc == col)
			{
				return true; // A shell will hit the cell by that time
			}
		}
	}
	return false;
}

/**
 * @brief Checks if the player is aligned (same row or same column) with the opponent.
 *
 * @param opponentPos Opponent's current position.
 * @return True if aligned, false otherwise.
 */
bool TankAlgorithm_212535058_324022904::isAlignedWithOpponent(pair<int, int> opponentPos)
{
	auto [r, c] = battle_info->getPosition();
	return r == opponentPos.first || c == opponentPos.second;
}

/**
 * @brief Determines the rotation action needed to align from the current direction to the desired direction.
 *
 * @param currentDir Current facing direction.
 * @param desiredDir Target direction.
 * @return Action representing the rotation to apply.
 */
ActionRequest TankAlgorithm_212535058_324022904::determineRotation(Direction currentDir, Direction desiredDir)
{
	int rotationSteps = (static_cast<int>(desiredDir) - static_cast<int>(currentDir) + 8) % 8;
	if (rotationSteps == 1 || rotationSteps == 2)
		return ActionRequest::RotateRight45;
	if (rotationSteps == 6 || rotationSteps == 7)
		return ActionRequest::RotateLeft45;
	if (rotationSteps == 3)
		return ActionRequest::RotateRight90;
	if (rotationSteps == 5)
		return ActionRequest::RotateLeft90;

	// If no rotation required
	return ActionRequest::RotateRight45; // Arbitrary fallback
}

/**
 * @brief Calculates the general direction from the current position to the target position.
 *
 * @param currRow Current row position.
 * @param currCol Current column position.
 * @param targetRow Target row position.
 * @param targetCol Target column position.
 * @return Direction to move toward the target, or current direction if already at the target.
 */
Direction TankAlgorithm_212535058_324022904::calculateDirection(int currRow, int currCol, int targetRow, int targetCol)
{
	if (currRow < targetRow && currCol == targetCol)
		return Direction::D;
	if (currRow > targetRow && currCol == targetCol)
		return Direction::U;
	if (currRow == targetRow && currCol < targetCol)
		return Direction::R;
	if (currRow == targetRow && currCol > targetCol)
		return Direction::L;
	if (currRow < targetRow && currCol < targetCol)
		return Direction::DR;
	if (currRow < targetRow && currCol > targetCol)
		return Direction::DL;
	if (currRow > targetRow && currCol < targetCol)
		return Direction::UR;
	if (currRow > targetRow && currCol > targetCol)
		return Direction::UL;

	return None; // Default to current direction if no match
}

/**
 * @brief Returns the closest opponent based on minimal required actions.
 *
 * Finds the opponent that requires the fewest moves and rotations to reach.
 *
 * @return OppData Information about the closest opponent, or an empty opponent if none found.
 */
OppData TankAlgorithm_212535058_324022904::getClosestOpponent()
{
	vector<OppData> opponents = battle_info->getOpponents();
	int min_movements = 1e8;
	int min_pos = -1;
	int index = 0;
	for (auto opp : opponents)
	{
		int curr_moves = calculateActionsToOpponent(opp);
		if (min_movements > curr_moves)
		{
			min_pos = index;
			min_movements = curr_moves;
		}
		++index;
	}

	return min_pos != -1 ? opponents[min_pos] : OppData{{-1, -1}, Direction::None};
}

/**
 * @brief Calculates the estimated number of actions to reach an opponent.
 *
 * Estimates the total number of moves and rotations needed to reach the opponent's position and direction.
 *
 * @param opp The opponent data containing position and direction.
 * @return int Estimated total actions (moves + rotations).
 */
int TankAlgorithm_212535058_324022904::calculateActionsToOpponent(const OppData &opp)
{
	auto myPos = battle_info->getPosition();
	pair<int, int> oppPos = opp.opponentPos;

	int dx = oppPos.first - myPos.first;
	int dy = oppPos.second - myPos.second;

	Direction requiredDir = None;
	int movements = 0;
	int rotations = 0;

	if (dx == 0 && dy == 0)
	{
		return 0; // Already at opponent's position
	}

	// Determine the required direction and movements
	if (dx != 0)
	{
		requiredDir = calculateDirection(myPos.first, myPos.second, oppPos.first, oppPos.second);
		movements = abs(dx);
	}
	else if (dy != 0)
	{
		requiredDir = calculateDirection(myPos.first, myPos.second, oppPos.first, oppPos.second);
		movements = abs(dy);
	}

	// Calculate rotations to face the required direction
	if (requiredDir == opp.opponentDir)
		++rotations;
	else
		rotations = 2;

	// Total actions = rotations + movements
	return rotations + movements;
}


/**
 * @brief Updates the internal info after executing an action.
 *
 * Handles position updates, backward cooldowns, and shell cooldowns.
 *
 * @param action The action that was executed.
 */
void TankAlgorithm_212535058_324022904::updateInnerInfoAfterAction(ActionRequest action)
{
	pair<int, int> newPos = {-1, -1};
	if (battle_info->isWaitingToReverse() && battle_info->getWaitingForBackward() == 0)
		action = ActionRequest::MoveBackward; // back after 3 rounds, no matter what the other action now is, it is ignored?
	switch (action)
	{
	case ActionRequest::MoveForward:
		if (battle_info->isWaitingToReverse())
		{
			battle_info->setBackwardCooldown(0);
			battle_info->setWaitingForBackward(false);
			// Tank stays in place
		}
		else
		{
			newPos = nextStep(true, battle_info->getPosition(), battle_info->getDirection());
			battle_info->setPosition(newPos.first, newPos.second);
		}
		break;

	case ActionRequest::MoveBackward:
		if (battle_info->getMovedBackwardLast() || (battle_info->isWaitingToReverse() && battle_info->getWaitingForBackward() == 0))
		{ // Instant backward move
			newPos = nextStep(false, battle_info->getPosition(), battle_info->getDirection());
			battle_info->setWaitingForBackward(false);
			battle_info->setPosition(newPos.first, newPos.second);
		}
		else if (!battle_info->isWaitingToReverse())
		{
			// Start backward cooldown (waiting for 2 steps)
			battle_info->setBackwardCooldown(2);
			battle_info->setWaitingForBackward(true);
		}
		break;

	case ActionRequest::Shoot:
		if (!battle_info->isWaitingToShoot() && battle_info->getRemainingShells() > 0)
		{
			battle_info->decreaseRemainingShells();
			battle_info->setShootCooldown(4);
		}
		break;

	case ActionRequest::RotateLeft45:
	case ActionRequest::RotateRight45:
	case ActionRequest::RotateLeft90:
	case ActionRequest::RotateRight90:
		rotate(action);
		break;

	case ActionRequest::DoNothing:
	case ActionRequest::GetBattleInfo: // handled in GameMmanager
		break;
	}
}


/** ----------------------------- Advanced Actions -------------------------------- **/

/**
 * @brief Simulates a rotation and checks if moving forward is possible afterward.
 *
 * @param act The rotation ActionRequest to simulate.
 * @return The new direction after the rotation, or Direction::None if not safe.
 */
Direction TankAlgorithm_212535058_324022904::simulateRotation(ActionRequest act) {
	Direction currentDir = battle_info->getDirection();
	rotate(act);
	Direction newDir = battle_info->getDirection();
	if(canMoveFwd()) {
		battle_info->setDirection(currentDir);
		return newDir;
	}
	return Direction::None;
}

/**
 * @brief Checks whether the tank should shoot the opponent based on their predicted movement.
 *
 * @param opp The opponent's data.
 * @return True if shooting is advantageous, false otherwise.
 */
bool TankAlgorithm_212535058_324022904::shouldShootOpponent(OppData& opp) {
	if (battle_info->isWaitingToShoot() || battle_info->getRemainingShells() <= 0) {
		return false;
	}

	auto [currentRow, currentCol] = battle_info->getPosition();
	auto [targetRow, targetCol] = opp.opponentPos;

	// Basic opponent always moves forward when possible, else rotates right
	Direction oppDir = opp.opponentDir;

	// Predict opponent's next 2 positions
	vector<pair<int,int>> predictedPositions = {
		{targetRow, targetCol},  // Current position
	};

    if(oppDir != None){
      auto [h,w] = battle_info->getMapSize();
      auto [dr, dc] = offsets[static_cast<int>(oppDir)];
      predictedPositions.push_back({wrap(targetRow + dr,h), wrap(targetCol + dc,w)}); // Next position
      predictedPositions.push_back({wrap(targetRow + dr*2,h), wrap(targetCol + dc*2,w)}); // Position after next
    }

    // Check if we're aligned with any predicted position
    for (const auto& [r, c] : predictedPositions) {
        if (battle_info->getDirection() == battle_info->calculateRealDirection(currentRow, currentCol, r, c)) {
            return true;
        }
    }
    return false;
}

/**
 * @brief Simulates a rotation to check if shooting is possible afterward.
 *
 * @param targetDir The direction to simulate.
 * @param opp The opponent to shoot at.
 * @return True if the shot would be aligned, false otherwise.
 */
bool TankAlgorithm_212535058_324022904::canShootAfterRotate(Direction targetDir, OppData& opp) {
    // Simulate rotation
    Direction currentDir = battle_info->getDirection();
    battle_info->setDirection(targetDir);

    // Check if we could shoot after rotating
    bool canShoot = shouldShootOpponent(opp);

    // Restore direction
    battle_info->setDirection(currentDir);
    return canShoot;
}

/**
 * @brief Calculates the best rotation to escape from danger.
 *
 * @return The best escape rotation ActionRequest, or ActionRequest::None if none found.
 */
ActionRequest TankAlgorithm_212535058_324022904::calculateBestEscapeRotation() {
	Direction currentDir = battle_info->getDirection();

	vector<RotationOption> options;
	RotationOption option;
	// Evaluate each rotation option
	for (ActionRequest rotation : rotations) {
		// Simulate rotation
		rotate(rotation);
		Direction newDir = battle_info->getDirection();

		option = rotationOption(rotation, newDir, currentDir);
		options.push_back(option);
	}

    // Sort options by safety score (descending)
    sort(options.begin(), options.end(), [](const RotationOption& a, const RotationOption& b) {
        if (a.safetyScore == b.safetyScore) {
            return a.canMove > b.canMove; // Prefer options that allow movement
        }
        return a.safetyScore > b.safetyScore;
    });

	return options.empty() ? ActionRequest::DoNothing : options[0].action;
}

/**
 * @brief Evaluates a rotation option, assigning a safety score.
 *
 * @param rotation The rotation action to consider.
 * @param newDir The new direction after rotation.
 * @param oldDir The old direction before rotation.
 * @return A RotationOption containing safety evaluation.
 */
RotationOption TankAlgorithm_212535058_324022904::rotationOption(ActionRequest rotation, Direction newDir,Direction oldDir ) {
	RotationOption option;
	option.action = rotation;
	option.newDir = newDir;
	battle_info->setDirection(newDir);
	option.canMove = canMoveFwd();

	// Calculate safety score (higher is better)
	option.safetyScore = 0;

	// Check immediate safety
	auto [nextRow, nextCol] = nextStep(true, battle_info->getPosition(), battle_info->getDirection());
	if (!willBeHitIn(nextRow, nextCol, 1)) option.safetyScore += 2;
	if (!willBeHitIn(nextRow, nextCol, 2)) option.safetyScore += 1;

	// Bonus for moving towards open space
	if (option.canMove) {
		option.safetyScore += 2;

		// Check if this direction leads to more open space
		int openSpace = countOpenSpaceInDirection({nextRow, nextCol});
		option.safetyScore += openSpace;
	}
	// Reset direction
	battle_info->setDirection(oldDir);
	return option;
}

/**
 * @brief Counts the number of open spaces around a given position.
 *
 * @param pos The position to check around.
 * @return The number of open adjacent cells.
 */
int TankAlgorithm_212535058_324022904::countOpenSpaceInDirection(pair<int,int> pos) {
	auto [row, col] = pos;
	int openCount = 0;
	for(auto [dr, dc] : offsets) {
		int newRow = row + dr;
		int newCol = col + dc;

        if (isOccupierFree({newRow, newCol})){
            openCount++;
         }
  	}
        return openCount;
  }


/**
 * @brief Updates the positions of known shell objects on the map.
 *
 * Simulates shell movement by two steps in their current direction
 * and updates their location in the known objects map.
 */
void TankAlgorithm_212535058_324022904::moveKnownShells()
{
	auto knownObj = battle_info->getKnownObjects();
	auto copyKnownObj = battle_info->getKnownObjects();
	vector<pair<int,int>> pos_to_del;

	for (auto& [pos, objs] : knownObj){
		for (auto it = objs.begin(); it != objs.end();)
		{
			auto obj = *it;
			if (obj->getSymbol() == '*')
			{
				auto* shell = dynamic_cast<Shell*>(obj);
				if (shell && shell->getDirection() != None)
				{
					pair<int, int> oldPos = shell->getPos();
					pair<int, int> newPos = nextStep(true, oldPos, shell->getDirection());
					newPos = nextStep(true, newPos, shell->getDirection());

					// Record the move (old position, new position, object)
					obj->setPos(newPos); // Update the object's position
					copyKnownObj[newPos].push_back(obj);

					// Remove from current position (safe because we're using the iterator)
					it = objs.erase(it);
                    if(it == objs.end()) pos_to_del.push_back(pos);
					continue;
				}
			}
			++it;
		}
	}

    for(auto pos : pos_to_del) {
      copyKnownObj.erase(pos);
    }

    battle_info->setKnownObjects(copyKnownObj);
}