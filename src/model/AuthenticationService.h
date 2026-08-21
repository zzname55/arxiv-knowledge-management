// ---------------------------------------------------------------------------
// AuthenticationService — MVC layer: MODEL
// An-/Abmeldung, haelt die Sitzung. Kennt keine Widgets.
// ---------------------------------------------------------------------------
#pragma once

#include <QString>
#include <optional>
#include "model/User.h"

class UserRepository;

struct AnmeldeErgebnis {
    bool     successful = false;
    QString  errorMessage;
    User user;
};

class AuthenticationService
{
public:
    explicit AuthenticationService(UserRepository &repository);

    AnmeldeErgebnis login(const QString &username, const QString &password);
    void logout();
    bool isLoggedIn() const;
    std::optional<User> currentUser() const;

    static const QString kMessageInvalidCredentials;
    static const QString kMessageRequiredFieldMissing;
    static const QString kMessageAccountDeactivated;

private:
    UserRepository     &m_repository;
    std::optional<User> m_currentUser;
};
