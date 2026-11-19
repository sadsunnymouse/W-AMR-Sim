#ifndef SIMULATIONMAPWIDGET_H
#define SIMULATIONMAPWIDGET_H

#include "CTask.h"
#include "CWarehouseMap.h"
#include <QWidget>
#include <vector>

enum class MapPickMode
{
    None,
    PickStart,
    PickTarget,
    MoveTargetSelect,
    MoveTargetPlace
};

class SimulationMapWidget : public QWidget
{
    Q_OBJECT

private:
    const CWarehouseMap *mapRef;
    MapPickMode pickMode;
    int cellSize;

    bool hasStart;
    int startX;
    int startY;
    std::vector<CTask> tasks;
    int selectedTaskIndex;

public:
    explicit SimulationMapWidget(QWidget *parent = nullptr);

    void setMap(const CWarehouseMap *map);
    void setPickMode(MapPickMode mode);
    void setStartPosition(int x, int y);
    void clearStartPosition();
    void setTasks(const std::vector<CTask> &taskList);
    void setSelectedTaskIndex(int index);

    bool getHasStart() const;
    int getStartX() const;
    int getStartY() const;
    int getSelectedTaskIndex() const;
    MapPickMode getPickMode() const;

signals:
    void startCellPicked(int x, int y);
    void targetCellPicked(int x, int y);
    void existingTargetSelected(int taskIndex);
    void targetMoved(int taskIndex, int newX, int newY);
    void invalidCellClicked(int x, int y);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
};

#endif