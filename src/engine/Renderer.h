#pragma once
#include <SDL.h>

class Renderer
{
public:
    bool Init(SDL_Window* window, bool useHardwareAcceleration, bool useVsync);
    void SetLogicalSize(int w, int h);

    void BeginFrame();
    void EndFrame();
    void Shutdown();

    // Debug draw (temporary)
    void DrawRect(int x, int y, int w, int h);

    SDL_Renderer* Raw() const { return renderer_; }

    void DrawTexture(SDL_Texture* tex, int x, int y, int w, int h);
    void DrawTextureRegion(SDL_Texture* tex, const SDL_Rect& src, int x, int y, int w, int h);

    void Clear(Uint8 r, Uint8 g, Uint8 b, Uint8 a = 255);
    void SetDrawColor(Uint8 r, Uint8 g, Uint8 b, Uint8 a = 255);
    void DrawLine(int x1, int y1, int x2, int y2);
    void FillRect(int x, int y, int w, int h);

private:
    SDL_Renderer* renderer_ = nullptr;
};
