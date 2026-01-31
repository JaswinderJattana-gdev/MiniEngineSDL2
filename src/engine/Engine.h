#pragma once

#include <SDL.h>
#include "Input.h"
#include "Renderer.h"
#include "SceneManager.h"
#include "EngineContext.h"
#include "InputMap.h"
#include <memory>
#include "Config.h"

class Engine
{
public:
    Engine() = default;
    ~Engine() = default;

    bool Init();
    void Run();
    void Shutdown();

private:
    void PumpEvents();
    void Update(double dtSeconds);
    void Render(double alpha);

    int windowW_ = 800;
    int windowH_ = 450;

private:
    SDL_Window* window_ = nullptr;
    bool running_ = false;
    Input input_;
    Renderer renderer_;
    EngineConfig engineCfg_{};
    SceneManager scenes_;
    EngineContext ctx_{};
    InputMap inputMap_;
};
