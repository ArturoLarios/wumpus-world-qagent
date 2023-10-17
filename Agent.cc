// Agent.cc

#include <iostream>
#include "Agent.h"
#include "WumpusWorld.h"

using namespace std;

// Create the Q-table
double qTable[2048][6];

// Probability of selecting a random action (epsilon-greedy policy)
double epsilon = 0.1;

// Bellman equation parameters
int learningRate = 1;
int discountFactor = 1;

double maxInRow(double* tableRow, int nColumns)
{
	double max = 0;
	for (int i = 0; i < nColumns; i++) {
		if (tableRow[i] > max)
			max = tableRow[i];
	}
	return max;
}

int getMaxColumn(double* tableRow, int nColumns, double max)
{
	for (int i = 0; i < 6; i++)
		if (tableRow[i] == max)
			return i;
	return 0;
}

double bellmanEquation(int state, Action action, int reward, int nextState)
{
	double nextMax = maxInRow(qTable[nextState], 6);
	return qTable[state][static_cast<int>(action)] + learningRate * (reward + (discountFactor * nextMax) - qTable[state][static_cast<int>(action)]);
}

int Agent::observedReward ()
{
	// Killed the Wumpus
	if (currentPercept.Scream)
		return 10;
	// Grabbed the gold
	else if (previousAction == GRAB && previousPercept.Glitter)
		return 100;
	// Ran into a wall
	if (previousAction == CLIMB || previousAction == SHOOT || previousAction == GRAB || currentPercept.Bump)
		return -100;
	// Moved to a different space or changed orientation
	else
		return -1;
}

Agent::Agent ()
{
	// Initialize Q-table cells to 0s
	memset(qTable, 0, sizeof(qTable));
}

Agent::~Agent ()
{

}

void Agent::Initialize ()
{
	numMoves = 0;
	carryingGold = false;
	state[11] = '\0';
	previousAction = CLIMB;
	x = 0;
	y = 0;
	orientation = RIGHT;
}

void Agent::updateOrientation ()
{
	if (previousAction == TURNLEFT)
		orientation = (orientation == DOWN) ? RIGHT:static_cast<Orientation>(static_cast<int>(orientation) + 1);
	else if (previousAction == TURNRIGHT)
		orientation = (orientation == RIGHT) ? DOWN:static_cast<Orientation>(static_cast<int>(orientation) - 1); 
}

void Agent::updateLocation ()
{
	if (orientation == RIGHT)
		x += 1;
	else if (orientation == UP)
		y += 1;
	else if (orientation == LEFT)
		x -= 1;
	else if (orientation == DOWN)
		y -= 1;
}

void Agent::calculateState ()
{
	state[0] = currentPercept.Stench ? '1':'0';
	state[1] = currentPercept.Breeze ? '1':'0';
	state[2] = carryingGold ? '1':'0';
	
	bitset<3> xBitset(x);
	string xString = xBitset.to_string();
	const char* xArray = xString.c_str();
	state[3] = xArray[0];
	state[4] = xArray[1];
	state[5] = xArray[2];

	bitset<3> yBitset(y);
	string yString = yBitset.to_string();
	const char* yArray = yString.c_str();
	state[6] = yArray[0];
	state[7] = yArray[1];
	state[8] = yArray[2];

	bitset<2> orientationBitset(static_cast<int>(orientation));
	string orientationString = orientationBitset.to_string();
	const char* orientationArray = orientationString.c_str();
	state[9] = orientationArray[0];
	state[10] = orientationArray[1];

	stateAsDecimal = bitset<32>(state).to_ulong();
}

Action Agent::Process (Percept& percept)
{
	int epsilonGreedy;
	int reward;
	Action action;
	currentPercept = percept;

	if (previousAction == GOFORWARD && ! currentPercept.Bump)
		updateLocation();
	else if (previousAction == TURNLEFT || previousAction == TURNRIGHT)
		updateOrientation();
	else if (previousAction == GRAB && previousPercept.Glitter == true)
		carryingGold = true;
	calculateState();

	reward = observedReward();
	if (previousAction != CLIMB)
		qTable[previousStateAsDecimal][static_cast<int>(previousAction)] = bellmanEquation(previousStateAsDecimal, previousAction, reward, stateAsDecimal);

	/* Generate random number between 1 and 100 */
	epsilonGreedy = rand() % 100 + 1;
	/* If the value is less than epsilon select a random action */
	if (epsilonGreedy <= epsilon * 100)
		action = static_cast<Action>(rand() % 6);
	/* Otherwise select the action based on the Q-table */
	else {
		double maxQ = maxInRow(qTable[stateAsDecimal], 6);
		action = static_cast<Action>(getMaxColumn(qTable[stateAsDecimal], 6, maxQ));
	}

	previousStateAsDecimal = stateAsDecimal;
	previousAction = action;
	previousPercept = percept;

	numMoves++;
	return action;
}

void Agent::GameOver (int score)
{
	int reward;
	// Escaped with the gold
	if (previousAction == CLIMB && carryingGold)
		qTable[stateAsDecimal][static_cast<int>(previousAction)] = 100000;
	// Escaped without gold, fell into a pit or died to the Wumpus
	else if (numMoves < 1000)
		qTable[stateAsDecimal][static_cast<int>(previousAction)] = -100000;
}

