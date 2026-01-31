#pragma once
#include <SDL.h>
#include <string>

class Texture2D
{
public:
    ~Texture2D();

    bool LoadFromBMP(SDL_Renderer* renderer, const std::string& path,
        bool useColorKey = false,
        Uint8 keyR = 255, Uint8 keyG = 255, Uint8 keyB = 255);
    void Destroy();

    SDL_Texture* Get() const { return tex_; }
    SDL_Texture* Raw() const { return tex_; }

    int Width() const { return w_; }
    int Height() const { return h_; }

private:
    SDL_Texture* tex_ = nullptr;
    int w_ = 0;
    int h_ = 0;
};
