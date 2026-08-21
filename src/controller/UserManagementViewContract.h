// ---------------------------------------------------------------------------
// UserManagementViewContract — Schnittstelle zwischen Controller und View (US-08)
// ---------------------------------------------------------------------------
#pragma once

#include <QList>
#include <QString>
#include "model/User.h"

class UserManagementViewContract
{
public:
    virtual ~UserManagementViewContract() = default;

    virtual void showUsers(const QList<User> &userList) = 0;
    virtual void showMessage(const QString &message) = 0;
    virtual void showError(const QString &message) = 0;
    virtual void resetForm() = 0;
};
