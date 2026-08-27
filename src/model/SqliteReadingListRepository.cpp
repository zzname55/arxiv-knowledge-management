#include "model/SqliteReadingListRepository.h"

#include <QSqlError>
#include <QSqlQuery>
#include <QVariant>

#include "model/Database.h"

namespace {

const QString kBaseQuery = QStringLiteral(R"SQL(
SELECT l.id, l.user_id, l.publication_id, l.status, l.rating,
       l.note, l.created_at, l.changed_at, b.displayName, v.title
FROM reading_list_entry l
JOIN user          b ON b.id = l.user_id
JOIN publication v ON v.id = l.publication_id
)SQL");

const QString kOrdering = QStringLiteral("ORDER BY l.created_at DESC, l.id DESC");

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

ReadingListEntry SqliteReadingListRepository::fromQuery(const QSqlQuery &query)
{
    ReadingListEntry entry;
    entry.setId(query.value(0).toInt());
    entry.setUserId(query.value(1).toInt());
    entry.setPublicationId(query.value(2).toInt());

    const std::optional<ReadingStatus> status = readingStatusFromText(query.value(3).toString());
    if (status.has_value()) {
        entry.setStatus(*status);
    }

    const QVariant rating = query.value(4);
    entry.setRating(rating.isNull() ? std::nullopt : std::optional<int>(rating.toInt()));

    entry.setNote(query.value(5).toString());
    entry.setCreatedAt(QDateTime::fromString(query.value(6).toString(), Qt::ISODate));
    entry.setChangedAt(QDateTime::fromString(query.value(7).toString(), Qt::ISODate));
    entry.setUserDisplayName(query.value(8).toString());
    entry.setPublicationTitle(query.value(9).toString());

    return entry;
}

QList<ReadingListEntry> SqliteReadingListRepository::readList(QSqlQuery &query) const
{
    QList<ReadingListEntry> entries;
    if (!query.exec()) {
        m_lastError = query.lastError().text();
        return entries;
    }
    while (query.next()) {
        entries.append(fromQuery(query));
    }
    return entries;
}

std::optional<ReadingListEntry> SqliteReadingListRepository::findById(int id) const
{
    m_lastError.clear();

    QSqlQuery query(m_database.connection());
    query.prepare(kBaseQuery + QStringLiteral("WHERE l.id = :id"));
    query.bindValue(QStringLiteral(":id"), id);

    if (!query.exec()) {
        m_lastError = query.lastError().text();
        return std::nullopt;
    }
    return query.next() ? std::optional<ReadingListEntry>(fromQuery(query)) : std::nullopt;
}

QList<ReadingListEntry> SqliteReadingListRepository::findForUser(int userId) const
{
    m_lastError.clear();

    QSqlQuery query(m_database.connection());
    query.prepare(kBaseQuery + QStringLiteral("WHERE l.user_id = :user_id ") + kOrdering);
    query.bindValue(QStringLiteral(":user_id"), userId);

    return readList(query);
}

QList<ReadingListEntry> SqliteReadingListRepository::findAll() const
{
    m_lastError.clear();

    QSqlQuery query(m_database.connection());
    query.prepare(kBaseQuery + kOrdering);

    return readList(query);
}

QList<ReadingListEntry> SqliteReadingListRepository::findByStatus(ReadingStatus status) const
{
    m_lastError.clear();

    QSqlQuery query(m_database.connection());
    query.prepare(kBaseQuery + QStringLiteral("WHERE l.status = :status ") + kOrdering);
    query.bindValue(QStringLiteral(":status"), readingStatusToText(status));

    return readList(query);
}

bool SqliteReadingListRepository::entryExists(int userId, int publicationId) const
{
    m_lastError.clear();

    QSqlQuery query(m_database.connection());
    query.prepare(QStringLiteral(
        "SELECT 1 FROM reading_list_entry WHERE user_id = :user_id AND publication_id = :publication_id"));
    query.bindValue(QStringLiteral(":user_id"),          userId);
    query.bindValue(QStringLiteral(":publication_id"), publicationId);

    if (!query.exec()) {
        m_lastError = query.lastError().text();
        return false;
    }
    return query.next();
}

bool SqliteReadingListRepository::save(ReadingListEntry &entry)
{
    m_lastError.clear();
    return entry.isPersisted() ? update(entry) : insert(entry);
}

bool SqliteReadingListRepository::insert(ReadingListEntry &entry)
{
    QSqlQuery query(m_database.connection());
    query.prepare(QStringLiteral(
        "INSERT INTO reading_list_entry "
        "(user_id, publication_id, status, rating, note, created_at, changed_at) "
        "VALUES (:user_id, :publication_id, :status, :rating, :note, :created_at, :changed_at)"));

    query.bindValue(QStringLiteral(":user_id"),          entry.userId());
    query.bindValue(QStringLiteral(":publication_id"), entry.publicationId());
    query.bindValue(QStringLiteral(":status"),               readingStatusToText(entry.status()));
    query.bindValue(QStringLiteral(":rating"),
                      entry.rating().has_value() ? QVariant(*entry.rating()) : QVariant());
    query.bindValue(QStringLiteral(":note"),                withoutNullValue(entry.note()));
    query.bindValue(QStringLiteral(":created_at"),  entry.createdAt().toUTC().toString(Qt::ISODate));
    query.bindValue(QStringLiteral(":changed_at"), entry.changedAt().toUTC().toString(Qt::ISODate));

    if (!query.exec()) {
        m_lastError = QStringLiteral("The reading list entry could not be created: %1").arg(query.lastError().text());
        return false;
    }

    entry.setId(query.lastInsertId().toInt());
    return true;
}

bool SqliteReadingListRepository::update(const ReadingListEntry &entry)
{
    QSqlQuery query(m_database.connection());
    query.prepare(QStringLiteral(
        "UPDATE reading_list_entry SET status       = :status, "
        "                             rating    = :rating, "
        "                             note        = :note, "
        "                             changed_at = :changed_at "
        "WHERE id = :id"));

    query.bindValue(QStringLiteral(":status"), readingStatusToText(entry.status()));
    query.bindValue(QStringLiteral(":rating"),
                      entry.rating().has_value() ? QVariant(*entry.rating()) : QVariant());
    query.bindValue(QStringLiteral(":note"), withoutNullValue(entry.note()));
    query.bindValue(QStringLiteral(":changed_at"), entry.changedAt().toUTC().toString(Qt::ISODate));
    query.bindValue(QStringLiteral(":id"), entry.id());

    if (!query.exec()) {
        m_lastError = QStringLiteral("The reading list entry could not be updated: %1").arg(query.lastError().text());
        return false;
    }
    return true;
}

bool SqliteReadingListRepository::remove(int id)
{
    m_lastError.clear();

    QSqlQuery query(m_database.connection());
    query.prepare(QStringLiteral("DELETE FROM reading_list_entry WHERE id = :id"));
    query.bindValue(QStringLiteral(":id"), id);

    if (!query.exec()) {
        m_lastError = QStringLiteral("The reading list entry could not be removed: %1").arg(query.lastError().text());
        return false;
    }
    return true;
}

QString SqliteReadingListRepository::lastError() const
{
    return m_lastError;
}
