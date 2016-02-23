#include "Game.h"



void Game::initVectors(int size){

	for (int i = 0; i < size; i++){
		team1tab.push_back(0);
		team2tab.push_back(0);
		gameTab.push_back(0);
	}
	
}

void Game::newGame(int size){
	srand(time(NULL));
	turn = rand() % 2 + 1;
	initVectors(9);
}

Game::Game() { }
Game::~Game(){ }
