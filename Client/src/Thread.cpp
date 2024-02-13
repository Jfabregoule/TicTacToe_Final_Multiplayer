#include "../include/Thread.h"

Thread::Thread() : threadHandle_(NULL) {}



void Thread::start() {
    threadHandle_ = CreateThread(NULL, 0, &Thread::threadFunctionStatic, this, 0, NULL);
    if (threadHandle_ == NULL) {
        std::cerr << "Erreur lors de la création du thread" << std::endl;
    }
}

void Thread::join() {
    if (threadHandle_ != NULL) {
        WaitForSingleObject(threadHandle_, INFINITE);
        CloseHandle(threadHandle_);
        threadHandle_ = NULL;
    }
}

DWORD WINAPI Thread::threadFunctionStatic(LPVOID lpParam) {
    Thread* myThread = static_cast<Thread*>(lpParam);
    myThread->EnterThreadFunction();
    myThread->ExecuteThreadFunction();
    myThread->EnterThreadFunction();
    return 0;
}

void Thread::EnterThreadFunction() {
    std::cout << "Enter" << std::endl;
}

void Thread::ExecuteThreadFunction() {
    std::cout << "Execute" << std::endl;
}

void Thread::ExitThreadFunction() {
    std::cout << "Exit" << std::endl;
}