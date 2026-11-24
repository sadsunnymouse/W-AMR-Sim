#include "MainWindow.h"
#include <QDialog>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      isLoggedIn(false),
      currentUserLabel(nullptr),
      switchUserBtn(nullptr),
      roleTabs(nullptr),
      adminWidget(nullptr),
      mapEditorWidget(nullptr),
      operatorWidget(nullptr),
      simulationWidget(nullptr)
{
    auto *central = new QWidget(this);
    auto *rootLayout = new QVBoxLayout(central);

    auto *topBar = new QHBoxLayout();
    currentUserLabel = new QLabel("Пользователь: не авторизован", this);
    switchUserBtn = new QPushButton("Войти / Сменить пользователя", this);
    topBar->addWidget(currentUserLabel);
    topBar->addStretch();
    topBar->addWidget(switchUserBtn);

    roleTabs = new QTabWidget(this);

    adminWidget = new AdminUsersWidget(this);
    mapEditorWidget = new MapEditorWidget(this);
    operatorWidget = new OperatorScenarioWidget(this);
    simulationWidget = new SimulationControlWidget(this);

    rootLayout->addLayout(topBar);
    rootLayout->addWidget(roleTabs);
    setCentralWidget(central);

    setWindowTitle("W-AMR Sim — Система моделирования автономного складского робота");
    resize(1180, 760);

    connect(switchUserBtn, &QPushButton::clicked, this, &MainWindow::onSwitchUserClicked);
    connect(adminWidget, &AdminUsersWidget::usersChanged, this, &MainWindow::onUsersUpdated);
    connect(operatorWidget, &OperatorScenarioWidget::scenarioApplied,
            this, &MainWindow::onScenarioApplied);
    connect(mapEditorWidget, &MapEditorWidget::mapApproved,
            this, &MainWindow::onMapApproved);
    connect(simulationWidget, &SimulationControlWidget::reportGenerated,
            this, &MainWindow::onReportGenerated);
    initDatabaseAndData();
    rebuildTabsForCurrentUser();
    onSwitchUserClicked();
}

void MainWindow::initDefaultRolesAndUsers()
{
    roles = {
        CRole(1, "Администратор системы", 1, "Управление учётными записями и ролями"),
        CRole(2, "Руководитель склада", 2, "Конструирование и утверждение топологии склада"),
        CRole(3, "Оператор симуляции", 4, "Параметризация, запуск и мониторинг симуляции")};

    users = {
        CUser(1, "Администратор", "admin", "admin123", false, 1),
        CUser(2, "Руководитель склада", "manager", "manager123", false, 2),
        CUser(3, "Оператор симуляции", "operator", "operator123", false, 3)};
}
void MainWindow::syncApprovedMapsToOperator()
{
    if (operatorWidget == nullptr)
    {
        return;
    }

    operatorWidget->setAvailableMaps(storedMaps);
}
void MainWindow::rebuildTabsForCurrentUser()
{
    while (roleTabs->count() > 0)
    {
        roleTabs->removeTab(0);
    }

    adminWidget->setParent(this);
    mapEditorWidget->setParent(this);
    operatorWidget->setParent(this);
    simulationWidget->setParent(this);
    adminWidget->hide();
    mapEditorWidget->hide();
    operatorWidget->hide();
    simulationWidget->hide();

    if (!isLoggedIn)
    {
        currentUserLabel->setText("Пользователь: не авторизован");
        return;
    }

    if (simulationWidget != nullptr)
    {
        simulationWidget->setCurrentOperator(currentUser.getUserId(), currentUser.fullName);
    }

    constexpr int kPermAdmin = 1;
    constexpr int kPermMapEditor = 2;
    constexpr int kPermSimulation = 4;

    const CRole *activeRole = nullptr;
    for (const auto &r : roles)
    {
        if (r.getRoleId() == currentUser.roleId)
        {
            activeRole = &r;
            break;
        }
    }

    QString roleTitle = activeRole ? QString::fromStdString(activeRole->title) : "Неизвестная роль";
    currentUserLabel->setText(
        QString("Пользователь: %1 (%2) | Роль: %3")
            .arg(QString::fromStdString(currentUser.fullName))
            .arg(QString::fromStdString(currentUser.getLogin()))
            .arg(roleTitle));

    if (activeRole == nullptr)
    {
        return;
    }

    if (activeRole->hasPermission(kPermAdmin))
    {
        adminWidget->setData(roles, users);
        adminWidget->show();
        roleTabs->addTab(adminWidget, "Администрирование пользователей (А1)");
    }
    if (activeRole->hasPermission(kPermMapEditor))
    {
        mapEditorWidget->setCurrentAuthorId(currentUser.getUserId());
        mapEditorWidget->setCatalogMaps(storedMaps);
        mapEditorWidget->show();
        roleTabs->addTab(mapEditorWidget, "Редактор топологии и экспорт/импорт (М1 / М2)");
    }
    if (activeRole->hasPermission(kPermSimulation))
    {
        syncApprovedMapsToOperator();
        operatorWidget->show();
        simulationWidget->show();
        roleTabs->addTab(operatorWidget, "Параметризация сценария (О1 / О2)");
        roleTabs->addTab(simulationWidget, "Запуск и мониторинг (О3)");
    }
}

