// Agent.h

#ifndef AGENT_H
#define AGENT_H

#include <cstring>
#include <string>
#include <bitset>
#include <limits>

#include "Action.h"
#include "Percept.h"
#include "Orientation.h"

class Agent
{
private:
	Action previousAction;
	Percept previousPercept;
	Percept currentPercept;

	int numMoves;

	int x, y;
	Orientation orientation;

	bool carryingGold;

	char state[12];
	int stateAsDecimal;
	int previousStateAsDecimal;
public:
	Agent ();
	~Agent ();
	void Initialize ();
	int observedReward ();
	void updateOrientation ();
	void updateLocation ();
	void calculateState ();
	Action Process (Percept& percept);
	void GameOver (int score);
};

#endif // AGENT_H
