#pragma once

#define NOMINMAX
#include "BaseSocket.h"

namespace SocketLibrary {
	class ClientSocket : public BaseSocket
	{
	private:
		const char* _serverIpAddr;

		bool CreateSocket() override;

	public:
		ClientSocket(const char* serverIpAddr, const char* port);
		ClientSocket(const char* serverIpAddr, const char* port, EventListener* eListener);
		~ClientSocket();

		bool Initialize() override;
		bool Send(const char* buffer);
	};
}