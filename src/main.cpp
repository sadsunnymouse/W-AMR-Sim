#include "CWarehouseMap.h"
#include "OperatorScenarioWidget.h"
#include "SimulationControlWidget.h"
#include <QApplication>
#include <QTabWidget>
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

    auto *tabs = new QTabWidget();
    tabs->setWindowTitle("W-AMR Sim — Симулятор автономного складского робота");

    auto *opWidget = new OperatorScenarioWidget(tabs);
    opWidget->setAvailableMaps(maps);

    auto *simWidget = new SimulationControlWidget(tabs);

    int simTabIndex = tabs->addTab(opWidget, "1. Параметризация сценария");
    int runTabIndex = tabs->addTab(simWidget, "2. Запуск и мониторинг (O3)");

    QObject::connect(opWidget, &OperatorScenarioWidget::scenarioApplied,
                     [tabs, simWidget, runTabIndex](const ScenarioProfile &profile, const CWarehouseMap &map)
                     {
                         if (simWidget->loadScenario(profile, map))
                         {
                             tabs->setCurrentIndex(runTabIndex);
                         }
                     });

    tabs->setCurrentIndex(simTabIndex);
    tabs->resize(1120, 720);
    tabs->show();

    int exitCode = app.exec();
    delete tabs;
    return exitCode;
}