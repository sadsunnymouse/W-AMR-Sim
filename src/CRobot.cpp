#include "CRobot.h"

CRobot::CRobot()
    : robotId(1),
      currentX(0),
      currentY(0),
      batteryCapacity(100.0),
      currentCharge(100.0),
      criticalThreshold(20.0),
      energyPerStep(1.0),
      totalBatteryConsumed(0.0),
      rechargeCount(0),
      fsmState("Ожидание"),
      totalSteps(0) {}

CRobot::CRobot(int id, int startX, int startY)
{
    create(id, startX, startY);
}

void CRobot::create(int id, int startX, int startY)
{
    robotId = id;
    currentX = startX;
    currentY = startY;
    batteryCapacity = 100.0;
    currentCharge = 100.0;
    criticalThreshold = 20.0;
    energyPerStep = 1.0;
    fsmState = "Ожидание";
    totalSteps = 0;
    totalBatteryConsumed = 0.0;
    rechargeCount = 0;
}

bool CRobot::setBatteryParams(double capacity,
                              double initialChargePercent,
                              double thresholdPercent,
                              double stepEnergy)
{
    if (capacity <= 0.0 || stepEnergy < 0.0)
    {
        return false;
    }
    if (initialChargePercent < 1.0 || initialChargePercent > 100.0)
    {
        return false;
    }
    if (thresholdPercent < 1.0 || thresholdPercent > 100.0)
    {
        return false;
    }
    if (thresholdPercent > initialChargePercent)
    {
        return false;
    }

    batteryCapacity = capacity;
    currentCharge = initialChargePercent;
    criticalThreshold = thresholdPercent;
    energyPerStep = stepEnergy;
    totalBatteryConsumed = 0.0;
    rechargeCount = 0;
    return true;
}

bool CRobot::hasEnoughCharge(int stepsNeeded) const
{
    if (stepsNeeded < 0)
    {
        return false;
    }
    double requiredPercent = (static_cast<double>(stepsNeeded) * energyPerStep * 100.0) / batteryCapacity;
    double remainingAfterMove = currentCharge - requiredPercent;
    return remainingAfterMove >= criticalThreshold;
}

void CRobot::moveStep(int nextX, int nextY)
{
    currentX = nextX;
    currentY = nextY;
    ++totalSteps;
}

void CRobot::consumeEnergy()
{
    double chargeBefore = currentCharge;
    double deltaPercent = (energyPerStep * 100.0) / batteryCapacity;
    currentCharge -= deltaPercent;
    if (currentCharge < 0.0)
    {
        currentCharge = 0.0;
    }
    if (currentCharge < chargeBefore)
    {
        totalBatteryConsumed += (chargeBefore - currentCharge);
    }
}

void CRobot::recharge()
{
    if (currentCharge < 100.0)
    {
        ++rechargeCount;
    }
    currentCharge = 100.0;
}

void CRobot::setState(const std::string &newState)
{
    fsmState = newState;
}

int CRobot::getRobotId() const
{
    return robotId;
}

int CRobot::getCurrentX() const
{
    return currentX;
}

int CRobot::getCurrentY() const
{
    return currentY;
}

double CRobot::getBatteryCapacity() const
{
    return batteryCapacity;
}

double CRobot::getCurrentCharge() const
{
    return currentCharge;
}

double CRobot::getCriticalThreshold() const
{
    return criticalThreshold;
}

double CRobot::getEnergyPerStep() const
{
    return energyPerStep;
}

double CRobot::getTotalBatteryConsumed() const
{
    return totalBatteryConsumed;
}

int CRobot::getRechargeCount() const
{
    return rechargeCount;
}