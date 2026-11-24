#include "CSimulationSession.h"
#include "PathFinder.h"

#include <algorithm>

CSimulationSession::CSimulationSession(std::shared_ptr<IPathFinder> finder)
    : pathFinder(finder ? std::move(finder) : std::make_shared<PathFinder>()),
      sessionId(1),
      mapId(0),
      status("Ожидание"),
      currentStep(0),
      idleTicks(0),
      initialStartX(0),
      initialStartY(0),
      initialCapacity(100.0),
      initialChargePercent(100.0),
      initialThresholdPercent(20.0),
      initialStepEnergy(1.0),
      segmentIndex(0),
      pathCursor(0),
      lastEventMessage("Сеанс не инициализирован") {}

void CSimulationSession::setPathFinder(std::shared_ptr<IPathFinder> finder)
{
    if (finder)
    {
        pathFinder = std::move(finder);
    }
}

bool CSimulationSession::buildMissionPlan(const CWarehouseMap &approvedMap,
                                          int startX,
                                          int startY,
                                          double capacity,
                                          double chargePercent,
                                          double thresholdPercent,
                                          double stepEnergy,
                                          const std::vector<CTask> &tasks,
                                          std::vector<MissionSegment> &outPlan,
                                          std::string &outError) const
{
    outPlan.clear();

    int currX = startX;
    int currY = startY;
    double currCharge = chargePercent;

    for (size_t i = 0; i < tasks.size(); ++i)
    {
        const CTask &task = tasks[i];
        const int tx = task.getTargetX();
        const int ty = task.getTargetY();
        const bool hasNextTask = (i + 1 < tasks.size());

        EnergyPathResult leg = pathFinder->findEnergyAwarePath(
            approvedMap,
            currX,
            currY,
            tx,
            ty,
            currCharge,
            capacity,
            thresholdPercent,
            stepEnergy,
            hasNextTask);

        if (!leg.found || leg.steps.empty())
        {
            outError = "Маршрут отклонён перед заданием №" +
                       std::to_string(task.getTaskNumber()) + ": " + leg.errorReason +
                       " Измените параметры робота или архитектуру помещения.";
            return false;
        }

        // Разбиваем найденный путь на чистые сегменты: движение к зарядке -> зарядка -> движение к цели
        std::vector<GridPoint> currentPath;
        for (const auto &st : leg.steps)
        {
            if (st.rechargeHere)
            {
                if (currentPath.size() > 1)
                {
                    outPlan.push_back({MissionSegmentType::MoveToCharger, currentPath, i});
                }
                outPlan.push_back({MissionSegmentType::Recharge, {}, i});
                currentPath = {GridPoint{st.x, st.y}};
            }
            else
            {
                currentPath.push_back(GridPoint{st.x, st.y});
            }
        }

        outPlan.push_back({MissionSegmentType::MoveToTask, currentPath, i});

        currX = tx;
        currY = ty;
        currCharge = leg.endCharge;
    }

    return true;
}

