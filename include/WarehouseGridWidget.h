#ifndef WAREHOUSEGRIDWIDGET_H
#define WAREHOUSEGRIDWIDGET_H

#include "CWarehouseMap.h"
#include <QWidget>
#include <set>
#include <stack>
#include <string>
#include <utility>
#include <vector>

struct CellEditAction
{
    int x;
    int y;
    std::string oldType;
    std::string newType;
};

class WarehouseGridWidget : public QWidget
{
    Q_OBJECT

private:
    CWarehouseMap *mapRef;
    std::string selectedTool;
    int cellSize;
    std::set<std::pair<int, int>> highlightedProblems;
    std::stack<CellEditAction> undoStack;
    std::stack<CellEditAction> redoStack;

public:
    explicit WarehouseGridWidget(QWidget *parent = nullptr);

    void setMap(CWarehouseMap *map);
    void setSelectedTool(const std::string &tool);
    void setProblemCells(const std::vector<std::pair<int, int>> &problems);
    void clearProblemCells();

    bool undo();
    bool redo();
    void clearHistory();

signals:
    void cellEdited(int x, int y);
    void historyChanged();

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
};

#endif