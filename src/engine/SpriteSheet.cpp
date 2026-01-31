#include "SpriteSheet.h"
#include "Log.h"
#include <string>

bool SpriteSheet::LoadFromBMP(SDL_Renderer* r, const std::string& path,
    int frameW, int frameH,
    int columns, int rows)
{
    frameW_ = frameW;
    frameH_ = frameH;
    columns_ = columns;
    rows_ = rows;

    // Treat pure white as transparent (BMP color key)
    if (!tex_.LoadFromBMP(r, path, true, 255, 255, 255))
        return false;

    // Optional sanity check
    const int expectedW = frameW_ * columns_;
    const int expectedH = frameH_ * rows_;

    if (tex_.Width() < expectedW || tex_.Height() < expectedH)
    {
        Log::Warn("SpriteSheet BMP smaller than expected grid (check frameW/frameH/cols/rows).");
    }

    return true;
}

SDL_Rect SpriteSheet::FrameRect(int col, int row) const
{
    SDL_Rect rc;
    rc.x = col * frameW_;
    rc.y = row * frameH_;
    rc.w = frameW_;
    rc.h = frameH_;
    return rc;
}
