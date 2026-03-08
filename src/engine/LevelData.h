#pragma once
#include <vector>
#include <SDL.h>

struct LevelData
{
    int worldW = 4000;
    int worldH = 4000;

    SDL_Point playerSpawn{ 200, 200 };

    std::vector<SDL_Rect> obstacles;
};