#ifndef PATHFINDER_H
#define PATHFINDER_H

#include "IPathFinder.h"

class PathFinder : public IPathFinder
{
public:
    PathResult findShortestPath(const CWarehouseMap &map,
                                int startX,
                                int startY,
                                int goalX,
                                int goalY) const override;

    PathResult findPathToNearestCharging(const CWarehouseMap &map,
                                         int startX,
                                         int startY) const override;

    EnergyPathResult findEnergyAwarePath(const CWarehouseMap &map,
                                         int startX,
                                         int startY,
                                         int goalX,
                                         int goalY,
                                         double startCharge,
                                         double capacity,
                                         double thresholdPercent,
                                         double stepEnergy,
                                         bool requireExitToCharger) const override;
};

#endif