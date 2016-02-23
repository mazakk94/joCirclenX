#include <stdio.h>  //printf
#include <cstdlib>  //system(pause)
#include <winsock2.h> //windows socket
#include <Ws2tcpip.h>
#include <vector>
#include <string>
#include <iostream> // cout <<
#include <ctime> //srand time
#include "Game.h"

#pragma comment(lib,"ws2_32.lib") //Winsock Library

/*
TODO:
[x] podzial na kolejki (blokowanie tych co teraz nie graja) 
[x] wysylanie graczowi info o tym czyja kolej jest - po tym bedzie wiedzial czy udalo mu sie zaglosowac
[x] wybór ruchu dla graczy na podstawie glosowania
[x] zajmowanie pól, wysylanie wybranego pola do graczy (jakos trzeba to wyroznic)
[x] zerowanie g³osowania i informowanie graczy o nastepnym glosowaniu
[x] blokowanie glosowania na pola które s¹ ju¿ zajête
[x] migracja do klasy Game
[x] logika gry
[ ] resetowanie gry jak koniec - czekamy, az wszyscy gracze klikna okejke (jak ktos sie rozlaczy to tez trzeba wiedziec zeby na niego nie czekac)
[ ] jak ktos wyjdzie w trakcie gry to musi to wykryc i zaktualizowac zeby nie bylo zakleszczenia


[ ] jak sie nie uda polaczyc to obsluga musi byc
[ ] brak druzyny przeciwnej
[ ] algorytm nagle'a

extra:
[ ] pousuwac niepotrzebnie przekazywane parametry, ktore sa juz w klasie Game
[ ] co jak serwer padnie?
[ ] czemu tak dlugo laczy sie z serwerem?
*/

using namespace std;

/* INIT GLOBALI */
WSADATA wsaData;
SOCKET master, new_socket, s;
int max_clients = 10;
int activity, addrlen, valread;
vector<SOCKET> clients(max_clients, 0);
struct sockaddr_in server, address; 

//dajemy na poczatek wiadomosci stan gry dla gracza odpowiedniej druzyny
string initBuffer(const char * buffer, int team, Game game){
	bool flag = false;
	string str_buffer(buffer);
	//cout << "bufor zaraz po wejsciu w init: " << str_buffer << endl;
	string new_buffer = "";
	if (team == 1){
		for (unsigned int i = 0; i < game.team1tab.size(); i++){
			new_buffer += to_string(game.team1tab[i]);
		}
	} else if (team == 2) {
		for (unsigned int i = 0; i < game.team1tab.size(); i++){
			new_buffer += to_string(game.team2tab[i]);
		}
	} else {
		flag = true;
		new_buffer += "000000000"; //pusty stan gry dla nowego gracza
		//printf("wrong team/new player!\n");
	}
	//cout << "old_buffer: " << new_buffer << endl;
	new_buffer += to_string(game.turn); //dorzucamy czyja kolej jest
	//new_buffer += str_buffer; //
	//cout << "new_buffer: " << new_buffer << endl;
	//const char * ret_buffer = new_buffer.c_str();
	//cout << "ret_buffer: " << ret_buffer << endl;

	for (unsigned int i = 0; i < game.gameTab.size(); i++){
		new_buffer += to_string(game.gameTab[i]);
	}
	
	if (flag){ //dorzucamy wiadomosc powitalna nowemu graczowi
		new_buffer += str_buffer;
	}
	return new_buffer;
}

void initServer(){
	//Windows Socket init
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		fprintf(stderr, "WSAStartup failed.\n");
		exit(1);
	}
	//Create a socket
	if ((master = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
		printf("Could not create socket : %d", WSAGetLastError());
		exit(EXIT_FAILURE);
	}

	//Prepare the sockaddr_in structure
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = inet_pton(AF_INET, "localhost", &(server.sin_addr));
	server.sin_port = htons(1111);

	//Bind
	if (bind(master, (struct sockaddr *)&server, sizeof(server)) == SOCKET_ERROR) {
		printf("Bind failed with error code : %d", WSAGetLastError());
		exit(EXIT_FAILURE);
	}

	puts("Zbindowanie zakonczone pomyslnie");
}

bool isClientHere(int valread, char tmp[INET_ADDRSTRLEN], int i){
	bool flag = true;
	if (valread == SOCKET_ERROR) {
		int error_code = WSAGetLastError();
		if (error_code == WSAECONNRESET) {
			//Somebody disconnected , get his details and print
			printf("Host disconnected unexpectedly , ip %s , port %d \n", inet_ntop(AF_INET, &(address.sin_addr), tmp, INET_ADDRSTRLEN), ntohs(address.sin_port));

			//Close the socket and mark as 0 in list for reuse
			closesocket(s);

		}
		else {
			printf("recv failed with error code : %d", error_code);
		}
		flag = false;
	}

	if (valread == 0) {
		//Somebody disconnected , get his details and print
		printf("Host disconnected , ip %s , port %d \n", inet_ntop(AF_INET, &(address.sin_addr), tmp, INET_ADDRSTRLEN), ntohs(address.sin_port));

		//Close the socket and mark as 0 in list for reuse
		closesocket(s);
		flag = false;
	}
	return flag;
}

