// Agent.cc

#include "Agent.h"

using namespace std;

// Create the Q-Table as an unordered_map where each key (state) maps to an array of integers (Q-Values for the action set).
unordered_map<int, array<double, 6>> qTable;

// Probability of selecting a random action (epsilon-greedy policy)
// Different probabilities can be set for the agent while looking for the gold and while carrying it
double epsilonFindingGold = 0.5, epsilonCarryingGold = 0.5;

// Bellman equation parameters
int learningRate = 1;
int discountFactor = 1;

void Agent::saveModel() {
    // Get the file name from an environment variable
    const char* modelFileName = getenv("WW_SAVE_MODEL");

    if (modelFileName == nullptr) {
        cerr << "Set the WW_SAVE_MODEL environment variable to save the trained model to a file." << endl;
        return;
    }

    // Open the model file for writing
    ofstream modelFile(modelFileName);
    if (!modelFile) {
        cerr << "An error occurred while attempting to save the model (couldn't open file)." << endl;
        return;
    }

    // Iterate through the qTable and write the data to the file
    for (const auto& entry : qTable) {
        int key = entry.first;
        const array<double, 6>& values = entry.second;

        modelFile << key << " "; // Write the key

        for (double value : values) {
            modelFile << value << " "; // Write the values
        }
        modelFile << endl; // Add a newline after each row
    }

    // Close the model file
    modelFile.close();
}

bool Agent::loadModel() {
    // Get the file name from an environment variable
    const char* modelFileName = getenv("WW_LOAD_MODEL");

    if (modelFileName == nullptr) {
        cerr << "Set the WW_LOAD_MODEL environment variable to load a model from a file." << endl;
        return false;
    }

    // Open the model file for reading
    ifstream modelFile(modelFileName);
    if (!modelFile) {
        cerr << "An error occurred while attempting to load the model (couldn't open file: " << modelFileName << ")." << endl;
        return false; // Exit with an error code
    }

    // Clear the existing qTable
    qTable.clear();

    // Read data from the file and populate the qTable
    int key;
    array<double, 6> values;

    while (modelFile >> key) {
        for (double& value : values) {
            if (!(modelFile >> value)) {
                cerr << "Error reading values from the file." << endl;
                return false; // Exit with an error code
            }
        }
        qTable[key] = values;
    }

    // Check for end-of-file
    if (!modelFile.eof()) {
        cerr << "Error reading the entire file." << endl;
        return false; // Exit with an error code
    }

    // Close the model file
    modelFile.close();

    return true;
}

int maxQIndex(int state)
{
	double max = 0;
	int indexOfMax = 0;

	for (int i = 0; i < 6; i++) {
		if (qTable[state][i] > max) {
			max = qTable[state][i];
			indexOfMax = i;
		}
	}

	return indexOfMax;
}

double bellmanEquation(int state, Action action, int reward, int nextState)
{
	double nextStateMaxQ = qTable[nextState][maxQIndex(nextState)];
	return qTable[state][static_cast<int>(action)] + learningRate * (reward + (discountFactor * nextStateMaxQ) - qTable[state][static_cast<int>(action)]);
}

int Agent::observedReward ()
{
	// Killed the Wumpus
	if (currentPercept.Scream)
		return 10;
	// Grabbed the gold
	else if (previousAction == GRAB && previousPercept.Glitter) {
		if (epsilonFindingGold > 0)
			epsilonFindingGold -= 0.01;
		return 100;
	}
	// Tried to climb not at the entrance, wasted arrow, tried to grab nothing, or ran into a wall
	if (previousAction == CLIMB || previousAction == SHOOT || previousAction == GRAB || currentPercept.Bump)
		return -100;
	// Moved to a different space or changed orientation
	else
		return -1;
}

Agent::Agent ()
{
    // Get the file name from an environment variable
    const char* trainingMode = getenv("WW_TRAINING_MODE");

    if (trainingMode == nullptr)
        cerr << "Set the WW_TRAINING_MODE environment variable to disable training mode." << endl;
    else {
		epsilonFindingGold = 0;
		epsilonCarryingGold = 0;
	}

	// If an error occurs while loading the model, clear whatever was loaded
	if (!loadModel())
		qTable.clear();
}

