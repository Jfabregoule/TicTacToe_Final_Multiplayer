#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <vector>

#include "../include/ConnectServer.h"
#include "../thirdparties/jsoncpp/include/json/json.h"
#include "../include/GameManager.h"

#define DEFAULT_PORT "21"
#define DEFAULT_BUFLEN 512

ConnectServer::ConnectServer(GameManager& gm) : gameManager(gm), serverSocket(INVALID_SOCKET), hWnd(NULL) {
    clientSockets.clear();
    Initialize();
}

ConnectServer::~ConnectServer() {
    Cleanup(serverSocket);
}

bool ConnectServer::InitializeWinsock() {
    WSADATA wsaData;
    int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        printf("WSAStartup failed: %d\n", iResult);
        return false;
    }
    return true;
}

bool ConnectServer::CreateClientSocket() {
    struct addrinfo* result = NULL,
        * ptr = NULL,
        hints;

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    int iResult = getaddrinfo(nullptr, DEFAULT_PORT, &hints, &result);
    if (iResult != 0) {
        printf("getaddrinfo failed: %d\n", iResult);
        return false;
    }

    serverSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (serverSocket == INVALID_SOCKET) {
        printf("Error at socket(): %ld\n", WSAGetLastError());
        freeaddrinfo(result);
        Cleanup(serverSocket);
        return false;
    }

    iResult = bind(serverSocket, result->ai_addr, (int)result->ai_addrlen);
    if (iResult == SOCKET_ERROR) {
        printf("bind failed: %d\n", WSAGetLastError());
        freeaddrinfo(result);
        Cleanup(serverSocket);
        return false;
    }

    freeaddrinfo(result);

    return true;
}

bool ConnectServer::StartListening() {
    if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR) {
        printf("listen failed: %d\n", WSAGetLastError());   
        Cleanup(serverSocket);
        return false;
    }
    return true;
}

bool ConnectServer::CreateHiddenWindow() {
    WNDCLASS windowClass = { 0 };
    windowClass.lpfnWndProc = ServerWindowProc;
    windowClass.hInstance = GetModuleHandle(NULL);
    windowClass.lpszClassName = L"MyWindowClass";

    if (!RegisterClass(&windowClass)) {
        printf("RegisterClass failed with error: %d\n", GetLastError());
        return false;
    }

    hWnd = CreateWindowEx(0, L"MyWindowClass", NULL, 0, 0, 0, 0, 0, HWND_MESSAGE, NULL, NULL, NULL);
    if (hWnd == NULL) {
        printf("CreateWindowEx failed with error: %d\n", GetLastError());
        return false;
    }

    return true;
}

bool ConnectServer::AssociateWithWindow() {
    LONG events = FD_ACCEPT;
    if (WSAAsyncSelect(serverSocket, hWnd, WM_USER + 1, events) == SOCKET_ERROR) {
        printf("WSAAsyncSelect failed with error: %d\n", WSAGetLastError());
        return false;
    }
    return true;
}

void ConnectServer::Cleanup(SOCKET socket = NULL) {
    if (socket) closesocket(socket);
    WSACleanup();
}

bool ConnectServer::Initialize() {
    if (!InitializeWinsock())
        return false;

    if (!CreateClientSocket()) {
        Cleanup();
        return false;
    }

    if (!StartListening()) {
        Cleanup(serverSocket);
        return false;
    }

    if (!CreateHiddenWindow()) {
        Cleanup(serverSocket);
        return false;
    }

    SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)this);

    if (!AssociateWithWindow()) {
        Cleanup(serverSocket);
        return false;
    }

    return true;
}

void ConnectServer::UpdatePlayers() {
    Json::Value root;
    root["Key"] = "Picked";
    root["Player1"] = gameManager.m_player1;
    root["Player2"] = gameManager.m_player2;
    std::string jsonToSend = root.toStyledString();

    for (SOCKET clientSocket : clientSockets) {
        int bytesSent = send(clientSocket, jsonToSend.c_str(), jsonToSend.length(), 0);
        if (bytesSent == SOCKET_ERROR) {
            std::cerr << "Error sending data to client" << std::endl;
            // Gérer l'erreur, par exemple, fermer la connexion avec le client défaillant
        }
    }
}

void ConnectServer::Update() {
    Json::Value root;
    root["Key"] = "Play";
    root["FirstLine"] = gameManager.m_map[0];
    root["SecondLine"] = gameManager.m_map[1];
    root["ThirdLine"] = gameManager.m_map[2];
    root["CurrentPlayer"] = gameManager.m_currentPlayer;
    std::cout << "Sending" << std::endl;
    std::cout << root << std::endl;
    std::string jsonToSend = root.toStyledString();

    for (SOCKET clientSocket : clientSockets) {
        int bytesSent = send(clientSocket, jsonToSend.c_str(), jsonToSend.length(), 0);
        if (bytesSent == SOCKET_ERROR) {
            std::cerr << "Error sending data to client" << std::endl;
            // Gérer l'erreur, par exemple, fermer la connexion avec le client défaillant
        }
    }
}

