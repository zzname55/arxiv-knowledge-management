// ---------------------------------------------------------------------------
// LoginViewContract — Schnittstelle zwischen Controller und View (US-01)
// ---------------------------------------------------------------------------
#pragma once

#include <QString>
#include "model/User.h"

class LoginViewContract
{
public:
    virtual ~LoginViewContract() = default;

    virtual QString username() const = 0;
    virtual QString password() const = 0;
    virtual void showError(const QString &message) = 0;
    virtual void clearErrorDisplay() = 0;
    virtual void loginCompleted(const User &user) = 0;
    virtual void logoutCompleted() = 0;
};
