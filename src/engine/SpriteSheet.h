#pragma once
#include "Texture2D.h"
#include <SDL.h>
#include <string>

class SpriteSheet
{
public:
    bool LoadFromBMP(SDL_Renderer* r, const std::string& path,
        int frameW, int frameH,
        int columns, int rows);

    void SetTexture(SDL_Texture* tex, int frameW, int frameH, int columns, int rows);

    SDL_Texture* Texture() const;

    int FrameW() const { return frameW_; }
    int FrameH() const { return frameH_; }
    int Columns() const { return columns_; }
    int Rows() const { return rows_; }

    SDL_Rect FrameRect(int col, int row) const;

private:
    Texture2D tex_;
    SDL_Texture* externalTex_ = nullptr;
    bool usingExternalTexture_ = false;

    int frameW_ = 0;
    int frameH_ = 0;
    int columns_ = 0;
    int rows_ = 0;
};