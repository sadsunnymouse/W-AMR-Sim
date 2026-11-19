#ifndef OPERATORSCENARIOWIDGET_H
#define OPERATORSCENARIOWIDGET_H

#include "CRobot.h"
#include "CTask.h"
#include "CWarehouseMap.h"
#include "SimulationMapWidget.h"
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QEvent>
#include <QLabel>
#include <QListWidget>
#include <QObject>
#include <QPushButton>
#include <QWidget>
#include <vector>

struct ScenarioProfile
{
    bool isVerified = false;
    int mapId = 0;
    int startX = 0;
    int startY = 0;
    double batteryCapacity = 100.0;
    double initialCharge = 100.0;
    double criticalThreshold = 20.0;
    double energyPerStep = 1.0;
    std::vector<CTask> tasks;
};

class OperatorScenarioWidget : public QWidget
{
    Q_OBJECT

private:
    std::vector<CWarehouseMap> approvedMaps;
    const CWarehouseMap *activeMap;
    SimulationMapWidget *mapWidget;

    QComboBox *mapCombo;
    QLabel *mapModeLabel;
    QLabel *startPosLabel;

    QDoubleSpinBox *capacitySpin;
    QDoubleSpinBox *initialChargeSpin;
    QDoubleSpinBox *thresholdSpin;
    QDoubleSpinBox *energyPerStepSpin;
    QLabel *batteryValidationLabel;

    QListWidget *taskListWidget;
    QPushButton *pickStartBtn;
    QPushButton *pickTargetBtn;
    QPushButton *moveTargetBtn;
    QPushButton *clearTasksBtn;
    QPushButton *applyParamsBtn;
    QPushButton *readyToRunBtn;
    QLabel *scenarioStatusLabel;

    std::vector<CTask> currentTasks;
    ScenarioProfile verifiedProfile;

    void reloadApprovedMapsCombo();
    bool validateBatteryInputs(bool showWarnings);
    void refreshTaskListUI();
    void refreshApplyButtonState();
    void resetMapModes(QPushButton *exceptButton = nullptr);
    void applyModeButtonStyle(QPushButton *btn);

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

signals:
    void scenarioApplied(const ScenarioProfile &profile, const CWarehouseMap &map);

private slots:
    void onMapSelected(int index);
    void onPickStartToggled(bool checked);
    void onPickTargetToggled(bool checked);
    void onMoveTargetToggled(bool checked);
    void onClearTasksClicked();
    void onStartCellPicked(int x, int y);
    void onTargetCellPicked(int x, int y);
    void onExistingTargetSelected(int taskIndex);
    void onTargetMoved(int taskIndex, int newX, int newY);
    void onTaskListRowSelected(int row);
    void onInvalidCellClicked(int x, int y);
    void onBatteryParamChanged();
    void onApplyParametersClicked();

public:
    explicit OperatorScenarioWidget(QWidget *parent = nullptr);

    void setAvailableMaps(const std::vector<CWarehouseMap> &maps);
    const ScenarioProfile &getVerifiedProfile() const;
};

#endif