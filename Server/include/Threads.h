#pragma once
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <iostream>

class Threads
{
public:

	Threads();

	static DWORD WINAPI threadFunctionStatic(LPVOID lpParam);

	virtual void EnterThreadFunction();
	virtual void ExecuteThreadFunction();
	virtual void ExitThreadFunction();

	void start();

	void join();

protected:

	HANDLE threadHandle_;
};

