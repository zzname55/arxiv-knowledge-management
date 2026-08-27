// ---------------------------------------------------------------------------
// SqliteUserRepository — MVC layer: MODEL
// Umsetzung from UserRepository auf SQLite. Nur gebundene Werte, keine
// zusammengesetzten SQL-Strings (Schutz vor SQL-Einschleusung).
// ---------------------------------------------------------------------------
#pragma once

#include <QString>
#include "model/UserRepository.h"

class Database;
class QSqlQuery;

class SqliteUserRepository : public UserRepository
{
public:
    explicit SqliteUserRepository(Database &database);

    std::optional<User> findByUsername(const QString &username) const override;
    std::optional<User> findById(int id) const override;
    QList<User>         all() const override;
    bool                    save(User &user) override;
    bool                    usernameExists(const QString &username) const override;

    /// Legt ma01/wm01/admin01 an, sofern nicht vorhanden. Mehrfacher Aufruf unschaedlich.
    bool createDefaultUsers();

    QString lastError() const;

    static const QString kDefaultPassword;

private:
    static User fromQuery(const QSqlQuery &query);
    bool insert(User &user);
    bool update(const User &user);

    Database      &m_database;
    mutable QString m_lastError;
};
