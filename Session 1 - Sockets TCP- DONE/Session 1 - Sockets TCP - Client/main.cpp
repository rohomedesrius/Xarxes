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

void client(const char *serverAddrStr, int port)
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

	// Client string
	std::string pingString("Ping");

	// Server Address
	struct sockaddr_in serverAddr;
	const int serverAddrLen = sizeof(serverAddr);
	serverAddr.sin_family = AF_INET; // IPv4
	inet_pton(AF_INET, serverAddrStr, &serverAddr.sin_addr);
	serverAddr.sin_port = htons(port); // Port

	// Connect to server
	int connectRes = connect(s, (const sockaddr*)&serverAddr, serverAddrLen);
	if (connectRes == SOCKET_ERROR) {
		printWSErrorAndExit("connect");
	}
	std::cout << "connect done" << std::endl;

	// Input buffer
	const int inBufferLen = 1300;
	char inBuffer[inBufferLen];

	for (int i = 0; i < 5; ++i)
	{
		// Send
		int bytes = send(s, pingString.c_str(), pingString.size() + 1, 0);
		if (bytes > 0)
		{
			std::cout << "Sent: " << pingString.c_str() << std::endl;

			std::cout << "Waiting for server data... " << std::flush;

			// Receive
			bytes = recv(s, inBuffer, inBufferLen, 0);
			if (bytes == 0) {
				std::cout << "connection closed by server" << std::endl;
				break;
			} else if (bytes == SOCKET_ERROR) {
				printWSErrorAndExit("recv");
			}

			std::cout << "Received: " << inBuffer << std::endl;

			// Wait 1 second
			Sleep(1000);
		}
		else
		{
			int err = WSAGetLastError();
			if (err == WSAECONNRESET) {
				std::cout << "connection closed by the server" << std::endl;
				break;
			}
			printWSErrorAndExit("send");
		}
	}

	// Close socket
	closesocket(s);

	// Winsock shutdown
	WSACleanup();
}

void printUsage()
{
	std::cout << "Usage: ./server server_address server_port" << std::endl;
}

int main(int argc, char **argv)
{
	if (argc != 3) {
		printUsage();
		PAUSE_AND_EXIT();
	}

	const char *serverStr = argv[1];
	int port = atoi(argv[2]);
	client(serverStr, port);

	PAUSE_AND_EXIT();
}
