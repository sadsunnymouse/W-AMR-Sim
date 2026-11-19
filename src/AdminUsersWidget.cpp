#include "AdminUsersWidget.h"
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QVBoxLayout>

AdminUsersWidget::AdminUsersWidget(QWidget *parent)
    : QWidget(parent), nextUserId(10)
{
    auto *mainLayout = new QHBoxLayout(this);

    // Левая часть: таблица учётных записей
    usersTable = new QTableWidget(this);
    usersTable->setColumnCount(5);
    usersTable->setHorizontalHeaderLabels(
        {"ID", "ФИО", "Логин", "Роль", "Статус"});
    usersTable->horizontalHeader()->setStretchLastSection(true);
    usersTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    usersTable->setSelectionMode(QAbstractItemView::SingleSelection);
    usersTable->setEditTriggers(QAbstractItemView::NoEditTriggers);

    // Правая часть: форма создания и редактирования
    auto *rightLayout = new QVBoxLayout();
    auto *formGroup = new QGroupBox("Учётная запись пользователя", this);
    auto *form = new QFormLayout(formGroup);

    fullNameEdit = new QLineEdit(this);
    fullNameEdit->setPlaceholderText("Например: Иванов Иван Иванович");

    loginEdit = new QLineEdit(this);
    loginEdit->setPlaceholderText("Уникальный логин");

    passwordEdit = new QLineEdit(this);
    passwordEdit->setEchoMode(QLineEdit::Password);
    passwordEdit->setPlaceholderText("Пароль / хэш пароля");

    roleCombo = new QComboBox(this);
    blockedCheck = new QCheckBox("Учётная запись заблокирована", this);

    form->addRow("ФИО:", fullNameEdit);
    form->addRow("Логин:", loginEdit);
    form->addRow("Пароль:", passwordEdit);
    form->addRow("Роль:", roleCombo);
    form->addRow("", blockedCheck);

    addUserBtn = new QPushButton("Создать пользователя", this);
    updateUserBtn = new QPushButton("Сохранить изменения", this);
    toggleBlockBtn = new QPushButton("Блокировать / Разблокировать", this);
    resetPasswordBtn = new QPushButton("Сбросить пароль", this);

    rightLayout->addWidget(formGroup);
    rightLayout->addWidget(addUserBtn);
    rightLayout->addWidget(updateUserBtn);
    rightLayout->addWidget(toggleBlockBtn);
    rightLayout->addWidget(resetPasswordBtn);
    rightLayout->addStretch();

    mainLayout->addWidget(usersTable, 2);
    mainLayout->addLayout(rightLayout, 1);

    connect(usersTable, &QTableWidget::itemSelectionChanged,
            this, &AdminUsersWidget::onTableSelectionChanged);
    connect(addUserBtn, &QPushButton::clicked,
            this, &AdminUsersWidget::onAddUserClicked);
    connect(updateUserBtn, &QPushButton::clicked,
            this, &AdminUsersWidget::onUpdateUserClicked);
    connect(toggleBlockBtn, &QPushButton::clicked,
            this, &AdminUsersWidget::onToggleBlockClicked);
    connect(resetPasswordBtn, &QPushButton::clicked,
            this, &AdminUsersWidget::onResetPasswordClicked);
}

void AdminUsersWidget::setData(const std::vector<CRole> &roleList, const std::vector<CUser> &userList)
{
    roles = roleList;
    users = userList;

    int maxId = 0;
    for (const auto &u : users)
    {
        if (u.getUserId() > maxId)
        {
            maxId = u.getUserId();
        }
    }
    nextUserId = maxId + 1;

    refreshRolesCombo();
    refreshUsersTable();
}

const std::vector<CUser> &AdminUsersWidget::getUsers() const
{
    return users;
}

void AdminUsersWidget::refreshRolesCombo()
{
    roleCombo->clear();
    for (const auto &r : roles)
    {
        roleCombo->addItem(QString::fromStdString(r.title), r.getRoleId());
    }
}

void AdminUsersWidget::refreshUsersTable()
{
    usersTable->setRowCount(static_cast<int>(users.size()));

    for (size_t i = 0; i < users.size(); ++i)
    {
        const CUser &u = users[i];
        QString roleName = "Не назначена";
        for (const auto &r : roles)
        {
            if (r.getRoleId() == u.roleId)
            {
                roleName = QString::fromStdString(r.title);
                break;
            }
        }

        int row = static_cast<int>(i);
        usersTable->setItem(row, 0, new QTableWidgetItem(QString::number(u.getUserId())));
        usersTable->setItem(row, 1, new QTableWidgetItem(QString::fromStdString(u.fullName)));
        usersTable->setItem(row, 2, new QTableWidgetItem(QString::fromStdString(u.getLogin())));
        usersTable->setItem(row, 3, new QTableWidgetItem(roleName));
        usersTable->setItem(row, 4, new QTableWidgetItem(u.getIsBlocked() ? "Заблокирован" : "Активен"));
    }
}

