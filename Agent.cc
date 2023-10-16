// Agent.cc

#include <iostream>
#include "Agent.h"
#include "WumpusWorld.h"

using namespace std;

Agent::Agent ()
{

}

Agent::~Agent ()
{

}

void Agent::Initialize ()
{
	state[11] = '\0'; 
	prevAction = CLIMB;
	x = 0;
	y = 0;
	orientation = RIGHT;
}

void Agent::updateOrientation (Action action)
{
	if (action == TURNLEFT)
		orientation = (orientation == 3) ? 0:orientation + 1;
	else if (action == TURNRIGHT)
		orientation = (orientation == 0) ? 3:orientation - 1; 
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

void Agent::calculateState (bool stench, bool breeze, bool glitter)
{
	state[0] = stench ? '1':'0';
	state[1] = breeze ? '1':'0';
	state[2] = glitter ? '1':'0';
	
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

	bitset<2> orientationBitset(orientation);
	string orientationString = orientationBitset.to_string();
	const char* orientationArray = orientationString.c_str();
	state[9] = orientationArray[0];
	state[10] = orientationArray[1];

	stateAsDecimal = bitset<32>(state).to_ulong();
}

Action Agent::Process (Percept& percept)
{
	char c;
	Action action;
	bool validAction = false;

	if (prevAction == GOFORWARD && ! percept.Bump)
		updateLocation();
	cout << "At (" << x << ", " << y << ") facing " << orientationToString[orientation] << endl;
	calculateState(percept.Stench, percept.Breeze, percept.Glitter);
	cout << "State: " << state << " (" << stateAsDecimal << ")" << endl;

	while (! validAction)
	{
		validAction = true;
		cout << "Action? ";
		cin >> c;
		if (c == 'f') {
			action = GOFORWARD;
		} else if (c == 'l') {
			action = TURNLEFT;
			updateOrientation(action);
		} else if (c == 'r') {
			action = TURNRIGHT;
			updateOrientation(action);
		} else if (c == 'g') {
			action = GRAB;
		} else if (c == 's') {
			action = SHOOT;
		} else if (c == 'c') {
			action = CLIMB;
		} else {
			cout << "Huh?" << endl;
			validAction = false;
		}
	}
	prevAction = action;

	return action;
}

void Agent::GameOver (int score)
{

}

