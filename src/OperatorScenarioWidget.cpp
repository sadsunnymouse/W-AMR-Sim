#include "OperatorScenarioWidget.h"
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QScrollArea>
#include <QVBoxLayout>

OperatorScenarioWidget::OperatorScenarioWidget(QWidget *parent)
    : QWidget(parent), activeMap(nullptr)
{
    auto *mainLayout = new QHBoxLayout(this);

    // Левая часть: утверждённая карта в режиме "только для чтения"
    auto *leftLayout = new QVBoxLayout();
    auto *mapTopBar = new QHBoxLayout();
    mapCombo = new QComboBox(this);
    mapModeLabel = new QLabel("Режим: только для чтения", this);
    mapTopBar->addWidget(new QLabel("Утверждённая схема:", this));
    mapTopBar->addWidget(mapCombo);
    mapTopBar->addWidget(mapModeLabel);

    mapWidget = new SimulationMapWidget(this);
    auto *scrollArea = new QScrollArea(this);
    scrollArea->setWidget(mapWidget);
    scrollArea->setAlignment(Qt::AlignCenter);

    leftLayout->addLayout(mapTopBar);
    leftLayout->addWidget(scrollArea);

    // Правая часть: параметризация сценария (O2)
    auto *rightLayout = new QVBoxLayout();

    // 1. Позиция робота
    auto *posGroup = new QGroupBox("1. Начальное положение робота", this);
    auto *posLayout = new QVBoxLayout(posGroup);
    startPosLabel = new QLabel("Старт не задан", this);
    pickStartBtn = new QPushButton("Указать старт на карте", this);
    pickStartBtn->setCheckable(true);
    applyModeButtonStyle(pickStartBtn);
    posLayout->addWidget(startPosLabel);
    posLayout->addWidget(pickStartBtn);

    // 2. Параметры батареи
    auto *batGroup = new QGroupBox("2. Параметры батареи", this);
    auto *batForm = new QFormLayout(batGroup);

    capacitySpin = new QDoubleSpinBox(this);
    capacitySpin->setRange(1.0, 100000.0);
    capacitySpin->setValue(100.0);
    capacitySpin->setDecimals(1);

    initialChargeSpin = new QDoubleSpinBox(this);
    initialChargeSpin->setRange(-50.0, 200.0);
    initialChargeSpin->setValue(100.0);
    initialChargeSpin->setDecimals(1);

    thresholdSpin = new QDoubleSpinBox(this);
    thresholdSpin->setRange(-50.0, 200.0);
    thresholdSpin->setValue(20.0);
    thresholdSpin->setDecimals(1);

    energyPerStepSpin = new QDoubleSpinBox(this);
    energyPerStepSpin->setRange(0.0, 1000.0);
    energyPerStepSpin->setValue(2.0);
    energyPerStepSpin->setDecimals(2);

    batteryValidationLabel = new QLabel("Параметры батареи корректны (1–100%, порог <= заряда).", this);
    batteryValidationLabel->setWordWrap(true);

    batForm->addRow("Ёмкость (А·ч / усл. ед.):", capacitySpin);
    batForm->addRow("Начальный заряд (%):", initialChargeSpin);
    batForm->addRow("Критический порог (%):", thresholdSpin);
    batForm->addRow("Расход на шаг:", energyPerStepSpin);
    batForm->addRow(batteryValidationLabel);

    // 3. Очередь целевых точек
    auto *taskGroup = new QGroupBox("3. Очередь маршрутных заданий", this);
    auto *taskLayout = new QVBoxLayout(taskGroup);
    taskListWidget = new QListWidget(this);

    auto *taskBtnsTop = new QHBoxLayout();
    pickTargetBtn = new QPushButton("Добавить цель на карте", this);
    pickTargetBtn->setCheckable(true);
    applyModeButtonStyle(pickTargetBtn);

    moveTargetBtn = new QPushButton("Переместить цель", this);
    moveTargetBtn->setCheckable(true);
    applyModeButtonStyle(moveTargetBtn);

    taskBtnsTop->addWidget(pickTargetBtn);
    taskBtnsTop->addWidget(moveTargetBtn);

    clearTasksBtn = new QPushButton("Очистить очередь", this);

    taskLayout->addWidget(taskListWidget);
    taskLayout->addLayout(taskBtnsTop);
    taskLayout->addWidget(clearTasksBtn);

    // Применение параметров
    applyParamsBtn = new QPushButton("Применить параметры", this);
    readyToRunBtn = new QPushButton("К запуску симуляции (доступно после применения)", this);
    readyToRunBtn->setEnabled(false);
    scenarioStatusLabel = new QLabel("Статус профиля: не применён", this);
    scenarioStatusLabel->setWordWrap(true);

    rightLayout->addWidget(posGroup);
    rightLayout->addWidget(batGroup);
    rightLayout->addWidget(taskGroup);
    rightLayout->addWidget(applyParamsBtn);
    rightLayout->addWidget(readyToRunBtn);
    rightLayout->addWidget(scenarioStatusLabel);

    mainLayout->addLayout(leftLayout, 2);
    mainLayout->addLayout(rightLayout, 1);

    // Фильтр событий для выхода из активного режима карты при нажатии на прочие элементы
    capacitySpin->installEventFilter(this);
    initialChargeSpin->installEventFilter(this);
    thresholdSpin->installEventFilter(this);
    energyPerStepSpin->installEventFilter(this);
    applyParamsBtn->installEventFilter(this);

    connect(mapCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &OperatorScenarioWidget::onMapSelected);
    connect(pickStartBtn, &QPushButton::toggled,
            this, &OperatorScenarioWidget::onPickStartToggled);
    connect(pickTargetBtn, &QPushButton::toggled,
            this, &OperatorScenarioWidget::onPickTargetToggled);
    connect(moveTargetBtn, &QPushButton::toggled,
            this, &OperatorScenarioWidget::onMoveTargetToggled);
    connect(clearTasksBtn, &QPushButton::clicked,
            this, &OperatorScenarioWidget::onClearTasksClicked);

    connect(mapWidget, &SimulationMapWidget::startCellPicked,
            this, &OperatorScenarioWidget::onStartCellPicked);
    connect(mapWidget, &SimulationMapWidget::targetCellPicked,
            this, &OperatorScenarioWidget::onTargetCellPicked);
    connect(mapWidget, &SimulationMapWidget::existingTargetSelected,
            this, &OperatorScenarioWidget::onExistingTargetSelected);
    connect(mapWidget, &SimulationMapWidget::targetMoved,
            this, &OperatorScenarioWidget::onTargetMoved);
    connect(mapWidget, &SimulationMapWidget::invalidCellClicked,
            this, &OperatorScenarioWidget::onInvalidCellClicked);

    connect(taskListWidget, &QListWidget::currentRowChanged,
            this, &OperatorScenarioWidget::onTaskListRowSelected);

    connect(initialChargeSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &OperatorScenarioWidget::onBatteryParamChanged);
    connect(thresholdSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &OperatorScenarioWidget::onBatteryParamChanged);
    connect(capacitySpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &OperatorScenarioWidget::onBatteryParamChanged);
    connect(energyPerStepSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &OperatorScenarioWidget::onBatteryParamChanged);

    connect(applyParamsBtn, &QPushButton::clicked,
            this, &OperatorScenarioWidget::onApplyParametersClicked);

    validateBatteryInputs(false);
    refreshApplyButtonState();
}

