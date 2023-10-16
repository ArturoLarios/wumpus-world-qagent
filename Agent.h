// Agent.h

#ifndef AGENT_H
#define AGENT_H

#include <string>
#include <bitset>

#include "Action.h"
#include "Percept.h"
#include "Orientation.h"

class Agent
{
private:
	char state[12];
	int stateAsDecimal;
	Action prevAction;
	string orientationToString[4]  = {"RIGHT", "UP", "LEFT", "DOWN"};
	int x, y, orientation;
public:
	Agent ();
	~Agent ();
	void Initialize ();
	void updateOrientation (Action action);
	void updateLocation ();
	void calculateState (bool stench, bool breeze, bool glitter);
	Action Process (Percept& percept);
	void GameOver (int score);
};

#endif // AGENT_H
