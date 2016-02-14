#include <stdio.h>  //printf
#include <cstdlib>  //system(pause)
#include <winsock2.h> //windows socket
#include <Ws2tcpip.h>

#pragma comment(lib,"ws2_32.lib") //Winsock Library

WSADATA wsaData;
SOCKET s, new_socket;
struct sockaddr_in server, client;


void initServer(){
	//Windows Socket init
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		fprintf(stderr, "WSAStartup failed.\n");
		exit(1);
	}
	//Create a socket
	if ((s = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
		printf("Could not create socket : %d", WSAGetLastError());
	}

	//Prepare the sockaddr_in structure
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = inet_pton(AF_INET, "localhost", &(server.sin_addr));
	server.sin_port = htons(1111);

	//Bind
	if (bind(s, (struct sockaddr *)&server, sizeof(server)) == SOCKET_ERROR) {
		printf("Bind failed with error code : %d", WSAGetLastError());
	}

	puts("Bind done");
}

int main()
{
	int c;
	char *message;

	initServer();

	//Listen to incoming connections
	listen(s, 3);

	//Accept and incoming connection
	puts("Waiting for incoming connections...");

	c = sizeof(struct sockaddr_in);
	new_socket = accept(s, (struct sockaddr *)&client, &c);
	if (new_socket == INVALID_SOCKET) {
		printf("accept failed with error code : %d", WSAGetLastError());
	}

	puts("Connection accepted");

	//Reply to client
	message = "Hello Client , I have received your connection. But I have to go now, bye\n";
	send(new_socket, message, strlen(message), 0);

	getchar();

	closesocket(s);
	WSACleanup();

	printf("Hello socket\n");
	system("Pause");
	return 0;
}