Agent::~Agent ()
{
	saveModel();
}

void Agent::Initialize ()
{
	numMoves = 0;
	wumpusIsDead = false;
	carryingGold = false;
	state[9] = '\0';
	previousAction = CLIMB;
	x = 0;
	y = 0;
	orientation = RIGHT;
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

void Agent::updateOrientation ()
{
	if (previousAction == TURNLEFT)
		orientation = (orientation == DOWN) ? RIGHT:static_cast<Orientation>(static_cast<int>(orientation) + 1);
	else if (previousAction == TURNRIGHT)
		orientation = (orientation == RIGHT) ? DOWN:static_cast<Orientation>(static_cast<int>(orientation) - 1); 
}

void Agent::calculateState ()
{
	state[0] = carryingGold ? '1':'0';
	
	bitset<3> xBitset(x);
	string xString = xBitset.to_string();
	const char* xArray = xString.c_str();
	state[1] = xArray[0];
	state[2] = xArray[1];
	state[3] = xArray[2];

	bitset<3> yBitset(y);
	string yString = yBitset.to_string();
	const char* yArray = yString.c_str();
	state[4] = yArray[0];
	state[5] = yArray[1];
	state[6] = yArray[2];

	bitset<2> orientationBitset(static_cast<int>(orientation));
	string orientationString = orientationBitset.to_string();
	const char* orientationArray = orientationString.c_str();
	state[7] = orientationArray[0];
	state[8] = orientationArray[1];

	stateAsDecimal = bitset<32>(state).to_ulong();
}

Action Agent::Process (Percept& percept)
{
	int epsilonGreedy;
	int reward;
	Action action;
	currentPercept = percept;

	if (previousAction == SHOOT && currentPercept.Scream)
		wumpusIsDead = true;
	else if (previousAction == GOFORWARD && ! currentPercept.Bump)
		updateLocation();
	else if (previousAction == TURNLEFT || previousAction == TURNRIGHT)
		updateOrientation();
	else if (previousAction == GRAB && previousPercept.Glitter == true)
		carryingGold = true;
	if (wumpusIsDead)
		currentPercept.Stench = false;

	calculateState();

	// If the state has not been seen before, add it to the qTable
	if (! qTable.count(stateAsDecimal))
		qTable[stateAsDecimal] = {0, 0, 0, 0, 0, 0};

	reward = observedReward();
	if (previousAction != CLIMB)
		qTable[previousStateAsDecimal][static_cast<int>(previousAction)] = bellmanEquation(previousStateAsDecimal, previousAction, reward, stateAsDecimal);

	/* Generate random number between 1 and 100 */
	epsilonGreedy = rand() % 100 + 1;
	// Select epsilon based on if the agent is finding or carrying the gold
	double epsilon = epsilonFindingGold;
	if (carryingGold)
		epsilon = epsilonCarryingGold;
	/* If the value is less than epsilon select a random action */
	if (epsilonGreedy <= epsilon * 100)
		action = static_cast<Action>(rand() % 6);
	/* Otherwise select the action based on the Q-table */
	else
		action = static_cast<Action>(maxQIndex(stateAsDecimal));

	previousStateAsDecimal = stateAsDecimal;
	previousAction = action;
	previousPercept = percept;

	numMoves++;
	return action;
}

void Agent::GameOver (int score)
{
	// Escaped with the gold
	if (previousAction == CLIMB && carryingGold) {
		qTable[stateAsDecimal][static_cast<int>(previousAction)] = 100000;
		if (epsilonCarryingGold > 0)
			epsilonCarryingGold -= 0.01;
	}
	// Escaped without gold, fell into a pit or died to the Wumpus
	else if (numMoves < 1000)
		qTable[stateAsDecimal][static_cast<int>(previousAction)] = -100000;
}