void MainWindow::onSwitchUserClicked()
{
    QDialog dialog(this);
    dialog.setWindowTitle("Вход в систему — W-AMR Sim (П1)");

    auto *layout = new QVBoxLayout(&dialog);
    auto *form = new QFormLayout();

    auto *loginInput = new QLineEdit(&dialog);
    loginInput->setPlaceholderText("admin / manager / operator");

    auto *passInput = new QLineEdit(&dialog);
    passInput->setEchoMode(QLineEdit::Password);
    passInput->setPlaceholderText("Введите пароль");

    auto *hintLabel = new QLabel(
        "Тестовые учётные записи:\n"
        "• admin / admin123 (Администратор системы)\n"
        "• manager / manager123 (Руководитель склада)\n"
        "• operator / operator123 (Оператор симуляции)",
        &dialog);

    form->addRow("Логин:", loginInput);
    form->addRow("Пароль:", passInput);

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    layout->addLayout(form);
    layout->addWidget(hintLabel);
    layout->addWidget(buttons);

    if (dialog.exec() != QDialog::Accepted)
    {
        return;
    }

    QString enteredLogin = loginInput->text().trimmed();
    QString enteredPass = passInput->text();

    for (const auto &u : users)
    {
        if (QString::fromStdString(u.getLogin()) == enteredLogin)
        {
            if (u.getIsBlocked())
            {
                QMessageBox::warning(this, "Доступ запрещён",
                                     "Учётная запись заблокирована администратором.");
                return;
            }
            if (u.authenticate(enteredLogin.toStdString(), enteredPass.toStdString()))
            {
                currentUser = u;
                isLoggedIn = true;
                rebuildTabsForCurrentUser();
                return;
            }
        }
    }

    QMessageBox::warning(this, "Ошибка входа", "Неверный логин или пароль.");
}

void MainWindow::onUsersUpdated(const std::vector<CUser> &updatedUsers)
{
    users = updatedUsers;
    if (dbManager.isOpen() && !dbManager.saveUsers(users))
    {
        QMessageBox::warning(this, "Ошибка сохранения в БД",
                             "Не удалось сохранить изменения пользователей в SQLite: " +
                                 QString::fromStdString(dbManager.getLastError()));
    }
}

void MainWindow::onScenarioApplied(const ScenarioProfile &profile, const CWarehouseMap &map)
{
    if (simulationWidget->loadScenario(profile, map))
    {
        for (int i = 0; i < roleTabs->count(); ++i)
        {
            if (roleTabs->widget(i) == simulationWidget)
            {
                roleTabs->setCurrentIndex(i);
                break;
            }
        }
    }
}

