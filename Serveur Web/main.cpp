#define _WINSOCK_DEPRECATED_NO_WARNINGS
#pragma comment(lib, "ws2_32.lib")

#include <WinSock2.h>
#include <iostream>
#include <string>
#include <stdio.h>
#include <stdlib.h>

#define DEFAULT_BUFLEN 512;
const char* SERVER_IP_ADDR = "192.168.1.18";

int main()
{
    std::cout << "Hello World!\n";

    SOCKET wsocket;
    SOCKET new_wsocket;
    WSADATA wsaData;
    struct sockaddr_in server;
    int server_len;
    int BUFFER_SIZE = 30720;

    //Initialize
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cout << "Could not initialize \n";
    }

    //Create a socket
    wsocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (wsocket == INVALID_SOCKET)
    {
        std::cout << "Could not create socket \n";
    }

    //bind socket to address

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr(SERVER_IP_ADDR);
    server.sin_port = htons(2579);
    server_len = sizeof(server);

    if (bind(wsocket, (SOCKADDR*)&server, server_len) != 0) {
        std::cout << "Could not bind socket \n";
    }

    //listen to address
    if (listen(wsocket, 20) != 0) {
        std::cout << "Could not start listening \n";
    }

    std::cout << "Listening on " << SERVER_IP_ADDR << std::endl;

    int bytes = 0;

    std::string strMap = "000<br>000<br>000";

    while (true)
    {
        //accept client request
        new_wsocket = accept(wsocket, (SOCKADDR*)&server, &server_len);

        if (new_wsocket == INVALID_SOCKET) {
            std::cout << "Could not accept \n";
        }

        //read request

        char buff[30720] = { 0 };
        bytes = recv(new_wsocket, buff, BUFFER_SIZE, 0);
        if (bytes < 0) {
            std::cout << "Could not read client request";
        }
        else
        {
            std::cout << bytes << std::endl;
        }

        size_t length = bytes;

        // Convert char array to std::string with explicit length

        if (bytes == 11)
        {
            strMap = buff;
            std::string tempMap = "";

            for (int i = 0; i < bytes; i++)
            {
                std::cout << "bite" << std::endl;
                if (strMap[i] == '\n')
                {
                    tempMap += "<br>";
                }
                else
                {
                    tempMap += strMap[i];
                }
            }
            std::cout << strMap << tempMap << std::endl;
            strMap = tempMap;
        }

        std::string serverMessage = "HTTP/1.1 200 OK\nContent-Type: text/html\nContent-Length:";

        std::string response =
            "<html>"
            "<head>"
            "<style>"
            "h1 {"
            "display: flex;"
            "justify-content: center;"
            "margin-top: 15%;"
            "font-size: 800%;"
            "}"
            "</style>"
            "<script>"
            "setTimeout(function() { location.reload(); }, 1000);"
            "</script>"
            "</head>"
            "<body>"
            "<h1>" + strMap + "</h1>"
            "</body>"
            "</html>";
        serverMessage.append(std::to_string(response.size()));
        serverMessage.append("\n\n");
        serverMessage.append(response);

        int bytesSent = 0;
        int totalBytesSent = 0;
        while (totalBytesSent < serverMessage.size()) {
            bytesSent = send(new_wsocket, serverMessage.c_str(), serverMessage.size(), 0);
            if (bytesSent < 0) {
                std::cout << "Could not send response";
            }
            totalBytesSent += bytesSent;
        }

        std::cout << "Sent response to client";

        closesocket(new_wsocket);
    }
        closesocket(wsocket);
        WSACleanup();

        return 0;
    }




//void UpdateMap(const char*)
//{
//    if (strMap.isMember("FirstLine") || strMap.isMember("SecondLine") || strMap.isMember("ThirdLine")) {
//        std::string mapString;
//        if (strMap.isMember("FirstLine"))
//        {
//            mapString = strMap["FirstLine"].asString();
//            //std::cout << mapString << std::endl;
//            for (int i = 0; i < 3; ++i) {
//                gameManager.m_map[0][i] = mapString[i];
//            }
//            gameManager.m_map[0][3] = '\0';
//        }
//        if (strMap.isMember("SecondLine"))
//        {
//            mapString = strMap["SecondLine"].asString();
//            //std::cout << mapString << std::endl;
//            for (int i = 0; i < 3; ++i) {
//                gameManager.m_map[1][i] = mapString[i];
//            }
//            gameManager.m_map[1][3] = '\0';
//        }
//        if (strMap.isMember("ThirdLine"))
//        {
//            mapString = strMap["ThirdLine"].asString();
//            //std::cout << mapString << std::endl;
//            for (int i = 0; i < 3; ++i) {
//                gameManager.m_map[2][i] = mapString[i];
//            }
//            gameManager.m_map[2][3] = '\0';
//        }
//        if (strMap.isMember("CurrentPlayer"))
//        {
//            gameManager.m_currentPlayer = play["CurrentPlayer"].asInt();
//        }
//    }
//    if (play.isMember("FirstLine") || play.isMember("SecondLine"))
//    {
//
//    }
//}

//void HandleRead(SOCKET sock) {
//    char recvbuf[512];
//    int bytesRead = recv(sock, recvbuf, 512, 0);
//    //std::cout << "Received : " << recvbuf << std::endl;
//    if (bytesRead > 0) {
//        // Analyser la chaîne JSON reçue
//        
//    }
//}