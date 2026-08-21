#include "model/SqliteReadingListRepository.h"

#include <QSqlError>
#include <QSqlQuery>
#include <QVariant>

#include "model/Database.h"

namespace {

const QString kBaseQuery = QStringLiteral(R"SQL(
SELECT l.id, l.user_id, l.publication_id, l.status, l.rating,
       l.note, l.erstellt_am, l.geaendert_am, b.displayName, v.title
FROM leselisteneintrag l
JOIN user          b ON b.id = l.user_id
JOIN publication v ON v.id = l.publication_id
)SQL");

const QString kOrdering = QStringLiteral("ORDER BY l.erstellt_am DESC, l.id DESC");

/// Wandelt eine "null"-Zeichenkette in eine leere um. Ein default-konstruiertes
/// QString ist NULL, nicht leer, und wuerde als SQL-NULL gebunden werden --
/// das verletzt die NOT-NULL-Bedingung der column "note" statt deren DEFAULT ''
/// zu ziehen. Fachlich ist "keine Note" eine leere Note, kein fehlender Wert.
QString withoutNullValue(const QString &text)
{
    return text.isNull() ? QString::fromLatin1("") : text;
}

} // namespace

SqliteReadingListRepository::SqliteReadingListRepository(Database &database)
    : m_database(database)
{
}

ReadingListEntry SqliteReadingListRepository::fromQuery(const QSqlQuery &abfrage)
{
    ReadingListEntry entry;
    entry.setzeId(abfrage.value(0).toInt());
    entry.setzeBenutzerId(abfrage.value(1).toInt());
    entry.setzeVeroeffentlichungId(abfrage.value(2).toInt());

    const std::optional<ReadingStatus> status = readingStatusFromText(abfrage.value(3).toString());
    if (status.has_value()) {
        entry.setzeStatus(*status);
    }

    const QVariant rating = abfrage.value(4);
    entry.setzeBewertung(rating.isNull() ? std::nullopt : std::optional<int>(rating.toInt()));

    entry.setzeNotiz(abfrage.value(5).toString());
    entry.setzeErstelltAm(QDateTime::fromString(abfrage.value(6).toString(), Qt::ISODate));
    entry.setzeGeaendertAm(QDateTime::fromString(abfrage.value(7).toString(), Qt::ISODate));
    entry.setzeBenutzerAnzeigename(abfrage.value(8).toString());
    entry.setzeVeroeffentlichungTitel(abfrage.value(9).toString());

    return entry;
}

QList<ReadingListEntry> SqliteReadingListRepository::readList(QSqlQuery &abfrage) const
{
    QList<ReadingListEntry> entries;
    if (!abfrage.exec()) {
        m_lastError = abfrage.lastError().text();
        return entries;
    }
    while (abfrage.next()) {
        entries.append(fromQuery(abfrage));
    }
    return entries;
}

std::optional<ReadingListEntry> SqliteReadingListRepository::findById(int id) const
{
    m_lastError.clear();

    QSqlQuery abfrage(m_database.connection());
    abfrage.prepare(kBaseQuery + QStringLiteral("WHERE l.id = :id"));
    abfrage.bindValue(QStringLiteral(":id"), id);

    if (!abfrage.exec()) {
        m_lastError = abfrage.lastError().text();
        return std::nullopt;
    }
    return abfrage.next() ? std::optional<ReadingListEntry>(fromQuery(abfrage)) : std::nullopt;
}

QList<ReadingListEntry> SqliteReadingListRepository::findForUser(int userId) const
{
    m_lastError.clear();

    QSqlQuery abfrage(m_database.connection());
    abfrage.prepare(kBaseQuery + QStringLiteral("WHERE l.user_id = :user_id ") + kOrdering);
    abfrage.bindValue(QStringLiteral(":user_id"), userId);

    return readList(abfrage);
}

QList<ReadingListEntry> SqliteReadingListRepository::findAll() const
{
    m_lastError.clear();

    QSqlQuery abfrage(m_database.connection());
    abfrage.prepare(kBaseQuery + kOrdering);

    return readList(abfrage);
}

QList<ReadingListEntry> SqliteReadingListRepository::findByStatus(ReadingStatus status) const
{
    m_lastError.clear();

    QSqlQuery abfrage(m_database.connection());
    abfrage.prepare(kBaseQuery + QStringLiteral("WHERE l.status = :status ") + kOrdering);
    abfrage.bindValue(QStringLiteral(":status"), readingStatusToText(status));

    return readList(abfrage);
}

