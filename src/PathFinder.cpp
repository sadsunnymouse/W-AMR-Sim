#include "PathFinder.h"

#include <algorithm>
#include <functional>
#include <queue>
#include <vector>

namespace
{
    constexpr double kEps = 1e-6;
    constexpr int kDx[4] = {1, -1, 0, 0};
    constexpr int kDy[4] = {0, 0, 1, -1};

    bool isInside(const CWarehouseMap &map, int x, int y)
    {
        return x >= 0 && x < map.getWidth() && y >= 0 && y < map.getHeight();
    }

    // Единый BFS для поиска кратчайшего пути до любой клетки, удовлетворяющей isGoal
    PathResult runSimpleBfs(const CWarehouseMap &map,
                            int startX,
                            int startY,
                            const std::function<bool(int, int)> &isGoal)
    {
        PathResult result;
        const int w = map.getWidth();
        const int h = map.getHeight();

        if (w <= 0 || h <= 0 || map.cells.empty() ||
            !isInside(map, startX, startY) || !map.cells[startY][startX].checkWalkable())
        {
            return result;
        }

        if (isGoal(startX, startY))
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

        int foundX = -1;
        int foundY = -1;

        while (!q.empty())
        {
            GridPoint curr = q.front();
            q.pop();

            if (isGoal(curr.x, curr.y))
            {
                foundX = curr.x;
                foundY = curr.y;
                break;
            }

            for (int dir = 0; dir < 4; ++dir)
            {
                int nx = curr.x + kDx[dir];
                int ny = curr.y + kDy[dir];
                if (isInside(map, nx, ny) && !visited[ny][nx] && map.cells[ny][nx].checkWalkable())
                {
                    visited[ny][nx] = true;
                    parent[ny][nx] = curr;
                    q.push(GridPoint{nx, ny});
                }
            }
        }

        if (foundX == -1)
        {
            return result;
        }

        GridPoint curr{foundX, foundY};
        while (!(curr.x == -1 && curr.y == -1))
        {
            result.path.push_back(curr);
            if (curr.x == startX && curr.y == startY)
            {
                break;
            }
            curr = parent[curr.y][curr.x];
        }

        std::reverse(result.path.begin(), result.path.end());
        result.found = !result.path.empty() &&
                       result.path.front().x == startX &&
                       result.path.front().y == startY;
        return result;
    }

    struct EnergyNode
    {
        int x = 0;
        int y = 0;
        double charge = 0.0;
        int parentIdx = -1;
        bool rechargeHere = false;
    };
} // namespace

PathResult PathFinder::findShortestPath(const CWarehouseMap &map,
                                        int startX,
                                        int startY,
                                        int goalX,
                                        int goalY) const
{
    if (!isInside(map, goalX, goalY) || !map.cells[goalY][goalX].checkWalkable())
    {
        return {};
    }
    return runSimpleBfs(map, startX, startY, [goalX, goalY](int x, int y)
                        { return x == goalX && y == goalY; });
}

PathResult PathFinder::findPathToNearestCharging(const CWarehouseMap &map,
                                                 int startX,
                                                 int startY) const
{
    return runSimpleBfs(map, startX, startY, [&map](int x, int y)
                        { return map.cells[y][x].objectType == "charging"; });
}

