#include "CTask.h"

CTask::CTask()
    : taskNumber(0), targetX(0), targetY(0), targetCellId(0), isCompleted(false) {}

CTask::CTask(int number, int cellId, int x, int y, bool completed)
    : taskNumber(number), targetX(x), targetY(y), targetCellId(cellId), isCompleted(completed) {}

void CTask::create(int number, int cellId, int x, int y, bool completed)
{
    taskNumber = number;
    targetCellId = cellId;
    targetX = x;
    targetY = y;
    isCompleted = completed;
}

void CTask::markCompleted()
{
    isCompleted = true;
}

int CTask::getTaskNumber() const
{
    return taskNumber;
}

int CTask::getTargetX() const
{
    return targetX;
}

int CTask::getTargetY() const
{
    return targetY;
}