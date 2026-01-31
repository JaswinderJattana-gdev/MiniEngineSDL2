#include "Renderer.h"
#include "Log.h"

#include <string>
#include <iostream>


bool Renderer::Init(SDL_Window* window, bool useHardwareAcceleration, bool useVsync)
{
    auto tryCreate = [&](bool hw, bool vs) -> SDL_Renderer*
        {
            Uint32 flags = 0;
            flags |= hw ? SDL_RENDERER_ACCELERATED : SDL_RENDERER_SOFTWARE;
            if (vs) flags |= SDL_RENDERER_PRESENTVSYNC;
            return SDL_CreateRenderer(window, -1, flags);
        };

    // 1) Requested mode
    renderer_ = tryCreate(useHardwareAcceleration, useVsync);

    // 2) Fallback: same accel, no vsync
    if (!renderer_)
        renderer_ = tryCreate(useHardwareAcceleration, false);

    // 3) Fallback: software, no vsync
    if (!renderer_)
        renderer_ = tryCreate(false, false);

    if (!renderer_)
    {
        Log::Error(std::string("SDL_CreateRenderer failed (all modes): ") + SDL_GetError());
        return false;
    }

    return true;
}

void Renderer::BeginFrame()
{
    // White background
    //SDL_SetRenderDrawColor(renderer_, 255, 255, 255, 255);
    //SDL_RenderClear(renderer_);
}

void Renderer::DrawRect(int x, int y, int w, int h)
{
    SDL_Rect r{ x, y, w, h };
    SDL_SetRenderDrawColor(renderer_, 200, 200, 200, 255);
    SDL_RenderFillRect(renderer_, &r);
}

void Renderer::EndFrame()
{
    SDL_RenderPresent(renderer_);
}

void Renderer::Shutdown()
{
    if (renderer_)
    {
        SDL_DestroyRenderer(renderer_);
        renderer_ = nullptr;
    }
}
void Renderer::SetLogicalSize(int w, int h)
{
    if (!renderer_) return;

    // Enables automatic scaling to a logical coordinate system.
    // SDL will letterbox/pillarbox to preserve aspect ratio.
    SDL_RenderSetLogicalSize(renderer_, w, h);
}

void Renderer::DrawTexture(SDL_Texture* tex, int x, int y, int w, int h)
{
    if (!renderer_ || !tex) return;

    SDL_Rect dst{ x, y, w, h };
    SDL_RenderCopy(renderer_, tex, nullptr, &dst);
}

void Renderer::DrawTextureRegion(SDL_Texture* tex, const SDL_Rect& src, int x, int y, int w, int h)
{
    if (!renderer_ || !tex) return;

    SDL_Rect dst{ x, y, w, h };
    SDL_RenderCopy(renderer_, tex, &src, &dst);
}
void Renderer::Clear(Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
    if (!renderer_) return;
    SDL_SetRenderDrawColor(renderer_, r, g, b, a);
    SDL_RenderClear(renderer_);
}

void Renderer::SetDrawColor(Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
    if (!renderer_) return;
    SDL_SetRenderDrawColor(renderer_, r, g, b, a);
}

void Renderer::DrawLine(int x1, int y1, int x2, int y2)
{
    if (!renderer_) return;
    SDL_RenderDrawLine(renderer_, x1, y1, x2, y2);
}

void Renderer::FillRect(int x, int y, int w, int h)
{
    if (!renderer_) return;
    SDL_Rect rc{ x, y, w, h };
    SDL_RenderFillRect(renderer_, &rc);
}

