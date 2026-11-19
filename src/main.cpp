#include "CWarehouseMap.h"
#include "OperatorScenarioWidget.h"
#include <QApplication>
#include <vector>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    CWarehouseMap sampleMap;
    sampleMap.create(1, "Склад №1", 1, 10, 8, "Утверждена", 2);
    sampleMap.placeObject(3, 2, "wall");
    sampleMap.placeObject(3, 3, "rack");
    sampleMap.placeObject(1, 1, "charging");
    sampleMap.placeObject(8, 6, "zone");

    std::vector<CWarehouseMap> maps = {sampleMap};

    OperatorScenarioWidget opWidget;
    opWidget.setWindowTitle("W-AMR Sim — АРМ Оператора симуляции (Параметризация O1 / O2)");
    opWidget.setAvailableMaps(maps);
    opWidget.resize(1050, 680);
    opWidget.show();

    return app.exec();
}