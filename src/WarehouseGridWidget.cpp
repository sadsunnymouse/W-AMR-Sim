#include "WarehouseGridWidget.h"
#include <QMouseEvent>
#include <QPainter>
#include <QPen>

WarehouseGridWidget::WarehouseGridWidget(QWidget *parent)
    : QWidget(parent), mapRef(nullptr), selectedTool("wall"), cellSize(36) {}

void WarehouseGridWidget::setMap(CWarehouseMap *map)
{
    mapRef = map;
    highlightedProblems.clear();
    clearHistory();
    if (mapRef != nullptr)
    {
        setMinimumSize(mapRef->getWidth() * cellSize + 2,
                       mapRef->getHeight() * cellSize + 2);
    }
    update();
}

void WarehouseGridWidget::setSelectedTool(const std::string &tool)
{
    selectedTool = tool;
}

void WarehouseGridWidget::setProblemCells(const std::vector<std::pair<int, int>> &problems)
{
    highlightedProblems.clear();
    for (const auto &cell : problems)
    {
        highlightedProblems.insert(cell);
    }
    update();
}

void WarehouseGridWidget::clearProblemCells()
{
    highlightedProblems.clear();
    update();
}

void WarehouseGridWidget::clearHistory()
{
    while (!undoStack.empty())
    {
        undoStack.pop();
    }
    while (!redoStack.empty())
    {
        redoStack.pop();
    }
    emit historyChanged();
}

bool WarehouseGridWidget::undo()
{
    if (mapRef == nullptr || mapRef->getStatus() == "Утверждена" || undoStack.empty())
    {
        return false;
    }

    CellEditAction action = undoStack.top();
    undoStack.pop();

    if (action.y < 0 || action.y >= mapRef->getHeight() ||
        action.x < 0 || action.x >= mapRef->getWidth())
    {
        return false;
    }

    mapRef->cells[action.y][action.x].setObjectType(action.oldType);
    redoStack.push(action);
    highlightedProblems.clear();
    update();
    emit historyChanged();
    return true;
}

bool WarehouseGridWidget::redo()
{
    if (mapRef == nullptr || mapRef->getStatus() == "Утверждена" || redoStack.empty())
    {
        return false;
    }

    CellEditAction action = redoStack.top();
    redoStack.pop();

    if (action.y < 0 || action.y >= mapRef->getHeight() ||
        action.x < 0 || action.x >= mapRef->getWidth())
    {
        return false;
    }

    mapRef->cells[action.y][action.x].setObjectType(action.newType);
    undoStack.push(action);
    highlightedProblems.clear();
    update();
    emit historyChanged();
    return true;
}

void WarehouseGridWidget::paintEvent(QPaintEvent * /*event*/)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, false);

    if (mapRef == nullptr)
    {
        return;
    }

    int w = mapRef->getWidth();
    int h = mapRef->getHeight();

    for (int y = 0; y < h; ++y)
    {
        for (int x = 0; x < w; ++x)
        {
            QRect rect(x * cellSize, y * cellSize, cellSize, cellSize);
            const std::string &type = mapRef->cells[y][x].objectType;

            QColor fillColor = Qt::white;
            QString badge = "";

            if (type == "wall")
            {
                fillColor = QColor(60, 65, 75);
            }
            else if (type == "rack")
            {
                fillColor = QColor(180, 130, 70);
                badge = "R";
            }
            else if (type == "charging")
            {
                fillColor = QColor(80, 190, 110);
                badge = "⚡";
            }
            else if (type == "zone")
            {
                fillColor = QColor(90, 155, 225);
                badge = "Z";
            }

            painter.fillRect(rect, fillColor);
            painter.setPen(QPen(QColor(200, 205, 210), 1));
            painter.drawRect(rect);

            if (!badge.isEmpty())
            {
                painter.setPen(Qt::white);
                painter.drawText(rect, Qt::AlignCenter, badge);
            }

            if (highlightedProblems.count({x, y}) > 0)
            {
                painter.setPen(QPen(Qt::red, 3));
                painter.drawRect(rect.adjusted(1, 1, -1, -1));
            }
        }
    }
}

void WarehouseGridWidget::mousePressEvent(QMouseEvent *event)
{
    if (mapRef == nullptr || event->button() != Qt::LeftButton)
    {
        return;
    }

    if (mapRef->getStatus() == "Утверждена")
    {
        return;
    }

    int x = static_cast<int>(event->position().x()) / cellSize;
    int y = static_cast<int>(event->position().y()) / cellSize;

    if (y < 0 || y >= mapRef->getHeight() || x < 0 || x >= mapRef->getWidth())
    {
        return;
    }

    std::string previousType = mapRef->cells[y][x].objectType;
    if (previousType == selectedTool)
    {
        return;
    }

    if (mapRef->placeObject(x, y, selectedTool))
    {
        undoStack.push({x, y, previousType, selectedTool});
        while (!redoStack.empty())
        {
            redoStack.pop();
        }
        highlightedProblems.clear();
        update();
        emit historyChanged();
        emit cellEdited(x, y);
    }
}