# wumpus-world-qagent
An algorithm to train a Q-learning agent for Larry Holder's Wumpus World Simulator.

Steps:
1. Download the Wumpus World Simulator (https://github.com/holderlb/wumpus-world-simulator).
2. Copy this repository's files into the folder of the Wumpus World Simulator.
3. To run an already trained agent, run "make" followed by "./wumpsim -world testworld.txt" on a UNIX system (or a system that has the 'make' program installed and a C++ compiler).
4. To train an agent, modify the parameters.txt file. Replace testworld.txt as desired. Add -tries [# of episodes] to the previous command.

Parameters.txt:  
Line 1: Sets the training flag. Set to 1 to train an agent, 0 to disable training.  
Line 2: The file that a model will be loaded from. If it doesn't exist, a model isn't loaded and training continues.  
Line 3: The file the trained model will be saved to.  
Line 4: Sets epsilon, should be an integer value 0-100.  
Line 5: Sets the learning rate, should be a decimal between 0 and 1.  
Line 6: Sets the discount factor, should be a decimal between 0 and 1.  
