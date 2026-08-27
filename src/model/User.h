// ---------------------------------------------------------------------------
// User — MVC layer: MODEL
//
// Ein Benutzerkonto. Reiner Datentraeger; Password nur als Hash+Salt (AK-01.8).
// ---------------------------------------------------------------------------
#pragma once

#include <QString>
#include "model/UserRole.h"

class User
{
public:
    User() = default;

    int id() const { return m_id; }
    void setId(int id) { m_id = id; }

    QString username() const { return m_username; }
    void setUsername(const QString &username) { m_username = username; }

    QString displayName() const { return m_anzeigename; }
    void setDisplayName(const QString &displayName) { m_anzeigename = displayName; }

    UserRole role() const { return m_role; }
    void setRole(UserRole role) { m_role = role; }

    QString passwordHash() const { return m_passwortHash; }
    void setPasswordHash(const QString &hash) { m_passwortHash = hash; }

    QString salt() const { return m_salt; }
    void setSalt(const QString &salt) { m_salt = salt; }

    bool isActive() const { return m_istAktiv; }
    void setActive(bool aktiv) { m_istAktiv = aktiv; }

    bool isPersisted() const { return m_id != kNoId; }

    static constexpr int kNoId = 0;

private:
    int           m_id           = kNoId;
    QString       m_username;
    QString       m_anzeigename;
    QString       m_passwortHash;
    QString       m_salt;
    UserRole m_role        = UserRole::Employee;
    bool          m_istAktiv     = true;
};
