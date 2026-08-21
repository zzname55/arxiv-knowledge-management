#include "model/Database.h"

#include <QSqlError>
#include <QSqlQuery>
#include <utility>

const QString Database::kInMemoryPath = QStringLiteral(":memory:");

namespace {

const QString kTabelleBenutzer = QStringLiteral(R"SQL(
CREATE TABLE IF NOT EXISTS user (
    id            INTEGER PRIMARY KEY AUTOINCREMENT,
    username  TEXT    NOT NULL UNIQUE,
    displayName   TEXT    NOT NULL,
    password_hash TEXT    NOT NULL,
    salt          TEXT    NOT NULL,
    role         TEXT    NOT NULL,
    ist_aktiv     INTEGER NOT NULL DEFAULT 1
)
)SQL");

const QString kTabelleVeroeffentlichung = QStringLiteral(R"SQL(
CREATE TABLE IF NOT EXISTS publication (
    id                 INTEGER PRIMARY KEY AUTOINCREMENT,
    arxiv_id           TEXT NOT NULL UNIQUE,
    title              TEXT NOT NULL,
    authors            TEXT NOT NULL,
    summary    TEXT NOT NULL,
    arxiv_kategorie    TEXT NOT NULL,
    discipline          TEXT NOT NULL,
    veroeffentlicht_am TEXT NOT NULL,
    url                TEXT NOT NULL
)
)SQL");

const QString kTabelleLeselisteneintrag = QStringLiteral(R"SQL(
CREATE TABLE IF NOT EXISTS leselisteneintrag (
    id                   INTEGER PRIMARY KEY AUTOINCREMENT,
    user_id          INTEGER NOT NULL REFERENCES user(id)          ON DELETE CASCADE,
    publication_id INTEGER NOT NULL REFERENCES publication(id) ON DELETE CASCADE,
    status               TEXT    NOT NULL,
    rating            INTEGER,
    note                TEXT    NOT NULL DEFAULT '',
    erstellt_am          TEXT    NOT NULL,
    geaendert_am         TEXT    NOT NULL,
    UNIQUE(user_id, publication_id)
)
)SQL");

const QString kIndexVeroeffentlichung = QStringLiteral(
    "CREATE INDEX IF NOT EXISTS idx_publication_discipline_date "
    "ON publication (discipline, veroeffentlicht_am DESC)");

const QString kIndexLeseliste = QStringLiteral(
    "CREATE INDEX IF NOT EXISTS idx_leselisteneintrag_user "
    "ON leselisteneintrag (user_id)");

} // namespace

Database::Database(QString verbindungsname)
    : m_connectionName(std::move(verbindungsname))
{
}

Database::~Database()
{
    {
        QSqlDatabase database = QSqlDatabase::database(m_connectionName, false);
        if (database.isOpen()) {
            database.close();
        }
    }
    if (QSqlDatabase::contains(m_connectionName)) {
        QSqlDatabase::removeDatabase(m_connectionName);
    }
}

bool Database::open(const QString &dateipfad)
{
    m_lastError.clear();

    QSqlDatabase database = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), m_connectionName);
    database.setDatabaseName(dateipfad);

    if (!database.open()) {
        m_lastError = QStringLiteral("Die Database \"%1\" konnte nicht geoeffnet werden: %2")
                              .arg(dateipfad, database.lastError().text());
        return false;
    }

    if (!execute(QStringLiteral("PRAGMA foreign_keys = ON"))) {
        return false;
    }

    return true;
}

bool Database::isOpen() const
{
    return QSqlDatabase::database(m_connectionName, false).isOpen();
}

QSqlDatabase Database::connection() const
{
    return QSqlDatabase::database(m_connectionName, false);
}

bool Database::createSchema()
{
    m_lastError.clear();

    const QStringList anweisungen = {
        kTabelleBenutzer,
        kTabelleVeroeffentlichung,
        kTabelleLeselisteneintrag,
        kIndexVeroeffentlichung,
        kIndexLeseliste
    };

    for (const QString &anweisung : anweisungen) {
        if (!execute(anweisung)) {
            return false;
        }
    }

    return true;
}

QString Database::lastError() const
{
    return m_lastError;
}

bool Database::execute(const QString &sqlAnweisung)
{
    QSqlQuery abfrage(connection());
    if (!abfrage.exec(sqlAnweisung)) {
        m_lastError = QStringLiteral("SQL-Anweisung fehlgeschlagen: %1")
                              .arg(abfrage.lastError().text());
        return false;
    }
    return true;
}
