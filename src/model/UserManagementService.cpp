#include "model/UserManagementService.h"

#include "model/UserRepository.h"
#include "model/PasswordHasher.h"
#include "model/PermissionService.h"

const QString UserManagementService::kMessageUsernameTaken = QStringLiteral("That username is already taken.");
const QString UserManagementService::kMessagePasswordTooShort = QStringLiteral("The password must be at least 8 characters long.");
const QString UserManagementService::kMessageRequiredFieldMissing = QStringLiteral("Please fill in all required fields.");
const QString UserManagementService::kMessageUserUnknown = QStringLiteral("This account does not exist.");
const QString UserManagementService::kMessageSaveFailed = QStringLiteral("The change could not be saved.");

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
    newUser.setUsername(sanitizedUsername);
    newUser.setDisplayName(sanitizedDisplayName);
    newUser.setRole(role);
    newUser.setSalt(salt);
    newUser.setPasswordHash(PasswordHasher::hash(password, salt));
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

    targetUser->setRole(newRole);
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
