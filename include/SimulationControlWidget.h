#ifndef SIMULATIONCONTROLWIDGET_H
#define SIMULATIONCONTROLWIDGET_H

#include "CSimulationSession.h"
#include "CWarehouseMap.h"
#include "OperatorScenarioWidget.h"
#include <QLabel>
#include <QListWidget>
#include <QPushButton>
#include <QTimer>
#include <QWidget>
#include "CSimulationReport.h"
#include <string>

class SimulationCanvasWidget : public QWidget
{
    Q_OBJECT

private:
    const CSimulationSession *sessionRef;
    int cellSize;

public:
    explicit SimulationCanvasWidget(QWidget *parent = nullptr);
    void setSession(const CSimulationSession *session);

protected:
    void paintEvent(QPaintEvent *event) override;
};

class SimulationControlWidget : public QWidget
{
    Q_OBJECT

private:
    CSimulationSession session;
    bool hasSession;

    SimulationCanvasWidget *canvasWidget;
    QTimer *stepTimer;

    QPushButton *startBtn;
    QPushButton *pauseBtn;
    QPushButton *stepBtn;
    QPushButton *resetBtn;
    QPushButton *reportBtn;
    int currentOperatorId;
    std::string currentOperatorName;

    QLabel *sessionStatusLabel;
    QLabel *fsmStateLabel;
    QLabel *positionLabel;
    QLabel *batteryLabel;
    QLabel *stepCounterLabel;
    QLabel *eventLogLabel;
    QListWidget *tasksStatusList;

    void refreshTelemetryUI();
signals:
    void reportGenerated(const CSimulationReport &report);

private slots:
    void onStartClicked();
    void onPauseClicked();
    void onStepClicked();
    void onResetClicked();
    void onTimerTick();
    void onGenerateReportClicked();

public:
    explicit SimulationControlWidget(QWidget *parent = nullptr);
    bool loadScenario(const ScenarioProfile &profile, const CWarehouseMap &approvedMap);
    void setCurrentOperator(int operatorId, const std::string &operatorName);
};

#endif