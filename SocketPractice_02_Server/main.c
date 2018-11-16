#include<stdio.h>
#include<stdlib.h>
#include<winsock2.h>
#include<Windows.h>
#include <process.h>
#include "Thread.h"
#define MAX_IN 10

#pragma comment(lib,"ws2_32.lib") //Winsock Library
CRITICAL_SECTION crtsct;
SOCKET s;
struct sockaddr_in server;
SOCKET cSock[MAX_IN];
struct sockaddr_in cServ[MAX_IN];
int numOfJoin=0;
int start = 0;
HANDLE hThread[MAX_IN] = {NULL};
DWORD dwThreadID[MAX_IN] = { NULL };

void startWinSock(WSADATA* wsa) {
	if (WSAStartup(MAKEWORD(2, 2), wsa) != 0) {
		printf("Failed. Error Code : %d", WSAGetLastError());
		exit(1);
	}
}
void createSock(SOCKET* s) {
	if ((*s = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
		printf("Could not create socket : %d", WSAGetLastError());
		exit(1);
	}
}
void setServer(struct sockaddr_in* server, int port) {
	server->sin_family = AF_INET;
	server->sin_addr.s_addr = INADDR_ANY;
	server->sin_port = htons(port);
}
void bindServerSocket(SOCKET *s, struct sockaddr_in *server) {
	if (bind(*s, (struct sockaddr *)server, sizeof(*server)) == SOCKET_ERROR) {
		printf("Bind failed with error code : %d\n", WSAGetLastError());
		exit(1);
	}
}
void listenSocket(SOCKET *s, int lN) {
	if (listen(*s, 3) == SOCKET_ERROR) {
		printf("Listen failed with error code : %d\n", WSAGetLastError());
		exit(1);
	}
}
void getAccSocks(SOCKET *s, SOCKET *new_socket, int* c, struct sockaddr_in* client) {
	*c = sizeof(struct sockaddr_in);
	*new_socket = accept(*s, (struct sockaddr *)client, c);
	if (*new_socket == INVALID_SOCKET) {
		printf("accept failed with error code : %d\n", WSAGetLastError());
		exit(1);
	}
}

int recvDataFromClient(char* recv_message, SOCKET* new_socket, int* recv_size) {
	if ((*recv_size = recv(*new_socket, recv_message, 1024, 0)) == SOCKET_ERROR) {
		printf("recv failed : %d\n",WSAGetLastError());
		exit(1);
		return 1;
	}
	recv_message[*recv_size] = '\0';
	return 0;
}
int sendDataToClient(char* message, SOCKET* new_socket) {
	send(*new_socket, message, strlen(message), 0);
	return 0;
}

unsigned int _stdcall recvThread(void* arg) {
	SOCKET now_socket = *(SOCKET*)arg;
	int numOfSock = (SOCKET*)arg - cSock;
	while (1) {
		char message[1025] = {0};
		char sendMsg[1025] = { 0 };
		int size = 0;
		recvDataFromClient(message,&now_socket,&size);
		//printf("%s %d\n", message, size);
		if (strcmp(message, "[:exit]") == 0) { 
			hThread[numOfSock] = NULL;
			dwThreadID[numOfSock] = NULL;
			printf("%d Thread is Dead\n",numOfSock);
			return 0; 
		}
		synchronize(&crtsct);
		printf("Client %d : %s\n",numOfSock + 1,message);
		sprintf(sendMsg, "%s", message);
		for (int i = 0; i < numOfJoin; i++) {
			if(cSock[i]!=NULL)
				sendDataToClient(sendMsg,&cSock[i]);
		}
		synchronized(&crtsct);
	} 
	return 0;
}
unsigned int _stdcall getAccesThread(void* arg) {
	int c;
	for (int k = 0; k < 2; k++) {
		getAccSocks(&s, &cSock[numOfJoin], &c, &cServ[numOfJoin]);
		synchronize(&crtsct);
		numOfJoin++;
		printf("%d is Joined\n", numOfJoin);
		synchronized(&crtsct);
	}
	synchronize(&crtsct);
	start = 1;
	synchronized(&crtsct);
	hThread[0] = _beginthreadex(NULL, 0, recvThread, &cSock[0], 0, (unsigned*)dwThreadID[0]);
	hThread[1] = _beginthreadex(NULL, 0, recvThread, &cSock[1], 0, (unsigned*)dwThreadID[1]);
	printf("Begin 1, 2 Thread %d %d\n",hThread[0],hThread[1]);
	for (int k = 2; k < MAX_IN; k++) {
		getAccSocks(&s, &cSock[numOfJoin], &c, &cServ[numOfJoin]);
		synchronize(&crtsct);
		numOfJoin++;
		printf("%d is Joined\n", numOfJoin);
		synchronized(&crtsct);
		hThread[k] = _beginthreadex(NULL, 0, recvThread, &cSock[k], 0, (unsigned*)dwThreadID[k]);
		printf("Begin %d Thread\n",numOfJoin);
	}
	printf("is Fulled\n");
	while (1) {
		SOCKET tmp=NULL;
		struct sockaddr_in tmpServ;
		printf("get Access request\n");
		getAccSocks(&s, &tmp, &c, &tmpServ);
		for (int i = 0; i < MAX_IN; i++) {
			if (hThread[i] == NULL) {
				printf("%d is Empty\n",i+1);
				cSock[i] = tmp;
				cServ[i] = tmpServ;
				tmp = NULL;
				synchronize(&crtsct);
				printf("%d is Joined\n", i + 1);
				synchronized(&crtsct);
				hThread[i] = _beginthreadex(NULL, 0, recvThread, &cSock[i], 0, (unsigned*)dwThreadID[i]);
				printf("Begin %d Thread\n", i + 1);
			};
		}
		if (tmp != NULL) {
			sendDataToClient("is Fulled.", &tmp);
		}
		Sleep(10);
	}
	return 0;
}
unsigned int _stdcall commandThread(void* arg) {
	
}

int main(int argc, char *argv[]) {
	InitializeCriticalSection(&crtsct);
	WSADATA wsa;
	int c;
	int recv_size = 0;
	char recv_message[1025];
	char message[1025];
	startWinSock(&wsa);
	printf("Initialised.\n");
	//Create a socket
	createSock(&s);
	printf("Socket created.\n");
	setServer(&server, 8081);
	//Bind
	bindServerSocket(&s, &server);
	puts("Bind done");
	//Listen to incoming connections
	listenSocket(&s, 3);
	puts("Listen done");

	puts("Server is prepared");

	//Accept and incoming connection
	puts("Waiting for incoming connections...");
	HANDLE hThread = NULL;
	DWORD dwThreadID = NULL;
	hThread=_beginthreadex(NULL, 0, getAccesThread, NULL, 0, (unsigned*)&dwThreadID);
	while (1) {
		Sleep(1000);
	}
	closesocket(s);
	WSACleanup();
	DeleteCriticalSection(&crtsct);
	return 0;
}