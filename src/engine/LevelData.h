#pragma once
#include <vector>
#include <SDL.h>

struct LevelObjectData
{
    SDL_Rect rect{};
    int hp = 1;
};

struct LevelData
{
    int worldW = 4000;
    int worldH = 4000;

    SDL_Point playerSpawn{ 200, 200 };

    std::vector<SDL_Rect> obstacles;

    // Gameplay objects
    std::vector<LevelObjectData> enemies;
    std::vector<LevelObjectData> destructibles;
};