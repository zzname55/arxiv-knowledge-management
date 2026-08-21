#include "controller/UserManagementController.h"

#include "controller/UserManagementViewContract.h"
#include "model/AuthenticationService.h"
#include "model/UserManagementService.h"

const QString UserManagementController::kMessageUserCreated = QStringLiteral("Der User wurde angelegt.");
const QString UserManagementController::kMessageRoleChanged = QStringLiteral("Die Role wurde geändert.");
const QString UserManagementController::kMessageAccountDeactivated = QStringLiteral("Das Benutzerkonto wurde deaktiviert.");
const QString UserManagementController::kMessageAccountActivated = QStringLiteral("Das Benutzerkonto wurde wieder freigegeben.");

namespace {
const QString kMessageNotLoggedIn = QStringLiteral("Es ist kein User loggedIn.");
} // namespace

UserManagementController::UserManagementController(UserManagementService &userManagement,
                                                           AuthenticationService &authentifizierung,
                                                           UserManagementViewContract &ansicht)
    : m_userManagement(userManagement)
    , m_authentication(authentifizierung)
    , m_view(ansicht)
{
}

void UserManagementController::refreshList()
{
    const std::optional<User> actingUser = m_authentication.currentUser();
    if (!actingUser.has_value()) {
        m_view.showError(kMessageNotLoggedIn);
        return;
    }

    OperationResult      result;
    const QList<User> userList = m_userManagement.allUsers(*actingUser, &result);

    m_view.showUsers(userList);
    if (!result.successful) {
        m_view.showError(result.errorMessage);
    }
}

void UserManagementController::handleResult(bool successful, const QString &errorMessage, const QString &erfolgsmeldung)
{
    if (successful) {
        m_view.showMessage(erfolgsmeldung);
    } else {
        m_view.showError(errorMessage);
    }
    refreshList();
}

void UserManagementController::createUser(const QString &username, const QString &displayName,
                                                   const QString &password, UserRole role)
{
    const std::optional<User> actingUser = m_authentication.currentUser();
    if (!actingUser.has_value()) {
        m_view.showError(kMessageNotLoggedIn);
        return;
    }

    const OperationResult result = m_userManagement.createUser(*actingUser, username, displayName, password, role);

    if (result.successful) {
        m_view.resetForm();
    }
    handleResult(result.successful, result.errorMessage, kMessageUserCreated);
}

void UserManagementController::changeRole(int targetUserId, UserRole newRole)
{
    const std::optional<User> actingUser = m_authentication.currentUser();
    if (!actingUser.has_value()) {
        m_view.showError(kMessageNotLoggedIn);
        return;
    }
    const OperationResult result = m_userManagement.changeRole(*actingUser, targetUserId, newRole);
    handleResult(result.successful, result.errorMessage, kMessageRoleChanged);
}

void UserManagementController::deactivateAccount(int targetUserId)
{
    const std::optional<User> actingUser = m_authentication.currentUser();
    if (!actingUser.has_value()) {
        m_view.showError(kMessageNotLoggedIn);
        return;
    }
    const OperationResult result = m_userManagement.deactivateAccount(*actingUser, targetUserId);
    handleResult(result.successful, result.errorMessage, kMessageAccountDeactivated);
}

void UserManagementController::activateAccount(int targetUserId)
{
    const std::optional<User> actingUser = m_authentication.currentUser();
    if (!actingUser.has_value()) {
        m_view.showError(kMessageNotLoggedIn);
        return;
    }
    const OperationResult result = m_userManagement.activateAccount(*actingUser, targetUserId);
    handleResult(result.successful, result.errorMessage, kMessageAccountActivated);
}
