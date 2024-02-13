#include <SFML/Graphics.hpp>
#include <SFML/System/Sleep.hpp>
#include <SFML/Audio.hpp>

#include "Thread.h"
#include "include/GameManager.h"

int main()
{
    GameManager        gameThread;

    gameThread.start();
    while (true);
    return 0;
}