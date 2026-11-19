#include "CCell.h"

CCell::CCell()
    : cellId(0), x(0), y(0), objectType("passage"), isWalkable(true) {}

CCell::CCell(int id, int posX, int posY, const std::string &type, bool walkable)
    : cellId(id), x(posX), y(posY), objectType(type), isWalkable(walkable)
{
    setObjectType(type);
}

void CCell::create(int id, int posX, int posY, const std::string &type, bool walkable)
{
    cellId = id;
    x = posX;
    y = posY;
    isWalkable = walkable;
    setObjectType(type);
}

void CCell::setObjectType(const std::string &type)
{
    objectType = type;
    if (type == "wall" || type == "rack" || type == "obstacle" ||
        type == "стена" || type == "стеллаж")
    {
        isWalkable = false;
    }
    else
    {
        isWalkable = true;
    }
}

bool CCell::checkWalkable() const
{
    return isWalkable;
}

int CCell::getCellId() const
{
    return cellId;
}

int CCell::getX() const
{
    return x;
}

int CCell::getY() const
{
    return y;
}