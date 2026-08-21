// ---------------------------------------------------------------------------
// LoginView — MVC layer: VIEW (W1, US-01)
// Modaler Dialog; solange niemand loggedIn ist, gibt es kein Hauptfenster.
// ---------------------------------------------------------------------------
#pragma once

#include <QDialog>
#include "controller/LoginViewContract.h"

class QLabel;
class QLineEdit;
class QPushButton;

class LoginView : public QDialog, public LoginViewContract
{
    Q_OBJECT

public:
    explicit LoginView(QWidget *parentWidget = nullptr);

    QString username() const override;
    QString password() const override;
    void    showError(const QString &message) override;
    void    clearErrorDisplay() override;
    void    loginCompleted(const User &user) override;
    void    logoutCompleted() override;

    User currentUser() const { return m_currentUser; }

signals:
    void loginRequested();

private:
    void buildUi();

    QLineEdit   *m_usernameField = nullptr;
    QLineEdit   *m_passwordField     = nullptr;
    QLabel      *m_errorLabel    = nullptr;
    QPushButton *m_loginButton    = nullptr;

    User m_currentUser;
};
