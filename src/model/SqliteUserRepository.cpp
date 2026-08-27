#include "model/SqliteUserRepository.h"

#include <QSqlError>
#include <QSqlQuery>

#include "model/Database.h"
#include "model/PasswordHasher.h"

const QString SqliteUserRepository::kDefaultPassword = QStringLiteral("start1234");

namespace {

const QString kColumns = QStringLiteral(
    "id, username, displayName, password_hash, salt, role, is_active");

struct Standardkonto {
    QString       username;
    QString       displayName;
    UserRole role;
};

const QList<Standardkonto> kStandardkonten = {
    { QStringLiteral("ma01"),    QStringLiteral("Max Mustermann"), UserRole::Employee },
    { QStringLiteral("wm01"),    QStringLiteral("Wera Weiss"),     UserRole::KnowledgeManager },
    { QStringLiteral("admin01"), QStringLiteral("Anna Adler"),     UserRole::Administrator }
};

} // namespace

SqliteUserRepository::SqliteUserRepository(Database &database)
    : m_database(database)
{
}

User SqliteUserRepository::fromQuery(const QSqlQuery &query)
{
    User user;
    user.setId(query.value(0).toInt());
    user.setUsername(query.value(1).toString());
    user.setDisplayName(query.value(2).toString());
    user.setPasswordHash(query.value(3).toString());
    user.setSalt(query.value(4).toString());

    const std::optional<UserRole> role = roleFromText(query.value(5).toString());
    if (role.has_value()) {
        user.setRole(*role);
    }

    user.setActive(query.value(6).toInt() != 0);
    return user;
}

std::optional<User> SqliteUserRepository::findByUsername(const QString &username) const
{
    m_lastError.clear();

    QSqlQuery query(m_database.connection());
    query.prepare(QStringLiteral("SELECT %1 FROM user WHERE username = :username").arg(kColumns));
    query.bindValue(QStringLiteral(":username"), username);

    if (!query.exec()) {
        m_lastError = query.lastError().text();
        return std::nullopt;
    }
    return query.next() ? std::optional<User>(fromQuery(query)) : std::nullopt;
}

std::optional<User> SqliteUserRepository::findById(int id) const
{
    m_lastError.clear();

    QSqlQuery query(m_database.connection());
    query.prepare(QStringLiteral("SELECT %1 FROM user WHERE id = :id").arg(kColumns));
    query.bindValue(QStringLiteral(":id"), id);

    if (!query.exec()) {
        m_lastError = query.lastError().text();
        return std::nullopt;
    }
    return query.next() ? std::optional<User>(fromQuery(query)) : std::nullopt;
}

QList<User> SqliteUserRepository::all() const
{
    m_lastError.clear();
    QList<User> userList;

    QSqlQuery query(m_database.connection());
    if (!query.exec(QStringLiteral("SELECT %1 FROM user ORDER BY username").arg(kColumns))) {
        m_lastError = query.lastError().text();
        return userList;
    }
    while (query.next()) {
        userList.append(fromQuery(query));
    }
    return userList;
}

bool SqliteUserRepository::save(User &user)
{
    m_lastError.clear();
    return user.isPersisted() ? update(user) : insert(user);
}

bool SqliteUserRepository::insert(User &user)
{
    QSqlQuery query(m_database.connection());
    query.prepare(QStringLiteral(
        "INSERT INTO user (username, displayName, password_hash, salt, role, is_active) "
        "VALUES (:username, :displayName, :password_hash, :salt, :role, :is_active)"));

    query.bindValue(QStringLiteral(":username"),  user.username());
    query.bindValue(QStringLiteral(":displayName"),   user.displayName());
    query.bindValue(QStringLiteral(":password_hash"), user.passwordHash());
    query.bindValue(QStringLiteral(":salt"),          user.salt());
    query.bindValue(QStringLiteral(":role"),         roleToText(user.role()));
    query.bindValue(QStringLiteral(":is_active"),     user.isActive() ? 1 : 0);

    if (!query.exec()) {
        m_lastError = QStringLiteral("The user \"%1\" could not be created: %2")
                              .arg(user.username(), query.lastError().text());
        return false;
    }

    user.setId(query.lastInsertId().toInt());
    return true;
}

bool SqliteUserRepository::update(const User &user)
{
    QSqlQuery query(m_database.connection());
    query.prepare(QStringLiteral(
        "UPDATE user SET username  = :username, "
        "                    displayName   = :displayName, "
        "                    password_hash = :password_hash, "
        "                    salt          = :salt, "
        "                    role         = :role, "
        "                    is_active     = :is_active "
        "WHERE id = :id"));

    query.bindValue(QStringLiteral(":username"),  user.username());
    query.bindValue(QStringLiteral(":displayName"),   user.displayName());
    query.bindValue(QStringLiteral(":password_hash"), user.passwordHash());
    query.bindValue(QStringLiteral(":salt"),          user.salt());
    query.bindValue(QStringLiteral(":role"),         roleToText(user.role()));
    query.bindValue(QStringLiteral(":is_active"),     user.isActive() ? 1 : 0);
    query.bindValue(QStringLiteral(":id"),            user.id());

    if (!query.exec()) {
        m_lastError = QStringLiteral("The user \"%1\" could not be updated: %2")
                              .arg(user.username(), query.lastError().text());
        return false;
    }
    return true;
}

bool SqliteUserRepository::usernameExists(const QString &username) const
{
    return findByUsername(username).has_value();
}

bool SqliteUserRepository::createDefaultUsers()
{
    m_lastError.clear();

    for (const Standardkonto &konto : kStandardkonten) {
        if (usernameExists(konto.username)) {
            continue;
        }

        const QString salt = PasswordHasher::generateSalt();

        User user;
        user.setUsername(konto.username);
        user.setDisplayName(konto.displayName);
        user.setRole(konto.role);
        user.setSalt(salt);
        user.setPasswordHash(PasswordHasher::hash(kDefaultPassword, salt));
        user.setActive(true);

        if (!insert(user)) {
            return false;
        }
    }
    return true;
}

QString SqliteUserRepository::lastError() const
{
    return m_lastError;
}