void MainWindow::initDatabaseAndData()
{
    // Ищем каталог проекта/db относительно текущей директории или каталога исполняемого файла
    QStringList candidateDbPaths = {
        "db/wamr_sim.db",
        QCoreApplication::applicationDirPath() + "/../db/wamr_sim.db",
        QCoreApplication::applicationDirPath() + "/../../db/wamr_sim.db"};
    QStringList candidateSchemaPaths = {
        "db/schema.sql",
        QCoreApplication::applicationDirPath() + "/../db/schema.sql",
        QCoreApplication::applicationDirPath() + "/../../db/schema.sql"};

    QString dbFile = candidateDbPaths.first();
    for (const QString &p : candidateDbPaths)
    {
        if (QFileInfo::exists(p))
        {
            dbFile = p;
            break;
        }
    }

    QString schemaFile;
    for (const QString &p : candidateSchemaPaths)
    {
        if (QFileInfo::exists(p))
        {
            schemaFile = p;
            break;
        }
    }

    // Создаём папку db, если её ещё нет
    QFileInfo dbInfo(dbFile);
    QDir().mkpath(dbInfo.absolutePath());

    if (!dbManager.open(dbFile.toStdString()))
    {
        QMessageBox::warning(this, "База данных SQLite",
                             "Не удалось открыть БД: " + QString::fromStdString(dbManager.getLastError()) +
                                 "\nПриложение продолжит работу с данными в памяти.");
        initDefaultRolesAndUsers();
        syncApprovedMapsToOperator();
        return;
    }

    if (!schemaFile.isEmpty())
    {
        dbManager.applySchemaFile(schemaFile.toStdString());
    }

    roles = dbManager.loadRoles();
    users = dbManager.loadUsers();

    if (roles.empty() || users.empty())
    {
        initDefaultRolesAndUsers();
        dbManager.saveUsers(users);
    }

    std::vector<CWarehouseMap> loadedMaps = dbManager.loadMaps();
    if (loadedMaps.empty())
    {
        CWarehouseMap demoApproved;
        demoApproved.create(10, "Склад №1 (Утверждённый)", 1, 10, 8, "Черновик", 2);
        demoApproved.placeObject(3, 2, "wall");
        demoApproved.placeObject(3, 3, "rack");
        demoApproved.placeObject(1, 1, "charging");
        demoApproved.placeObject(8, 6, "zone");

        std::vector<std::pair<int, int>> problems;
        demoApproved.approveMap(problems);

        int savedId = dbManager.saveMap(demoApproved);
        if (savedId > 0)
        {
            loadedMaps = dbManager.loadMaps();
        }
        else
        {
            loadedMaps.push_back(demoApproved);
        }
    }

    storedMaps = loadedMaps;
    syncApprovedMapsToOperator();
}
void MainWindow::onMapApproved(const CWarehouseMap &map)
{
    if (dbManager.isOpen())
    {
        int savedId = dbManager.saveMap(map);
        if (savedId > 0)
        {
            storedMaps = dbManager.loadMaps();
        }
        else
        {
            QMessageBox::warning(
                this,
                "Ошибка сохранения карты в БД",
                "Не удалось сохранить утверждённую схему в SQLite: " +
                    QString::fromStdString(dbManager.getLastError()));
            storedMaps.push_back(map);
        }
    }
    else
    {
        storedMaps.push_back(map);
    }

    syncApprovedMapsToOperator();
    if (mapEditorWidget != nullptr)
    {
        mapEditorWidget->setCatalogMaps(storedMaps);
    }
}

void MainWindow::onReportGenerated(const CSimulationReport &report)
{
    if (!dbManager.isOpen())
    {
        return;
    }

    int sessionId = dbManager.saveSimulationSession(
        report.getMapId(),
        report.getOperatorId(),
        report.getCreatedAt(),
        report.getSessionStatus(),
        report.getTotalTicks(),
        report.getCompletedTasks(),
        report.getTotalTasks());

    if (sessionId <= 0)
    {
        QMessageBox::warning(this, "Ошибка БД",
                             "Не удалось сохранить сеанс симуляции в SQLite: " +
                                 QString::fromStdString(dbManager.getLastError()));
        return;
    }

    SimulationReportRecord rec;
    rec.sessionId = sessionId;
    rec.createdAt = report.getCreatedAt();
    rec.totalTimeSec = report.getTotalTimeSec();
    rec.totalTicks = report.getTotalTicks();
    rec.totalDistance = report.getTotalDistance();
    rec.batteryConsumed = report.getBatteryConsumed();
    rec.rechargeCount = report.getRechargeCount();
    rec.idleTicks = report.getIdleTicks();
    rec.reportFilePath = report.getReportFilePath();

    if (dbManager.saveSimulationReport(rec) <= 0)
    {
        QMessageBox::warning(this, "Ошибка БД",
                             "Не удалось сохранить аналитический отчёт в SQLite: " +
                                 QString::fromStdString(dbManager.getLastError()));
    }
}