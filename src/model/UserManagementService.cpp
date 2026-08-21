#include "model/UserManagementService.h"

#include "model/UserRepository.h"
#include "model/PasswordHasher.h"
#include "model/PermissionService.h"

const QString UserManagementService::kMessageUsernameTaken = QStringLiteral("Der Username ist bereits vergeben.");
const QString UserManagementService::kMessagePasswordTooShort = QStringLiteral("Das Password muss mindestens 8 Zeichen lang sein.");
const QString UserManagementService::kMessageRequiredFieldMissing = QStringLiteral("Bitte all Pflichtfelder ausfüllen.");
const QString UserManagementService::kMessageUserUnknown = QStringLiteral("Dieses Benutzerkonto ist nicht vorhanden.");
const QString UserManagementService::kMessageSaveFailed = QStringLiteral("Die Änderung konnte nicht gespeichert werden.");

UserManagementService::UserManagementService(UserRepository &repository)
    : m_repository(repository)
{
}

OperationResult UserManagementService::createUser(const User &actingUser, const QString &username,
                                                             const QString &displayName, const QString &password, UserRole role)
{
    if (!PermissionService::canManageUsers(actingUser.role())) {
        return OperationResult::failure(PermissionService::kMessageNoPermission);
    }

    const QString sanitizedUsername = username.trimmed();
    const QString sanitizedDisplayName  = displayName.trimmed();

    if (sanitizedUsername.isEmpty() || sanitizedDisplayName.isEmpty() || password.isEmpty()) {
        return OperationResult::failure(kMessageRequiredFieldMissing);
    }
    if (password.length() < kMinimumPasswordLength) {
        return OperationResult::failure(kMessagePasswordTooShort);
    }
    if (m_repository.usernameExists(sanitizedUsername)) {
        return OperationResult::failure(kMessageUsernameTaken);
    }

    const QString salt = PasswordHasher::generateSalt();

    User newUser;
    newUser.setzeBenutzername(sanitizedUsername);
    newUser.setzeAnzeigename(sanitizedDisplayName);
    newUser.setzeRolle(role);
    newUser.setzeSalt(salt);
    newUser.setzePasswortHash(PasswordHasher::hash(password, salt));
    newUser.setActive(true);

    if (!m_repository.save(newUser)) {
        return OperationResult::failure(kMessageSaveFailed);
    }
    return OperationResult::success();
}

OperationResult UserManagementService::changeRole(const User &actingUser, int targetUserId, UserRole newRole)
{
    if (!PermissionService::canManageUsers(actingUser.role())) {
        return OperationResult::failure(PermissionService::kMessageNoPermission);
    }

    std::optional<User> targetUser = m_repository.findById(targetUserId);
    if (!targetUser.has_value()) {
        return OperationResult::failure(kMessageUserUnknown);
    }

    targetUser->setzeRolle(newRole);
    if (!m_repository.save(*targetUser)) {
        return OperationResult::failure(kMessageSaveFailed);
    }
    return OperationResult::success();
}

OperationResult UserManagementService::deactivateAccount(const User &actingUser, int targetUserId)
{
    return setActiveFlag(actingUser, targetUserId, false);
}

OperationResult UserManagementService::activateAccount(const User &actingUser, int targetUserId)
{
    return setActiveFlag(actingUser, targetUserId, true);
}

OperationResult UserManagementService::setActiveFlag(const User &actingUser, int targetUserId, bool aktiv)
{
    if (!PermissionService::canDeactivateUser(actingUser, targetUserId)) {
        return OperationResult::failure(PermissionService::kMessageNoPermission);
    }

    std::optional<User> targetUser = m_repository.findById(targetUserId);
    if (!targetUser.has_value()) {
        return OperationResult::failure(kMessageUserUnknown);
    }

    targetUser->setActive(aktiv);
    if (!m_repository.save(*targetUser)) {
        return OperationResult::failure(kMessageSaveFailed);
    }
    return OperationResult::success();
}

QList<User> UserManagementService::allUsers(const User &actingUser, OperationResult *result) const
{
    if (!PermissionService::canManageUsers(actingUser.role())) {
        if (result != nullptr) {
            *result = OperationResult::failure(PermissionService::kMessageNoPermission);
        }
        return {};
    }
    if (result != nullptr) {
        *result = OperationResult::success();
    }
    return m_repository.all();
}