void OperatorScenarioWidget::applyModeButtonStyle(QPushButton *btn)
{
    if (!btn)
    {
        return;
    }
    btn->setStyleSheet(
        "QPushButton:checked {"
        "  background-color: #2b6cb0;"
        "  color: #ffffff;"
        "  font-weight: bold;"
        "}");
}

void OperatorScenarioWidget::resetMapModes(QPushButton *exceptButton)
{
    QPushButton *modeButtons[] = {pickStartBtn, pickTargetBtn, moveTargetBtn};
    for (QPushButton *btn : modeButtons)
    {
        if (btn && btn != exceptButton && btn->isChecked())
        {
            btn->blockSignals(true);
            btn->setChecked(false);
            btn->blockSignals(false);
        }
    }
    if (exceptButton == nullptr)
    {
        mapWidget->setPickMode(MapPickMode::None);
    }
}

bool OperatorScenarioWidget::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonPress)
    {
        if (watched == capacitySpin || watched == initialChargeSpin ||
            watched == thresholdSpin || watched == energyPerStepSpin ||
            watched == applyParamsBtn)
        {
            resetMapModes(nullptr);
            scenarioStatusLabel->setText("Режим работы с картой отключён.");
        }
    }
    return QWidget::eventFilter(watched, event);
}

void OperatorScenarioWidget::setAvailableMaps(const std::vector<CWarehouseMap> &maps)
{
    approvedMaps.clear();
    for (const auto &m : maps)
    {
        if (m.getStatus() == "Утверждена")
        {
            approvedMaps.push_back(m);
        }
    }
    reloadApprovedMapsCombo();
}

