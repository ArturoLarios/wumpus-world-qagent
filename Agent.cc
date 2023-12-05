// Agent.cc

#include "Agent.h"

// Create the Q-Table as an unordered_map (dictionary) where each key (state) maps to an array of integers (Q-Values for the action set).
unordered_map<int, array<double, 4>> qTable;

// Training mode on = 1 or off = 0
int trainingFlag;

// Names of model files to load from and save to
string loadFile, saveFile;

// Probability of selecting a random action (epsilon-greedy policy)
// Different probabilities can be set for the agent while looking for the gold and while carrying it
int epsilon;

// Bellman equation parameters
double learningRate;
double discountFactor;

// Number of episodes completed
int episodeCount = 0;
int converged = 0;
int convergenceCount = 0;

// Save the current agent model (Q-table) to a file
void Agent::saveModel(string saveFile) {
    // Open the model file for writing
    ofstream modelFile(saveFile);
    if (!modelFile) {
        cerr << "An error occurred while attempting to save the model (couldn't open file)." << endl;
        return;
    }

    // Iterate through the qTable and write the data to the file
    for (const auto& entry : qTable) {
        int key = entry.first;
        const array<double, 4>& values = entry.second;

        modelFile << key << " "; // Write the key

        for (double value : values) {
            modelFile << value << " "; // Write the values
        }
        modelFile << endl; // Add a newline after each row
    }

    // Close the model file
    modelFile.close();
}

