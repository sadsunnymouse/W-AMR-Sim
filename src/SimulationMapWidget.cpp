#include "SimulationMapWidget.h"
#include "MapRenderer.h"
#include <QMouseEvent>
#include <QPainter>
#include <QPen>

SimulationMapWidget::SimulationMapWidget(QWidget *parent)
    : QWidget(parent),
      mapRef(nullptr),
      pickMode(MapPickMode::None),
      cellSize(36),
      hasStart(false),
      startX(-1),
      startY(-1),
      selectedTaskIndex(-1) {}

MapPickMode SimulationMapWidget::getPickMode() const
{
    return pickMode;
}

void SimulationMapWidget::setMap(const CWarehouseMap *map)
{
    mapRef = map;
    hasStart = false;
    startX = -1;
    startY = -1;
    tasks.clear();
    selectedTaskIndex = -1;
    if (mapRef != nullptr)
    {
        setMinimumSize(mapRef->getWidth() * cellSize + 2,
                       mapRef->getHeight() * cellSize + 2);
    }
    else
    {
        setMinimumSize(200, 150);
    }
    update();
}

void SimulationMapWidget::setPickMode(MapPickMode mode)
{
    pickMode = mode;
    update();
}

void SimulationMapWidget::setStartPosition(int x, int y)
{
    hasStart = true;
    startX = x;
    startY = y;
    update();
}

void SimulationMapWidget::clearStartPosition()
{
    hasStart = false;
    startX = -1;
    startY = -1;
    update();
}

void SimulationMapWidget::setTasks(const std::vector<CTask> &taskList)
{
    tasks = taskList;
    if (selectedTaskIndex >= static_cast<int>(tasks.size()))
    {
        selectedTaskIndex = -1;
    }
    update();
}

void SimulationMapWidget::setSelectedTaskIndex(int index)
{
    if (index >= 0 && index < static_cast<int>(tasks.size()))
    {
        selectedTaskIndex = index;
    }
    else
    {
        selectedTaskIndex = -1;
    }
    update();
}

bool SimulationMapWidget::getHasStart() const
{
    return hasStart;
}

int SimulationMapWidget::getStartX() const
{
    return startX;
}

int SimulationMapWidget::getStartY() const
{
    return startY;
}

int SimulationMapWidget::getSelectedTaskIndex() const
{
    return selectedTaskIndex;
}

void SimulationMapWidget::paintEvent(QPaintEvent * /*event*/)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, false);

    if (mapRef == nullptr)
    {
        painter.setPen(Qt::gray);
        painter.drawText(rect(), Qt::AlignCenter, "Выберите утверждённую карту склада");
        return;
    }

    int w = mapRef->getWidth();
    int h = mapRef->getHeight();
    MapRenderer::drawGrid(painter, *mapRef, cellSize);
    // Отрисовка целевых точек очереди с сохранением порядкового номера задания
    for (size_t i = 0; i < tasks.size(); ++i)
    {
        int tx = tasks[i].getTargetX();
        int ty = tasks[i].getTargetY();
        if (tx >= 0 && tx < w && ty >= 0 && ty < h)
        {
            QRect tRect(tx * cellSize + 3, ty * cellSize + 3, cellSize - 6, cellSize - 6);
            bool isSelected = (static_cast<int>(i) == selectedTaskIndex);

            painter.setPen(QPen(isSelected ? QColor(200, 30, 30) : QColor(230, 140, 20),
                                isSelected ? 3 : 2));
            painter.setBrush(isSelected ? QColor(255, 215, 160, 220) : QColor(255, 240, 200, 180));
            painter.drawRect(tRect);

            painter.setPen(isSelected ? QColor(150, 10, 10) : QColor(140, 70, 0));
            painter.drawText(tRect, Qt::AlignCenter, QString::number(tasks[i].getTaskNumber()));
        }
    }

    // Отрисовка стартовой позиции робота
    if (hasStart && startX >= 0 && startX < w && startY >= 0 && startY < h)
    {
        QRect sRect(startX * cellSize + 4, startY * cellSize + 4, cellSize - 8, cellSize - 8);
        painter.setPen(QPen(QColor(25, 90, 190), 2));
        painter.setBrush(QColor(50, 130, 240, 200));
        painter.drawEllipse(sRect);

        painter.setPen(Qt::white);
        painter.drawText(sRect, Qt::AlignCenter, "S");
    }
}

void SimulationMapWidget::mousePressEvent(QMouseEvent *event)
{
    if (mapRef == nullptr || event->button() != Qt::LeftButton)
    {
        return;
    }

    int x = static_cast<int>(event->position().x()) / cellSize;
    int y = static_cast<int>(event->position().y()) / cellSize;

    if (y < 0 || y >= mapRef->getHeight() || x < 0 || x >= mapRef->getWidth())
    {
        return;
    }

    if (pickMode == MapPickMode::None)
    {
        return;
    }

    if (pickMode == MapPickMode::MoveTargetSelect)
    {
        for (size_t i = 0; i < tasks.size(); ++i)
        {
            if (tasks[i].getTargetX() == x && tasks[i].getTargetY() == y)
            {
                selectedTaskIndex = static_cast<int>(i);
                update();
                emit existingTargetSelected(selectedTaskIndex);
                return;
            }
        }
        emit invalidCellClicked(x, y);
        return;
    }

    if (!mapRef->cells[y][x].checkWalkable())
    {
        emit invalidCellClicked(x, y);
        return;
    }

    if (pickMode == MapPickMode::PickStart)
    {
        emit startCellPicked(x, y);
    }
    else if (pickMode == MapPickMode::PickTarget)
    {
        emit targetCellPicked(x, y);
    }
    else if (pickMode == MapPickMode::MoveTargetPlace)
    {
        if (selectedTaskIndex >= 0 && selectedTaskIndex < static_cast<int>(tasks.size()))
        {
            emit targetMoved(selectedTaskIndex, x, y);
        }
    }
}