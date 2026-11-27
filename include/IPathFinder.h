#ifndef IPATHFINDER_H
#define IPATHFINDER_H

#include "CWarehouseMap.h"
#include <string>
#include <vector>

struct GridPoint
{
    int x = 0;
    int y = 0;
};

struct PathResult
{
    bool found = false;
    std::vector<GridPoint> path;
};

struct EnergyPathStep
{
    int x = 0;
    int y = 0;
    bool rechargeHere = false;
};

struct EnergyPathResult
{
    bool found = false;
    double endCharge = 0.0;
    std::vector<EnergyPathStep> steps;
    std::string errorReason;
};

class IPathFinder
{
public:
    virtual ~IPathFinder() = default;

    virtual PathResult findShortestPath(const CWarehouseMap &map,
                                        int startX,
                                        int startY,
                                        int goalX,
                                        int goalY) const = 0;

    virtual PathResult findPathToNearestCharging(const CWarehouseMap &map,
                                                 int startX,
                                                 int startY) const = 0;

    virtual EnergyPathResult findEnergyAwarePath(const CWarehouseMap &map,
                                                 int startX,
                                                 int startY,
                                                 int goalX,
                                                 int goalY,
                                                 double startCharge,
                                                 double capacity,
                                                 double thresholdPercent,
                                                 double stepEnergy,
                                                 bool requireExitToCharger) const = 0;
};

#endif