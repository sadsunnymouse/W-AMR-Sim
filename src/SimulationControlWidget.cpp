#include "SimulationControlWidget.h"
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QPainter>
#include <QPen>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QDateTime>
#include <QFileDialog>
#include <QMessageBox>

SimulationCanvasWidget::SimulationCanvasWidget(QWidget *parent)
    : QWidget(parent), sessionRef(nullptr), cellSize(36) {}

void SimulationCanvasWidget::setSession(const CSimulationSession *session)
{
    sessionRef = session;
    if (sessionRef != nullptr && sessionRef->getMap().getWidth() > 0)
    {
        setMinimumSize(sessionRef->getMap().getWidth() * cellSize + 2,
                       sessionRef->getMap().getHeight() * cellSize + 2);
    }
    update();
}

void SimulationCanvasWidget::paintEvent(QPaintEvent * /*event*/)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    if (sessionRef == nullptr || sessionRef->getMap().getWidth() <= 0)
    {
        painter.setPen(Qt::gray);
        painter.drawText(rect(), Qt::AlignCenter, "Сценарий не загружен");
        return;
    }

    const CWarehouseMap &map = sessionRef->getMap();
    int w = map.getWidth();
    int h = map.getHeight();

    for (int y = 0; y < h; ++y)
    {
        for (int x = 0; x < w; ++x)
        {
            QRect rect(x * cellSize, y * cellSize, cellSize, cellSize);
            const std::string &type = map.cells[y][x].objectType;

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
        }
    }

    // Отрисовка активного маршрута
    const auto &path = sessionRef->getActivePath();
    if (path.size() > 1)
    {
        painter.setPen(QPen(QColor(45, 125, 210, 180), 3, Qt::DashLine));
        for (size_t i = 0; i + 1 < path.size(); ++i)
        {
            QPoint p1(path[i].x * cellSize + cellSize / 2, path[i].y * cellSize + cellSize / 2);
            QPoint p2(path[i + 1].x * cellSize + cellSize / 2, path[i + 1].y * cellSize + cellSize / 2);
            painter.drawLine(p1, p2);
        }
    }

    // Отрисовка задач очереди
    const auto &tasks = sessionRef->getTaskQueue();
    for (const auto &t : tasks)
    {
        int tx = t.getTargetX();
        int ty = t.getTargetY();
        if (tx >= 0 && tx < w && ty >= 0 && ty < h)
        {
            QRect tRect(tx * cellSize + 4, ty * cellSize + 4, cellSize - 8, cellSize - 8);
            painter.setPen(QPen(t.isCompleted ? QColor(40, 160, 70) : QColor(230, 140, 20), 2));
            painter.setBrush(t.isCompleted ? QColor(200, 245, 210, 200) : QColor(255, 240, 200, 180));
            painter.drawRect(tRect);
            painter.setPen(Qt::black);
            painter.drawText(tRect, Qt::AlignCenter, QString::number(t.getTaskNumber()));
        }
    }

    // Отрисовка робота
    const CRobot &robot = sessionRef->getRobot();
    int rx = robot.getCurrentX();
    int ry = robot.getCurrentY();
    if (rx >= 0 && rx < w && ry >= 0 && ry < h)
    {
        QRect rRect(rx * cellSize + 3, ry * cellSize + 3, cellSize - 6, cellSize - 6);
        QColor botColor = QColor(30, 110, 220);
        if (robot.fsmState == "Аварийная остановка")
        {
            botColor = QColor(215, 45, 45);
        }
        else if (robot.fsmState == "Зарядка")
        {
            botColor = QColor(35, 175, 95);
        }
        else if (robot.fsmState == "Движение к зарядной станции")
        {
            botColor = QColor(225, 150, 20);
        }

        painter.setPen(QPen(Qt::black, 2));
        painter.setBrush(botColor);
        painter.drawEllipse(rRect);
        painter.setPen(Qt::white);
        painter.drawText(rRect, Qt::AlignCenter, "AMR");
    }
}

