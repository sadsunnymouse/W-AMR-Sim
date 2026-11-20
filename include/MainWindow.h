#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "AdminUsersWidget.h"
#include "CRole.h"
#include "CUser.h"
#include "CWarehouseMap.h"
#include "MapEditorWidget.h"
#include "OperatorScenarioWidget.h"
#include "SimulationControlWidget.h"
#include "CDatabaseManager.h"
#include <QLabel>
#include <QMainWindow>
#include <QPushButton>
#include <QTabWidget>
#include <vector>

class MainWindow : public QMainWindow
{
    Q_OBJECT

private:
    std::vector<CRole> roles;
    std::vector<CUser> users;
    CUser currentUser;
    bool isLoggedIn;

    QLabel *currentUserLabel;
    QPushButton *switchUserBtn;
    QTabWidget *roleTabs;

    AdminUsersWidget *adminWidget;
    MapEditorWidget *mapEditorWidget;
    OperatorScenarioWidget *operatorWidget;
    SimulationControlWidget *simulationWidget;
    CDatabaseManager dbManager;
    std::vector<CWarehouseMap> storedMaps;

    void initDatabaseAndData();

    void initDefaultRolesAndUsers();
    void rebuildTabsForCurrentUser();
    void syncApprovedMapsToOperator();

private slots:
    void onSwitchUserClicked();
    void onUsersUpdated(const std::vector<CUser> &updatedUsers);
    void onScenarioApplied(const ScenarioProfile &profile, const CWarehouseMap &map);

public:
    explicit MainWindow(QWidget *parent = nullptr);
};

#endif