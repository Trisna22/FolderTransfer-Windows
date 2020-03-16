#pragma once
#include "stdafx.h"

#ifndef FileReceiving_H
#define FileReceiving_H

#define MAX_BYTES_RECV		10000000
#define DEBUG 0
class FileReceiving
{
private:
	SOCKET connection = NULL;
	

	bool sendConfirm()
	{
		string msg = "[RECV_OK]";
		if (send(connection, msg.c_str(), msg.length(), 0) == -1)
		{
			printf("Failed to send the confirm message! Error code: %d\n\n", WSAGetLastError());
			return false;
		}

		return true;
	}

	bool receiveFile(string fileName, int fileSize)
	{
		printf("###FILE %s, %d\n", fileName.c_str(), fileSize);

		ofstream outputFile(fileName.c_str(), ios::trunc | ios::binary);
		if (outputFile.is_open() == false)
		{
			printf("\nFailed to open file to write data to! Error code: %d\n\n", GetLastError());
			return false;
		}
		
		// DEBUG
		#if DEBUG == 1
			fclose(fileSend);
			return true;
		#endif		

		fd_set fds;
		FD_ZERO(&fds);
		FD_SET(connection, &fds);

		int result = select(connection, &fds, 0, 0, 0);
		if (result == -1)
		{
			printf("\nFailed to receive file data!\n\n");
			outputFile.close();
			return false;
		}

		char* buffer = (char*)malloc(fileSize);
		int bytes = recv(connection, buffer, fileSize, 0);
		if (bytes == -1)
		{
			printf("Failed to receive data from socket! Error code: %d\n\n", WSAGetLastError());
			outputFile.close();
			return false;
		}

		outputFile.write(buffer, fileSize);
		outputFile.close();
		return sendConfirm();
	}

public:
	bool testReceive(string fileName, int fileSize)
	{
		printf("Receiving file test: %s\n", fileName.c_str());
		FILE* fileSend = NULL;
		//int result = fopen_s(&fileSend, fileName.c_str(), "w");
		//if (result == -1)
		//{
		//	printf("\nFailed to open file to write data to! Error code: %d\n\n", GetLastError());
		//	return false;
		//}

		// DEBUG
		ofstream outputFile(fileName.c_str(), ios::trunc | ios::binary);


		printf("CREATING SOCKET ON PORT 3333\n");
		SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
		if (sock == SOCKET_ERROR)
		{
			printf("\nFailed to create socket!\n\n");
			return SOCKET_ERROR;
		}

		SOCKADDR_IN serverAddr;
		serverAddr.sin_addr.s_addr = INADDR_ANY;
		serverAddr.sin_port = htons(3333);
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

		char* buffer = (char*)malloc(fileSize);
		int bytes = recv(serverSock, buffer, fileSize, 0);
		if (bytes == -1)
		{
			printf("Failed to receive data from socket! Error code: %d\n\n", WSAGetLastError());
			return false;
		}

		outputFile.write(buffer, fileSize);
		//if (fwrite(buffer, 1, bytes, fileSend) == -1)
		//{
		//	printf("Failed to write data to file! Error code: %d\n\n", WSAGetLastError());
		//	return false;
		//}
		//fclose(fileSend);

		outputFile.close();
		printf("File received!!! bytes %d\n\n", bytes);

		closesocket(sock);
		return true;
	}

	void startReceivingFolder(SOCKET connection)
	{
		this->connection = connection;

		while (true) 
		{	
			// WAIT for data.
			fd_set fds;
			FD_ZERO(&fds);
			FD_SET(connection, &fds);

			int result = select(connection, &fds, 0, 0, 0);
			if (result == 0)
			{
				printf("Connection dropped by peer!\n\n");
				return;
			}
			else if (result == -1)
			{
				printf("Failed to wait for files!\n\n");
				return;
			}

			// Receive the file header.
			char* buffer = (char*)malloc(128);
			int bytes = recv(connection, buffer, 128, 0);
			if (bytes == SOCKET_ERROR)
			{
				printf("\nFailed to receive the folder header! Error code: %d\n\n",
					WSAGetLastError());
				return;
			}
			else if (bytes == 0)
			{
				printf("\nConnection dropped by remote peer!\n\n");
				return;
			}
			else
			{
				buffer[bytes] = '\0';

				string action = buffer;
				if (action.find("MKDIR ") != string::npos) {

					string a = action.substr(6);
					string folderName = a.substr(0, a.find("XXX"));
					if (CreateDirectoryA(folderName.c_str(), NULL) == FALSE) {
						printf("Failed to create directory!! Error code: %d\n\n", GetLastError());
						return;
					}

					printf("###MKDIR %s\n", folderName.c_str());
					continue;
				}
				else if (action.find("[[FOLDER_SEND_END]]") != string::npos)
				{
					printf("\nAll data received succesfully! Closing connection...\n\n");
					closesocket(connection);
					return;
				}

				string strBuffer = buffer;
				string fileName = strBuffer.substr(0, strBuffer.find(","));
				string a = strBuffer.substr(strBuffer.find(",") + 1);
				string fileSize = a.substr(0, a.find("XXX"));

				// RECEIVE DATA from file.
				if (receiveFile(fileName, atoi(fileSize.c_str())) == false) 
				{
					printf("\n##Receiving file failed!!\n\n");
					return;
				}

				//Sleep(300);
				continue;
			}
			
		}
	}
};
#endif // !FileReceiving_H
