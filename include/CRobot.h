#ifndef CROBOT_H
#define CROBOT_H

#include <string>

class CRobot
{
private:
    int robotId;
    int currentX;
    int currentY;
    double batteryCapacity;
    double currentCharge;
    double criticalThreshold;
    double energyPerStep;
    double totalBatteryConsumed;
    int rechargeCount;

public:
    std::string fsmState;
    int totalSteps;

    CRobot();
    CRobot(int id, int startX, int startY);

    void create(int id, int startX, int startY);
    bool setBatteryParams(double capacity,
                          double initialChargePercent,
                          double thresholdPercent,
                          double stepEnergy);
    bool hasEnoughCharge(int stepsNeeded) const;
    void moveStep(int nextX, int nextY);
    void consumeEnergy();
    void recharge();
    void setState(const std::string &newState);

    int getRobotId() const;
    int getCurrentX() const;
    int getCurrentY() const;
    double getBatteryCapacity() const;
    double getCurrentCharge() const;
    double getCriticalThreshold() const;
    double getEnergyPerStep() const;
    double getTotalBatteryConsumed() const;
    int getRechargeCount() const;
};

#endif