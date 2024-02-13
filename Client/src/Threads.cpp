#define NOMINMAX
#include "../include/Threads.h"

#include <SFML/Graphics.hpp>

Threads::Threads() : threadHandle_(NULL) {}



void Threads::start() {
    sf::Context context;
    threadHandle_ = CreateThread(NULL, 0, &Threads::threadFunctionStatic, this, 0, NULL);
    if (threadHandle_ == NULL) {
        std::cerr << "Erreur lors de la création du thread" << std::endl;
    }
}

void Threads::join() {
    if (threadHandle_ != NULL) {
        WaitForSingleObject(threadHandle_, INFINITE);
        CloseHandle(threadHandle_);
        threadHandle_ = NULL;
    }
}

DWORD WINAPI Threads::threadFunctionStatic(LPVOID lpParam) {
    Threads* myThread = static_cast<Threads*>(lpParam);
    myThread->EnterThreadFunction();
    myThread->ExecuteThreadFunction();
    myThread->ExitThreadFunction();
    return 0;
}

void Threads::EnterThreadFunction() {
    std::cout << "Enter" << std::endl;
}

void Threads::ExecuteThreadFunction() {
    std::cout << "Execute" << std::endl;
}

void Threads::ExitThreadFunction() {
    std::cout << "Exit" << std::endl;
}