SimulationControlWidget::SimulationControlWidget(QWidget *parent)
    : QWidget(parent), hasSession(false), reportBtn(nullptr),
      currentOperatorId(0), currentOperatorName("Оператор")
{
    auto *mainLayout = new QHBoxLayout(this);

    canvasWidget = new SimulationCanvasWidget(this);
    auto *scrollArea = new QScrollArea(this);
    scrollArea->setWidget(canvasWidget);
    scrollArea->setAlignment(Qt::AlignCenter);

    auto *sideLayout = new QVBoxLayout();

    auto *ctrlGroup = new QGroupBox("Управление симуляцией", this);
    auto *ctrlGrid = new QHBoxLayout(ctrlGroup);
    startBtn = new QPushButton("Старт", this);
    pauseBtn = new QPushButton("Пауза", this);
    stepBtn = new QPushButton("Шаг", this);
    resetBtn = new QPushButton("Сброс", this);
    reportBtn = new QPushButton("Сформировать отчёт (О4)", this);
    reportBtn->setMinimumWidth(reportBtn->sizeHint().width() + 16);
    ctrlGrid->addWidget(startBtn);
    ctrlGrid->addWidget(pauseBtn);
    ctrlGrid->addWidget(stepBtn);
    ctrlGrid->addWidget(resetBtn);

    auto *telemGroup = new QGroupBox("Телеметрия и состояние FSM", this);
    auto *form = new QFormLayout(telemGroup);
    sessionStatusLabel = new QLabel("—", this);
    fsmStateLabel = new QLabel("—", this);
    positionLabel = new QLabel("—", this);
    batteryLabel = new QLabel("—", this);
    stepCounterLabel = new QLabel("0", this);
    eventLogLabel = new QLabel("Ожидание загрузки сценария", this);
    eventLogLabel->setWordWrap(true);

    form->addRow("Статус сеанса:", sessionStatusLabel);
    form->addRow("Состояние робота:", fsmStateLabel);
    form->addRow("Координаты:", positionLabel);
    form->addRow("Заряд батареи:", batteryLabel);
    form->addRow("Пройдено шагов:", stepCounterLabel);
    form->addRow("Событие:", eventLogLabel);

    auto *tasksGroup = new QGroupBox("Задания маршрута", this);
    auto *tasksLayout = new QVBoxLayout(tasksGroup);
    tasksStatusList = new QListWidget(this);
    tasksLayout->addWidget(tasksStatusList);

    sideLayout->addWidget(ctrlGroup);
    sideLayout->addWidget(reportBtn);
    sideLayout->addWidget(telemGroup);
    sideLayout->addWidget(tasksGroup);

    mainLayout->addWidget(scrollArea, 2);
    mainLayout->addLayout(sideLayout, 1);

    stepTimer = new QTimer(this);
    stepTimer->setInterval(250);

    connect(startBtn, &QPushButton::clicked, this, &SimulationControlWidget::onStartClicked);
    connect(pauseBtn, &QPushButton::clicked, this, &SimulationControlWidget::onPauseClicked);
    connect(stepBtn, &QPushButton::clicked, this, &SimulationControlWidget::onStepClicked);
    connect(resetBtn, &QPushButton::clicked, this, &SimulationControlWidget::onResetClicked);
    connect(stepTimer, &QTimer::timeout, this, &SimulationControlWidget::onTimerTick);
    connect(reportBtn, &QPushButton::clicked, this, &SimulationControlWidget::onGenerateReportClicked);
    refreshTelemetryUI();
}

bool SimulationControlWidget::loadScenario(const ScenarioProfile &profile, const CWarehouseMap &approvedMap)
{
    stepTimer->stop();
    bool ok = session.initialize(
        1,
        approvedMap,
        profile.startX,
        profile.startY,
        profile.batteryCapacity,
        profile.initialCharge,
        profile.criticalThreshold,
        profile.energyPerStep,
        profile.tasks);
    hasSession = ok;
    canvasWidget->setSession(hasSession ? &session : nullptr);
    refreshTelemetryUI();

    if (!ok)
    {
        QMessageBox::warning(
            this,
            "Маршрут отклонён",
            QString::fromStdString(session.getLastEventMessage()));
    }
    return ok;
}

void SimulationControlWidget::onStartClicked()
{
    if (!hasSession)
    {
        return;
    }
    if (session.start())
    {
        stepTimer->start();
    }
    refreshTelemetryUI();
}

void SimulationControlWidget::onPauseClicked()
{
    stepTimer->stop();
    session.pause();
    refreshTelemetryUI();
}

void SimulationControlWidget::onStepClicked()
{
    if (!hasSession)
    {
        return;
    }
    stepTimer->stop();
    session.step();
    refreshTelemetryUI();
}

