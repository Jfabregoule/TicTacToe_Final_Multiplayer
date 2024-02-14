#define _WINSOCK_DEPRECATED_NO_WARNINGS
#pragma comment(lib, "ws2_32.lib")

#include <WinSock2.h>
#include <iostream>
#include <string>
#include <stdio.h>
#include <stdlib.h>

class WebServer
{
private:

	SOCKET wsocket;
	SOCKET new_wsocket;
	WSADATA wsaData;
	struct sockaddr_in server;
	int server_len;
	int BUFFER_SIZE;
	int bytes;
	std::string strMap;
	char buff[30720];
	size_t length;
	std::string serverMessage;
	std::string response;

public:
	WebServer();
	int MapRefresh(std::string map);
};
