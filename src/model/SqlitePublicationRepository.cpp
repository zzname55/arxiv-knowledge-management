#include "model/SqlitePublicationRepository.h"

#include <QSqlError>
#include <QSqlQuery>

#include "model/Database.h"

const QString SqlitePublicationRepository::kAuthorSeparator = QStringLiteral(" | ");

namespace {

const QString kColumns = QStringLiteral(
    "id, arxiv_id, title, authors, summary, arxiv_category, "
    "discipline, published_at, url");

const QString kOrdering = QStringLiteral("ORDER BY published_at DESC, id DESC");

} // namespace

SqlitePublicationRepository::SqlitePublicationRepository(Database &database)
    : m_database(database)
{
}

Publication SqlitePublicationRepository::fromQuery(const QSqlQuery &query)
{
    Publication publication;
    publication.setId(query.value(0).toInt());
    publication.setArxivId(query.value(1).toString());
    publication.setTitle(query.value(2).toString());

    const QString authorsAsText = query.value(3).toString();
    publication.setAuthors(authorsAsText.split(kAuthorSeparator, Qt::SkipEmptyParts));

    publication.setSummary(query.value(4).toString());
    publication.setArxivCategory(query.value(5).toString());

    const std::optional<Discipline> discipline = disciplineFromText(query.value(6).toString());
    publication.setDiscipline(discipline.value_or(Discipline::Other));

    publication.setPublishedAt(QDateTime::fromString(query.value(7).toString(), Qt::ISODate));
    publication.setUrl(query.value(8).toString());

    return publication;
}

std::optional<Publication> SqlitePublicationRepository::findById(int id) const
{
    m_lastError.clear();

    QSqlQuery query(m_database.connection());
    query.prepare(QStringLiteral("SELECT %1 FROM publication WHERE id = :id").arg(kColumns));
    query.bindValue(QStringLiteral(":id"), id);

    if (!query.exec()) {
        m_lastError = query.lastError().text();
        return std::nullopt;
    }
    return query.next() ? std::optional<Publication>(fromQuery(query)) : std::nullopt;
}

std::optional<Publication> SqlitePublicationRepository::findByArxivId(const QString &arxivId) const
{
    m_lastError.clear();

    QSqlQuery query(m_database.connection());
    query.prepare(QStringLiteral("SELECT %1 FROM publication WHERE arxiv_id = :arxiv_id").arg(kColumns));
    query.bindValue(QStringLiteral(":arxiv_id"), arxivId);

    if (!query.exec()) {
        m_lastError = query.lastError().text();
        return std::nullopt;
    }
    return query.next() ? std::optional<Publication>(fromQuery(query)) : std::nullopt;
}

QList<Publication> SqlitePublicationRepository::findByDiscipline(Discipline discipline, int maxCount) const
{
    m_lastError.clear();
    QList<Publication> publications;

    const bool mitFilter = (discipline != Discipline::Alle);
    const bool mitGrenze = (maxCount != kUnlimited);

    QString statement = QStringLiteral("SELECT %1 FROM publication ").arg(kColumns);
    if (mitFilter) {
        statement += QStringLiteral("WHERE discipline = :discipline ");
    }
    statement += kOrdering;
    if (mitGrenze) {
        statement += QStringLiteral(" LIMIT :max_count");
    }

    QSqlQuery query(m_database.connection());
    query.prepare(statement);
    if (mitFilter) {
        query.bindValue(QStringLiteral(":discipline"), disciplineToText(discipline));
    }
    if (mitGrenze) {
        query.bindValue(QStringLiteral(":max_count"), maxCount);
    }

    if (!query.exec()) {
        m_lastError = query.lastError().text();
        return publications;
    }
    while (query.next()) {
        publications.append(fromQuery(query));
    }
    return publications;
}

bool SqlitePublicationRepository::save(Publication &publication)
{
    m_lastError.clear();

    if (!publication.isPersisted()) {
        const std::optional<Publication> bereitsBekannt = findByArxivId(publication.arxivId());
        if (bereitsBekannt.has_value()) {
            publication.setId(bereitsBekannt->id());
        }
    }

    return publication.isPersisted() ? update(publication) : insert(publication);
}