void ConnectServer::HandleAccept(SOCKET sock) {
    SOCKET incomingSocket;
    incomingSocket = accept(sock, NULL, NULL);
    if (incomingSocket == INVALID_SOCKET) {
        Cleanup(serverSocket);
        std::cout << "Error accepting an incomming socket !" << std::endl;
        return;
    }
    clientSockets.push_back(incomingSocket);
    WSAAsyncSelect(incomingSocket, hWnd, WM_USER + 1, FD_READ | FD_CLOSE);
}

void ConnectServer::PickPlayer(Json::Value picked)
{
    if (picked.isMember("Player1"))
        if (picked["Player1"] == 1)
            gameManager.m_player1 = 1;
    if (picked.isMember("Player2"))
        if (picked["Player2"] == 1)
            gameManager.m_player2 = 1;
    UpdatePlayers();
}

void ConnectServer::UpdateMap(Json::Value play)
{
    if (play.isMember("FirstLine") || play.isMember("SecondLine") || play.isMember("ThirdLine")) {
        std::string mapString;
        if (play.isMember("FirstLine"))
        {
            mapString = play["FirstLine"].asString();
            std::cout << mapString << std::endl;
            for (int i = 0; i < 3; ++i) {
                gameManager.m_map[0][i] = mapString[i];
            }
            gameManager.m_map[0][3] = '\0';
        }
        if (play.isMember("SecondLine"))
        {
            mapString = play["SecondLine"].asString();
            std::cout << mapString << std::endl;
            for (int i = 0; i < 3; ++i) {
                gameManager.m_map[1][i] = mapString[i];
            }
            gameManager.m_map[1][3] = '\0';
        }
        if (play.isMember("ThirdLine"))
        {
            mapString = play["ThirdLine"].asString();
            std::cout << mapString << std::endl;
            for (int i = 0; i < 3; ++i) {
                gameManager.m_map[2][i] = mapString[i];
            }
            gameManager.m_map[2][3] = '\0';
        }
        if (play.isMember("CurrentPlayer"))
        {
            if (play["CurrentPlayer"].asInt() == 1)
                gameManager.m_currentPlayer = 2;
            else if (play["CurrentPlayer"].asInt() == 2)
                gameManager.m_currentPlayer = 1;
        }
    }
    Update();
}

void ConnectServer::HandleRead(SOCKET sock) {
    char recvbuf[DEFAULT_BUFLEN];
    int bytesRead = recv(sock, recvbuf, DEFAULT_BUFLEN, 0);
    std::cout << "Received : " << recvbuf << std::endl;
    if (bytesRead > 0) {
        // Analyser la chaîne JSON reçue
        std::string jsonReceived(recvbuf, bytesRead);
        Json::Value root;
        Json::Reader reader;
        bool parsingSuccessful = reader.parse(jsonReceived, root);
        if (!parsingSuccessful) {
            std::cout << "Erreur lors de l'analyse du JSON reçu : " << reader.getFormattedErrorMessages() << std::endl;
            return;
        }

        if (root.isMember("Key") && root["Key"] == "Picked")
            PickPlayer(root);
        if (root.isMember("Key") && root["Key"] == "Play")
            UpdateMap(root);
    }
}

void ConnectServer::HandleClose(SOCKET sock) {
    std::cout << "Connection closed" << std::endl;
    for (int i = clientSockets.size() - 1; i >= 0; i--)
    {
        if (clientSockets[i] == sock) {
            clientSockets.erase(clientSockets.begin() + i);
            break;
        }
    }
    closesocket(sock);
}

void ConnectServer::EventDispatcher(int fdEvent, SOCKET sock) {
    switch (fdEvent) {
    case FD_ACCEPT:
        HandleAccept(sock);
        break;
    case FD_READ:
        HandleRead(sock);
        break;
    case FD_CLOSE:
        HandleClose(sock);
        break;
    default:
        std::cout << "Event not found: " << fdEvent << " !" << std::endl;
        break;
    }
}

LRESULT CALLBACK ConnectServer::ServerWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    ConnectServer* pServer = (ConnectServer*)GetWindowLongPtr(hwnd, GWLP_USERDATA);

    switch (uMsg) {
    case WM_USER + 1:
    {
        int fdEvent = WSAGETSELECTEVENT(lParam);
        SOCKET sock = wParam; // Socket client qui fait la requête

        pServer->EventDispatcher(fdEvent, sock);
        if (pServer->gameManager.m_currentPlayer == 1)
            pServer->gameManager.m_currentPlayer = 2;
        else
            pServer->gameManager.m_currentPlayer = 1;
    }
    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
        break;
    }
    return 0;
}