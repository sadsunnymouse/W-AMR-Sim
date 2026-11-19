#include "MapEditorWidget.h"
#include <QAbstractItemView>
#include <QHBoxLayout>
#include <QKeySequence>
#include <QMessageBox>
#include <QMouseEvent>
#include <QScrollArea>
#include <QVBoxLayout>

MapEditorWidget::MapEditorWidget(QWidget *parent) : QWidget(parent)
{
    currentMap.create(1, "Склад №1", 1, 12, 8, "Черновик", 2);

    auto *mainLayout = new QVBoxLayout(this);

    // Верхняя панель создания новой сетки
    auto *topBar = new QHBoxLayout();
    titleEdit = new QLineEdit(QString::fromStdString(currentMap.title), this);
    widthSpin = new QSpinBox(this);
    widthSpin->setRange(4, 50);
    widthSpin->setValue(12);

    heightSpin = new QSpinBox(this);
    heightSpin->setRange(4, 50);
    heightSpin->setValue(8);

    auto *createBtn = new QPushButton("Создать сетку", this);
    topBar->addWidget(new QLabel("Название:", this));
    topBar->addWidget(titleEdit);
    topBar->addWidget(new QLabel("Ширина:", this));
    topBar->addWidget(widthSpin);
    topBar->addWidget(new QLabel("Длина:", this));
    topBar->addWidget(heightSpin);
    topBar->addWidget(createBtn);

    // Панель инструментов и утверждения
    auto *toolBar = new QHBoxLayout();
    toolCombo = new QComboBox(this);
    toolCombo->addItem("Стена (непроходимо)", "wall");
    toolCombo->addItem("Стеллаж (непроходимо)", "rack");
    toolCombo->addItem("Свободный проход", "passage");
    toolCombo->addItem("Зарядная станция", "charging");
    toolCombo->addItem("Зона погрузки/разгрузки", "zone");

    if (toolCombo->view() && toolCombo->view()->viewport())
    {
        toolCombo->view()->viewport()->installEventFilter(this);
    }

    approveButton = new QPushButton("Проверить и утвердить", this);
    newDraftButton = new QPushButton("Создать черновик новой версии", this);

    toolBar->addWidget(new QLabel("Объект:", this));
    toolBar->addWidget(toolCombo);
    toolBar->addStretch();
    toolBar->addWidget(approveButton);
    toolBar->addWidget(newDraftButton);

    // Область прокрутки с 2D-картой
    gridWidget = new WarehouseGridWidget(this);
    gridWidget->setMap(&currentMap);
    gridWidget->setSelectedTool("wall");

    auto *scrollArea = new QScrollArea(this);
    scrollArea->setWidget(gridWidget);
    scrollArea->setAlignment(Qt::AlignCenter);

    statusLabel = new QLabel(this);

    mainLayout->addLayout(topBar);
    mainLayout->addLayout(toolBar);
    mainLayout->addWidget(scrollArea);
    mainLayout->addWidget(statusLabel);

    undoShortcut = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_Z), this);
    redoShortcut = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_Y), this);

    connect(createBtn, &QPushButton::clicked, this, &MapEditorWidget::onCreateNewGrid);
    connect(toolCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MapEditorWidget::onToolChanged);
    connect(approveButton, &QPushButton::clicked, this, &MapEditorWidget::onApproveClicked);
    connect(newDraftButton, &QPushButton::clicked, this, &MapEditorWidget::onCreateDraftCopyClicked);
    connect(undoShortcut, &QShortcut::activated, this, &MapEditorWidget::onUndo);
    connect(redoShortcut, &QShortcut::activated, this, &MapEditorWidget::onRedo);
    connect(gridWidget, &WarehouseGridWidget::historyChanged, this, &MapEditorWidget::refreshStatusUI);

    refreshStatusUI();
}

bool MapEditorWidget::eventFilter(QObject *watched, QEvent *event)
{
    if (toolCombo && toolCombo->view() && watched == toolCombo->view()->viewport())
    {
        if (event->type() == QEvent::MouseButtonPress)
        {
            auto *mouseEvent = static_cast<QMouseEvent *>(event);
            if (mouseEvent->button() == Qt::LeftButton)
            {
                QModelIndex idx = toolCombo->view()->indexAt(mouseEvent->pos());
                if (idx.isValid() && idx.row() == toolCombo->currentIndex())
                {
                    toolCombo->hidePopup();
                    return true;
                }
            }
        }
    }
    return QWidget::eventFilter(watched, event);
}

void MapEditorWidget::refreshStatusUI()
{
    bool isApproved = (currentMap.getStatus() == "Утверждена");
    statusLabel->setText(
        QString("Схема: %1 | Версия: %2 | Статус: %3 | Горячие клавиши: Ctrl+Z (отмена), Ctrl+Y (повтор)")
            .arg(QString::fromStdString(currentMap.title))
            .arg(currentMap.getVersion())
            .arg(QString::fromStdString(currentMap.getStatus())));
    toolCombo->setEnabled(!isApproved);
    approveButton->setEnabled(!isApproved);
    newDraftButton->setEnabled(isApproved);
    undoShortcut->setEnabled(!isApproved);
    redoShortcut->setEnabled(!isApproved);
}

void MapEditorWidget::onCreateNewGrid()
{
    std::string name = titleEdit->text().trimmed().toStdString();
    if (name.empty())
    {
        name = "Склад без названия";
    }
    currentMap.create(1, name, 1, widthSpin->value(), heightSpin->value(), "Черновик", 2);
    gridWidget->setMap(&currentMap);
    refreshStatusUI();
}

void MapEditorWidget::onToolChanged(int index)
{
    QString toolData = toolCombo->itemData(index).toString();
    gridWidget->setSelectedTool(toolData.toStdString());
}

void MapEditorWidget::onApproveClicked()
{
    currentMap.title = titleEdit->text().trimmed().toStdString();
    std::vector<std::pair<int, int>> problems;

    if (currentMap.approveMap(problems))
    {
        gridWidget->clearHistory();
        gridWidget->clearProblemCells();
        refreshStatusUI();
        QMessageBox::information(this, "Утверждение топологии",
                                 "Граф проходов связен. Схема переведена в статус «Утверждена» и защищена от изменений.");
    }
    else
    {
        gridWidget->setProblemCells(problems);
        refreshStatusUI();
        QMessageBox::warning(this, "Нарушение связности",
                             QString("Обнаружены изолированные зоны или заблокированные объекты (%1 ячеек выделено красным). Схема оставлена в статусе «Черновик».")
                                 .arg(problems.size()));
    }
}

void MapEditorWidget::onCreateDraftCopyClicked()
{
    currentMap = currentMap.createDraftCopy();
    gridWidget->setMap(&currentMap);
    refreshStatusUI();
}

void MapEditorWidget::onUndo()
{
    gridWidget->undo();
}

void MapEditorWidget::onRedo()
{
    gridWidget->redo();
}

const CWarehouseMap &MapEditorWidget::getCurrentMap() const
{
    return currentMap;
}