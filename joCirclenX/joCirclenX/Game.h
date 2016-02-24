#include <string>
#include <vector>
#include <time.h>
#include <iostream>

class Game
{
public:
	std::vector<int> clientsTeams;// (max_clients, -1);
	std::vector<int> madeMove;
	std::vector<int> team1tab;// (9, 0)
	std::vector<int> team2tab;// (9, 0);
	std::vector<int> gameTab;// (9, 0);
	int turn = 0;// = 2;
	int max_clients = 0;

	bool areAllVotes(int team);
	int chooseMove(int team);
	void clearVotes(int team, int max_clients);
	int getTeamCount(int team);
	bool initNewPlayer(char buffer0, int i);
	void initVectors(int size);
	int isOver();
	void newGame(int size);
	void newGame();
	void removePlayer(int player);
	void setElement(int move, int turn);
	void setVote(int vote, int team);

	void printVotes();
	void printGameTab();
	Game();
	~Game();
};

