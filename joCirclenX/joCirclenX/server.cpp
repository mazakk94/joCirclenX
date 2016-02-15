#include <stdio.h>  //printf
#include <cstdlib>  //system(pause)
#include <winsock2.h> //windows socket
#include <Ws2tcpip.h>
#include <vector>
#include <string>
#include <iostream> // cout <<


/*
TODO:
- podzial na kolejki (blokowanie tych co teraz nie graja)
- wybór ruchu dla graczy na podstawie glosowania
- zajmowanie pól, wysylanie wybranego pola do graczy (jakos trzeba to wyroznic)
- zerowanie g³osowania i informowanie graczy o nastepnym glosowaniu
- blokowanie wyboru pól które s¹ ju¿ zajête
*/

#pragma comment(lib,"ws2_32.lib") //Winsock Library

using namespace std;

/* INIT GLOBALI */
WSADATA wsaData;
SOCKET master, new_socket, s;
int max_clients = 10;
int activity, addrlen, i, valread;
vector<SOCKET> clients(max_clients, 0);
vector<int> clientsTeams(max_clients, -1);
vector<int> madeMove(max_clients, -1);
struct sockaddr_in server, address; //client;


vector<int> team1tab(10, 0);
vector<int> team2tab(10, 0);

int getTeamCount(int team){
	int count = 0;
	for (int i = 0; i < max_clients; i++)
		if (clientsTeams[i] == team)
			count++;
	return count;
}

bool areAllVotes(int team){
	int count = 0;
	for (int i = 0; i < max_clients; i++){
		if (clientsTeams[i] == team)
			if (madeMove[i] != team)
				return false;
	}
	return true;
}

string initBuffer(const char * buffer, int team){

	string str_buffer(buffer);

	string new_buffer = "";
	if (team == 1){
		for (unsigned int i = 0; i < team1tab.size(); i++){
			new_buffer += to_string(team1tab[i]);
		}
	} else if (team == 2) {
		for (unsigned int i = 0; i < team1tab.size(); i++){
			new_buffer += to_string(team2tab[i]);
		}
	} else {
		printf("wrong team!\n");
		return "";
	}

	new_buffer += str_buffer;
	cout << "new_buffer: " << new_buffer << endl;
	//const char * ret_buffer = new_buffer.c_str();
	//cout << "ret_buffer: " << ret_buffer << endl;

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

void setVote(int vote, int team){
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
	} else {
		printf("vote error!");
	}
}

void printVotes(){
	for (int i = 1; i < 10; i++){
		printf("%d ", team1tab[i]);
		if (i % 3 == 0)
			printf("\n");
	}
	printf("\n\n");
	for (int i = 1; i < 10; i++){
		printf("%d ", team2tab[i]);
		if (i % 3 == 0)
			printf("\n");
	}
}

int main()
{
//	int c;
	char *message = "000000000Witaj, gracz";

	initServer();

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
		for (i = 0; i < max_clients; i++) {
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
			if (send(new_socket, message, strlen(message), 0) != strlen(message)) {
				perror("send failed");
			}

			puts("Welcome message sent successfully"); // nie chcemy wysylac wiadomosci powitalnej

			//add new socket to array of sockets
			for (i = 0; i < max_clients; i++) {
				if (clients[i] == 0){
					clients[i] = new_socket;
					clientsTeams[i] = 0;
					printf("Adding to list of sockets at index %d \n", i);
					break;
				}
			}
		}

		//else its some IO operation on some other socket :)
		for (i = 0; i < max_clients; i++) {
			s = clients[i];
			//if client presend in read sockets             
			if (FD_ISSET(s, &readfds)) {
				//get details of the client
				getpeername(s, (struct sockaddr*)&address, (int*)&addrlen);

				//Check if it was for closing , and also read the incoming message
				//recv does not place a null terminator at the end of the string (whilst printf %s assumes there is one).
				valread = recv(s, buffer, MAXRECV, 0);


				if (valread == SOCKET_ERROR) {
					int error_code = WSAGetLastError();
					if (error_code == WSAECONNRESET) {
						//Somebody disconnected , get his details and print
						printf("Host disconnected unexpectedly , ip %s , port %d \n", inet_ntop(AF_INET, &(address.sin_addr), tmp, INET_ADDRSTRLEN), ntohs(address.sin_port));

						//Close the socket and mark as 0 in list for reuse
						closesocket(s);
						clients[i] = 0;
						clientsTeams[i] = -1;
						madeMove[i] = -1;
					} else {
						printf("recv failed with error code : %d", error_code);
					}
				}
				if (valread == 0) {
					//Somebody disconnected , get his details and print
					printf("Host disconnected , ip %s , port %d \n", inet_ntop(AF_INET, &(address.sin_addr), tmp, INET_ADDRSTRLEN), ntohs(address.sin_port));

					//Close the socket and mark as 0 in list for reuse
					closesocket(s);
					clients[i] = 0;
					clientsTeams[i] = -1;
					madeMove[i] = -1;


				} else { //udalo sie odebrac msg //Echo back the message that came in
					bool newPlayer = false;
					//add null character, if you want to use with printf/puts or other string handling functions
					buffer[valread] = '\0';
					printf("tmp buf: %s\n", buffer);
					if (clientsTeams[i] == 0){		 // gracz nie wybral druzyny
						int team = (int)buffer[0] - (int)48;
						//printf("(int)buffer[0] - 48 = %d \n", team);
						if (team == 1 || team == 2){ //
							clientsTeams[i] = team;
							madeMove[i] = 0;
							printf("clientsTeams[%d] = %d \n", i, team);
							newPlayer = true;
						} else {
							printf("sizeof buffer = %d, %s \n", sizeof(buffer), buffer);
						}
					}
						
					printf("%s:%d - %s \n", inet_ntop(AF_INET, &(address.sin_addr), tmp, INET_ADDRSTRLEN), ntohs(address.sin_port), buffer);
					if (!newPlayer){
						setVote((int)buffer[0] - (int)48, clientsTeams[i]);
						madeMove[i] = clientsTeams[i]; //gracz teamu clientsTeams[i] wykonal ruch
						if (areAllVotes(1) && areAllVotes(2)){
							//WYBIERZ RUCHY DLA DRUZYN
						}
					}
					
					printVotes();
					for (int j = 0; j < max_clients; j++){ //chce wyslac tez info o zmianie ruchu do pozostalych graczy w druzynie
						if (clientsTeams[j] == clientsTeams[i]){
							int team = clientsTeams[j];
							string str = initBuffer(buffer, team);
							str += "Gracz " + to_string(i) + " zaglosowal...";
							cout << "str: " << str << endl;
							const char * msg = str.c_str();
							cout << "msg: " << msg << endl;
							send(clients[j], msg, valread + 10, 0);
							//send(clients[j], msg, valread + 10, 0);
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