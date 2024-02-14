#include "../include/WebServer.h"

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#pragma comment(lib, "ws2_32.lib")

#include <WinSock2.h>
#include <iostream>
#include <string>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>

#define DEFAULT_BUFLEN 512;

WebServer::WebServer() {
    BUFFER_SIZE = 30720;
    MapRefresh("000<br>000<br>000");
}

int WebServer::MapRefresh(std::string strMap) {
    std::cout << "Hello World!\n";

    //strMap = "000<br>000<br>000";

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr("10.1.144.29");
    server.sin_port = htons(2579);
    server_len = sizeof(server);

    std::string tempMap = "";

    for (int i = 0; i < strMap.size(); i++)
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


    serverMessage = "HTTP/1.1 200 OK\nContent-Type: text/html\nContent-Length:";

    response =
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

    return 0;
}