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

    int x = static_cast<int>(event->position().x()) / cellSize;
    int y = static_cast<int>(event->position().y()) / cellSize;

    if (mapRef->placeObject(x, y, selectedTool))
    {
        highlightedProblems.clear();
        update();
        emit cellEdited(x, y);
    }
}