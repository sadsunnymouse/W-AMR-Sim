#ifndef WAREHOUSEGRIDWIDGET_H
#define WAREHOUSEGRIDWIDGET_H

#include "CWarehouseMap.h"
#include <QWidget>
#include <set>
#include <utility>

class WarehouseGridWidget : public QWidget
{
    Q_OBJECT

private:
    CWarehouseMap *mapRef;
    std::string selectedTool;
    int cellSize;
    std::set<std::pair<int, int>> highlightedProblems;

public:
    explicit WarehouseGridWidget(QWidget *parent = nullptr);

    void setMap(CWarehouseMap *map);
    void setSelectedTool(const std::string &tool);
    void setProblemCells(const std::vector<std::pair<int, int>> &problems);
    void clearProblemCells();

signals:
    void cellEdited(int x, int y);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
};

#endif