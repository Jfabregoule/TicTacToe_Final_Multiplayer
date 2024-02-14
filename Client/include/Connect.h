#pragma once
#define NOMINMAX
#include <winsock2.h>
#include <ws2tcpip.h>
#include "../thirdparties/jsoncpp/include/json/json.h"
#include "../include/Threads.h"

class GameManager;
class CriticalSection;

class Connect : public Threads
{
private:
    WSADATA wsaData;
    SOCKET ConnectSocket;
    struct addrinfo* result = NULL,
        * ptr = NULL,
        hints;
    char* recvbuf;
    int iResult;
    int recvbuflen;
    HWND hWnd;
    GameManager& gameManager;
    CRITICAL_SECTION m_critical;
    BOOL bRet;
    MSG msg;
public:
    Connect(GameManager& gm);
    ~Connect();
    bool InitializeWinSock();
    SOCKET CreateAndConnectSocket(const char* serverAddress);
    bool CreateSocket(const char* serverAddress);
    bool CreateHiddenWindow();
    bool AssociateSocketWithWindow(HWND window, LONG events);
    void CleanupSocket(SOCKET socket);
    void CleanupWinsock();
    int Send(char* buff);
    int Send(const char* buff);
    int initialize();

    void EventDispatcher(int fdEvent, SOCKET sock);
    void HandleAccept(SOCKET sock);
    void HandleRead(SOCKET sock);
    void HandleClose(SOCKET sock);

    void UpdateScore(Json::Value score);
    void PickPlayer(Json::Value picked);
    void UpdateMap(Json::Value play);


    virtual void EnterThreadFunction();
    virtual void ExecuteThreadFunction();
    virtual void ExitThreadFunction();

    static LRESULT CALLBACK ClientWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

};