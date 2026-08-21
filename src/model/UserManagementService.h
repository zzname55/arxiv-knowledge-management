// ---------------------------------------------------------------------------
// UserManagementService — MVC layer: MODEL
// Anlegen, Aendern, Sperren von Konten (US-08). Passwoerter werden sofort
// mit frischem Salt gehasht.
// ---------------------------------------------------------------------------
#pragma once

#include <QList>
#include <QString>
#include "model/User.h"
#include "model/UserRole.h"
#include "model/OperationResult.h"

class UserRepository;

class UserManagementService
{
public:
    explicit UserManagementService(UserRepository &repository);

    OperationResult createUser(const User &actingUser, const QString &username,
                                     const QString &displayName, const QString &password, UserRole role);
    OperationResult changeRole(const User &actingUser, int targetUserId, UserRole newRole);
    OperationResult deactivateAccount(const User &actingUser, int targetUserId);
    OperationResult activateAccount(const User &actingUser, int targetUserId);
    QList<User> allUsers(const User &actingUser, OperationResult *result = nullptr) const;

    static const QString kMessageUsernameTaken;
    static const QString kMessagePasswordTooShort;
    static const QString kMessageRequiredFieldMissing;
    static const QString kMessageUserUnknown;
    static const QString kMessageSaveFailed;

    static constexpr int kMinimumPasswordLength = 8;

private:
    OperationResult setActiveFlag(const User &actingUser, int targetUserId, bool aktiv);

    UserRepository &m_repository;
};
