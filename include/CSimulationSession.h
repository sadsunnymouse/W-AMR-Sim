#ifndef CSIMULATIONSESSION_H
#define CSIMULATIONSESSION_H

#include "CRobot.h"
#include "CTask.h"
#include "CWarehouseMap.h"
#include "PathFinder.h"
#include <string>
#include <vector>

class CSimulationSession
{
private:
    int sessionId;
    int mapId;
    std::string status; // "Ожидание", "Выполняется", "На паузе", "Завершена", "Аварийная остановка"
    int currentStep;

    CWarehouseMap map;
    CRobot robot;
    std::vector<CTask> taskQueue;
    size_t activeTaskIndex;

    // Параметры исходного сценария для корректного сброса
    int initialStartX;
    int initialStartY;
    double initialCapacity;
    double initialChargePercent;
    double initialThresholdPercent;
    double initialStepEnergy;
    std::vector<CTask> initialTasks;

    std::vector<GridPoint> activePath;
    size_t pathCursor;
    bool headingToCharger;
    std::string lastEventMessage;

    bool planRouteToCurrentTask();
    bool planRouteToCharger();

public:
    CSimulationSession();

    bool initialize(int id,
                    const CWarehouseMap &approvedMap,
                    int startX,
                    int startY,
                    double capacity,
                    double chargePercent,
                    double thresholdPercent,
                    double stepEnergy,
                    const std::vector<CTask> &tasks);

    bool start();
    void pause();
    bool step();
    void reset();

    int getSessionId() const;
    int getMapId() const;
    std::string getStatus() const;
    int getCurrentStep() const;
    const CRobot &getRobot() const;
    const CWarehouseMap &getMap() const;
    const std::vector<CTask> &getTaskQueue() const;
    const std::vector<GridPoint> &getActivePath() const;
    std::string getLastEventMessage() const;
};

#endif