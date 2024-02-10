#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include "../include/Connect.h"
#include <stdio.h>
#include <stdlib.h>
#include <cstring>

const char* SERVER_IP_ADDR = "192.168.1.136";

#define DEFAULT_BUFLEN 1024
#define DEFAULT_PORT "21"

Connect::Connect() : ConnectSocket(INVALID_SOCKET) {
    recvbuflen = DEFAULT_BUFLEN;
    initialize();
};

Connect::~Connect() {
    CleanupSocket(ConnectSocket);
    CleanupWinsock();
};

bool Connect::InitializeWinSock() {
    int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        printf("WSAStartup failed: %d\n", iResult);
        return false;
    }
    return true;
}

SOCKET Connect::CreateAndConnectSocket(const char* serverAddress) {
    struct addrinfo* result = NULL,
        * ptr = NULL,
        hints;

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    // Resolve the server address and port
    int iResult = getaddrinfo(serverAddress, DEFAULT_PORT, &hints, &result);
    if (iResult != 0) {
        printf("getaddrinfo failed: %d\n", iResult);
        return INVALID_SOCKET;
    }

    // Attempt to connect to an address until one succeeds
    for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {
        // Create a SOCKET for connecting to server
        ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
        if (ConnectSocket == INVALID_SOCKET) {
            printf("socket failed: %ld\n", WSAGetLastError());
            continue;
        }

        // Connect to server.
        iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
        if (iResult == SOCKET_ERROR) {
            closesocket(ConnectSocket);
            ConnectSocket = INVALID_SOCKET;
            continue;
        }
        break;
    }

    freeaddrinfo(result);

    if (ConnectSocket == INVALID_SOCKET) {
        printf("Unable to connect to server!\n");
        return INVALID_SOCKET;
    }

    return ConnectSocket;
}

bool Connect::AssociateSocketWithWindow(HWND window, LONG events) {
    if (WSAAsyncSelect(ConnectSocket, window, WM_USER + 1, events) == SOCKET_ERROR) {
        printf("WSAAsyncSelect failed with error: %d\n", WSAGetLastError());
        return false;
    }
    return true;
}

void Connect::CleanupSocket(SOCKET socket) {
    closesocket(socket);
}

void Connect::CleanupWinsock() {
    WSACleanup();
}

int Connect::initialize() {
    if (!InitializeWinSock()) {
        return 1;
    }
    CreateAndConnectSocket(SERVER_IP_ADDR);
    if (ConnectSocket == INVALID_SOCKET) {
        CleanupWinsock();
        return 1;
    }
    if (!AssociateSocketWithWindow(GetConsoleWindow(), FD_READ | FD_CLOSE)) {
        CleanupSocket(ConnectSocket);
        CleanupWinsock();
        return 1;
    }
    return 0;
}

int Connect::Send(const char* buff) {
    int iResult = send(ConnectSocket, buff, strlen(buff), 0);
    if (iResult == SOCKET_ERROR) {
        printf("send failed: %d\n", WSAGetLastError());
        CleanupSocket(ConnectSocket);
        CleanupWinsock();
        return 1;
    }
    printf("Bytes Sent: %d\n", iResult);
    return 0;
}