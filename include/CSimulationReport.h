#ifndef CSIMULATIONREPORT_H
#define CSIMULATIONREPORT_H

#include <string>

class CSimulationReport
{
private:
    int mapId;
    std::string mapTitle;
    int operatorId;
    std::string operatorName;
    std::string createdAt;
    std::string sessionStatus;

    double totalTimeSec;
    int totalTicks;
    int completedTasks;
    int totalTasks;
    double totalDistance;
    double batteryConsumed;
    int rechargeCount;
    int idleTicks;

    std::string reportFilePath;

public:
    CSimulationReport();

    void setContext(int mapIdValue,
                    const std::string &mapTitleValue,
                    int operatorIdValue,
                    const std::string &operatorNameValue,
                    const std::string &createdAtValue,
                    const std::string &statusValue);

    void setMetrics(double totalTimeSecValue,
                    int totalTicksValue,
                    int completedTasksValue,
                    int totalTasksValue,
                    double totalDistanceValue,
                    double batteryConsumedValue,
                    int rechargeCountValue,
                    int idleTicksValue);

    std::string generateTextReport() const;
    std::string generateCsvReport() const;
    bool saveToFile(const std::string &filePath, std::string &errorMessage);

    int getMapId() const;
    std::string getMapTitle() const;
    int getOperatorId() const;
    std::string getOperatorName() const;
    std::string getCreatedAt() const;
    std::string getSessionStatus() const;

    double getTotalTimeSec() const;
    int getTotalTicks() const;
    int getCompletedTasks() const;
    int getTotalTasks() const;
    double getTotalDistance() const;
    double getBatteryConsumed() const;
    int getRechargeCount() const;
    int getIdleTicks() const;
    std::string getReportFilePath() const;
};

#endif