int AdminUsersWidget::selectedUserIndex() const
{
    QList<QTableWidgetItem *> items = usersTable->selectedItems();
    if (items.isEmpty())
    {
        return -1;
    }
    return items.first()->row();
}

void AdminUsersWidget::onTableSelectionChanged()
{
    int idx = selectedUserIndex();
    if (idx < 0 || idx >= static_cast<int>(users.size()))
    {
        return;
    }

    const CUser &u = users[idx];
    fullNameEdit->setText(QString::fromStdString(u.fullName));
    loginEdit->setText(QString::fromStdString(u.getLogin()));
    passwordEdit->clear();
    blockedCheck->setChecked(u.getIsBlocked());

    for (int i = 0; i < roleCombo->count(); ++i)
    {
        if (roleCombo->itemData(i).toInt() == u.roleId)
        {
            roleCombo->setCurrentIndex(i);
            break;
        }
    }
}

void AdminUsersWidget::onAddUserClicked()
{
    QString name = fullNameEdit->text().trimmed();
    QString login = loginEdit->text().trimmed();
    QString pass = passwordEdit->text();

    if (name.isEmpty() || login.isEmpty() || pass.isEmpty())
    {
        QMessageBox::warning(this, "Неполные данные",
                             "Заполните ФИО, логин и пароль нового пользователя.");
        return;
    }

    for (const auto &u : users)
    {
        if (u.authenticate(login.toStdString(), pass.toStdString()) ||
            QString::fromStdString(u.getLogin()).compare(login, Qt::CaseInsensitive) == 0)
        {
            QMessageBox::warning(this, "Дубликат логина",
                                 "Пользователь с таким логином уже существует.");
            return;
        }
    }

    int roleId = roleCombo->currentData().toInt();
    CUser newUser(nextUserId++, name.toStdString(), login.toStdString(),
                  pass.toStdString(), blockedCheck->isChecked(), roleId);
    users.push_back(newUser);
    refreshUsersTable();
    passwordEdit->clear();
    emit usersChanged(users);
}

void AdminUsersWidget::onUpdateUserClicked()
{
    int idx = selectedUserIndex();
    if (idx < 0 || idx >= static_cast<int>(users.size()))
    {
        QMessageBox::information(this, "Выбор пользователя", "Выберите строку пользователя в таблице.");
        return;
    }

    QString name = fullNameEdit->text().trimmed();
    if (name.isEmpty())
    {
        QMessageBox::warning(this, "Некорректные данные", "ФИО не может быть пустым.");
        return;
    }

    int roleId = roleCombo->currentData().toInt();
    users[idx].fullName = name.toStdString();
    users[idx].setRole(roleId);
    users[idx].setBlockStatus(blockedCheck->isChecked());

    if (!passwordEdit->text().isEmpty())
    {
        users[idx].resetPassword(passwordEdit->text().toStdString());
        passwordEdit->clear();
    }

    refreshUsersTable();
    emit usersChanged(users);
}

void AdminUsersWidget::onToggleBlockClicked()
{
    int idx = selectedUserIndex();
    if (idx < 0 || idx >= static_cast<int>(users.size()))
    {
        QMessageBox::information(this, "Выбор пользователя", "Выберите пользователя в таблице.");
        return;
    }

    bool nextBlocked = !users[idx].getIsBlocked();
    users[idx].setBlockStatus(nextBlocked);
    blockedCheck->setChecked(nextBlocked);
    refreshUsersTable();
    emit usersChanged(users);
}

void AdminUsersWidget::onResetPasswordClicked()
{
    int idx = selectedUserIndex();
    if (idx < 0 || idx >= static_cast<int>(users.size()))
    {
        QMessageBox::information(this, "Выбор пользователя", "Выберите пользователя в таблице.");
        return;
    }

    QString newPass = passwordEdit->text();
    if (newPass.isEmpty())
    {
        newPass = "1234";
    }

    users[idx].resetPassword(newPass.toStdString());
    passwordEdit->clear();
    QMessageBox::information(this, "Пароль обновлён",
                             "Пароль для выбранной учётной записи успешно сброшен.");
    emit usersChanged(users);
}