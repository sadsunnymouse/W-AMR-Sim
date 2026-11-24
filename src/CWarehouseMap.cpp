#include "CWarehouseMap.h"
#include <queue>
#include <fstream>
#include <sstream>

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
    int chargingStations = 0;

    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            if (cells[y][x].objectType == "charging")
            {
                ++chargingStations;
            }
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

    if (totalWalkable == 0 || startX == -1 || chargingStations == 0)
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

bool CWarehouseMap::exportToFile(const std::string &filePath) const
{
    if (width <= 0 || height <= 0 || cells.empty())
    {
        return false;
    }

    std::ofstream out(filePath, std::ios::out | std::ios::trunc);
    if (!out.is_open())
    {
        return false;
    }

    out << "WAMR_MAP_V1\n";
    out << "id=" << mapId << "\n";
    out << "title=" << title << "\n";
    out << "version=" << version << "\n";
    out << "width=" << width << "\n";
    out << "height=" << height << "\n";
    out << "status=" << status << "\n";
    out << "author_id=" << authorId << "\n";
    out << "GRID_BEGIN\n";

    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            if (x > 0)
            {
                out << ";";
            }
            out << cells[y][x].objectType;
        }
        out << "\n";
    }

    out << "GRID_END\n";
    return out.good();
}

bool CWarehouseMap::importFromFile(const std::string &filePath)
{
    std::ifstream in(filePath, std::ios::in);
    if (!in.is_open())
    {
        return false;
    }

    std::string headerLine;
    if (!std::getline(in, headerLine))
    {
        return false;
    }
    if (!headerLine.empty() && headerLine.back() == '\r')
    {
        headerLine.pop_back();
    }
    if (headerLine != "WAMR_MAP_V1")
    {
        return false;
    }

    int parsedId = 0;
    std::string parsedTitle = "Импортированная схема";
    int parsedVersion = 1;
    int parsedWidth = 0;
    int parsedHeight = 0;
    std::string parsedStatus = "Черновик";
    int parsedAuthorId = 0;

    std::string line;
    bool inGrid = false;
    std::vector<std::vector<std::string>> parsedRows;

    while (std::getline(in, line))
    {
        if (!line.empty() && line.back() == '\r')
        {
            line.pop_back();
        }
        if (line.empty())
        {
            continue;
        }

        if (line == "GRID_BEGIN")
        {
            inGrid = true;
            continue;
        }
        if (line == "GRID_END")
        {
            inGrid = false;
            break;
        }

        if (!inGrid)
        {
            auto eqPos = line.find('=');
            if (eqPos == std::string::npos)
            {
                continue;
            }
            std::string key = line.substr(0, eqPos);
            std::string value = line.substr(eqPos + 1);

            try
            {
                if (key == "id")
                {
                    parsedId = std::stoi(value);
                }
                else if (key == "title")
                {
                    parsedTitle = value;
                }
                else if (key == "version")
                {
                    parsedVersion = std::stoi(value);
                }
                else if (key == "width")
                {
                    parsedWidth = std::stoi(value);
                }
                else if (key == "height")
                {
                    parsedHeight = std::stoi(value);
                }
                else if (key == "status")
                {
                    parsedStatus = value;
                }
                else if (key == "author_id")
                {
                    parsedAuthorId = std::stoi(value);
                }
            }
            catch (...)
            {
                return false;
            }
        }
        else
        {
            std::vector<std::string> row;
            std::stringstream ss(line);
            std::string cellType;
            while (std::getline(ss, cellType, ';'))
            {
                row.push_back(cellType);
            }
            parsedRows.push_back(row);
        }
    }

    constexpr int kMaxGridSize = 200;
    if (parsedWidth <= 0 || parsedHeight <= 0 ||
        parsedWidth > kMaxGridSize || parsedHeight > kMaxGridSize)
    {
        return false;
    }
    if (static_cast<int>(parsedRows.size()) != parsedHeight)
    {
        return false;
    }
    for (const auto &row : parsedRows)
    {
        if (static_cast<int>(row.size()) != parsedWidth)
        {
            return false;
        }
    }

    if (parsedStatus != "Черновик" && parsedStatus != "Утверждена" && parsedStatus != "В архиве")
    {
        parsedStatus = "Черновик";
    }

    create(parsedId, parsedTitle, parsedVersion, parsedWidth, parsedHeight, parsedStatus, parsedAuthorId);
    for (int y = 0; y < parsedHeight; ++y)
    {
        for (int x = 0; x < parsedWidth; ++x)
        {
            cells[y][x].setObjectType(parsedRows[y][x]);
        }
    }

    return true;
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