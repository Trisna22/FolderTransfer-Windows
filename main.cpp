#include "stdafx.h"

#include "FileReceiving.h"
#include "FileSending.h"

SOCKET createServerSocket()
{
	SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == SOCKET_ERROR)
	{
		printf("\nFailed to create socket!\n\n");
		return SOCKET_ERROR;
	}

	SOCKADDR_IN serverAddr;
	serverAddr.sin_addr.s_addr = INADDR_ANY;
	serverAddr.sin_port = htons(4444);
	serverAddr.sin_family = AF_INET;

	int result = bind(sock, (SOCKADDR*)&serverAddr, sizeof(serverAddr));
	if (result == SOCKET_ERROR)
	{
		printf("\nFailed to bind socket!\n\n");
		return SOCKET_ERROR;
	}

	result = listen(sock, SOMAXCONN);
	if (result == SOCKET_ERROR)
	{
		printf("\nFailed to listen for connections!\n\n");
		return SOCKET_ERROR;
	}

	int addrLen = sizeof(serverAddr);
	SOCKET serverSock = accept(sock, (SOCKADDR*)&serverAddr, &addrLen);
	if (serverSock == SOCKET_ERROR)
	{
		printf("\nFailed to accept the connections!\n\n");
		return SOCKET_ERROR;
	}

	printf("Server connected!\n");
	return serverSock;
}
SOCKET connectionSocket()
{
	SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == SOCKET_ERROR)
	{
		printf("\nFailed to create socket!\n\n");
		return SOCKET_ERROR;
	}

	SOCKADDR_IN serverAddr;
	serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	serverAddr.sin_port = htons(4444);
	serverAddr.sin_family = AF_INET;

	int result = connect(sock, (SOCKADDR*)&serverAddr, sizeof(serverAddr));
	if (result == SOCKET_ERROR)
	{
		printf("\nFailed to connect with remote peer!\n\n");
		return SOCKET_ERROR;
	}

	printf("Connected with server!\n");
	return sock;
}


// Confirmation messages.



int main(int argc, char* argv[])
{
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		printf("\nFailed to initialize WSA data!\n\n");
		return -1;
	}
	
	if (argc != 2)
	{
		// DEBUG
		//FileReceiving fr;
		//fr.testReceive("test.mp3", 3134595);
		//return 0;

		SOCKET sock = createServerSocket();
		if (sock == SOCKET_ERROR)
			return 0;
		FileReceiving fr;
		fr.startReceivingFolder(sock);
	}
	else
	{
		// DEBUG
		//FileSending fs;
		//fs.testSend("C:\\Users\\ramb0\\Desktop\\TestFolder\\example.mp3");
		//return 0;

		SOCKET sock = connectionSocket();
		if (sock == SOCKET_ERROR)
			return 0;
		FileSending fs;
		fs.startSendingFolder(sock, "C:\\Users\\ramb0\\Pictures\\Camera Roll\\*");
	}
	return 0;
}

