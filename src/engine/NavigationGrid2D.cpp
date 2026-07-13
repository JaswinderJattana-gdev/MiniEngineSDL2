#include "NavigationGrid2D.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <queue>

namespace
{
    struct OpenNode
    {
        int index = -1;
        double fScore = 0.0;
    };

    struct OpenNodeGreater
    {
        bool operator()(const OpenNode& a, const OpenNode& b) const
        {
            return a.fScore > b.fScore;
        }
    };

    double Heuristic(int ax, int ay, int bx, int by)
    {
        // Manhattan distance is suitable for four-direction movement.
        return static_cast<double>(
            std::abs(ax - bx) + std::abs(ay - by)
            );
    }
}

void NavigationGrid2D::Build(
    int worldW,
    int worldH,
    int cellSize,
    const std::vector<SDL_Rect>& obstacles
)
{
    Clear();

    if (worldW <= 0 || worldH <= 0 || cellSize <= 0)
        return;

    worldW_ = worldW;
    worldH_ = worldH;
    cellSize_ = cellSize;

    columns_ = (worldW_ + cellSize_ - 1) / cellSize_;
    rows_ = (worldH_ + cellSize_ - 1) / cellSize_;

    walkable_.assign(
        static_cast<std::size_t>(columns_ * rows_),
        static_cast<unsigned char>(1)
    );

    for (int y = 0; y < rows_; ++y)
    {
        for (int x = 0; x < columns_; ++x)
        {
            if (CellOverlapsObstacle(x, y, obstacles))
            {
                walkable_[CellIndex(x, y)] = 0;
            }
        }
    }
}

void NavigationGrid2D::Clear()
{
    worldW_ = 0;
    worldH_ = 0;
    cellSize_ = 0;
    columns_ = 0;
    rows_ = 0;
    walkable_.clear();
}

bool NavigationGrid2D::IsBuilt() const
{
    return cellSize_ > 0 &&
        columns_ > 0 &&
        rows_ > 0 &&
        !walkable_.empty();
}

bool NavigationGrid2D::IsCellWalkable(int cellX, int cellY) const
{
    if (!IsCellInside(cellX, cellY))
        return false;

    return walkable_[CellIndex(cellX, cellY)] != 0;
}

SDL_Point NavigationGrid2D::WorldToCell(
    const Vec2& worldPosition
) const
{
    if (!IsBuilt())
        return SDL_Point{ -1, -1 };

    int cellX = static_cast<int>(
        std::floor(worldPosition.x / cellSize_)
        );

    int cellY = static_cast<int>(
        std::floor(worldPosition.y / cellSize_)
        );

    cellX = std::clamp(cellX, 0, columns_ - 1);
    cellY = std::clamp(cellY, 0, rows_ - 1);

    return SDL_Point{ cellX, cellY };
}

Vec2 NavigationGrid2D::CellToWorldCenter(
    int cellX,
    int cellY
) const
{
    if (!IsCellInside(cellX, cellY))
        return Vec2{};

    return Vec2{
        cellX * static_cast<double>(cellSize_) +
            cellSize_ * 0.5,

        cellY * static_cast<double>(cellSize_) +
            cellSize_ * 0.5
    };
}

