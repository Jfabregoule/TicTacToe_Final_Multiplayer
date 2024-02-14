#include <SFML/Graphics.hpp>
#include <SFML/System/Sleep.hpp>
#include <SFML/Audio.hpp>

#include "include/GameManager.h"
#include "include/ConnectServer.h"

int main()
{
    GameManager        game;
    game.Init();
    game.Start();
    return 0;
}