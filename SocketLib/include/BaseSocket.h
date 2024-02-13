#pragma once

#define NOMINMAX
#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib")

namespace SocketLibrary {
	/// <summary>
	/// This class is abstract and thus meant to be inherited from.
	/// The child class may override the necessary methods to handle
	/// occuring events of the Socket.
	/// </summary>
	class EventListener {
	public:
		virtual void HandleAccept(SOCKET sender) {};
		virtual void HandleRead(SOCKET sender) = 0;
		virtual void HandleClose(SOCKET sender) {};
	};

	class BaseSocket
	{
	protected:
		SOCKET _socket;
		HWND _window;
		const char* _port;
		EventListener* _eListener;

		BaseSocket(const char* port);
		BaseSocket(const char* port, EventListener* eListener);
		~BaseSocket();

		bool InitializeWinsock();
		virtual bool CreateSocket() { return false; };
		
		bool CreateEventWindow(const LPCWSTR className);
		bool AssociateWithWindow(LONG events);

		virtual void EventDispatcher(int fdEvent, SOCKET sender);
		static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

		void Cleanup();

	public:
		virtual bool Initialize() = 0;
	};
}