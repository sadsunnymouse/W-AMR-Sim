#ifndef ADMINUSERSWIDGET_H
#define ADMINUSERSWIDGET_H

#include "CRole.h"
#include "CUser.h"
#include <QCheckBox>
#include <QComboBox>
#include <QLineEdit>
#include <QPushButton>
#include <QTableWidget>
#include <QWidget>
#include <vector>

class AdminUsersWidget : public QWidget
{
    Q_OBJECT

private:
    std::vector<CRole> roles;
    std::vector<CUser> users;
    int nextUserId;

    QTableWidget *usersTable;
    QLineEdit *fullNameEdit;
    QLineEdit *loginEdit;
    QLineEdit *passwordEdit;
    QComboBox *roleCombo;
    QCheckBox *blockedCheck;

    QPushButton *addUserBtn;
    QPushButton *updateUserBtn;
    QPushButton *toggleBlockBtn;
    QPushButton *resetPasswordBtn;

    void refreshRolesCombo();
    void refreshUsersTable();
    int selectedUserIndex() const;

signals:
    void usersChanged(const std::vector<CUser> &updatedUsers);

private slots:
    void onTableSelectionChanged();
    void onAddUserClicked();
    void onUpdateUserClicked();
    void onToggleBlockClicked();
    void onResetPasswordClicked();

public:
    explicit AdminUsersWidget(QWidget *parent = nullptr);

    void setData(const std::vector<CRole> &roleList, const std::vector<CUser> &userList);
    const std::vector<CUser> &getUsers() const;
};

#endif