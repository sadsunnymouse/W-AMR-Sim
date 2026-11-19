#ifndef CCELL_H
#define CCELL_H

#include <string>

class CCell
{
private:
    int cellId;
    int x;
    int y;

public:
    std::string objectType;
    bool isWalkable;

    CCell();
    CCell(int id, int posX, int posY, const std::string &type = "passage", bool walkable = true);

    void create(int id, int posX, int posY, const std::string &type = "passage", bool walkable = true);
    void setObjectType(const std::string &type);
    bool checkWalkable() const;

    int getCellId() const;
    int getX() const;
    int getY() const;
};

#endif