bool SqlitePublicationRepository::insert(Publication &publication)
{
    QSqlQuery query(m_database.connection());
    query.prepare(QStringLiteral(
        "INSERT INTO publication "
        "(arxiv_id, title, authors, summary, arxiv_category, discipline, published_at, url) "
        "VALUES (:arxiv_id, :title, :authors, :summary, :arxiv_category, :discipline, :published_at, :url)"));

    query.bindValue(QStringLiteral(":arxiv_id"),        publication.arxivId());
    query.bindValue(QStringLiteral(":title"),           publication.title());
    query.bindValue(QStringLiteral(":authors"),         publication.authors().join(kAuthorSeparator));
    query.bindValue(QStringLiteral(":summary"), publication.summary());
    query.bindValue(QStringLiteral(":arxiv_category"), publication.arxivCategory());
    query.bindValue(QStringLiteral(":discipline"),       disciplineToText(publication.discipline()));
    query.bindValue(QStringLiteral(":published_at"), publication.publishedAt().toUTC().toString(Qt::ISODate));
    query.bindValue(QStringLiteral(":url"),             publication.url());

    if (!query.exec()) {
        m_lastError = QStringLiteral("The publication \"%1\" could not be saved: %2")
                              .arg(publication.arxivId(), query.lastError().text());
        return false;
    }

    publication.setId(query.lastInsertId().toInt());
    return true;
}

bool SqlitePublicationRepository::update(const Publication &publication)
{
    QSqlQuery query(m_database.connection());
    query.prepare(QStringLiteral(
        "UPDATE publication SET title              = :title, "
        "                             authors            = :authors, "
        "                             summary    = :summary, "
        "                             arxiv_category    = :arxiv_category, "
        "                             discipline          = :discipline, "
        "                             published_at = :published_at, "
        "                             url                = :url "
        "WHERE id = :id"));

    query.bindValue(QStringLiteral(":title"),           publication.title());
    query.bindValue(QStringLiteral(":authors"),         publication.authors().join(kAuthorSeparator));
    query.bindValue(QStringLiteral(":summary"), publication.summary());
    query.bindValue(QStringLiteral(":arxiv_category"), publication.arxivCategory());
    query.bindValue(QStringLiteral(":discipline"),       disciplineToText(publication.discipline()));
    query.bindValue(QStringLiteral(":published_at"), publication.publishedAt().toUTC().toString(Qt::ISODate));
    query.bindValue(QStringLiteral(":url"),             publication.url());
    query.bindValue(QStringLiteral(":id"),              publication.id());

    if (!query.exec()) {
        m_lastError = QStringLiteral("The publication \"%1\" could not be updated: %2")
                              .arg(publication.arxivId(), query.lastError().text());
        return false;
    }
    return true;
}

int SqlitePublicationRepository::count() const
{
    m_lastError.clear();

    QSqlQuery query(m_database.connection());
    if (!query.exec(QStringLiteral("SELECT COUNT(*) FROM publication")) || !query.next()) {
        m_lastError = query.lastError().text();
        return 0;
    }
    return query.value(0).toInt();
}

int SqlitePublicationRepository::migrateOutdatedDisciplines()
{
    m_lastError.clear();

    // Collect first, write afterwards: an UPDATE issued while a SELECT on the
    // same table is still open is not reliable on SQLite.
    struct Correction {
        int        id;
        Discipline discipline;
    };
    QList<Correction> corrections;

    QSqlQuery read(m_database.connection());
    if (!read.exec(QStringLiteral("SELECT id, arxiv_category, discipline FROM publication"))) {
        m_lastError = QStringLiteral("The disciplines could not be read: %1").arg(read.lastError().text());
        return -1;
    }

    while (read.next()) {
        // A resolvable label is valid and stays untouched. Only what can no
        // longer be mapped originates from an older naming scheme.
        if (disciplineFromText(read.value(2).toString()).has_value()) {
            continue;
        }
        corrections.append({ read.value(0).toInt(),
                             disciplineFromArxivCategory(read.value(1).toString()) });
    }

    for (const Correction &correction : corrections) {
        QSqlQuery write(m_database.connection());
        write.prepare(QStringLiteral("UPDATE publication SET discipline = :discipline WHERE id = :id"));
        write.bindValue(QStringLiteral(":discipline"), disciplineToText(correction.discipline));
        write.bindValue(QStringLiteral(":id"),         correction.id);

        if (!write.exec()) {
            m_lastError = QStringLiteral("The discipline of row %1 could not be corrected: %2")
                              .arg(QString::number(correction.id), write.lastError().text());
            return -1;
        }
    }

    return static_cast<int>(corrections.size());
}

QString SqlitePublicationRepository::lastError() const
{
    return m_lastError;
}
