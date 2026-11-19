#include "CWarehouseMap.h"
#include <queue>

CWarehouseMap::CWarehouseMap()
    : mapId(0),
      version(1),
      width(0),
      height(0),
      status("Черновик"),
      authorId(0),
      title("Новая схема") {}

CWarehouseMap::CWarehouseMap(int id,
                             const std::string &mapTitle,
                             int mapVersion,
                             int gridWidth,
                             int gridHeight,
                             const std::string &mapStatus,
                             int creatorId)
{
    create(id, mapTitle, mapVersion, gridWidth, gridHeight, mapStatus, creatorId);
}

void CWarehouseMap::create(int id,
                           const std::string &mapTitle,
                           int mapVersion,
                           int gridWidth,
                           int gridHeight,
                           const std::string &mapStatus,
                           int creatorId)
{
    mapId = id;
    title = mapTitle;
    version = (mapVersion > 0) ? mapVersion : 1;
    width = (gridWidth > 0) ? gridWidth : 0;
    height = (gridHeight > 0) ? gridHeight : 0;
    status = mapStatus.empty() ? "Черновик" : mapStatus;
    authorId = creatorId;

    cells.assign(height, std::vector<CCell>(width));
    int cellCounter = 1;
    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            cells[y][x].create(cellCounter++, x, y, "passage", true);
        }
    }
}

bool CWarehouseMap::placeObject(int x, int y, const std::string &objectType)
{
    if (status == "Утверждена")
    {
        return false;
    }
    if (x < 0 || x >= width || y < 0 || y >= height)
    {
        return false;
    }
    cells[y][x].setObjectType(objectType);
    return true;
}

bool CWarehouseMap::validateConnectivity(std::vector<std::pair<int, int>> &problemCells) const
{
    problemCells.clear();
    if (width <= 0 || height <= 0)
    {
        return false;
    }

    int startX = -1;
    int startY = -1;
    int totalWalkable = 0;

    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            if (cells[y][x].checkWalkable())
            {
                ++totalWalkable;
                if (startX == -1)
                {
                    startX = x;
                    startY = y;
                }
            }
        }
    }

    if (totalWalkable == 0 || startX == -1)
    {
        return false;
    }

    std::vector<std::vector<bool>> visited(height, std::vector<bool>(width, false));
    std::queue<std::pair<int, int>> q;

    visited[startY][startX] = true;
    q.push({startX, startY});
    int visitedWalkable = 0;

    const int dx[4] = {1, -1, 0, 0};
    const int dy[4] = {0, 0, 1, -1};

    while (!q.empty())
    {
        auto [cx, cy] = q.front();
        q.pop();
        ++visitedWalkable;

        for (int dir = 0; dir < 4; ++dir)
        {
            int nx = cx + dx[dir];
            int ny = cy + dy[dir];
            if (nx >= 0 && nx < width && ny >= 0 && ny < height)
            {
                if (!visited[ny][nx] && cells[ny][nx].checkWalkable())
                {
                    visited[ny][nx] = true;
                    q.push({nx, ny});
                }
            }
        }
    }

    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            if (cells[y][x].checkWalkable() && !visited[y][x])
            {
                problemCells.push_back({x, y});
            }
        }
    }

    return (visitedWalkable == totalWalkable) && problemCells.empty();
}

bool CWarehouseMap::approveMap(std::vector<std::pair<int, int>> &problemCells)
{
    if (validateConnectivity(problemCells))
    {
        status = "Утверждена";
        return true;
    }
    status = "Черновик";
    return false;
}

CWarehouseMap CWarehouseMap::createDraftCopy() const
{
    CWarehouseMap copy(
        0,
        title,
        version + 1,
        width,
        height,
        "Черновик",
        authorId);

    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            copy.cells[y][x].setObjectType(cells[y][x].objectType);
        }
    }

    return copy;
}

int CWarehouseMap::getMapId() const
{
    return mapId;
}

int CWarehouseMap::getVersion() const
{
    return version;
}

int CWarehouseMap::getWidth() const
{
    return width;
}

int CWarehouseMap::getHeight() const
{
    return height;
}

std::string CWarehouseMap::getStatus() const
{
    return status;
}

int CWarehouseMap::getAuthorId() const
{
    return authorId;
}