// Agent.cc

#include "Agent.h"

// Create the Q-Table as an unordered_map (dictionary) where each key (state) maps to an array of integers (Q-Values for the action set).
unordered_map<int, array<double, 6>> qTable;

// Training mode on = 1 or off = 0
int trainingFlag;

// Names of model files to load from and save to
string loadFile, saveFile;

// Probability of selecting a random action (epsilon-greedy policy)
// Different probabilities can be set for the agent while looking for the gold and while carrying it
int epsilonFindingGold, epsilonCarryingGold;

// Bellman equation parameters
double learningRate;
double discountFactor;

// Number of episodes completed
int episodeCount = 0;

// Episode number when finding gold finished training
int findingGoldEpisodeCount = 0;

// Episode number when carrying gold finished training
int carryingGoldEpisodeCount = 0;

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

// Return the index of the action with the highest Q-value for a given state
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
	// Tried to climb not at the entrance, wasted arrow, tried to grab nothing, or ran into a wall
	else if (previousAction == CLIMB || previousAction == SHOOT || previousAction == GRAB || currentPercept.Bump)
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
        parametersFile >> epsilonFindingGold;    // Read integer
        parametersFile >> epsilonCarryingGold;   // Read integer
        parametersFile >> learningRate;          // Read float
        parametersFile >> discountFactor;        // Read float

        // Close the file
        parametersFile.close();

        // Display the values read from the file
        cout << "trainingFlag: " << trainingFlag << endl;
        cout << "loadModel: " << loadFile << endl;
        cout << "saveModel: " << saveFile << endl;
        cout << "epsilonFindingGold: " << epsilonFindingGold << endl;
        cout << "epsilonCarryingGold: " << epsilonCarryingGold << endl;
        cout << "learningRate: " << learningRate << endl;
        cout << "discountFactor: " << discountFactor << endl;
    }
	else cerr << "Unable to open file parameters.txt" << endl;

	// If the training flag was not set the agent applies a greedy policy
    if (!trainingFlag) {
		epsilonFindingGold = 0;
		epsilonCarryingGold = 0;
	}

	// If an error occurs while loading the model, clear whatever was loaded
	if (!loadModel(loadFile))
		qTable.clear();
}

// Agent destructor
Agent::~Agent ()
{
	if (carryingGoldEpisodeCount) {
		cout << "Finished gold finding training in " << findingGoldEpisodeCount << " episodes." << endl;
		cout << "Finished gold carrying training in " << carryingGoldEpisodeCount << " episodes." << endl;
	}
	else cout << "Failed to train agent." << endl;
	saveModel(saveFile);
}

// Values initialized each time the agent tries a world
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
	// Set the leftmost bit based on if the agent is carrying the gold
	state[0] = carryingGold ? '1':'0';
	
	// Set the next 3 bits to the agent's x-coordinate converted to binary
	bitset<3> xBitset(x);
	string xString = xBitset.to_string();
	const char* xArray = xString.c_str();
	state[1] = xArray[0];
	state[2] = xArray[1];
	state[3] = xArray[2];

	// Set the next 3 bits to the agent's y-coordinate converted to binary
	bitset<3> yBitset(y);
	string yString = yBitset.to_string();
	const char* yArray = yString.c_str();
	state[4] = yArray[0];
	state[5] = yArray[1];
	state[6] = yArray[2];

	// Set the last 2 bits to the agent's orientation converted to binary 
	// Based on the enumeration in Orientation.h
	bitset<2> orientationBitset(static_cast<int>(orientation));
	string orientationString = orientationBitset.to_string();
	const char* orientationArray = orientationString.c_str();
	state[7] = orientationArray[0];
	state[8] = orientationArray[1];

	// Convert the bitset to an unsigned integer
	stateAsDecimal = bitset<32>(state).to_ulong();
}

// Main agent loop called each time the environment updates
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
	else if (previousAction == GRAB && previousPercept.Glitter)
		carryingGold = true;
	if (wumpusIsDead)
		currentPercept.Stench = false;

	calculateState();

	// If the state has not been seen before, add it to the qTable
	if (! qTable.count(stateAsDecimal))
		qTable[stateAsDecimal] = {0, 0, 0, 0, 0, 0};

	reward = observedReward();
	// Grabbed the gold
	if (previousAction == GRAB && previousPercept.Glitter) {
		qTable[previousStateAsDecimal][GRAB] = 100000;
		// Decrease epsilon by 1% every time the agent successfully exits with the gold
		if (epsilonFindingGold > 0)
			epsilonFindingGold -= 1;
		// Log the number of episodes it took for epsilon to reach 0%
		if (epsilonFindingGold == 0 && findingGoldEpisodeCount == 0)
			findingGoldEpisodeCount = episodeCount;
	}
	else if (previousAction != CLIMB)
		qTable[previousStateAsDecimal][static_cast<int>(previousAction)] = bellmanEquation(previousStateAsDecimal, previousAction, reward, stateAsDecimal);

	/* Generate random number between 1 and 100 */
	epsilonGreedy = rand() % 100 + 1;
	// Select epsilon based on if the agent is finding or carrying the gold
	int epsilon = epsilonFindingGold;
	if (carryingGold)
		epsilon = epsilonCarryingGold;
	// If the agent is standing on the gold, grab it
	if (currentPercept.Glitter)
		action = GRAB;
	// If the agent is standing at the cave entrance with the gold, climb
	else if (carryingGold && x == 0 && y == 0)
		action = CLIMB;
	/* If the value is less than epsilon select a random action */
	else if (epsilonGreedy <= epsilon)
		action = static_cast<Action>(rand() % 6);
	/* Otherwise select the action based on the Q-table */
	else
		action = static_cast<Action>(maxQIndex(stateAsDecimal));

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
	// Escaped with the gold
	if (previousAction == CLIMB && carryingGold && numMoves < 999) {
		qTable[stateAsDecimal][static_cast<int>(previousAction)] = 100000;
		// Decrease epsilon by 1% every time the agent successfully exits with the gold
		if (epsilonCarryingGold > 0)
			epsilonCarryingGold -= 1;
		// Log the number of episodes it took for epsilon to reach 0%
		if (epsilonCarryingGold == 0 && carryingGoldEpisodeCount == 0)
			carryingGoldEpisodeCount = episodeCount;
	}
	// Escaped without gold, fell into a pit or died to the Wumpus
	else if (numMoves < 999)
		qTable[stateAsDecimal][static_cast<int>(previousAction)] = -100000;
	cout << epsilonCarryingGold << " " <<  epsilonFindingGold << endl;
}