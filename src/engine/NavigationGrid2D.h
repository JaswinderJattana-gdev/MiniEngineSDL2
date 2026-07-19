#pragma once

#include <SDL.h>
#include <vector>

#include "math/Vec2.h"
#include "Renderer.h"
#include "Camera2D.h"

class NavigationGrid2D
{
public:
    void Build(
        int worldW,
        int worldH,
        int cellSize,
        const std::vector<SDL_Rect>& obstacles
    );

    void Clear();

    bool IsBuilt() const;
    bool IsCellWalkable(int cellX, int cellY) const;

    SDL_Point WorldToCell(const Vec2& worldPosition) const;
    Vec2 CellToWorldCenter(int cellX, int cellY) const;

    // Returns world-space points from the start cell to the goal cell.
    // Returns an empty vector if no valid path exists.
    std::vector<Vec2> FindPath(
        const Vec2& startWorld,
        const Vec2& goalWorld
    ) const;

    int CellSize() const { return cellSize_; }
    int Columns() const { return columns_; }
    int Rows() const { return rows_; }

    void DebugRender(
        Renderer& renderer,
        const Camera2D& camera
    ) const;
private:
    bool IsCellInside(int cellX, int cellY) const;
    int CellIndex(int cellX, int cellY) const;
    SDL_Point CellFromIndex(int index) const;

    bool CellOverlapsObstacle(
        int cellX,
        int cellY,
        const std::vector<SDL_Rect>& obstacles
    ) const;
private:
    int worldW_ = 0;
    int worldH_ = 0;
    int cellSize_ = 0;

    int columns_ = 0;
    int rows_ = 0;

    // 1 = walkable, 0 = blocked
    std::vector<unsigned char> walkable_;
};