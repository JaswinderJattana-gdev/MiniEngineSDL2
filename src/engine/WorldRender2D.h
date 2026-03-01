#pragma once
#include "Renderer.h"
#include "Camera2D.h"

// Small rendering helpers for world-space 2D scenes.
// Only draws: scrolling grid + thick world boundary frame.
namespace WorldRender2D
{
    // Draw a smooth-scrolling grid that moves with camera.
    // cell: minor grid spacing in pixels (world units)
    // majorEvery: every N minor cells, draw a major line
    void DrawGrid(Renderer& renderer, const Camera2D& camera, int viewW, int viewH, int cell, int majorEvery);

    // Draw thick boundary frame inside world edges ("walls").
    void DrawWorldBoundsFrame(Renderer& renderer, const Camera2D& camera, int viewW, int viewH, int worldW, int worldH, int thickness);
}