// Load a model (Q-table) for the agent from a file
bool Agent::loadModel(string loadFile) {
    // Open the model file for reading
    ifstream modelFile(loadFile);
    if (!modelFile) {
        cerr << "An error occurred while attempting to load the model (couldn't open file: " << loadFile << ")." << endl;
        return false; // Exit with an error code
    }

    // Clear the existing qTable
    qTable.clear();

    // Read data from the file and populate the qTable
    int key;
    array<double, 4> values;

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

// Return the index of the action with the highest Q-value for a given state
int maxQIndex(int state)
{
	double max = 0;
	int indexOfMax = 0;

	for (int i = 0; i < 4; i++) {
		if (qTable[state][i] > max) {
			max = qTable[state][i];
			indexOfMax = i;
		}
	}

	return indexOfMax;
}

// Return the updated Q-value for a  state-action pair given the observed reward and next state
double bellmanEquation(int state, Action action, int reward, int nextState)
{
	double nextStateMaxQ = qTable[nextState][maxQIndex(nextState)];
	return qTable[state][static_cast<int>(action)] + learningRate * (reward + (discountFactor * nextStateMaxQ) - qTable[state][static_cast<int>(action)]);
}

// Calculate the observed reward for the previous state-action pair
int Agent::observedReward ()
{
	// Killed the Wumpus
	if (currentPercept.Scream)
		return 10;
	// Wasted arrow or ran into a wall
	else if (previousAction == SHOOT || currentPercept.Bump)
		return -100;
	// Moved to a different space or changed orientation
	else
		return -1;
}

// Agent constructor
Agent::Agent ()
{
    // Open the parameters file
    ifstream parametersFile("parameters.txt");

    if (parametersFile.is_open()) {
        // Read values from the file line by line
        parametersFile >> trainingFlag;          // Read integer
        parametersFile >> loadFile;             // Read string
        parametersFile >> saveFile;             // Read string
        parametersFile >> epsilon;    // Read integer
        parametersFile >> learningRate;          // Read float
        parametersFile >> discountFactor;        // Read float

        // Close the file
        parametersFile.close();
    }
	else cerr << "Unable to open file parameters.txt" << endl;

	// If the training flag was not set the agent applies a greedy policy
    if (!trainingFlag)
		epsilon = 0;

	// If an error occurs while loading the model, clear whatever was loaded
	if (!loadModel(loadFile))
		qTable.clear();
}

// Agent destructor
Agent::~Agent ()
{
	if (trainingFlag)
		cout << "Episodes to converge: " << convergenceCount << endl;
	saveModel(saveFile);
}

// Values initialized each time the agent tries a world
void Agent::Initialize ()
{
	numMoves = 0;
	carryingGold = false;
	previousAction = CLIMB;
	x = 0;
	y = 0;
	orientation = RIGHT;
	pathToEntrance.push(CLIMB);
	goingBackwards = false;
}

// Update the agent's knowledge of its location (x, y) based on the direction it faced before moving
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

// Update the agent's orientation based on the direction it faced and if it turned left or right
void Agent::updateOrientation ()
{
	if (previousAction == TURNLEFT)
		orientation = (orientation == DOWN) ? RIGHT:static_cast<Orientation>(static_cast<int>(orientation) + 1);
	else if (previousAction == TURNRIGHT)
		orientation = (orientation == RIGHT) ? DOWN:static_cast<Orientation>(static_cast<int>(orientation) - 1); 
}

// Calculate the environment state as a binary string, and convert it to a decimal value
void Agent::calculateState ()
{
	// Set the leftmost 3 bits to the agent's x-coordinate converted to binary
	std::bitset<3> xBitset(x);
	for (int i = 0; i < 3; ++i) {
		state[i] = xBitset[i];
	}

	// Set the next 3 bits to the agent's y-coordinate converted to binary
	std::bitset<3> yBitset(y);
	for (int i = 0; i < 3; ++i) {
		state[i + 3] = yBitset[i];
	}

	// Set the last 2 bits to the agent's orientation converted to binary 
	// Based on the enumeration in Orientation.h
	std::bitset<2> orientationBitset(static_cast<int>(orientation));
	for (int i = 0; i < 2; ++i) {
		state[i + 6] = orientationBitset[i];
	}

	// Convert the bitset to an unsigned integer
	stateAsDecimal = static_cast<int>(state.to_ulong());
}

// Main agent loop called each time the environment updates
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

	calculateState();

	// If the state has not been seen before, add it to the qTable
	if (! qTable.count(stateAsDecimal))
		qTable[stateAsDecimal] = {0, 0, 0, 0};

	reward = observedReward();
	if (previousAction != CLIMB)
		qTable[previousStateAsDecimal][static_cast<int>(previousAction)] = bellmanEquation(previousStateAsDecimal, previousAction, reward, stateAsDecimal);

	/* Generate random number between 1 and 100 */
	epsilonGreedy = rand() % 100 + 1;
	// If the agent is carrying the gold, take the path in reverse
	if (carryingGold) {
		action = pathToEntrance.top();
		if (! goingBackwards && action == GOFORWARD) {
			pathToEntrance.push(TURNLEFT);
			pathToEntrance.push(TURNLEFT);
			action = pathToEntrance.top();
			goingBackwards = true;
		}
		pathToEntrance.pop();
		return action;
	}
	// If the agent is standing on the gold, grab it
	if (currentPercept.Glitter) {
		qTable[previousStateAsDecimal][static_cast<int>(previousAction)] = 100;
		action = GRAB;
		carryingGold = true;
		// Decrease epsilon by 1% every time the agent successfully exits with the gold
		if (epsilon > 0)
			epsilon -= 1;
		if (epsilon == 0 && !converged) {
			convergenceCount = episodeCount;
			converged = 1;
		}
	}
	/* If the value is less than epsilon select a random action */
	else if (epsilonGreedy <= epsilon) {
		action = static_cast<Action>(rand() % 4);
		action = (static_cast<int>(action) == 3) ? static_cast<Action>(4):action;
	}
	/* Otherwise select the action based on the Q-table */
	else
		action = static_cast<Action>(maxQIndex(stateAsDecimal));

	if (action == GOFORWARD)
		pathToEntrance.push(GOFORWARD);
	else if (action == TURNLEFT)
		pathToEntrance.push(TURNRIGHT);
	else if (action == TURNRIGHT)
		pathToEntrance.push(TURNLEFT);
	

	previousStateAsDecimal = stateAsDecimal;
	previousAction = action;
	previousPercept = currentPercept;

	numMoves++;
	return action;
}

// Calculate rewards based on game over condition and update the Q-table
void Agent::GameOver (int score)
{
	episodeCount++;
	// Escaped without gold, fell into a pit or died to the Wumpus
	if (numMoves < 999)
		qTable[previousStateAsDecimal][static_cast<int>(previousAction)] = -100;
}