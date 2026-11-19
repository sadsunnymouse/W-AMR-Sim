#ifndef CWAREHOUSEMAP_H
#define CWAREHOUSEMAP_H

#include "CCell.h"
#include <string>
#include <vector>

class CWarehouseMap
{
private:
    int mapId;
    int version;
    int width;
    int height;
    std::string status;
    int authorId;

public:
    std::string title;
    std::vector<std::vector<CCell>> cells;

    CWarehouseMap();
    CWarehouseMap(int id,
                  const std::string &mapTitle,
                  int mapVersion,
                  int gridWidth,
                  int gridHeight,
                  const std::string &mapStatus,
                  int creatorId);

    void create(int id,
                const std::string &mapTitle,
                int mapVersion,
                int gridWidth,
                int gridHeight,
                const std::string &mapStatus,
                int creatorId);
    bool placeObject(int x, int y, const std::string &objectType);
    bool validateConnectivity(std::vector<std::pair<int, int>> &problemCells) const;
    bool approveMap(std::vector<std::pair<int, int>> &problemCells);
    CWarehouseMap createDraftCopy() const;

    int getMapId() const;
    int getVersion() const;
    int getWidth() const;
    int getHeight() const;
    std::string getStatus() const;
    int getAuthorId() const;
};

#endif