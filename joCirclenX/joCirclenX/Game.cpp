#include "Game.h"

bool Game::areAllVotes(int team){
	int count = 0;
	for (int i = 0; i < max_clients; i++){
		//cout << i << " " << madeMove[i] << "\t";
		if (clientsTeams[i] == team)
			if (madeMove[i] != team)
				return false;
	}
	return true;
}

int Game::chooseMove(int team){
	std::vector<int> maxTab;
	std::vector<int> tab;
	if (team == 1)
		tab = team1tab;
	else if (team == 2)
		tab = team2tab;
	else {
		std::cout << "niepoprawny team!" << std::endl;
		return 0;
	}

	int max = 0;
	int move = 0;
	for (int i = 0; i < tab.size(); i++)
		if (tab[i] > max)
			max = tab[i];
	//cout << "max = " << max << endl;
	//cout << "maxtab: ";
	for (int i = 0; i < tab.size(); i++){
		if (tab[i] == max){
			maxTab.push_back(i);
			//	cout << "dodaje ";
			//	cout << i << "\t";
		}
	}
	move = maxTab[rand() % maxTab.size()];
	std::cout << std::endl << "wylosowane pole: " << move << std::endl;

	return move;
}

void Game::clearVotes(int team, int max_clients){
	for (int i = 0; i < max_clients; i++)
		if (madeMove[i] == team)
			madeMove[i] = 0;
	if (team == 1){
		for (int i = 0; i < team1tab.size(); i++){
			team1tab[i] = 0;
		}
	}
	else if (team == 2){
		for (int i = 0; i < team2tab.size(); i++){
			team2tab[i] = 0;
		}
	}
}

int Game::getTeamCount(int team){
	int count = 0;
	for (int i = 0; i < max_clients; i++)
		if (clientsTeams[i] == team)
			count++;
	return count;
}

bool Game::initNewPlayer(char buffer0, int i){
	int team = (int)buffer0 - (int)48; //(pierwszy znak w pierwszej wiadomosci to druzyna która wybral)

	if (team == 1 || team == 2){			//przypisujemy mu team
		clientsTeams[i] = team;
		madeMove[i] = 0;
		//printf("clientsTeams[%d] = %d \n", i, team);
		return true;
	} else
		return false;
}

void Game::initVectors(int size){

	for (int i = 0; i < size; i++){
		team1tab.push_back(0);
		team2tab.push_back(0);
		gameTab.push_back(0);
	}

	for (int i = 0; i < max_clients; i++){
		madeMove.push_back(-1);
		clientsTeams.push_back(-1);
	}
	
}

int Game::isOver(){
	/*
	sprawdzamy czy :
	
	- poziomo takie same
	- pionowo takie same
	- skosy takie same
	- cala zapelniona tablica

	-1 - gra sie toczy dalej
	 0 - remis
	 1 - gracz 1 wygrywa
  	 2 - gracz 2 wygrywa
	
	*/

	//PIONOWO
	for (int i = 0, j = 0; i < gameTab.size()/3; i++, j+=3){
		if (gameTab[i] != 0 && gameTab[i + 3] == gameTab[i + 6] &&
			gameTab[i] == gameTab[i + 3] && gameTab[i] == gameTab[i + 6]){ // pionowo
			return gameTab[i];
		} else if (gameTab[j] != 0 && gameTab[j + 2] == gameTab[j + 1] &&
			gameTab[j + 2] == gameTab[j] && gameTab[j + 1] == gameTab[j]) {// poziomo
			return gameTab[j];
		} 
	}

	//SKOS
	if (gameTab[4] != 0){ //srodkowy jest zajety
		if (gameTab[2] == gameTab[4] && gameTab[4] == gameTab[6]){
			printf("/\n");
			return gameTab[4];
		}
		else if (gameTab[0] == gameTab[4] && gameTab[4] == gameTab[8]){
			printf("\\\n");
			return gameTab[4];
		}
	}

	//REMIS
	bool fullTab = true;
	for each (int i in gameTab) 
		if (i == 0)
			fullTab = false;
	
	if (fullTab == true)
		return 0;	

	return -1;
}

void Game::newGame(int max_clients){
	srand(time(NULL));
	turn = rand() % 2 + 1;
	this->max_clients = max_clients;
	initVectors(9);
}

void Game::removePlayer(int player){
	
	clientsTeams[player] = -1;
	madeMove[player] = -1;
}

void Game::setElement(int move, int turn){
	gameTab[move] = turn;
}

void Game::setVote(int vote, int team){
	if (vote >= 0 && vote < 10){
		if (team == 1){
			team1tab[vote]++;
		}
		else if (team == 2) {
			team2tab[vote]++;
		}
		else {
			printf("team error!");
		}
	}
	else {
		printf("vote error!");
	}
}

/* PRINTY */
void Game::printVotes(){
	for (int i = 1; i < 10; i++){
		//printf("%d ", team1tab[i]);
		if (i % 3 == 0){
			//printf("\n");
		}
			
	}
	//printf("\n\n");
	for (int i = 1; i < 10; i++){
		//printf("%d ", team2tab[i]);
		if (i % 3 == 0){
			//printf("\n");
		}
			
	}
}

void Game::printGameTab(){
	for (int i = 0; i < 9; i++){
		//printf("%d ", gameTab[i]);
		if (i + 1 % 3 == 0){
			//printf("\n");
		}
			
	}
}


Game::Game() { }
Game::~Game(){ }
