#include <SFML/Graphics.hpp>
#include <SFML/System/Sleep.hpp>
#include <SFML/Audio.hpp>

#include "include/Threads.h"
#include "include/Connect.h"
#include "include/GameManager.h"

int main()
{
    
    GameManager        gameThread;
    Connect            gameConnect(gameThread);
    gameConnect.start();
    gameThread.Start();
    return 0;
}