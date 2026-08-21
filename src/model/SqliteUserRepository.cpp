#include "model/SqliteUserRepository.h"

#include <QSqlError>
#include <QSqlQuery>

#include "model/Database.h"
#include "model/PasswordHasher.h"

const QString SqliteUserRepository::kDefaultPassword = QStringLiteral("start1234");

namespace {

const QString kColumns = QStringLiteral(
    "id, username, displayName, password_hash, salt, role, ist_aktiv");

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

User SqliteUserRepository::fromQuery(const QSqlQuery &abfrage)
{
    User user;
    user.setzeId(abfrage.value(0).toInt());
    user.setzeBenutzername(abfrage.value(1).toString());
    user.setzeAnzeigename(abfrage.value(2).toString());
    user.setzePasswortHash(abfrage.value(3).toString());
    user.setzeSalt(abfrage.value(4).toString());

    const std::optional<UserRole> role = roleFromText(abfrage.value(5).toString());
    if (role.has_value()) {
        user.setzeRolle(*role);
    }

    user.setActive(abfrage.value(6).toInt() != 0);
    return user;
}

std::optional<User> SqliteUserRepository::findByUsername(const QString &username) const
{
    m_lastError.clear();

    QSqlQuery abfrage(m_database.connection());
    abfrage.prepare(QStringLiteral("SELECT %1 FROM user WHERE username = :username").arg(kColumns));
    abfrage.bindValue(QStringLiteral(":username"), username);

    if (!abfrage.exec()) {
        m_lastError = abfrage.lastError().text();
        return std::nullopt;
    }
    return abfrage.next() ? std::optional<User>(fromQuery(abfrage)) : std::nullopt;
}

std::optional<User> SqliteUserRepository::findById(int id) const
{
    m_lastError.clear();

    QSqlQuery abfrage(m_database.connection());
    abfrage.prepare(QStringLiteral("SELECT %1 FROM user WHERE id = :id").arg(kColumns));
    abfrage.bindValue(QStringLiteral(":id"), id);

    if (!abfrage.exec()) {
        m_lastError = abfrage.lastError().text();
        return std::nullopt;
    }
    return abfrage.next() ? std::optional<User>(fromQuery(abfrage)) : std::nullopt;
}

QList<User> SqliteUserRepository::all() const
{
    m_lastError.clear();
    QList<User> userList;

    QSqlQuery abfrage(m_database.connection());
    if (!abfrage.exec(QStringLiteral("SELECT %1 FROM user ORDER BY username").arg(kColumns))) {
        m_lastError = abfrage.lastError().text();
        return userList;
    }
    while (abfrage.next()) {
        userList.append(fromQuery(abfrage));
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
    QSqlQuery abfrage(m_database.connection());
    abfrage.prepare(QStringLiteral(
        "INSERT INTO user (username, displayName, password_hash, salt, role, ist_aktiv) "
        "VALUES (:username, :displayName, :password_hash, :salt, :role, :ist_aktiv)"));

    abfrage.bindValue(QStringLiteral(":username"),  user.username());
    abfrage.bindValue(QStringLiteral(":displayName"),   user.displayName());
    abfrage.bindValue(QStringLiteral(":password_hash"), user.passwortHash());
    abfrage.bindValue(QStringLiteral(":salt"),          user.salt());
    abfrage.bindValue(QStringLiteral(":role"),         roleToText(user.role()));
    abfrage.bindValue(QStringLiteral(":ist_aktiv"),     user.isActive() ? 1 : 0);

    if (!abfrage.exec()) {
        m_lastError = QStringLiteral("Der User \"%1\" konnte nicht angelegt werden: %2")
                              .arg(user.username(), abfrage.lastError().text());
        return false;
    }

    user.setzeId(abfrage.lastInsertId().toInt());
    return true;
}

bool SqliteUserRepository::update(const User &user)
{
    QSqlQuery abfrage(m_database.connection());
    abfrage.prepare(QStringLiteral(
        "UPDATE user SET username  = :username, "
        "                    displayName   = :displayName, "
        "                    password_hash = :password_hash, "
        "                    salt          = :salt, "
        "                    role         = :role, "
        "                    ist_aktiv     = :ist_aktiv "
        "WHERE id = :id"));

    abfrage.bindValue(QStringLiteral(":username"),  user.username());
    abfrage.bindValue(QStringLiteral(":displayName"),   user.displayName());
    abfrage.bindValue(QStringLiteral(":password_hash"), user.passwortHash());
    abfrage.bindValue(QStringLiteral(":salt"),          user.salt());
    abfrage.bindValue(QStringLiteral(":role"),         roleToText(user.role()));
    abfrage.bindValue(QStringLiteral(":ist_aktiv"),     user.isActive() ? 1 : 0);
    abfrage.bindValue(QStringLiteral(":id"),            user.id());

    if (!abfrage.exec()) {
        m_lastError = QStringLiteral("Der User \"%1\" konnte nicht geaendert werden: %2")
                              .arg(user.username(), abfrage.lastError().text());
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
        user.setzeBenutzername(konto.username);
        user.setzeAnzeigename(konto.displayName);
        user.setzeRolle(konto.role);
        user.setzeSalt(salt);
        user.setzePasswortHash(PasswordHasher::hash(kDefaultPassword, salt));
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
