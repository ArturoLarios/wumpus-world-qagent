// Agent.h

#ifndef AGENT_H
#define AGENT_H

#include <cstdlib>
#include <cstring>
#include <string>
#include <bitset>
#include <limits>
#include <fstream>
#include <iostream>
#include <unordered_map>
#include <array>

#include "Action.h"
#include "Percept.h"
#include "Orientation.h"

using namespace std;

class Agent
{
private:
	Action previousAction;
	Percept previousPercept;
	Percept currentPercept;

	int numMoves;

	int x, y;
	Orientation orientation;

	bool wumpusIsDead;
	bool carryingGold;

	char state[10];
	int stateAsDecimal;
	int previousStateAsDecimal;
public:
	Agent ();
	~Agent ();
	bool loadModel (string loadFile);
	void saveModel (string saveFile);
	void Initialize ();
	int observedReward ();
	void updateLocation ();
	void updateOrientation ();
	void calculateState ();
	Action Process (Percept& percept);
	void GameOver (int score);
};

#endif // AGENT_H