bool CSimulationSession::initialize(int id,
                                    const CWarehouseMap &approvedMap,
                                    int startX,
                                    int startY,
                                    double capacity,
                                    double chargePercent,
                                    double thresholdPercent,
                                    double stepEnergy,
                                    const std::vector<CTask> &tasks)
{
    if (approvedMap.getStatus() != "Утверждена")
    {
        lastEventMessage = "Ошибка: допускаются только карты со статусом «Утверждена».";
        return false;
    }
    if (startX < 0 || startX >= approvedMap.getWidth() ||
        startY < 0 || startY >= approvedMap.getHeight())
    {
        lastEventMessage = "Ошибка: стартовые координаты вне границ карты.";
        return false;
    }
    if (!approvedMap.cells[startY][startX].checkWalkable())
    {
        lastEventMessage = "Ошибка: стартовая ячейка непроходима.";
        return false;
    }
    if (tasks.empty())
    {
        lastEventMessage = "Ошибка: очередь маршрутных заданий пуста.";
        return false;
    }

    for (const auto &task : tasks)
    {
        int tx = task.getTargetX();
        int ty = task.getTargetY();
        if (tx < 0 || tx >= approvedMap.getWidth() ||
            ty < 0 || ty >= approvedMap.getHeight() ||
            !approvedMap.cells[ty][tx].checkWalkable())
        {
            lastEventMessage = "Маршрут отклонён: целевая ячейка задания №" +
                               std::to_string(task.getTaskNumber()) +
                               " вне границ карты или непроходима.";
            return false;
        }
    }

    CRobot testRobot;
    if (!testRobot.setBatteryParams(capacity, chargePercent, thresholdPercent, stepEnergy))
    {
        lastEventMessage = "Ошибка: некорректные параметры батареи.";
        return false;
    }

    std::vector<MissionSegment> builtPlan;
    std::string planError;
    if (!buildMissionPlan(approvedMap,
                          startX,
                          startY,
                          capacity,
                          chargePercent,
                          thresholdPercent,
                          stepEnergy,
                          tasks,
                          builtPlan,
                          planError))
    {
        lastEventMessage = planError;
        return false;
    }

    sessionId = id;
    mapId = approvedMap.getMapId();
    map = approvedMap;

    initialStartX = startX;
    initialStartY = startY;
    initialCapacity = capacity;
    initialChargePercent = chargePercent;
    initialThresholdPercent = thresholdPercent;
    initialStepEnergy = stepEnergy;
    initialTasks = tasks;
    initialPlan = std::move(builtPlan);

    reset();
    return true;
}

void CSimulationSession::syncActivePathWithCurrentSegment()
{
    if (segmentIndex < missionPlan.size() &&
        missionPlan[segmentIndex].type != MissionSegmentType::Recharge)
    {
        activePath = missionPlan[segmentIndex].path;
    }
    else
    {
        activePath.clear();
    }
}

void CSimulationSession::reset()
{
    currentStep = 0;
    idleTicks = 0;
    segmentIndex = 0;
    pathCursor = 0;

    taskQueue = initialTasks;
    for (auto &t : taskQueue)
    {
        t.isCompleted = false;
    }
    missionPlan = initialPlan;
    syncActivePathWithCurrentSegment();

    robot.create(1, initialStartX, initialStartY);
    robot.setBatteryParams(initialCapacity,
                           initialChargePercent,
                           initialThresholdPercent,
                           initialStepEnergy);
    robot.setState("Ожидание");
    status = "Ожидание";
    lastEventMessage = "Сеанс сброшен к начальному состоянию сценария.";
}

bool CSimulationSession::start()
{
    if (status == "Завершена")
    {
        reset();
    }
    if (status == "Аварийная остановка")
    {
        lastEventMessage = "Требуется сброс (Reset) после аварийной остановки.";
        return false;
    }

    status = "Выполняется";
    if (robot.fsmState == "Ожидание" && segmentIndex < missionPlan.size())
    {
        const auto type = missionPlan[segmentIndex].type;
        if (type == MissionSegmentType::MoveToCharger)
        {
            robot.setState("Движение к зарядной станции");
        }
        else if (type == MissionSegmentType::Recharge)
        {
            robot.setState("Зарядка");
        }
        else
        {
            robot.setState("Движение к цели");
        }
    }

    lastEventMessage = "Симуляция запущена.";
    return true;
}

void CSimulationSession::pause()
{
    if (status == "Выполняется")
    {
        status = "На паузе";
        lastEventMessage = "Симуляция поставлена на паузу.";
    }
}

