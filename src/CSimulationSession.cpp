#include "CSimulationSession.h"

CSimulationSession::CSimulationSession()
    : sessionId(1),
      mapId(0),
      status("Ожидание"),
      currentStep(0),
      activeTaskIndex(0),
      initialStartX(0),
      initialStartY(0),
      initialCapacity(100.0),
      initialChargePercent(100.0),
      initialThresholdPercent(20.0),
      initialStepEnergy(1.0),
      pathCursor(0),
      headingToCharger(false),
      lastEventMessage("Сеанс не инициализирован") {}

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

    CRobot testRobot;
    if (!testRobot.setBatteryParams(capacity, chargePercent, thresholdPercent, stepEnergy))
    {
        lastEventMessage = "Ошибка: некорректные параметры батареи.";
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

    reset();
    return true;
}

void CSimulationSession::reset()
{
    currentStep = 0;
    activeTaskIndex = 0;
    pathCursor = 0;
    headingToCharger = false;
    activePath.clear();
    taskQueue = initialTasks;
    for (auto &t : taskQueue)
    {
        t.isCompleted = false;
    }

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
    if (robot.fsmState == "Ожидание")
    {
        robot.setState("Движение к цели");
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

bool CSimulationSession::planRouteToCurrentTask()
{
    while (activeTaskIndex < taskQueue.size() && taskQueue[activeTaskIndex].isCompleted)
    {
        ++activeTaskIndex;
    }
    if (activeTaskIndex >= taskQueue.size())
    {
        activePath.clear();
        pathCursor = 0;
        robot.setState("Миссия завершена");
        status = "Завершена";
        lastEventMessage = "Все маршрутные задания выполнены.";
        return false;
    }

    const CTask &target = taskQueue[activeTaskIndex];
    PathResult res = PathFinder::findShortestPath(
        map,
        robot.getCurrentX(),
        robot.getCurrentY(),
        target.getTargetX(),
        target.getTargetY());

    if (!res.found || res.path.empty())
    {
        activePath.clear();
        pathCursor = 0;
        robot.setState("Аварийная остановка");
        status = "Аварийная остановка";
        lastEventMessage = "Аварийная остановка: путь до целевой ячейки заблокирован.";
        return false;
    }

    activePath = res.path;
    pathCursor = 0;
    headingToCharger = false;
    robot.setState("Движение к цели");
    return true;
}

bool CSimulationSession::planRouteToCharger()
{
    PathResult chargerPath = PathFinder::findPathToNearestCharging(
        map,
        robot.getCurrentX(),
        robot.getCurrentY());

    if (!chargerPath.found || chargerPath.path.empty())
    {
        activePath.clear();
        pathCursor = 0;
        robot.setState("Аварийная остановка");
        status = "Аварийная остановка";
        lastEventMessage = "Аварийная остановка: зарядная станция недоступна.";
        return false;
    }

    activePath = chargerPath.path;
    pathCursor = 0;
    headingToCharger = true;
    robot.setState("Движение к зарядной станции");
    lastEventMessage = "Заряд ниже порога или недостаточен: проложен маршрут к зарядной станции.";
    return true;
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

    // Если робот уже стоит на зарядной станции и заряжается
    if (robot.fsmState == "Зарядка")
    {
        robot.recharge();
        ++currentStep;
        lastEventMessage = "Батарея заряжена до 100%. Возобновление маршрута к цели.";
        if (!planRouteToCurrentTask())
        {
            return false;
        }
    }

    // Проверяем необходимость возврата на зарядку
    if (!headingToCharger && robot.getCurrentCharge() <= robot.getCriticalThreshold())
    {
        if (!planRouteToCharger())
        {
            return false;
        }
    }

    // Если активный маршрут ещё не построен или уже пройден
    if (activePath.empty() || pathCursor + 1 >= activePath.size())
    {
        if (headingToCharger)
        {
            // Прибыли на зарядную станцию
            robot.setState("Зарядка");
            robot.recharge();
            ++currentStep;
            headingToCharger = false;
            activePath.clear();
            pathCursor = 0;
            lastEventMessage = "Робот встал на зарядную станцию и зарядился до 100%.";
            if (!planRouteToCurrentTask())
            {
                return false;
            }
        }
        else
        {
            if (!planRouteToCurrentTask())
            {
                return false;
            }
        }
    }

    // Если после планирования маршрут состоит из 0 шагов (уже на месте)
    if (!headingToCharger && activePath.size() == 1 && activeTaskIndex < taskQueue.size())
    {
        taskQueue[activeTaskIndex].markCompleted();
        lastEventMessage = "Цель №" + std::to_string(taskQueue[activeTaskIndex].getTaskNumber()) + " достигнута.";
        ++activeTaskIndex;
        if (!planRouteToCurrentTask())
        {
            return false;
        }
    }

    // Проверяем достаточность заряда хотя бы на 1 следующий шаг
    if (!robot.hasEnoughCharge(1) && robot.getCurrentCharge() <= 0.0)
    {
        robot.setState("Аварийная остановка");
        status = "Аварийная остановка";
        lastEventMessage = "Аварийная остановка: батарея полностью разряжена.";
        return false;
    }

    if (pathCursor + 1 < activePath.size())
    {
        GridPoint nextPt = activePath[pathCursor + 1];
        robot.moveStep(nextPt.x, nextPt.y);
        robot.consumeEnergy();
        ++pathCursor;
        ++currentStep;

        if (robot.getCurrentCharge() <= 0.0 && !headingToCharger)
        {
            // Проверяем, не нужно ли срочно на зарядку на следующем шаге
            lastEventMessage = "Шаг (" + std::to_string(nextPt.x) + ", " + std::to_string(nextPt.y) +
                               "). Заряд: " + std::to_string(static_cast<int>(robot.getCurrentCharge())) + "%.";
        }
        else
        {
            lastEventMessage = "Шаг (" + std::to_string(nextPt.x) + ", " + std::to_string(nextPt.y) +
                               ") [" + robot.fsmState + "].";
        }

        // Проверяем достижение конца текущего маршрута
        if (pathCursor + 1 >= activePath.size())
        {
            if (headingToCharger)
            {
                robot.setState("Зарядка");
                robot.recharge();
                headingToCharger = false;
                activePath.clear();
                pathCursor = 0;
                lastEventMessage = "Достигнута зарядная станция — батарея заряжена до 100%.";
            }
            else if (activeTaskIndex < taskQueue.size())
            {
                taskQueue[activeTaskIndex].markCompleted();
                int doneNum = taskQueue[activeTaskIndex].getTaskNumber();
                ++activeTaskIndex;
                lastEventMessage = "Выполнено задание №" + std::to_string(doneNum) + ".";
                if (activeTaskIndex >= taskQueue.size())
                {
                    robot.setState("Миссия завершена");
                    status = "Завершена";
                    lastEventMessage = "Все маршрутные задания успешно завершены.";
                }
            }
        }
    }

    return true;
}

int CSimulationSession::getSessionId() const
{
    return sessionId;
}

int CSimulationSession::getMapId() const
{
    return mapId;
}

std::string CSimulationSession::getStatus() const
{
    return status;
}

int CSimulationSession::getCurrentStep() const
{
    return currentStep;
}

const CRobot &CSimulationSession::getRobot() const
{
    return robot;
}

const CWarehouseMap &CSimulationSession::getMap() const
{
    return map;
}

const std::vector<CTask> &CSimulationSession::getTaskQueue() const
{
    return taskQueue;
}

const std::vector<GridPoint> &CSimulationSession::getActivePath() const
{
    return activePath;
}

std::string CSimulationSession::getLastEventMessage() const
{
    return lastEventMessage;
}

int CSimulationSession::getCompletedTasksCount() const
{
    return static_cast<int>(std::min(activeTaskIndex, taskQueue.size()));
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
    int diff = currentStep - robot.totalSteps;
    return diff > 0 ? diff : 0;
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