#include "Engine.h"
#include "Log.h"
#include "Assert.h"
#include "../game/MenuScene.h"
#include <iostream>
#include <algorithm>
#include <cstdio>
#include <string>
#include <memory>

bool Engine::Init()
{
    if (SDL_Init(SDL_INIT_VIDEO) != 0)
    {
        Log::Error(std::string("SDL_Init failed: ") + SDL_GetError());
        return false;
    }
    Log::Info("SDL initialized");

    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0"); 

    // Apply config -> context early
    ctx_.logicalW = ctx_.engineCfg.logicalW;
    ctx_.logicalH = ctx_.engineCfg.logicalH;

    Uint32 flags = SDL_WINDOW_SHOWN;
    if (ctx_.engineCfg.resizable)
        flags |= SDL_WINDOW_RESIZABLE;

    // Create window
    window_ = SDL_CreateWindow(
        ctx_.engineCfg.windowTitle,
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        ctx_.engineCfg.windowW, ctx_.engineCfg.windowH,
        flags
    );

    if (!window_)
    {
        const char* err = SDL_GetError();
        Log::Error(std::string("SDL_CreateWindow failed: ") + err);

        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,
            "MiniEngineSDL2 - Window Create Failed",
            err,
            nullptr);

        return false;
    }
    Log::Info("Window created");

    ctx_.window = window_;

    SDL_GetWindowSize(window_, &windowW_, &windowH_);
    ctx_.windowW = windowW_;
    ctx_.windowH = windowH_;

#if ME_DEBUG
    // Debug convenience: let the loop run freely unless you explicitly enable vsync in config
    // ctx_.engineCfg.useVsync = false;
#endif

    // Initialize renderer (only after window is valid)
    if (!renderer_.Init(window_, ctx_.engineCfg.useHardwareAcceleration, ctx_.engineCfg.useVsync))
        return false;
    Log::Info("Renderer created");

    ctx_.renderer = &renderer_;

    // Set logical size if enabled (now ctx_.logicalW/H are valid)
    if (ctx_.engineCfg.useLogicalSize)
        renderer_.SetLogicalSize(ctx_.logicalW, ctx_.logicalH);

    // Input
    input_.Init();
    ctx_.input = &inputMap_;

    // Default bindings
    inputMap_.Bind(Action::Confirm, SDL_SCANCODE_RETURN);
    inputMap_.Bind(Action::Back, SDL_SCANCODE_ESCAPE);
    inputMap_.Bind(Action::Pause, SDL_SCANCODE_P);

    inputMap_.Bind(Action::MoveLeft, SDL_SCANCODE_A);
    inputMap_.Bind(Action::MoveLeft, SDL_SCANCODE_LEFT);
    inputMap_.Bind(Action::MoveRight, SDL_SCANCODE_D);
    inputMap_.Bind(Action::MoveRight, SDL_SCANCODE_RIGHT);
    inputMap_.Bind(Action::MoveUp, SDL_SCANCODE_W);
    inputMap_.Bind(Action::MoveUp, SDL_SCANCODE_UP);
    inputMap_.Bind(Action::MoveDown, SDL_SCANCODE_S);
    inputMap_.Bind(Action::MoveDown, SDL_SCANCODE_DOWN);

    inputMap_.Bind(Action::Dash, SDL_SCANCODE_LSHIFT);
    inputMap_.Bind(Action::Dash, SDL_SCANCODE_RSHIFT);

    running_ = true;

    scenes_.Set(std::make_unique<MenuScene>(scenes_, ctx_));

    return true;
}

void Engine::PumpEvents()
{
    SDL_Event e;
    while (SDL_PollEvent(&e))
    {
        input_.ProcessEvent(e);

        if (e.type == SDL_WINDOWEVENT)
        {
            if (e.window.event == SDL_WINDOWEVENT_SIZE_CHANGED)
            {
                windowW_ = e.window.data1;
                windowH_ = e.window.data2;
                ctx_.windowW = windowW_;
                ctx_.windowH = windowH_;
            }
        }
    }

    if (input_.QuitRequested())
        running_ = false;
}

void Engine::Update(double dtSeconds)
{
    if (auto* scene = scenes_.Top())
        scene->Update(dtSeconds, input_, ctx_);
}

void Engine::Render(double /*alpha*/)
{
    renderer_.BeginFrame();

    // Render scene below top first (if it exists), then top on top of it.
    if (auto* below = scenes_.UnderTop())
        below->Render(renderer_, ctx_);

    if (auto* top = scenes_.Top())
        top->Render(renderer_, ctx_);

    renderer_.EndFrame();
}

void Engine::Run()
{
    Log::Info("Engine::Run started");

    const double dt = ctx_.engineCfg.fixedDt;

    const uint64_t freq = SDL_GetPerformanceFrequency();
    uint64_t prev = SDL_GetPerformanceCounter();

    double accumulator = 0.0;

    // FPS / frame time tracking
    double fpsTimer = 0.0;
    uint32_t frames = 0;

    while (running_)
    {
        static bool first = true;
        if (first)
        {
            Log::Info("Main loop entered");
            first = false;
        }

        const uint64_t now = SDL_GetPerformanceCounter();
        const double frameSeconds = static_cast<double>(now - prev) / static_cast<double>(freq);
        prev = now;

        // Prevent "spiral of death" on breakpoint / alt-tab / hitch
        const double clampedFrame = std::min(frameSeconds, engineCfg_.maxFrameTime);
        accumulator += clampedFrame;

        // Update FPS once per second
        fpsTimer += clampedFrame;
        frames++;

        if (fpsTimer >= 1.0)
        {
            const double fps = static_cast<double>(frames) / fpsTimer;
            const double ms = 1000.0 / fps;

            char title[128];
            std::snprintf(title, sizeof(title), "MiniEngineSDL2 | %.1f FPS | %.2f ms", fps, ms);
            SDL_SetWindowTitle(window_, title);

            fpsTimer = 0.0;
            frames = 0;
        }

        input_.BeginFrame();

        PumpEvents();

        while (accumulator >= dt)
        {
            Update(dt);
            accumulator -= dt;
        }

        const double alpha = accumulator / dt;
        Render(alpha);

        // Yield a tiny bit to reduce CPU usage.
        SDL_Delay(1);
    }
}

void Engine::Shutdown()
{
    if (window_)
    {
        SDL_DestroyWindow(window_);
        window_ = nullptr;
    }
    renderer_.Shutdown();
    SDL_Quit();
}
