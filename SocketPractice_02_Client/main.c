/*
Initialise Winsock
*/

#include<stdio.h>
#include <stdlib.h>
#include<winsock2.h>
#include <process.h>
#include "Thread.h"
#pragma comment(lib,"ws2_32.lib") //Winsock Library
SOCKET s;
struct sockaddr_in server;
CRITICAL_SECTION crtsct;
char name[100] = { 0 };

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
void setClient(struct sockaddr_in* server, int port, char* ip) {
	server->sin_family = AF_INET;
	server->sin_addr.s_addr = inet_addr(ip);
	server->sin_port = htons(port);
}
void connectToServer(SOCKET* s, struct sockaddr_in* server) {
	if (connect(*s, (struct sockaddr *)server, sizeof(*server)) < 0) {
		printf("connect error\n");
		exit(1);
	}
}
int sendToServer(char* message, SOCKET* s) {
	if (send(*s, message, strlen(message), 0) < 0) {
		exit(1);
	}
	return 0;
}
int recvFromServer(char* recv_message, SOCKET* s, int* recv_size) {
	if ((*recv_size = recv(*s, recv_message, 1024, 0)) == SOCKET_ERROR) {
		exit(1);
	}
	recv_message[*recv_size] = '\0';
	return 0;
}
unsigned int _stdcall recvThread(void* arg) {
	char message[1025] = { 0 };
	int size=0;
	while (1) {
		recvFromServer(message, &s, &size);
		if (strcmp(message, "is Fulled.") == 0) {
			printf("is Fulled\n");
			exit(0);
		}
		synchronize(&crtsct);
		printf("%s\n", message);
		synchronized(&crtsct);
	}
}
unsigned int _stdcall sendThread(void* arg) {
	while (1) {
		char message[1025] = { 0 };
		char buff[1025] = { 0 };
		fgets(message, 1024, stdin);
		int t = 0;
		while (message[t++]);
		message[t-2] = '\0';
		sprintf(buff, "%s : %s", name, message);
		if (strcmp(message, "[:exit]") == 0)
			sprintf(buff, message);
		synchronize(&crtsct);
		sendToServer(buff, &s);
		synchronized(&crtsct);
		if (strcmp(message, "[:exit]")==0)exit(0);
	}
}

int main(int argc, char *argv[]) {
	InitializeCriticalSection(&crtsct);
	char* iP = "127.0.0.1";
	iP = "172.16.1.158";
	WSADATA wsa;
	char message[1025] = { 0 }, server_reply[1025] = { 0 };
	int recv_size;
	HANDLE rThread = NULL, sThread = NULL;
	DWORD rThreadID = NULL, sThreadID = NULL;
	startWinSock(&wsa);
	printf("What is Your Name ? ");
	scanf("%s", name);
	fgets(message, 1024, stdin);

	puts("Initialised.");
	createSock(&s);
	//printf("%s\n", iP.c_str());
	setClient(&server, 8081, iP);

	//Connect to remote server
	//if (connect(s, (struct sockaddr *)&server, sizeof(server)) < 0){
	//	printf("connect error\n");
	//	WSACleanup();
	//	return 1;
	//}
	connectToServer(&s, &server);
	printf("Connected\n");
	rThread = _beginthreadex(NULL, 0, recvThread, NULL, 0, (unsigned*)rThreadID);
	sThread = _beginthreadex(NULL, 0, sendThread, NULL, 0, (unsigned*)sThreadID);
	printf("Begin Threads\n");
	while (1) {
		Sleep(1000);
	}
	closesocket(s);
	WSACleanup();
	DeleteCriticalSection(&crtsct);
	getchar();
	return 0;
}