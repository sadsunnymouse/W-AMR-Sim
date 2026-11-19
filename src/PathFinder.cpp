#include "PathFinder.h"
#include <algorithm>
#include <queue>
#include <vector>

namespace
{

    bool isInside(const CWarehouseMap &map, int x, int y)
    {
        return x >= 0 && x < map.getWidth() && y >= 0 && y < map.getHeight();
    }

    std::vector<GridPoint> reconstructPath(
        const std::vector<std::vector<GridPoint>> &parent,
        int startX,
        int startY,
        int goalX,
        int goalY)
    {
        std::vector<GridPoint> path;
        GridPoint curr{goalX, goalY};

        while (!(curr.x == -1 && curr.y == -1))
        {
            path.push_back(curr);
            if (curr.x == startX && curr.y == startY)
            {
                break;
            }
            curr = parent[curr.y][curr.x];
        }

        if (path.empty() || path.back().x != startX || path.back().y != startY)
        {
            return {};
        }

        std::reverse(path.begin(), path.end());
        return path;
    }

} // namespace

PathResult PathFinder::findShortestPath(const CWarehouseMap &map,
                                        int startX,
                                        int startY,
                                        int goalX,
                                        int goalY)
{
    PathResult result;
    int w = map.getWidth();
    int h = map.getHeight();

    if (w <= 0 || h <= 0 || map.cells.empty())
    {
        return result;
    }
    if (!isInside(map, startX, startY) || !isInside(map, goalX, goalY))
    {
        return result;
    }
    if (!map.cells[startY][startX].checkWalkable() || !map.cells[goalY][goalX].checkWalkable())
    {
        return result;
    }

    if (startX == goalX && startY == goalY)
    {
        result.found = true;
        result.path = {GridPoint{startX, startY}};
        return result;
    }

    std::vector<std::vector<bool>> visited(h, std::vector<bool>(w, false));
    std::vector<std::vector<GridPoint>> parent(h, std::vector<GridPoint>(w, GridPoint{-1, -1}));
    std::queue<GridPoint> q;

    visited[startY][startX] = true;
    q.push(GridPoint{startX, startY});

    const int dx[4] = {1, -1, 0, 0};
    const int dy[4] = {0, 0, 1, -1};

    bool reached = false;
    while (!q.empty())
    {
        GridPoint curr = q.front();
        q.pop();

        if (curr.x == goalX && curr.y == goalY)
        {
            reached = true;
            break;
        }

        for (int dir = 0; dir < 4; ++dir)
        {
            int nx = curr.x + dx[dir];
            int ny = curr.y + dy[dir];

            if (isInside(map, nx, ny) && !visited[ny][nx] && map.cells[ny][nx].checkWalkable())
            {
                visited[ny][nx] = true;
                parent[ny][nx] = curr;
                q.push(GridPoint{nx, ny});
            }
        }
    }

    if (reached)
    {
        result.path = reconstructPath(parent, startX, startY, goalX, goalY);
        result.found = !result.path.empty();
    }
    return result;
}

PathResult PathFinder::findPathToNearestCharging(const CWarehouseMap &map,
                                                 int startX,
                                                 int startY)
{
    PathResult result;
    int w = map.getWidth();
    int h = map.getHeight();

    if (w <= 0 || h <= 0 || map.cells.empty())
    {
        return result;
    }
    if (!isInside(map, startX, startY) || !map.cells[startY][startX].checkWalkable())
    {
        return result;
    }

    if (map.cells[startY][startX].objectType == "charging")
    {
        result.found = true;
        result.path = {GridPoint{startX, startY}};
        return result;
    }

    std::vector<std::vector<bool>> visited(h, std::vector<bool>(w, false));
    std::vector<std::vector<GridPoint>> parent(h, std::vector<GridPoint>(w, GridPoint{-1, -1}));
    std::queue<GridPoint> q;

    visited[startY][startX] = true;
    q.push(GridPoint{startX, startY});

    const int dx[4] = {1, -1, 0, 0};
    const int dy[4] = {0, 0, 1, -1};

    int foundX = -1;
    int foundY = -1;

    while (!q.empty())
    {
        GridPoint curr = q.front();
        q.pop();

        if (map.cells[curr.y][curr.x].objectType == "charging")
        {
            foundX = curr.x;
            foundY = curr.y;
            break;
        }

        for (int dir = 0; dir < 4; ++dir)
        {
            int nx = curr.x + dx[dir];
            int ny = curr.y + dy[dir];

            if (isInside(map, nx, ny) && !visited[ny][nx] && map.cells[ny][nx].checkWalkable())
            {
                visited[ny][nx] = true;
                parent[ny][nx] = curr;
                q.push(GridPoint{nx, ny});
            }
        }
    }

    if (foundX != -1 && foundY != -1)
    {
        result.path = reconstructPath(parent, startX, startY, foundX, foundY);
        result.found = !result.path.empty();
    }
    return result;
}