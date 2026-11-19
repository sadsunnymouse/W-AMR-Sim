#include "MainWindow.h"
#include <QDialog>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QMessageBox>
#include <QVBoxLayout>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), isLoggedIn(false)
{
    initDefaultRolesAndUsers();

    auto *central = new QWidget(this);
    auto *rootLayout = new QVBoxLayout(central);

    // Верхняя панель сессии пользователя
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

    // Загружаем начальную утверждённую карту в операторский модуль
    syncApprovedMapsToOperator();
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
    std::vector<CWarehouseMap> maps;
    maps.push_back(mapEditorWidget->getCurrentMap());

    // Добавляем демонстрационную утверждённую схему, если текущая ещё черновик
    CWarehouseMap demoApproved;
    demoApproved.create(10, "Склад №1 (Утверждённый)", 1, 10, 8, "Утверждена", 2);
    demoApproved.placeObject(3, 2, "wall");
    demoApproved.placeObject(3, 3, "rack");
    demoApproved.placeObject(1, 1, "charging");
    demoApproved.placeObject(8, 6, "zone");
    maps.push_back(demoApproved);

    operatorWidget->setAvailableMaps(maps);
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

    QString roleTitle = "Неизвестная роль";
    for (const auto &r : roles)
    {
        if (r.getRoleId() == currentUser.roleId)
        {
            roleTitle = QString::fromStdString(r.title);
            break;
        }
    }

    currentUserLabel->setText(
        QString("Пользователь: %1 (%2) | Роль: %3")
            .arg(QString::fromStdString(currentUser.fullName))
            .arg(QString::fromStdString(currentUser.getLogin()))
            .arg(roleTitle));

    if (currentUser.roleId == 1)
    {
        adminWidget->setData(roles, users);
        adminWidget->show();
        roleTabs->addTab(adminWidget, "Администрирование пользователей (А1)");
    }
    else if (currentUser.roleId == 2)
    {
        mapEditorWidget->show();
        roleTabs->addTab(mapEditorWidget, "Редактор топологии и экспорт/импорт (М1 / М2)");
    }
    else if (currentUser.roleId == 3)
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