void OperatorScenarioWidget::reloadApprovedMapsCombo()
{
    mapCombo->blockSignals(true);
    mapCombo->clear();

    for (const auto &m : approvedMaps)
    {
        QString label = QString("%1 (v%2, %3x%4)")
                            .arg(QString::fromStdString(m.title))
                            .arg(m.getVersion())
                            .arg(m.getWidth())
                            .arg(m.getHeight());
        mapCombo->addItem(label);
    }

    mapCombo->blockSignals(false);

    if (!approvedMaps.empty())
    {
        mapCombo->setCurrentIndex(0);
        onMapSelected(0);
    }
    else
    {
        activeMap = nullptr;
        mapWidget->setMap(nullptr);
        startPosLabel->setText("Старт не задан");
        currentTasks.clear();
        refreshTaskListUI();
        refreshApplyButtonState();
    }
}

void OperatorScenarioWidget::onMapSelected(int index)
{
    resetMapModes(nullptr);

    if (index < 0 || index >= static_cast<int>(approvedMaps.size()))
    {
        activeMap = nullptr;
        mapWidget->setMap(nullptr);
        return;
    }

    activeMap = &approvedMaps[index];
    mapWidget->setMap(activeMap);
    startPosLabel->setText("Старт не задан");
    currentTasks.clear();
    verifiedProfile.isVerified = false;
    readyToRunBtn->setEnabled(false);
    scenarioStatusLabel->setText("Статус профиля: не применён (загружена схема в режиме «только для чтения»)");
    refreshTaskListUI();
    refreshApplyButtonState();
}

void OperatorScenarioWidget::onPickStartToggled(bool checked)
{
    if (checked)
    {
        if (activeMap == nullptr)
        {
            pickStartBtn->blockSignals(true);
            pickStartBtn->setChecked(false);
            pickStartBtn->blockSignals(false);
            QMessageBox::information(this, "Выбор карты", "Сначала выберите утверждённую карту склада.");
            return;
        }
        resetMapModes(pickStartBtn);
        mapWidget->setPickMode(MapPickMode::PickStart);
        scenarioStatusLabel->setText("Режим выбора старта активен: кликайте по проходимым ячейкам для установки старта.");
    }
    else if (mapWidget->getHasStart() || true)
    {
        if (!pickTargetBtn->isChecked() && !moveTargetBtn->isChecked())
        {
            mapWidget->setPickMode(MapPickMode::None);
            scenarioStatusLabel->setText("Режим выбора старта выключен.");
        }
    }
}

void OperatorScenarioWidget::onPickTargetToggled(bool checked)
{
    if (checked)
    {
        if (activeMap == nullptr)
        {
            pickTargetBtn->blockSignals(true);
            pickTargetBtn->setChecked(false);
            pickTargetBtn->blockSignals(false);
            QMessageBox::information(this, "Выбор карты", "Сначала выберите утверждённую карту склада.");
            return;
        }
        resetMapModes(pickTargetBtn);
        mapWidget->setPickMode(MapPickMode::PickTarget);
        scenarioStatusLabel->setText("Режим добавления целей активен: кликайте по проходимым ячейкам.");
    }
    else
    {
        if (!pickStartBtn->isChecked() && !moveTargetBtn->isChecked())
        {
            mapWidget->setPickMode(MapPickMode::None);
            scenarioStatusLabel->setText("Режим добавления целей выключен.");
        }
    }
}

void OperatorScenarioWidget::onMoveTargetToggled(bool checked)
{
    if (checked)
    {
        if (activeMap == nullptr || currentTasks.empty())
        {
            moveTargetBtn->blockSignals(true);
            moveTargetBtn->setChecked(false);
            moveTargetBtn->blockSignals(false);
            QMessageBox::information(this, "Перемещение цели",
                                     "Сначала выберите карту и добавьте хотя бы одну цель.");
            return;
        }
        resetMapModes(moveTargetBtn);
        mapWidget->setPickMode(MapPickMode::MoveTargetSelect);
        scenarioStatusLabel->setText("Режим перемещения цели: нажмите на существующую цель на карте.");
    }
    else
    {
        mapWidget->setSelectedTaskIndex(-1);
        if (!pickStartBtn->isChecked() && !pickTargetBtn->isChecked())
        {
            mapWidget->setPickMode(MapPickMode::None);
            scenarioStatusLabel->setText("Режим перемещения цели выключен.");
        }
    }
}