int main() {

	srand(time(NULL));
	char *message = "Witaj, graczu";

	initServer();
	Game game;
	game.newGame(10); //max_clients -> 10

	int MAXRECV = 1024;
	fd_set readfds; //deskryptory
	char *buffer;
	buffer = (char*)malloc((MAXRECV + 1) * sizeof(char));

	listen(master, 3);

	printf("Czekam na polaczenia graczy\n");

	addrlen = sizeof(struct sockaddr_in);
	char tmp[INET_ADDRSTRLEN];


	while (1) {
		
		FD_ZERO(&readfds); //czyscimy liste deskryptorow

		//add master socket to fd set
		FD_SET(master, &readfds);

		//add child sockets to fd set
		for (int i = 0; i < max_clients; i++) {
			s = clients[i];
			if (s > 0)
				FD_SET(s, &readfds);
		}

		//wait for an activity on any of the sockets, timeout is NULL , so wait indefinitely
		activity = select(0, &readfds, NULL, NULL, NULL);

		if (activity == SOCKET_ERROR) {
			printf("select call failed with error code : %d", WSAGetLastError());
			exit(EXIT_FAILURE);
		}

		//If something happened on the master socket , then its an incoming connection
		if (FD_ISSET(master, &readfds))	{
			if ((new_socket = accept(master, (struct sockaddr *)&address, (int *)&addrlen))<0) {
				perror("accept");
				exit(EXIT_FAILURE);
			}

			printf("New connection , socket fd is %d , ip is : %s , port : %d \n", new_socket, inet_ntop(AF_INET, &(address.sin_addr), tmp, INET_ADDRSTRLEN) , ntohs(address.sin_port));

				//send new connection greeting message
			string new_msg = initBuffer(message, 0, game);
			const char * tmpmsg = new_msg.c_str();
			if (send(new_socket, tmpmsg, strlen(tmpmsg), 0) != strlen(tmpmsg)) {
				perror("send failed");
			}

			puts("Welcome message sent successfully"); 

			//add new socket to array of sockets
			for (int i = 0; i < max_clients; i++) {
				if (clients[i] == 0){
					clients[i] = new_socket;
					game.clientsTeams[i] = 0;
					printf("Adding to list of sockets at index %d \n", i);
					break;
				}
			}
		}

		//obsluga wszystkich graczy
		for (int i = 0; i < max_clients; i++) {
			s = clients[i];

			//if client present in read sockets             
			if (FD_ISSET(s, &readfds)) {

				//get details of the client
				getpeername(s, (struct sockaddr*)&address, (int*)&addrlen);

				//Check if it was for closing , and also read the incoming message
				//recv does not place a null terminator at the end of the string (whilst printf %s assumes there is one).
				valread = recv(s, buffer, MAXRECV, 0);
				
				// KTOŒ SIÊ ROZ£¥CZY£ 
				if (!isClientHere(valread, tmp, i)) { 
					
					game.removePlayer(i);
					clients[i] = 0;
				} else {	//udalo sie odebrac msg 
					bool newPlayer = false;
					buffer[valread] = '\0';
					if (game.clientsTeams[i] == 0)	// gracz nie wybral druzyny 
						newPlayer = game.initNewPlayer(buffer[0], i); //jak uda sie dodac gracza to newplayer = true
					
					
					//printf("%s:%d - %s \n", inet_ntop(AF_INET, &(address.sin_addr), tmp, INET_ADDRSTRLEN), ntohs(address.sin_port), buffer);
					

					if (!newPlayer){									//bierzemy pod uwage gracza ktory juz siedzi w grze (ma druzyne)
						if (game.turn == game.clientsTeams[i]){					//jezeli byla kolejka gracza to liczymy jego glosowanie
							game.setVote((int)buffer[0] - (int)48, game.clientsTeams[i]); 
							game.madeMove[i] = game.clientsTeams[i];				//gracz teamu clientsTeams[i] wykonal ruch
						} else {										
							//klient ma po swojej stronie zablokowane okno i komunikat, ze teraz kolej na druzyne przeciwna
						}	

						//if (areAllVotes(1) && areAllVotes(2)){ 
						if (game.areAllVotes(game.turn)){	//jesli zostaly wykonane wszystkie ruchy danej druzyny
							//cout << "wszystkie ruchy !" << endl;
							game.printGameTab();
							game.setElement(game.chooseMove(game.turn), game.turn);
							int isOver = game.isOver();
							if (isOver != -1) {	//jezeli koniec gry
								if (isOver == 1){
									printf("wygral gracz 1\n");
								} else if (isOver == 2){
									printf("wygral gracz 2\n");
								} else // 0
									printf("remis!\n");
							}
							game.printGameTab();
							game.clearVotes(game.turn, max_clients);

							game.turn = (game.turn % 2) + 1;
							//cout << "turn: " << game.turn << endl;
							
							/*


								tu trzeba zrobic obsluge co sie dzieje jak zostana wykonane wszystkie glosy
							

							*/
						} else {
							//cout << "jeszcze ktos musi zrobic ruch!" << endl;
						}
					}
					
					//printVotes();
					for (int j = 0; j < max_clients; j++){ //chce wyslac tez info o zmianie ruchu do pozostalych graczy w druzynie
						if (game.clientsTeams[j] != -1){

							int team = game.clientsTeams[j];
							string str = initBuffer(buffer, team, game);
							if (!newPlayer)
								str += "Gracz " + to_string(i) + " zaglosowal...";
							else
								str += "Gracz " + to_string(i) + " dolaczyl do gry...";
							const char * msg = str.c_str();
							//send(clients[j], msg, valread + 10, 0);
							send(clients[j], msg, strlen(msg), 0);
							//}//innym graczom wysylamy info o tym czyja kolej jest
						}
						
					}
					newPlayer = false;
					//send(s, buffer, valread, 0);


					
				}
			}
		}
	}

	closesocket(s);
	WSACleanup();

	system("Pause");
	return 0;
}