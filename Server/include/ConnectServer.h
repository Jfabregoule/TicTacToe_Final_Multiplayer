#pragma once

#include <vector>
#define NOMINMAX
#include <winsock2.h>
#include <ws2tcpip.h>

class GameManager;

class ConnectServer
{
private:
	GameManager&				gameManager;
	SOCKET						serverSocket;
	HWND						hWnd;
	std::vector<SOCKET>			clientSockets;

public:
	ConnectServer(GameManager& gm);
	~ConnectServer();

	bool Initialize();
	bool InitializeWinsock();
	bool CreateClientSocket();
	bool StartListening();
	bool CreateHiddenWindow();
	bool AssociateWithWindow();
	void Cleanup(SOCKET socket);
	void Update();

	void EventDispatcher(int fdEvent, SOCKET sock);
	void HandleAccept(SOCKET sock);
	void HandleRead(SOCKET sock);
	void HandleClose(SOCKET sock);

	static LRESULT CALLBACK ServerWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
};