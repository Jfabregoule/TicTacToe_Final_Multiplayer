#include <SFML/Graphics.hpp>
#include <SFML/System/Sleep.hpp>
#include <SFML/Audio.hpp>

#include "include/Threads.h"
#include "include/Connect.h"
#include "include/GameManager.h"

int main()
{
    
    GameManager        gameThread;
    if (gameThread.Init() != 0) {
        std::cout << "GameManager Init failed" << std::endl;
        return 1;
    }
    gameThread.Start();
    return 0;
}