void SimulationControlWidget::onResetClicked()
{
    stepTimer->stop();
    session.reset();
    refreshTelemetryUI();
}

void SimulationControlWidget::onTimerTick()
{
    if (!hasSession)
    {
        stepTimer->stop();
        return;
    }
    bool canContinue = session.step();
    refreshTelemetryUI();
    if (!canContinue || session.getStatus() != "Выполняется")
    {
        stepTimer->stop();
    }
}

void SimulationControlWidget::refreshTelemetryUI()
{
    bool active = hasSession;
    bool running = active && (session.getStatus() == "Выполняется");
    bool terminal = active && (session.getStatus() == "Завершена" || session.getStatus() == "Аварийная остановка");

    startBtn->setEnabled(active && !running);
    pauseBtn->setEnabled(running);
    stepBtn->setEnabled(active && !running && !terminal);
    resetBtn->setEnabled(active);
    if (reportBtn != nullptr)
    {
        reportBtn->setEnabled(active);
    }

    if (!hasSession)
    {
        sessionStatusLabel->setText("Не инициализирован");
        fsmStateLabel->setText("—");
        positionLabel->setText("—");
        batteryLabel->setText("—");
        stepCounterLabel->setText("0");
        tasksStatusList->clear();
        canvasWidget->update();
        return;
    }

    const CRobot &r = session.getRobot();
    sessionStatusLabel->setText(QString::fromStdString(session.getStatus()));
    fsmStateLabel->setText(QString::fromStdString(r.fsmState));
    positionLabel->setText(QString("(%1, %2)").arg(r.getCurrentX()).arg(r.getCurrentY()));
    batteryLabel->setText(QString("%1% (порог %2%)")
                              .arg(QString::number(r.getCurrentCharge(), 'f', 1))
                              .arg(QString::number(r.getCriticalThreshold(), 'f', 1)));
    stepCounterLabel->setText(QString::number(r.totalSteps));
    eventLogLabel->setText(QString::fromStdString(session.getLastEventMessage()));

    tasksStatusList->clear();
    for (const auto &t : session.getTaskQueue())
    {
        QString mark = t.isCompleted ? "[Выполнено]" : "[В очереди]";
        tasksStatusList->addItem(
            QString("%1 Задание №%2 -> (%3, %4)")
                .arg(mark)
                .arg(t.getTaskNumber())
                .arg(t.getTargetX())
                .arg(t.getTargetY()));
    }

    canvasWidget->update();
}

void SimulationControlWidget::setCurrentOperator(int operatorId, const std::string &operatorName)
{
    currentOperatorId = operatorId;
    currentOperatorName = operatorName;
}

void SimulationControlWidget::onGenerateReportClicked()
{
    if (!hasSession)
    {
        QMessageBox::information(this, "Отчёт симуляции",
                                 "Сначала загрузите сценарий и запустите симуляцию.");
        return;
    }

    if (stepTimer != nullptr && stepTimer->isActive())
    {
        stepTimer->stop();
        session.pause();
        refreshTelemetryUI();
    }

    const int intervalMs = (stepTimer != nullptr && stepTimer->interval() > 0)
                               ? stepTimer->interval()
                               : 250;
    const double totalTimeSec =
        static_cast<double>(session.getCurrentStep()) * (static_cast<double>(intervalMs) / 1000.0);
    const std::string createdAt =
        QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss").toStdString();

    CSimulationReport report =
        session.buildReport(currentOperatorId, currentOperatorName, createdAt, totalTimeSec);

    QString defaultName = QString("wamr_report_%1.txt")
                              .arg(QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss"));
    QString filePath = QFileDialog::getSaveFileName(
        this,
        "Сохранить аналитический отчёт",
        defaultName,
        "Текстовый отчёт (*.txt);;Таблица CSV (*.csv)");

    if (filePath.isEmpty())
    {
        return;
    }

    std::string errorMsg;
    if (report.saveToFile(filePath.toStdString(), errorMsg))
    {
        QMessageBox::information(this, "Отчёт сохранён",
                                 "Аналитический отчёт успешно сохранён в файл:\n" + filePath);
        emit reportGenerated(report);
    }
    else
    {
        QMessageBox::warning(this, "Ошибка сохранения",
                             QString::fromStdString(errorMsg));
    }
}