void OperatorScenarioWidget::onClearTasksClicked()
{
    resetMapModes(nullptr);
    currentTasks.clear();
    mapWidget->setTasks(currentTasks);
    refreshTaskListUI();
    verifiedProfile.isVerified = false;
    readyToRunBtn->setEnabled(false);
    refreshApplyButtonState();
}

void OperatorScenarioWidget::onStartCellPicked(int x, int y)
{
    // Режим PickStart намеренно сохраняется до переключения на другой режим или элемент
    mapWidget->setStartPosition(x, y);
    startPosLabel->setText(QString("Старт: (%1, %2)").arg(x).arg(y));
    scenarioStatusLabel->setText(QString("Старт установлен в (%1, %2). Режим выбора старта остаётся активным.").arg(x).arg(y));
    verifiedProfile.isVerified = false;
    readyToRunBtn->setEnabled(false);
    refreshApplyButtonState();
}

void OperatorScenarioWidget::onTargetCellPicked(int x, int y)
{
    if (activeMap == nullptr)
    {
        return;
    }
    int cellId = activeMap->cells[y][x].getCellId();
    int nextNumber = static_cast<int>(currentTasks.size()) + 1;
    CTask task(nextNumber, cellId, x, y, false);
    currentTasks.push_back(task);

    mapWidget->setTasks(currentTasks);
    refreshTaskListUI();
    verifiedProfile.isVerified = false;
    readyToRunBtn->setEnabled(false);
    refreshApplyButtonState();
}

void OperatorScenarioWidget::onExistingTargetSelected(int taskIndex)
{
    if (taskIndex < 0 || taskIndex >= static_cast<int>(currentTasks.size()))
    {
        return;
    }
    taskListWidget->setCurrentRow(taskIndex);
    mapWidget->setPickMode(MapPickMode::MoveTargetPlace);
    int num = currentTasks[taskIndex].getTaskNumber();
    scenarioStatusLabel->setText(
        QString("Выбрана цель №%1. Теперь нажмите на проходимую ячейку, куда нужно её переместить.")
            .arg(num));
}

void OperatorScenarioWidget::onTargetMoved(int taskIndex, int newX, int newY)
{
    if (activeMap == nullptr || taskIndex < 0 || taskIndex >= static_cast<int>(currentTasks.size()))
    {
        return;
    }

    int preservedNumber = currentTasks[taskIndex].getTaskNumber();
    bool completed = currentTasks[taskIndex].isCompleted;
    int newCellId = activeMap->cells[newY][newX].getCellId();

    currentTasks[taskIndex].create(preservedNumber, newCellId, newX, newY, completed);
    mapWidget->setTasks(currentTasks);
    mapWidget->setSelectedTaskIndex(taskIndex);
    refreshTaskListUI();
    taskListWidget->setCurrentRow(taskIndex);

    // Возвращаемся к выбору следующей цели для перемещения, пока режим moveTargetBtn активен
    mapWidget->setPickMode(MapPickMode::MoveTargetSelect);
    scenarioStatusLabel->setText(
        QString("Цель №%1 перемещена в ячейку (%2, %3), cellId=%4.")
            .arg(preservedNumber)
            .arg(newX)
            .arg(newY)
            .arg(newCellId));

    verifiedProfile.isVerified = false;
    readyToRunBtn->setEnabled(false);
    refreshApplyButtonState();
}

void OperatorScenarioWidget::onTaskListRowSelected(int row)
{
    mapWidget->setSelectedTaskIndex(row);
    if (moveTargetBtn->isChecked() && row >= 0 && row < static_cast<int>(currentTasks.size()))
    {
        mapWidget->setPickMode(MapPickMode::MoveTargetPlace);
        scenarioStatusLabel->setText(
            QString("Выбрана цель №%1 из списка. Нажмите на целевую ячейку на карте для перемещения.")
                .arg(currentTasks[row].getTaskNumber()));
    }
}

void OperatorScenarioWidget::onInvalidCellClicked(int x, int y)
{
    if (mapWidget->getPickMode() == MapPickMode::MoveTargetSelect)
    {
        QMessageBox::information(this, "Цель не найдена",
                                 QString("В ячейке (%1, %2) нет назначенной цели.").arg(x).arg(y));
        return;
    }
    QMessageBox::warning(this, "Непроходимая ячейка",
                         QString("Указанная ячейка (%1, %2) непроходима для робота").arg(x).arg(y));
}

