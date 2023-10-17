// Agent.h

#ifndef AGENT_H
#define AGENT_H

#include <cstring>
#include <string>
#include <bitset>

#include "Action.h"
#include "Percept.h"
#include "Orientation.h"

int maxInRow(int* tableRow, int nColumns);

class Agent
{
private:
	Action previousAction;
	Percept previousPercept;
	Percept currentPercept;

	int x, y;
	Orientation orientation;

	bool carryingGold;

	char state[12];
	int stateAsDecimal;
public:
	Agent ();
	~Agent ();
	void Initialize ();
	void updateOrientation ();
	void updateLocation ();
	void calculateState ();
	Action Process (Percept& percept);
	void GameOver (int score);
};

#endif // AGENT_H
