#pragma once

#include "CWarehouseMap.h"
#include <QPainter>
#include <QPen>

namespace MapRenderer
{
    inline void drawGrid(QPainter &painter, const CWarehouseMap &map, int cellSize)
    {
        const int w = map.getWidth();
        const int h = map.getHeight();

        for (int y = 0; y < h; ++y)
        {
            for (int x = 0; x < w; ++x)
            {
                QRect rect(x * cellSize, y * cellSize, cellSize, cellSize);
                const std::string &type = map.cells[y][x].objectType;

                QColor fillColor = Qt::white;
                QString badge;

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
            }
        }
    }
} // namespace MapRenderer