#include "model/AuthenticationService.h"

#include "model/UserRepository.h"
#include "model/PasswordHasher.h"

const QString AuthenticationService::kMessageInvalidCredentials =
    QStringLiteral("Username or password is incorrect.");
const QString AuthenticationService::kMessageRequiredFieldMissing =
    QStringLiteral("Please enter a username and password.");
const QString AuthenticationService::kMessageAccountDeactivated =
    QStringLiteral("This account is deactivated.");

AuthenticationService::AuthenticationService(UserRepository &repository)
    : m_repository(repository)
{
}

AnmeldeErgebnis AuthenticationService::login(const QString &username, const QString &password)
{
    AnmeldeErgebnis result;

    if (username.isEmpty() || password.isEmpty()) {
        result.errorMessage = kMessageRequiredFieldMissing;
        return result;
    }

    const std::optional<User> found = m_repository.findByUsername(username);
    if (!found.has_value()) {
        result.errorMessage = kMessageInvalidCredentials;
        return result;
    }

    if (!PasswordHasher::verify(password, found->salt(), found->passwordHash())) {
        result.errorMessage = kMessageInvalidCredentials;
        return result;
    }

    if (!found->isActive()) {
        result.errorMessage = kMessageAccountDeactivated;
        return result;
    }

    m_currentUser = found;
    result.successful = true;
    result.user = *found;
    return result;
}

void AuthenticationService::logout()
{
    m_currentUser.reset();
}

bool AuthenticationService::isLoggedIn() const
{
    return m_currentUser.has_value();
}

std::optional<User> AuthenticationService::currentUser() const
{
    return m_currentUser;
}