bool OperatorScenarioWidget::validateBatteryInputs(bool /*showWarnings*/)
{
    double initCharge = initialChargeSpin->value();
    double threshold = thresholdSpin->value();
    double capacity = capacitySpin->value();
    double stepEnergy = energyPerStepSpin->value();

    bool initValid = (initCharge >= 1.0 && initCharge <= 100.0);
    bool thresholdValid = (threshold >= 1.0 && threshold <= 100.0);
    bool relationValid = (threshold <= initCharge);
    bool otherValid = (capacity > 0.0 && stepEnergy >= 0.0);

    initialChargeSpin->setStyleSheet(initValid ? "" : "border: 2px solid red;");
    thresholdSpin->setStyleSheet((thresholdValid && relationValid) ? "" : "border: 2px solid red;");

    if (!initValid || !thresholdValid)
    {
        batteryValidationLabel->setText(
            "Ошибка: начальный заряд и критический порог должны быть в диапазоне 1–100%.");
        return false;
    }
    if (!relationValid)
    {
        batteryValidationLabel->setText(
            "Ошибка: критический порог не может превышать начальный уровень заряда.");
        return false;
    }
    if (!otherValid)
    {
        batteryValidationLabel->setText(
            "Ошибка: ёмкость батареи должна быть больше 0, расход на шаг >= 0.");
        return false;
    }

    batteryValidationLabel->setText("Параметры батареи корректны (1–100%, порог <= заряда).");
    return true;
}

void OperatorScenarioWidget::onBatteryParamChanged()
{
    verifiedProfile.isVerified = false;
    readyToRunBtn->setEnabled(false);
    refreshApplyButtonState();
}

void OperatorScenarioWidget::refreshTaskListUI()
{
    taskListWidget->blockSignals(true);
    taskListWidget->clear();
    for (const auto &t : currentTasks)
    {
        taskListWidget->addItem(
            QString("Цель №%1: (%2, %3) [cellId=%4]")
                .arg(t.getTaskNumber())
                .arg(t.getTargetX())
                .arg(t.getTargetY())
                .arg(t.targetCellId));
    }
    taskListWidget->blockSignals(false);
}

void OperatorScenarioWidget::refreshApplyButtonState()
{
    bool batteryOk = validateBatteryInputs(false);
    bool hasMapAndStart = (activeMap != nullptr) && mapWidget->getHasStart();
    bool hasTasks = !currentTasks.empty();
    applyParamsBtn->setEnabled(batteryOk && hasMapAndStart && hasTasks);
}

void OperatorScenarioWidget::onApplyParametersClicked()
{
    resetMapModes(nullptr);

    if (activeMap == nullptr || !mapWidget->getHasStart() || currentTasks.empty())
    {
        QMessageBox::warning(this, "Неполные данные",
                             "Выберите утверждённую карту, задайте стартовую позицию и хотя бы одну цель.");
        return;
    }

    if (!validateBatteryInputs(true))
    {
        QMessageBox::warning(this, "Некорректные параметры батареи",
                             batteryValidationLabel->text());
        return;
    }

    CRobot tempRobot;
    if (!tempRobot.setBatteryParams(capacitySpin->value(),
                                    initialChargeSpin->value(),
                                    thresholdSpin->value(),
                                    energyPerStepSpin->value()))
    {
        QMessageBox::warning(this, "Ошибка валидации",
                             "Не удалось применить параметры батареи.");
        return;
    }

    verifiedProfile.isVerified = true;
    verifiedProfile.mapId = activeMap->getMapId();
    verifiedProfile.startX = mapWidget->getStartX();
    verifiedProfile.startY = mapWidget->getStartY();
    verifiedProfile.batteryCapacity = capacitySpin->value();
    verifiedProfile.initialCharge = initialChargeSpin->value();
    verifiedProfile.criticalThreshold = thresholdSpin->value();
    verifiedProfile.energyPerStep = energyPerStepSpin->value();
    verifiedProfile.tasks = currentTasks;

    readyToRunBtn->setEnabled(true);
    scenarioStatusLabel->setText(
        QString("Профиль верифицирован: старт (%1, %2), задач в очереди: %3.")
            .arg(verifiedProfile.startX)
            .arg(verifiedProfile.startY)
            .arg(verifiedProfile.tasks.size()));

    emit scenarioApplied(verifiedProfile, *activeMap);
}

const ScenarioProfile &OperatorScenarioWidget::getVerifiedProfile() const
{
    return verifiedProfile;
}