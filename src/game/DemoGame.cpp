#include "DemoGame.h"
#include "../engine/Input.h"
#include "../engine/Renderer.h"
#include <algorithm>
#include "../engine/Config.h"

void DemoGame::Init(const GameConfig& cfg, int /*windowW*/, int /*windowH*/)
{
    w_ = cfg.playerW;
    h_ = cfg.playerH;
    speed_ = cfg.playerSpeed;
}

void DemoGame::Update(double dtSeconds, const Input& input, int windowW, int windowH)
{
    if (!input.HasFocus())
        return;

    double dx = 0.0;
    double dy = 0.0;

    if (input.KeyDown(SDL_SCANCODE_A) || input.KeyDown(SDL_SCANCODE_LEFT))  dx -= 1.0;
    if (input.KeyDown(SDL_SCANCODE_D) || input.KeyDown(SDL_SCANCODE_RIGHT)) dx += 1.0;
    if (input.KeyDown(SDL_SCANCODE_W) || input.KeyDown(SDL_SCANCODE_UP))    dy -= 1.0;
    if (input.KeyDown(SDL_SCANCODE_S) || input.KeyDown(SDL_SCANCODE_DOWN))  dy += 1.0;

    if (dx != 0.0 && dy != 0.0)
    {
        dx *= 0.70710678118;
        dy *= 0.70710678118;
    }

    x_ += dx * speed_ * dtSeconds;
    y_ += dy * speed_ * dtSeconds;

    // Clamp
    x_ = std::clamp(x_, 0.0, static_cast<double>(windowW - w_));
    y_ = std::clamp(y_, 0.0, static_cast<double>(windowH - h_));
}

void DemoGame::Render(Renderer& renderer)
{
    renderer.DrawRect(static_cast<int>(x_), static_cast<int>(y_), w_, h_);
}
