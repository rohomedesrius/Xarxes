#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#include "Windows.h"
#include "WinSock2.h"
#include "Ws2tcpip.h"

#include <iostream>
#include <cstdlib>


void Client (const char* clientAddrStr, int port)
{
	//Win Init-------------------------------------------------
	WSADATA wsaData;
	int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != NO_ERROR)
	{
		std::cout << "WSAStartup ERROR" << std::endl;
	}
	std::cout << "WSAStartup DONE" << std::endl;

	//Create Socket UDP-----------------------------------------
	SOCKET clientSock = socket(AF_INET, SOCK_DGRAM, 0);
	if (clientSock == INVALID_SOCKET) {
		std::cout << "Socket ERROR" << std::endl;
	}
	std::cout << "Socket DONE" << std::endl;

	//Client Ping-----------------------------------------------
	std::string pingString("Ping");

	//Client Adress---------------------------------------------
	struct sockaddr_in clientAddr;
	clientAddr.sin_family = AF_INET;
	inet_pton(AF_INET, clientAddrStr, &clientAddr.sin_addr);
	clientAddr.sin_port = htons(port);

	//Bind---------------------------------------------------
	int bindRes = bind(clientSock, (const sockaddr*)&clientAddr, sizeof(clientAddr));
	if (bindRes == SOCKET_ERROR) {
		std::cout << "Bind ERROR" << std::endl;
	}
	std::cout << "Bind DONE" << std::endl;

	struct sockaddr fromAddr;

	//Input buffer----------------------------------------------
	const int inBufferLen = 1300;
	char inBuffer[inBufferLen];

	//Interaction-----------------------------------------------
	for (int i = 0; i < 5; ++i)
	{
		// Send
		int bytes = sendto(clientSock, pingString.c_str(), pingString.size() + 1, 0, (sockaddr*)&clientAddr, sizeof(clientAddr));
		if (bytes > 0)
		{
			std::cout << "Sent: " << pingString.c_str() << std::endl;

			std::cout << "Waiting for server data... " << std::flush;

			// Receive
			int fromAddrSize = sizeof(fromAddr);
			bytes = recvfrom(clientSock, inBuffer, inBufferLen, 0, &fromAddr, &fromAddrSize);
			if (bytes == SOCKET_ERROR) {
				std::cout << "Reception ERROR" << std::endl;
			}

			std::cout << "Received: " << inBuffer << std::endl;

			// Wait 1 second
			Sleep(1000);
		}
		else
		{
			std::cout << "Interaction ERROR" << std::endl;
		}
	}

	// Close socket
	closesocket(clientSock);

	// Winsock shutdown
	WSACleanup();
}

int main(int argc, char** argv)
{
	const char *clientStr = argv[1];
	int port = atoi(argv[2]);
	Client(clientStr, port);

	getchar();
	return 0;
}