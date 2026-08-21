// ---------------------------------------------------------------------------
// UserManagementView — MVC layer: VIEW (W7, US-08)
// ---------------------------------------------------------------------------
#pragma once

#include <QList>
#include <QWidget>
#include "controller/UserManagementViewContract.h"

class QComboBox;
class QLabel;
class QLineEdit;
class QPushButton;
class QTableWidget;

class UserManagementView : public QWidget, public UserManagementViewContract
{
    Q_OBJECT

public:
    explicit UserManagementView(QWidget *parentWidget = nullptr);

    void showUsers(const QList<User> &userList) override;
    void showMessage(const QString &message) override;
    void showError(const QString &message) override;
    void resetForm() override;

signals:
    void createRequested(const QString &username, const QString &displayName, const QString &password, UserRole role);
    void changeRoleRequested(int userId, UserRole newRole);
    void deactivateRequested(int userId);
    void activateRequested(int userId);

private:
    void buildUi();
    void selectionChanged();
    UserRole selectedRole() const;
    const User *selectedUser() const;

    enum column { SpalteBenutzername = 0, SpalteAnzeigename = 1, SpalteRolle = 2, SpalteStatus = 3, SpaltenAnzahl = 4 };

    QTableWidget *m_table = nullptr;

    QLineEdit *m_usernameField = nullptr;
    QLineEdit *m_displayNameField  = nullptr;
    QLineEdit *m_passwordField     = nullptr;
    QComboBox *m_roleSelector    = nullptr;

    QPushButton *m_createButton      = nullptr;
    QPushButton *m_changeRoleButton = nullptr;
    QPushButton *m_deactivateButton = nullptr;
    QPushButton *m_activateButton   = nullptr;

    QLabel *m_messageLabel = nullptr;

    QList<User> m_displayedUsers;
};
