#include "CSimulationReport.h"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <iomanip>
#include <sstream>

CSimulationReport::CSimulationReport()
    : mapId(0),
      mapTitle(""),
      operatorId(0),
      operatorName(""),
      createdAt(""),
      sessionStatus("Остановлена"),
      totalTimeSec(0.0),
      totalTicks(0),
      completedTasks(0),
      totalTasks(0),
      totalDistance(0.0),
      batteryConsumed(0.0),
      rechargeCount(0),
      idleTicks(0),
      reportFilePath("") {}

void CSimulationReport::setContext(int mapIdValue,
                                   const std::string &mapTitleValue,
                                   int operatorIdValue,
                                   const std::string &operatorNameValue,
                                   const std::string &createdAtValue,
                                   const std::string &statusValue)
{
    mapId = mapIdValue;
    mapTitle = mapTitleValue;
    operatorId = operatorIdValue;
    operatorName = operatorNameValue;
    createdAt = createdAtValue;
    sessionStatus = statusValue;
}

void CSimulationReport::setMetrics(double totalTimeSecValue,
                                   int totalTicksValue,
                                   int completedTasksValue,
                                   int totalTasksValue,
                                   double totalDistanceValue,
                                   double batteryConsumedValue,
                                   int rechargeCountValue,
                                   int idleTicksValue)
{
    totalTimeSec = totalTimeSecValue;
    totalTicks = totalTicksValue;
    completedTasks = completedTasksValue;
    totalTasks = totalTasksValue;
    totalDistance = totalDistanceValue;
    batteryConsumed = batteryConsumedValue;
    rechargeCount = rechargeCountValue;
    idleTicks = idleTicksValue;
}

std::string CSimulationReport::generateTextReport() const
{
    std::ostringstream out;
    out << std::fixed << std::setprecision(2);
    out << "==================================================\n";
    out << "        АНАЛИТИЧЕСКИЙ ОТЧЕТ СИМУЛЯЦИИ W-AMR       \n";
    out << "==================================================\n";
    out << "Дата формирования:       " << createdAt << "\n";
    out << "Оператор:                " << operatorName << " (ID: " << operatorId << ")\n";
    out << "Схема склада:            " << mapTitle << " (ID: " << mapId << ")\n";
    out << "Статус сеанса:           " << sessionStatus << "\n";
    out << "--------------------------------------------------\n";
    out << "СВОДНЫЕ ПОКАЗАТЕЛИ:\n";
    out << "  Общее время симуляции: " << totalTimeSec << " сек\n";
    out << "  Количество тактов:     " << totalTicks << "\n";
    out << "  Выполнено задач:       " << completedTasks << " из " << totalTasks << "\n";
    out << "  Пройденный путь:       " << totalDistance << " ед.\n";
    out << "  Расход батареи:        " << batteryConsumed << " %\n";
    out << "  Количество подзарядок: " << rechargeCount << "\n";
    out << "  Такты простоя:         " << idleTicks << "\n";
    out << "==================================================\n";
    return out.str();
}

std::string CSimulationReport::generateCsvReport() const
{
    auto escapeCsv = [](const std::string &value) -> std::string
    {
        std::string escaped = "\"";
        for (char ch : value)
        {
            if (ch == '"')
            {
                escaped += "\"\"";
            }
            else
            {
                escaped += ch;
            }
        }
        escaped += "\"";
        return escaped;
    };

    std::ostringstream out;
    out << std::fixed << std::setprecision(2);
    out << "created_at;operator_id;operator_name;map_id;map_title;status;"
           "total_time_sec;total_ticks;completed_tasks;total_tasks;"
           "total_distance;battery_consumed;recharge_count;idle_ticks\n";
    out << escapeCsv(createdAt) << ";"
        << operatorId << ";"
        << escapeCsv(operatorName) << ";"
        << mapId << ";"
        << escapeCsv(mapTitle) << ";"
        << escapeCsv(sessionStatus) << ";"
        << totalTimeSec << ";"
        << totalTicks << ";"
        << completedTasks << ";"
        << totalTasks << ";"
        << totalDistance << ";"
        << batteryConsumed << ";"
        << rechargeCount << ";"
        << idleTicks << "\n";
    return out.str();
}

bool CSimulationReport::saveToFile(const std::string &filePath, std::string &errorMessage)
{
    if (filePath.empty())
    {
        errorMessage = "Не указан путь для сохранения отчёта.";
        return false;
    }

    std::string lowerPath = filePath;
    std::transform(lowerPath.begin(), lowerPath.end(), lowerPath.begin(),
                   [](unsigned char ch)
                   { return static_cast<char>(std::tolower(ch)); });

    const bool isCsv = lowerPath.size() >= 4 &&
                       lowerPath.substr(lowerPath.size() - 4) == ".csv";

    std::ofstream file(filePath, std::ios::out | std::ios::trunc);
    if (!file.is_open())
    {
        errorMessage = "Не удалось открыть файл для записи: " + filePath;
        return false;
    }

    file << (isCsv ? generateCsvReport() : generateTextReport());
    if (!file.good())
    {
        errorMessage = "Ошибка при записи данных в файл: " + filePath;
        return false;
    }

    file.close();
    reportFilePath = filePath;
    return true;
}

int CSimulationReport::getMapId() const { return mapId; }
std::string CSimulationReport::getMapTitle() const { return mapTitle; }
int CSimulationReport::getOperatorId() const { return operatorId; }
std::string CSimulationReport::getOperatorName() const { return operatorName; }
std::string CSimulationReport::getCreatedAt() const { return createdAt; }
std::string CSimulationReport::getSessionStatus() const { return sessionStatus; }

double CSimulationReport::getTotalTimeSec() const { return totalTimeSec; }
int CSimulationReport::getTotalTicks() const { return totalTicks; }
int CSimulationReport::getCompletedTasks() const { return completedTasks; }
int CSimulationReport::getTotalTasks() const { return totalTasks; }
double CSimulationReport::getTotalDistance() const { return totalDistance; }
double CSimulationReport::getBatteryConsumed() const { return batteryConsumed; }
int CSimulationReport::getRechargeCount() const { return rechargeCount; }
int CSimulationReport::getIdleTicks() const { return idleTicks; }
std::string CSimulationReport::getReportFilePath() const { return reportFilePath; }