std::vector<Vec2> NavigationGrid2D::FindPath(
    const Vec2& startWorld,
    const Vec2& goalWorld
) const
{
    std::vector<Vec2> result;

    if (!IsBuilt())
        return result;

    const SDL_Point startCell = WorldToCell(startWorld);
    const SDL_Point goalCell = WorldToCell(goalWorld);

    if (!IsCellWalkable(startCell.x, startCell.y) ||
        !IsCellWalkable(goalCell.x, goalCell.y))
    {
        return result;
    }

    const int startIndex = CellIndex(startCell.x, startCell.y);
    const int goalIndex = CellIndex(goalCell.x, goalCell.y);

    if (startIndex == goalIndex)
    {
        result.push_back(
            CellToWorldCenter(startCell.x, startCell.y)
        );
        return result;
    }

    const int nodeCount = columns_ * rows_;
    const double infinity =
        std::numeric_limits<double>::infinity();

    std::vector<double> gScore(
        static_cast<std::size_t>(nodeCount),
        infinity
    );

    std::vector<int> parent(
        static_cast<std::size_t>(nodeCount),
        -1
    );

    std::vector<unsigned char> closed(
        static_cast<std::size_t>(nodeCount),
        static_cast<unsigned char>(0)
    );

    std::priority_queue<
        OpenNode,
        std::vector<OpenNode>,
        OpenNodeGreater
    > open;

    gScore[startIndex] = 0.0;

    open.push(OpenNode{
        startIndex,
        Heuristic(
            startCell.x,
            startCell.y,
            goalCell.x,
            goalCell.y
        )
        });

    constexpr int directions[4][2] =
    {
        { 1, 0 },
        { -1, 0 },
        { 0, 1 },
        { 0, -1 }
    };

    bool found = false;

    while (!open.empty())
    {
        const OpenNode currentNode = open.top();
        open.pop();

        const int currentIndex = currentNode.index;

        if (closed[currentIndex] != 0)
            continue;

        closed[currentIndex] = 1;

        if (currentIndex == goalIndex)
        {
            found = true;
            break;
        }

        const SDL_Point currentCell =
            CellFromIndex(currentIndex);

        for (const auto& direction : directions)
        {
            const int nextX =
                currentCell.x + direction[0];

            const int nextY =
                currentCell.y + direction[1];

            if (!IsCellWalkable(nextX, nextY))
                continue;

            const int nextIndex = CellIndex(nextX, nextY);

            if (closed[nextIndex] != 0)
                continue;

            const double tentativeG =
                gScore[currentIndex] + 1.0;

            if (tentativeG >= gScore[nextIndex])
                continue;

            parent[nextIndex] = currentIndex;
            gScore[nextIndex] = tentativeG;

            const double fScore =
                tentativeG +
                Heuristic(
                    nextX,
                    nextY,
                    goalCell.x,
                    goalCell.y
                );

            open.push(OpenNode{ nextIndex, fScore });
        }
    }

    if (!found)
        return result;

    std::vector<int> reversedIndices;

    int current = goalIndex;
    while (current != -1)
    {
        reversedIndices.push_back(current);

        if (current == startIndex)
            break;

        current = parent[current];
    }

    if (reversedIndices.empty() ||
        reversedIndices.back() != startIndex)
    {
        return {};
    }

    std::reverse(
        reversedIndices.begin(),
        reversedIndices.end()
    );

    result.reserve(reversedIndices.size());

    for (const int index : reversedIndices)
    {
        const SDL_Point cell = CellFromIndex(index);

        result.push_back(
            CellToWorldCenter(cell.x, cell.y)
        );
    }

    return result;
}

bool NavigationGrid2D::IsCellInside(
    int cellX,
    int cellY
) const
{
    return cellX >= 0 &&
        cellY >= 0 &&
        cellX < columns_ &&
        cellY < rows_;
}

int NavigationGrid2D::CellIndex(
    int cellX,
    int cellY
) const
{
    return cellY * columns_ + cellX;
}

SDL_Point NavigationGrid2D::CellFromIndex(
    int index
) const
{
    return SDL_Point{
        index % columns_,
        index / columns_
    };
}

bool NavigationGrid2D::CellOverlapsObstacle(
    int cellX,
    int cellY,
    const std::vector<SDL_Rect>& obstacles
) const
{
    SDL_Rect cellRect{
        cellX * cellSize_,
        cellY * cellSize_,
        cellSize_,
        cellSize_
    };

    // Clamp cells on the far world edges to the actual world size.
    if (cellRect.x + cellRect.w > worldW_)
        cellRect.w = worldW_ - cellRect.x;

    if (cellRect.y + cellRect.h > worldH_)
        cellRect.h = worldH_ - cellRect.y;

    for (const SDL_Rect& obstacle : obstacles)
    {
        if (SDL_HasIntersection(&cellRect, &obstacle))
            return true;
    }

    return false;
}