bool CSimulationSession::step()
{
    if (status == "Завершена" || status == "Аварийная остановка")
    {
        return false;
    }
    if (status == "Ожидание")
    {
        status = "Выполняется";
    }
    if (segmentIndex >= missionPlan.size())
    {
        robot.setState("Миссия завершена");
        status = "Завершена";
        activePath.clear();
        lastEventMessage = "Все маршрутные задания успешно завершены.";
        return false;
    }

    const MissionSegment &seg = missionPlan[segmentIndex];

    // 1. Такт подзарядки на станции
    if (seg.type == MissionSegmentType::Recharge)
    {
        robot.setState("Зарядка");
        robot.recharge();
        ++currentStep;
        ++segmentIndex;
        pathCursor = 0;
        syncActivePathWithCurrentSegment();
        lastEventMessage = "Батарея заряжена до 100%. Возобновление движения.";
        return true;
    }

    // 2. Движение к цели или к зарядной станции
    const bool toCharger = (seg.type == MissionSegmentType::MoveToCharger);
    robot.setState(toCharger ? "Движение к зарядной станции" : "Движение к цели");
    activePath = seg.path;

    // Если сегмент требует шага — проверяем заряд и перемещаемся на 1 клетку
    if (pathCursor + 1 < seg.path.size())
    {
        const double stepCost = (robot.getEnergyPerStep() * 100.0) / robot.getBatteryCapacity();
        if (robot.getCurrentCharge() + 1e-6 < stepCost)
        {
            robot.setState("Аварийная остановка");
            status = "Аварийная остановка";
            ++idleTicks;
            lastEventMessage = "Аварийная остановка: недостаточно заряда для следующего шага.";
            return false;
        }

        const GridPoint nextPt = seg.path[pathCursor + 1];
        robot.moveStep(nextPt.x, nextPt.y);
        robot.consumeEnergy();
        ++pathCursor;
        ++currentStep;

        lastEventMessage = "Шаг (" + std::to_string(nextPt.x) + ", " + std::to_string(nextPt.y) +
                           ") [" + robot.fsmState + "].";
    }
    else
    {
        // Цель уже совпадает с текущей позицией робота (0 шагов)
        ++currentStep;
    }

    // 3. Проверка завершения текущего сегмента
    if (pathCursor + 1 >= seg.path.size())
    {
        if (toCharger)
        {
            robot.setState("Зарядка");
            lastEventMessage = "Робот прибыл на зарядную станцию (начата зарядка).";
        }
        else if (seg.taskIndex < taskQueue.size())
        {
            taskQueue[seg.taskIndex].markCompleted();
            lastEventMessage = "Выполнено задание №" +
                               std::to_string(taskQueue[seg.taskIndex].getTaskNumber()) + ".";
        }

        ++segmentIndex;
        pathCursor = 0;

        if (segmentIndex >= missionPlan.size())
        {
            robot.setState("Миссия завершена");
            status = "Завершена";
            activePath.clear();
            lastEventMessage = "Все маршрутные задания успешно завершены.";
        }
        else
        {
            syncActivePathWithCurrentSegment();
        }
    }

    return true;
}

int CSimulationSession::getSessionId() const { return sessionId; }
int CSimulationSession::getMapId() const { return mapId; }
std::string CSimulationSession::getStatus() const { return status; }
int CSimulationSession::getCurrentStep() const { return currentStep; }
const CRobot &CSimulationSession::getRobot() const { return robot; }
const CWarehouseMap &CSimulationSession::getMap() const { return map; }
const std::vector<CTask> &CSimulationSession::getTaskQueue() const { return taskQueue; }
const std::vector<GridPoint> &CSimulationSession::getActivePath() const { return activePath; }
std::string CSimulationSession::getLastEventMessage() const { return lastEventMessage; }

int CSimulationSession::getCompletedTasksCount() const
{
    int count = 0;
    for (const auto &t : taskQueue)
    {
        if (t.isCompleted)
        {
            ++count;
        }
    }
    return count;
}

int CSimulationSession::getTotalTasksCount() const
{
    return static_cast<int>(taskQueue.size());
}

double CSimulationSession::getTotalDistance() const
{
    return static_cast<double>(robot.totalSteps);
}

double CSimulationSession::getTotalBatteryConsumed() const
{
    return robot.getTotalBatteryConsumed();
}

int CSimulationSession::getRechargeCount() const
{
    return robot.getRechargeCount();
}

int CSimulationSession::getIdleTicks() const
{
    return idleTicks;
}

CSimulationReport CSimulationSession::buildReport(int operatorId,
                                                  const std::string &operatorName,
                                                  const std::string &createdAt,
                                                  double totalTimeSec) const
{
    CSimulationReport report;
    report.setContext(mapId,
                      map.title,
                      operatorId,
                      operatorName,
                      createdAt,
                      status);
    report.setMetrics(totalTimeSec,
                      currentStep,
                      getCompletedTasksCount(),
                      getTotalTasksCount(),
                      getTotalDistance(),
                      getTotalBatteryConsumed(),
                      getRechargeCount(),
                      getIdleTicks());
    return report;
}