EnergyPathResult PathFinder::findEnergyAwarePath(const CWarehouseMap &map,
                                                 int startX,
                                                 int startY,
                                                 int goalX,
                                                 int goalY,
                                                 double startCharge,
                                                 double capacity,
                                                 double thresholdPercent,
                                                 double stepEnergy,
                                                 bool requireExitToCharger) const
{
    EnergyPathResult result;
    const int w = map.getWidth();
    const int h = map.getHeight();

    if (w <= 0 || h <= 0 || map.cells.empty() || capacity <= 0.0 || stepEnergy < 0.0)
    {
        result.errorReason = "некорректные параметры карты или батареи.";
        return result;
    }

    // 1. Быстрая проверка геометрической достижимости цели
    PathResult geometric = findShortestPath(map, startX, startY, goalX, goalY);
    if (!geometric.found || geometric.path.empty())
    {
        result.errorReason = "путь к целевой ячейке заблокирован.";
        return result;
    }

    const double stepCost = (stepEnergy * 100.0) / capacity;

    // 2. Сколько заряда нужно сохранить в целевой точке, чтобы не оказаться в ловушке
    double requiredChargeAtGoal = thresholdPercent;
    if (requireExitToCharger)
    {
        PathResult exitToCharger = findPathToNearestCharging(map, goalX, goalY);
        if (exitToCharger.found && !exitToCharger.path.empty())
        {
            double exitCost = static_cast<double>(exitToCharger.path.size() - 1) * stepCost;
            if (exitCost > requiredChargeAtGoal)
            {
                requiredChargeAtGoal = exitCost;
            }
        }
    }

    // 3. Плоский BFS по пространству (x, y) с отслеживанием максимального заряда в каждой клетке
    std::vector<std::vector<double>> bestCharge(h, std::vector<double>(w, -1.0));
    std::vector<EnergyNode> nodes;
    std::queue<int> q;

    nodes.push_back({startX, startY, startCharge, -1, false});
    bestCharge[startY][startX] = startCharge;
    q.push(0);

    int goalNodeIdx = -1;
    constexpr size_t kMaxSearchNodes = 250000;

    while (!q.empty())
    {
        int currIdx = q.front();
        q.pop();

        const EnergyNode curr = nodes[currIdx];

        // Проверяем достижение цели с достаточным остатком заряда
        if (curr.x == goalX && curr.y == goalY && curr.charge + kEps >= requiredChargeAtGoal)
        {
            goalNodeIdx = currIdx;
            break;
        }

        // Если мы на зарядной станции и заряд не 100%, пробуем зарядиться здесь
        if (map.cells[curr.y][curr.x].objectType == "charging" &&
            curr.charge < 100.0 - kEps &&
            100.0 > bestCharge[curr.y][curr.x] + kEps &&
            nodes.size() < kMaxSearchNodes)
        {
            bestCharge[curr.y][curr.x] = 100.0;
            int rechargeIdx = static_cast<int>(nodes.size());
            nodes.push_back({curr.x, curr.y, 100.0, currIdx, true});
            q.push(rechargeIdx);
        }

        // Переходы в 4 соседние проходимые клетки
        const double nextCharge = curr.charge - stepCost;
        if (nextCharge + kEps < 0.0)
        {
            continue;
        }

        for (int dir = 0; dir < 4; ++dir)
        {
            int nx = curr.x + kDx[dir];
            int ny = curr.y + kDy[dir];
            if (!isInside(map, nx, ny) || !map.cells[ny][nx].checkWalkable())
            {
                continue;
            }

            if (nextCharge > bestCharge[ny][nx] + kEps && nodes.size() < kMaxSearchNodes)
            {
                bestCharge[ny][nx] = nextCharge;
                int nextIdx = static_cast<int>(nodes.size());
                nodes.push_back({nx, ny, nextCharge, currIdx, false});
                q.push(nextIdx);
            }
        }
    }

    if (goalNodeIdx == -1)
    {
        result.errorReason = "недостаточно заряда для прохождения маршрута даже с учётом подзарядок "
                             "(либо зарядные станции недоступны / расположены слишком далеко).";
        return result;
    }

    // 4. Восстановление маршрута с отметками остановок на зарядку
    for (int idx = goalNodeIdx; idx != -1; idx = nodes[idx].parentIdx)
    {
        result.steps.push_back({nodes[idx].x, nodes[idx].y, nodes[idx].rechargeHere});
    }
    std::reverse(result.steps.begin(), result.steps.end());

    result.found = true;
    result.endCharge = nodes[goalNodeIdx].charge;
    return result;
}