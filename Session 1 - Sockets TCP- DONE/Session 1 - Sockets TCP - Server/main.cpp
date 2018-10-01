#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include "Windows.h"
#include "WinSock2.h"
#include "Ws2tcpip.h"

#include <iostream>
#include <cstdlib>

#define PAUSE_AND_EXIT() system("pause"); exit(-1)

void printWSErrorAndExit(const char *msg)
{
	wchar_t *s = NULL;
	FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPWSTR)&s, 0, NULL);
	fprintf(stderr, "%s: %S\n", msg, s);
	LocalFree(s);
	PAUSE_AND_EXIT();
}

void server(int port)
{
	// Winsock init
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != NO_ERROR) {
		printWSErrorAndExit("WSAStartup");
	}
	std::cout << "WSAStartup done" << std::endl;

	// Create socket (IPv4, stream, TCP)
	SOCKET s = socket(AF_INET, SOCK_STREAM, 0);
	if (s == INVALID_SOCKET) {
		printWSErrorAndExit("socket");
	}
	std::cout << "socket done" << std::endl;

	// Reuse address
	int enable = 1;
	int result = setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (const char*)&enable, sizeof(int));
	if (result == SOCKET_ERROR) {
		printWSErrorAndExit("setsockopt");
	}
	std::cout << "setsockopt SO_REUSEADDR done" << std::endl;

	// Address (server)
	struct sockaddr_in serverAddr;
	serverAddr.sin_family = AF_INET; // IPv4
	serverAddr.sin_addr.S_un.S_addr = INADDR_ANY; // Any address, will be localhost
	serverAddr.sin_port = htons(port); // Port

	// Bind socket
	int bindRes = bind(s, (struct sockaddr *)&serverAddr, sizeof(serverAddr));
	if (bindRes == SOCKET_ERROR) {
		printWSErrorAndExit("bind");
	}
	std::cout << "bind done on port " << port << std::endl;

	// Listen
	int listenRes = listen(s, 1);
	if (listenRes == SOCKET_ERROR) {
		printWSErrorAndExit("listen");
	}
	std::cout << "listen done" << std::endl;

	// Accept
	struct sockaddr clientAddr;
	int clientAddrLen = sizeof(clientAddr);
	SOCKET clientSocket = accept(s, (struct sockaddr*)&clientAddr, &clientAddrLen);
	if (clientSocket == INVALID_SOCKET) {
		printWSErrorAndExit("accept");
	}
	std::cout << "accept done" << std::endl;

	// Server string
	std::string pongString("Pong");

	// Input buffer
	const int inBufferLen = 1300;
	char inBuffer[inBufferLen];

	while (true)
	{
		std::cout << "Waiting for client data... " << std::flush;

		// Receive
		int bytes = recv(clientSocket, inBuffer, inBufferLen, 0);
		if (bytes > 0)
		{
			std::cout << "Received: " << inBuffer << std::endl;

			// Wait 1 second
			Sleep(1000);

			// Send
			bytes = send(clientSocket, pongString.c_str(), pongString.size() + 1, 0);
			if (bytes == SOCKET_ERROR) {
				printWSErrorAndExit("send");
			}

			std::cout << "Sent: '" << pongString.c_str() << "' sent" << std::endl;
		}
		else if (bytes == 0)
		{
			std::cout << "connection closed by client" << std::endl;
			break;
		}
		else
		{
			printWSErrorAndExit("recv");
		}
	}

	// Close socket
	closesocket(s);

	// Winsock shutdown
	WSACleanup();
}

void printUsage()
{
	std::cout << "Usage: ./server listen_port" << std::endl;
}

int main(int argc, char **argv)
{
	if (argc != 2) {
		printUsage();
		PAUSE_AND_EXIT();
	}

	int port = atoi(argv[1]);
	server(port);

	PAUSE_AND_EXIT();
}
