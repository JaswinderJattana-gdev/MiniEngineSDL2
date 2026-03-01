#include "Texture2D.h"
#include "Log.h"
#include <string>
#include <cstring>

Texture2D::~Texture2D()
{
    Destroy();
}

void Texture2D::Destroy()
{
    if (tex_)
    {
        SDL_DestroyTexture(tex_);
        tex_ = nullptr;
    }
    w_ = 0;
    h_ = 0;
}

bool Texture2D::LoadFromBMP(SDL_Renderer* renderer, const std::string& path,
    bool useColorKey, Uint8 keyR, Uint8 keyG, Uint8 keyB)
{
    Destroy();

    SDL_Surface* surf = SDL_LoadBMP(path.c_str());
    if (!surf)
    {
        Log::Error(std::string("SDL_LoadBMP failed: ") + SDL_GetError());
        return false;
    }

    if (useColorKey)
    {
        // 1) First, convert near-white pixels into pure key color to reduce halo.
        // Increase threshold to remove more halo; decrease if it eats character edges.
        const Uint8 threshold = 250;

        if (SDL_LockSurface(surf) == 0)
        {
            Uint8* pixels = static_cast<Uint8*>(surf->pixels);
            const int bpp = surf->format->BytesPerPixel;

            for (int y = 0; y < surf->h; ++y)
            {
                Uint8* row = pixels + y * surf->pitch;
                for (int x = 0; x < surf->w; ++x)
                {
                    Uint8* p = row + x * bpp;

                    Uint32 pixel = 0;
                    std::memcpy(&pixel, p, bpp);

                    Uint8 r, g, b;
                    SDL_GetRGB(pixel, surf->format, &r, &g, &b);

                    // "Near-white" => force to exact key color
                    if (r >= threshold && g >= threshold && b >= threshold)
                    {
                        const Uint32 keyPixel = SDL_MapRGB(surf->format, keyR, keyG, keyB);
                        std::memcpy(p, &keyPixel, bpp);
                    }
                }
            }

            SDL_UnlockSurface(surf);
        }

        // 2) Now set the color key (exact key color becomes transparent)
        const Uint32 key = SDL_MapRGB(surf->format, keyR, keyG, keyB);
        if (SDL_SetColorKey(surf, SDL_TRUE, key) != 0)
        {
            Log::Warn(std::string("SDL_SetColorKey failed: ") + SDL_GetError());
        }
    }

    w_ = surf->w;
    h_ = surf->h;

    tex_ = SDL_CreateTextureFromSurface(renderer, surf);
    SDL_FreeSurface(surf);

    if (!tex_)
    {
        Log::Error(std::string("SDL_CreateTextureFromSurface failed: ") + SDL_GetError());
        return false;
    }

    // Enable alpha blending (needed for color key to behave as transparent)
    SDL_SetTextureBlendMode(tex_, SDL_BLENDMODE_BLEND);

    return true;
}