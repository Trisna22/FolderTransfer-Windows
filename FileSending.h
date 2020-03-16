#pragma once
#include "stdafx.h"

#ifndef FileSending_H
#define FileSending_H

#define MAX_BYTES_SEND 10000000

class FileSending
{
private:
	SOCKET connection = NULL;
public:
	bool testSend(string fileName)
	{
		printf("SENDINGGG FILE %s on port 3333!!\n\n", fileName.c_str());
		FILE* fileSend = NULL;
		int result = fopen_s(&fileSend, fileName.c_str(), "rb");
		if (result != 0)
		{
			printf("\nFailed to send file to remote peer!\nReason: failed to open file!\n\n");
			return false;
		}

		fseek(fileSend, 0, SEEK_END);
		unsigned long fileSize = ftell(fileSend);
		rewind(fileSend);

		SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
		if (sock == SOCKET_ERROR)
		{
			printf("\nFailed to create socket!\n\n");
			return SOCKET_ERROR;
		}

		SOCKADDR_IN serverAddr;
		serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
		serverAddr.sin_port = htons(3333);
		serverAddr.sin_family = AF_INET;

		result = connect(sock, (SOCKADDR*)&serverAddr, sizeof(serverAddr));
		if (result == SOCKET_ERROR)
		{
			printf("\nFailed to connect with remote peer!\n\n");
			return SOCKET_ERROR;
		}

		printf("Connected with server!\n");

		char* buffer = (char*)malloc(fileSize);
		if (fread(buffer, 1, fileSize, fileSend) == -1)
		{
			printf("\nFailed to read buffer from file!\n Error code: %d\n\n", GetLastError());
			fclose(fileSend);
			return false;
		}

		if (send(sock, buffer, fileSize, 0) == -1)
		{
			printf("\nFailed to send data to remote peer!\n\n");
			fclose(fileSend);
			return false;
		}

		fclose(fileSend);
		closesocket(sock);
		return true;
	}

	bool receiveConfirm()
	{
		fd_set fds;
		FD_ZERO(&fds);
		FD_SET(connection, &fds);

		int sel = select(connection, &fds, 0, 0, 0);
		if (sel < 0)
		{
			printf("Failed to receive confirm! Failed to wait for data! Error code: %d\n\n", WSAGetLastError());
			return false;
		}

		char* buffer = (char*)malloc(10);
		int bytes = recv(connection, buffer, 10, 0);
		if (bytes == 0)
		{
			printf("Failed to receive confirm! Connection dropped!\n\n");
			return false;
		}

		buffer[10] = '\0';
		string confirm = buffer;
		if (confirm.find("[RECV_OK]") == string::npos)
		{
			printf("Failed to receive the correct confirm message!\n\n");
			return false;
		}

		return true;
	}

	bool sendFolder(string folderName)
	{
		string sendFolder = "MKDIR " + folderName;
		for (int i = sendFolder.length(); i < 128; i++)
			sendFolder.push_back('X');

		send(connection, sendFolder.c_str(), sendFolder.length(), 0);
		return true;
	}

	bool sendFile(string fileName, string virtualPath)
	{
		FILE* fileSend = NULL;
		int result = fopen_s(&fileSend, fileName.c_str(), "rb");
		if (result != 0)
		{
			printf("\nFailed to send file to remote peer!\nReason: failed to open file!\n\n");
			return false;
		}

		fseek(fileSend, 0, SEEK_END);
		int fileSize = ftell(fileSend);
		rewind(fileSend);

		string fileHeader = virtualPath + "," + to_string(fileSize);

		for (int i = fileHeader.length(); i < 128; i++)
			fileHeader.push_back('X');

		if (send(connection, fileHeader.c_str(), fileHeader.length(), 0) == -1)
		{
			printf("\nFailed to send the file header! Error code: %d\n\n", WSAGetLastError());
			return false;
		}

		printf("File sending: %s\n", fileName.c_str());
		printf("Bytes sending: %d\n\n", fileSize);

		
		//DEBUG
		#if DEBUG == 1
			return true;
		#endif	

		char* buffer = (char*)malloc(fileSize);
		fread(buffer, 1, fileSize, fileSend);

		if (send(connection, buffer, fileSize, 0) == -1)
		{
			printf("\nFailed to send file to remote peer!\nReason: failed to send data to remote peer!\n\n");
			return false;
		}
		fclose(fileSend);
		return receiveConfirm();
	}

	void startSendingFolder(SOCKET connection, string folder)
	{
		this->connection = connection;
		listFolder(folder);

		string endMessage = "[[FOLDER_SEND_END]]";
		send(connection, endMessage.c_str(), endMessage.length(), 0);
		printf("All files sent succesfully!\n\n");
	}

	void listFolder(string folder, string virtualFolder = "NO_FOLDER")
	{
		if (virtualFolder == "NO_FOLDER") {
			string a = folder.substr(0, folder.length() - 2);
			virtualFolder = a.substr(a.find_last_of("\\") +1);
		}
		cout << "\nSENDING " << virtualFolder << endl;

		sendFolder(virtualFolder);

		WIN32_FIND_DATAA winData;
		HANDLE hFind = FindFirstFileA(folder.c_str(), &winData);
		if (hFind == INVALID_HANDLE_VALUE)
		{
			printf("\nFailed to list folder! Error code: %d \n\n", GetLastError());
			return;
		}

		printf("\n");
		do {
			if ((string)winData.cFileName == "." || (string)winData.cFileName == "..")
				continue;

			if (winData.dwFileAttributes == FILE_ATTRIBUTE_DIRECTORY)
			{
				printf("<Folder>: %s\n", winData.cFileName);
				listFolder(folder.substr(0, folder.length() -2) + "\\" + winData.cFileName + "\\*",
					virtualFolder + "\\" + winData.cFileName);
			}
			else 
			{
				printf("<File>: %s\n", winData.cFileName);

				if (sendFile(folder.substr(0, folder.length() - 2) + "\\" + winData.cFileName,
					virtualFolder + "\\" + winData.cFileName) == false) {
					printf("Failed to send file to remote peer!\n\n");
					return;
				}
			}
			//Sleep(200);
		} while (FindNextFileA(hFind, &winData));

		if (GetLastError() != ERROR_NO_MORE_FILES)
		{
			printf("ALL FILES RECEIVED\n\n");
			return;
		}
	}

};
#endif // !FileSending_H