bool SqliteReadingListRepository::entryExists(int userId, int publicationId) const
{
    m_lastError.clear();

    QSqlQuery abfrage(m_database.connection());
    abfrage.prepare(QStringLiteral(
        "SELECT 1 FROM leselisteneintrag WHERE user_id = :user_id AND publication_id = :publication_id"));
    abfrage.bindValue(QStringLiteral(":user_id"),          userId);
    abfrage.bindValue(QStringLiteral(":publication_id"), publicationId);

    if (!abfrage.exec()) {
        m_lastError = abfrage.lastError().text();
        return false;
    }
    return abfrage.next();
}

bool SqliteReadingListRepository::save(ReadingListEntry &entry)
{
    m_lastError.clear();
    return entry.isPersisted() ? update(entry) : insert(entry);
}

bool SqliteReadingListRepository::insert(ReadingListEntry &entry)
{
    QSqlQuery abfrage(m_database.connection());
    abfrage.prepare(QStringLiteral(
        "INSERT INTO leselisteneintrag "
        "(user_id, publication_id, status, rating, note, erstellt_am, geaendert_am) "
        "VALUES (:user_id, :publication_id, :status, :rating, :note, :erstellt_am, :geaendert_am)"));

    abfrage.bindValue(QStringLiteral(":user_id"),          entry.userId());
    abfrage.bindValue(QStringLiteral(":publication_id"), entry.publicationId());
    abfrage.bindValue(QStringLiteral(":status"),               readingStatusToText(entry.status()));
    abfrage.bindValue(QStringLiteral(":rating"),
                      entry.rating().has_value() ? QVariant(*entry.rating()) : QVariant());
    abfrage.bindValue(QStringLiteral(":note"),                withoutNullValue(entry.note()));
    abfrage.bindValue(QStringLiteral(":erstellt_am"),  entry.createdAt().toUTC().toString(Qt::ISODate));
    abfrage.bindValue(QStringLiteral(":geaendert_am"), entry.changedAt().toUTC().toString(Qt::ISODate));

    if (!abfrage.exec()) {
        m_lastError = QStringLiteral("Der Leselisteneintrag konnte nicht angelegt werden: %1").arg(abfrage.lastError().text());
        return false;
    }

    entry.setzeId(abfrage.lastInsertId().toInt());
    return true;
}

bool SqliteReadingListRepository::update(const ReadingListEntry &entry)
{
    QSqlQuery abfrage(m_database.connection());
    abfrage.prepare(QStringLiteral(
        "UPDATE leselisteneintrag SET status       = :status, "
        "                             rating    = :rating, "
        "                             note        = :note, "
        "                             geaendert_am = :geaendert_am "
        "WHERE id = :id"));

    abfrage.bindValue(QStringLiteral(":status"), readingStatusToText(entry.status()));
    abfrage.bindValue(QStringLiteral(":rating"),
                      entry.rating().has_value() ? QVariant(*entry.rating()) : QVariant());
    abfrage.bindValue(QStringLiteral(":note"), withoutNullValue(entry.note()));
    abfrage.bindValue(QStringLiteral(":geaendert_am"), entry.changedAt().toUTC().toString(Qt::ISODate));
    abfrage.bindValue(QStringLiteral(":id"), entry.id());

    if (!abfrage.exec()) {
        m_lastError = QStringLiteral("Der Leselisteneintrag konnte nicht geaendert werden: %1").arg(abfrage.lastError().text());
        return false;
    }
    return true;
}

bool SqliteReadingListRepository::remove(int id)
{
    m_lastError.clear();

    QSqlQuery abfrage(m_database.connection());
    abfrage.prepare(QStringLiteral("DELETE FROM leselisteneintrag WHERE id = :id"));
    abfrage.bindValue(QStringLiteral(":id"), id);

    if (!abfrage.exec()) {
        m_lastError = QStringLiteral("Der Leselisteneintrag konnte nicht entfernt werden: %1").arg(abfrage.lastError().text());
        return false;
    }
    return true;
}

QString SqliteReadingListRepository::lastError() const
{
    return m_lastError;
}
