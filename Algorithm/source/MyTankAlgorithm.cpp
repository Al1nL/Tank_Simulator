#include "../header/MyTankAlgorithm.h"
#include "../header/TankBattleInfo.h"
#include "../../UserCommon/header/Shell.h"

using namespace UserCommon__212535058_324022904;
using namespace Algorithm_212535058_324022904;

/**
 * @brief Constructs a MyTankAlgorithm instance with game parameters.
 * @param player_index Index of the player that the tank will belong to.
 * @param tank_index Index of this tank.
 */
MyTankAlgorithm::MyTankAlgorithm(int player_index, int tank_index) : player_index(player_index), tank_index(tank_index), battle_info(make_unique<TankBattleInfo>(tank_index, player_index)) {
}

/**
 * @brief Updates the internal battle info object with the latest game state.
 *
 * Copies opponent data, position, direction, known objects, and shell count from the provided BattleInfo.
 *
 * @param info The BattleInfo object containing the current state.
 */
void MyTankAlgorithm::updateBattleInfo(BattleInfo &info)
{
	auto tank_info = dynamic_cast<TankBattleInfo &>(info);
	battle_info->setOpponents(tank_info.getOpponents());
	battle_info->setPosition(tank_info.getPosition().first, tank_info.getPosition().second);
	battle_info->setDirection(tank_info.getDirection());
	battle_info->setKnownObjects(tank_info.getKnownObjects());
	battle_info->setRemainingShells(tank_info.getRemainingShells());
	battle_info->setMapSize(tank_info.getMapSize().first, tank_info.getMapSize().second);
}

/**
 * @brief Determines and returns the next action.
 *
 * @return The decided ActionRequest.
 */
ActionRequest MyTankAlgorithm::getAction()
{
	ActionRequest action = decideAction();
	updateInnerInfoAfterAction(action);
	return action;
}

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
pair<int, int> MyTankAlgorithm::nextStep(bool forward, const pair<int, int> pos, const Direction dir)
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
void MyTankAlgorithm::rotate(ActionRequest action)
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
bool MyTankAlgorithm::isValidMove(ActionRequest action)
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
bool MyTankAlgorithm::isOccupierFree(pair<int, int> pos)
{
	return battle_info->getObjectByPosition(pos) == nullptr;
}

/**
 * @brief Determines if the player should shoot at the opponent based on alignment and available shells.
 *
 * @param opponentPos The opponent's position.
 * @return True if the player should shoot, false otherwise.
 */
bool MyTankAlgorithm::shouldShootOpponent(const pair<int, int> &opponentPos)
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
bool MyTankAlgorithm::canMoveFwd()
{
	auto [r, c] = nextStep(true, battle_info->getPosition(), battle_info->getDirection());
	return isOccupierFree({r, c}) && !willBeHitIn(r, c, 1);
}

/**
 * @brief Checks if the player can move backward safely.
 *
 * @return True if moving backward is possible and safe, false otherwise.
 */
bool MyTankAlgorithm::canMoveBack()
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
bool MyTankAlgorithm::canSafelyBack(int backR, int backC)
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
ActionRequest MyTankAlgorithm::checkForEscape()
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
bool MyTankAlgorithm::willBeHitIn(int row, int col, int t)
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
bool MyTankAlgorithm::isAlignedWithOpponent(pair<int, int> opponentPos)
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
ActionRequest MyTankAlgorithm::determineRotation(Direction currentDir, Direction desiredDir)
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
Direction MyTankAlgorithm::calculateDirection(int currRow, int currCol, int targetRow, int targetCol)
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
OppData MyTankAlgorithm::getClosestOpponent()
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
	;
}

/**
 * @brief Calculates the estimated number of actions to reach an opponent.
 *
 * Estimates the total number of moves and rotations needed to reach the opponent's position and direction.
 *
 * @param opp The opponent data containing position and direction.
 * @return int Estimated total actions (moves + rotations).
 */
int MyTankAlgorithm::calculateActionsToOpponent(const OppData &opp)
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
void MyTankAlgorithm::updateInnerInfoAfterAction(ActionRequest action)
{
	pair<int, int> newPos = {-1, -1};
	if (battle_info->isWaitingToReverse() && battle_info->getWaitingForBackward() == 0)
		action = ActionRequest::MoveBackward; // back after 3 rounds, no matter what the other action now is, it is ignored?
	if(action!=ActionRequest::Shoot){
		battle_info->decreaseShootCooldown();
		}
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