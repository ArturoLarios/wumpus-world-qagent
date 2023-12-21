# Wumpus World Q-Agent

The Wumpus World is a grid-based environment where an adventurer (the player or agent) explores a cave system to collect treasures (gold) while avoiding various hazards (the Wumpus and pits). This project utilizes Larry Holder’s Wumpus World Simulator, a command-line interface implementation of the Wumpus World, as its framework. Initially designed for a human player, the framework can be adapted to create an autonomous agent by modifying the Agent.h and Agent.cc files. This project focuses on implementing a Q-learning agent.

### Steps to Use:

1. **[Download the Wumpus World Simulator](https://github.com/holderlb/wumpus-world-simulator):** 
2. **Copy Repository Files:** Place the files from this repository into the folder of the Wumpus World Simulator.
3. **Run an Already Trained Agent:** Execute "make" followed by "./wumpsim -world testworld.txt" on a UNIX system (or a system with 'make' and a C++ compiler installed).
4. **Train an Agent:** Modify the 'parameters.txt' file. Replace 'testworld.txt' with your desired world. Add "-tries [# of episodes]" to the previous command to specify the number of training episodes.

### Explanation of Parameters in 'parameters.txt':

- **Line 1:** Sets the training flag. Set to 1 to train an agent, 0 to disable training.
- **Line 2:** Specifies the file from which a model will be loaded. If it doesn't exist, no model will be loaded, and training will continue.
- **Line 3:** Determines the file to which the trained model will be saved.
- **Line 4:** Sets epsilon, an integer value between 0 and 100.
- **Line 5:** Sets the learning rate, a decimal value between 0 and 1.
- **Line 6:** Sets the discount factor, a decimal value between 0 and 1.

