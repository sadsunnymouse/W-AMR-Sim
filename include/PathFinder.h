#ifndef PATHFINDER_H
#define PATHFINDER_H

#include "CWarehouseMap.h"
#include <utility>
#include <vector>

struct GridPoint
{
    int x = 0;
    int y = 0;
};

struct PathResult
{
    bool found = false;
    std::vector<GridPoint> path; // Включает стартовую и конечную точки при found == true
    int stepCount() const
    {
        return path.size() > 1 ? static_cast<int>(path.size()) - 1 : 0;
    }
};

class PathFinder
{
public:
    // Кратчайший путь между двумя ячейками на карте
    static PathResult findShortestPath(const CWarehouseMap &map,
                                       int startX,
                                       int startY,
                                       int goalX,
                                       int goalY);

    // Кратчайший путь от текущей позиции до ближайшей проходимой зарядной станции ("charging")
    static PathResult findPathToNearestCharging(const CWarehouseMap &map,
                                                int startX,
                                                int startY);
};

#endif