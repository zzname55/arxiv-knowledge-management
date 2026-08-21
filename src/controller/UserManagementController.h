// ---------------------------------------------------------------------------
// UserManagementController — MVC layer: CONTROLLER (US-08)
// ---------------------------------------------------------------------------
#pragma once

#include <QString>
#include "model/UserRole.h"

class AuthenticationService;
class UserManagementService;
class UserManagementViewContract;

class UserManagementController
{
public:
    UserManagementController(UserManagementService &userManagement, AuthenticationService &authentifizierung,
                                 UserManagementViewContract &ansicht);

    void refreshList();
    void createUser(const QString &username, const QString &displayName, const QString &password, UserRole role);
    void changeRole(int targetUserId, UserRole newRole);
    void deactivateAccount(int targetUserId);
    void activateAccount(int targetUserId);

    static const QString kMessageUserCreated;
    static const QString kMessageRoleChanged;
    static const QString kMessageAccountDeactivated;
    static const QString kMessageAccountActivated;

private:
    void handleResult(bool successful, const QString &errorMessage, const QString &erfolgsmeldung);

    UserManagementService &m_userManagement;
    AuthenticationService  &m_authentication;
    UserManagementViewContract &m_view;
};
