#pragma once

#include "CRobot.h"
#include "CSimulationReport.h"
#include "CTask.h"
#include "CWarehouseMap.h"
#include "IPathFinder.h"

#include <memory>
#include <string>
#include <vector>

enum class MissionSegmentType
{
    MoveToTask,
    MoveToCharger,
    Recharge
};

struct MissionSegment
{
    MissionSegmentType type = MissionSegmentType::MoveToTask;
    std::vector<GridPoint> path;
    size_t taskIndex = 0;
};

class CSimulationSession
{
public:
    explicit CSimulationSession(std::shared_ptr<IPathFinder> finder = nullptr);

    void setPathFinder(std::shared_ptr<IPathFinder> finder);

    bool initialize(int id,
                    const CWarehouseMap &approvedMap,
                    int startX,
                    int startY,
                    double capacity,
                    double chargePercent,
                    double thresholdPercent,
                    double stepEnergy,
                    const std::vector<CTask> &tasks);

    void reset();
    bool start();
    void pause();
    bool step();

    int getSessionId() const;
    int getMapId() const;
    std::string getStatus() const;
    int getCurrentStep() const;

    const CRobot &getRobot() const;
    const CWarehouseMap &getMap() const;
    const std::vector<CTask> &getTaskQueue() const;
    const std::vector<GridPoint> &getActivePath() const;
    std::string getLastEventMessage() const;

    int getCompletedTasksCount() const;
    int getTotalTasksCount() const;
    double getTotalDistance() const;
    double getTotalBatteryConsumed() const;
    int getRechargeCount() const;
    int getIdleTicks() const;

    CSimulationReport buildReport(int operatorId,
                                  const std::string &operatorName,
                                  const std::string &createdAt,
                                  double totalTimeSec) const;

private:
    bool buildMissionPlan(const CWarehouseMap &approvedMap,
                          int startX,
                          int startY,
                          double capacity,
                          double chargePercent,
                          double thresholdPercent,
                          double stepEnergy,
                          const std::vector<CTask> &tasks,
                          std::vector<MissionSegment> &outPlan,
                          std::string &outError) const;

    void syncActivePathWithCurrentSegment();

    std::shared_ptr<IPathFinder> pathFinder;

    int sessionId;
    int mapId;
    CWarehouseMap map;
    CRobot robot;
    std::string status;
    int currentStep;
    int idleTicks;

    int initialStartX;
    int initialStartY;
    double initialCapacity;
    double initialChargePercent;
    double initialThresholdPercent;
    double initialStepEnergy;
    std::vector<CTask> initialTasks;
    std::vector<MissionSegment> initialPlan;

    std::vector<CTask> taskQueue;
    std::vector<MissionSegment> missionPlan;
    size_t segmentIndex;
    size_t pathCursor;
    std::vector<GridPoint> activePath;
    std::string lastEventMessage;
};