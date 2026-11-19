#ifndef CTASK_H
#define CTASK_H

class CTask
{
private:
    int taskNumber;
    int targetX;
    int targetY;

public:
    int targetCellId;
    bool isCompleted;

    CTask();
    CTask(int number, int cellId, int x = 0, int y = 0, bool completed = false);

    void create(int number, int cellId, int x = 0, int y = 0, bool completed = false);
    void markCompleted();

    int getTaskNumber() const;
    int getTargetX() const;
    int getTargetY() const;
};

#endif