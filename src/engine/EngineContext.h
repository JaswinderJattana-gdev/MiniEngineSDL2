#pragma once

#include "Config.h"
#include <SDL.h>

class InputMap;

class Renderer;

struct EngineContext
{
    // Updated by the engine on resize
    int windowW = 0;
    int windowH = 0;

    int logicalW = 800;
    int logicalH = 450;

    // Shared configuration
    EngineConfig engineCfg{};
    GameConfig gameCfg{};

    InputMap* input = nullptr;
    Renderer* renderer = nullptr;

    SDL_Window* window = nullptr;
};
