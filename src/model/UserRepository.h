// ---------------------------------------------------------------------------
// UserRepository — MVC layer: MODEL
// Rein virtuelle Schnittstelle zur Datenhaltung der Benutzerkonten.
// ---------------------------------------------------------------------------
#pragma once

#include <QList>
#include <QString>
#include <optional>
#include "model/User.h"

class UserRepository
{
public:
    virtual ~UserRepository() = default;

    virtual std::optional<User> findByUsername(const QString &username) const = 0;
    virtual std::optional<User> findById(int id) const = 0;
    virtual QList<User> all() const = 0;
    virtual bool save(User &user) = 0;
    virtual bool usernameExists(const QString &username) const = 0;
};
