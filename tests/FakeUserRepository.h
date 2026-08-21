// ---------------------------------------------------------------------------
// Testdoppel fuer UserRepository. Haelt User im Arbeitsspeicher.
// ---------------------------------------------------------------------------
#pragma once

#include <QHash>
#include <QList>
#include <QString>
#include "model/User.h"
#include "model/UserRepository.h"

class FakeUserRepository : public UserRepository
{
public:
    void fuegeEinFuerTest(const User &user)
    {
        m_benutzerNachId.insert(user.id(), user);
    }

    std::optional<User> findByUsername(const QString &username) const override
    {
        for (const User &user : m_benutzerNachId) {
            if (user.username() == username) {
                return user;
            }
        }
        return std::nullopt;
    }

    std::optional<User> findById(int id) const override
    {
        const auto treffer = m_benutzerNachId.constFind(id);
        return treffer == m_benutzerNachId.constEnd() ? std::nullopt : std::optional<User>(*treffer);
    }

    QList<User> all() const override { return m_benutzerNachId.values(); }

    bool save(User &user) override
    {
        if (!user.isPersisted()) {
            user.setzeId(m_naechsteId++);
        }
        m_benutzerNachId.insert(user.id(), user);
        return true;
    }

    bool usernameExists(const QString &username) const override
    {
        return findByUsername(username).has_value();
    }

private:
    QHash<int, User> m_benutzerNachId;
    int                  m_naechsteId = 1;
};
