#include <string>
#include <vector>
#include <time.h>

class Game
{
public:

	std::vector<int> team1tab;// (9, 0);
	std::vector<int> team2tab;// (9, 0);
	std::vector<int> gameTab;// (9, 0);
	int turn = 0;// = 2;


	void initVectors(int size);
	void newGame(int size);
	Game();
	~Game();
};

