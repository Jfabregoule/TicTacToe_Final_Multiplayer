#pragma once
#define NOMINMAX
#include <vector>

#include "BaseSocket.h"

namespace SocketLibrary {
	class ServerSocket : public BaseSocket
	{
	private:
		bool CreateSocket() override;
		bool StartListening();

	public:
		std::vector<SOCKET> Clients;

		ServerSocket(const char* port);
		ServerSocket(const char* port, EventListener* eListener);
		~ServerSocket();

		bool Initialize() override;
	};
}