// CodingCPPAssignment3.cpp : This file contains the 'main' function. Program execution begins and ends there. Or does it?

#include <iostream>
#include <SFML/Graphics.hpp>
#include "GameEngine.h"

#include "Profiler.h"

int main()
{
    //PROFILE_FUNCTION();
    std::cout << "Booting up!\n";
    std::cout << "Passing assets to game engine now.\n";
    GameEngine g("assets/assets.txt");
    g.run();

    return 0;
}
