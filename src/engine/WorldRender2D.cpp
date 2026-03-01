#include "WorldRender2D.h"
#include <cmath>

namespace WorldRender2D
{
    void DrawGrid(Renderer& renderer, const Camera2D& camera, int viewW, int viewH, int cell, int majorEvery)
    {
        // Camera top-left in world space
        const double camXf = camera.Position().x;
        const double camYf = camera.Position().y;

        const int camX = static_cast<int>(std::floor(camXf));
        const int camY = static_cast<int>(std::floor(camYf));

        // Camera offset within a cell (0..cell-1)
        int offsetX = camX % cell;
        int offsetY = camY % cell;
        if (offsetX < 0) offsetX += cell;
        if (offsetY < 0) offsetY += cell;

        // Minor grid
        renderer.SetDrawColor(220, 220, 220, 255);
        for (int x = -offsetX; x <= viewW; x += cell)
            renderer.DrawLine(x, 0, x, viewH);

        for (int y = -offsetY; y <= viewH; y += cell)
            renderer.DrawLine(0, y, viewW, y);

        // Major grid
        const int major = cell * majorEvery;

        int offsetXMaj = camX % major;
        int offsetYMaj = camY % major;
        if (offsetXMaj < 0) offsetXMaj += major;
        if (offsetYMaj < 0) offsetYMaj += major;

        renderer.SetDrawColor(200, 200, 200, 255);
        for (int x = -offsetXMaj; x <= viewW; x += major)
            renderer.DrawLine(x, 0, x, viewH);

        for (int y = -offsetYMaj; y <= viewH; y += major)
            renderer.DrawLine(0, y, viewW, y);
    }

    void DrawWorldBoundsFrame(Renderer& renderer, const Camera2D& camera, int viewW, int viewH, int worldW, int worldH, int thickness)
    {
        const double camX = camera.Position().x;
        const double camY = camera.Position().y;

        // screen-space positions of world edges
        const int sxL = static_cast<int>(std::round(0.0 - camX));
        const int syT = static_cast<int>(std::round(0.0 - camY));
        const int sxR = static_cast<int>(std::round(static_cast<double>(worldW) - camX));
        const int syB = static_cast<int>(std::round(static_cast<double>(worldH) - camY));

        renderer.SetDrawColor(120, 120, 120, 255);

        // LEFT wall (inside the world): [sxL .. sxL+thickness)
        renderer.FillRect(sxL, 0, thickness, viewH);

        // TOP wall (inside the world): [syT .. syT+thickness)
        renderer.FillRect(0, syT, viewW, thickness);

        // RIGHT wall (inside): [sxR-thickness .. sxR)
        renderer.FillRect(sxR - thickness, 0, thickness, viewH);

        // BOTTOM wall (inside): [syB-thickness .. syB)
        renderer.FillRect(0, syB - thickness, viewW, thickness);

        // Optional darker edge lines
        renderer.SetDrawColor(70, 70, 70, 255);
        renderer.DrawLine(sxL + thickness, 0, sxL + thickness, viewH);
        renderer.DrawLine(sxR - thickness, 0, sxR - thickness, viewH);
        renderer.DrawLine(0, syT + thickness, viewW, syT + thickness);
        renderer.DrawLine(0, syB - thickness, viewW, syB - thickness);
    }
}
