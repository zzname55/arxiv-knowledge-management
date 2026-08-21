#include "model/SqlitePublicationRepository.h"

#include <QSqlError>
#include <QSqlQuery>

#include "model/Database.h"

const QString SqlitePublicationRepository::kAuthorSeparator = QStringLiteral(" | ");

namespace {

const QString kColumns = QStringLiteral(
    "id, arxiv_id, title, authors, summary, arxiv_kategorie, "
    "discipline, veroeffentlicht_am, url");

const QString kOrdering = QStringLiteral("ORDER BY veroeffentlicht_am DESC, id DESC");

} // namespace

SqlitePublicationRepository::SqlitePublicationRepository(Database &database)
    : m_database(database)
{
}

Publication SqlitePublicationRepository::fromQuery(const QSqlQuery &abfrage)
{
    Publication publication;
    publication.setzeId(abfrage.value(0).toInt());
    publication.setzeArxivId(abfrage.value(1).toString());
    publication.setzeTitel(abfrage.value(2).toString());

    const QString authorsAsText = abfrage.value(3).toString();
    publication.setzeAutoren(authorsAsText.split(kAuthorSeparator, Qt::SkipEmptyParts));

    publication.setzeZusammenfassung(abfrage.value(4).toString());
    publication.setzeArxivKategorie(abfrage.value(5).toString());

    const std::optional<Discipline> discipline = disciplineFromText(abfrage.value(6).toString());
    publication.setzeDisziplin(discipline.value_or(Discipline::Other));

    publication.setzeVeroeffentlichtAm(QDateTime::fromString(abfrage.value(7).toString(), Qt::ISODate));
    publication.setzeUrl(abfrage.value(8).toString());

    return publication;
}

std::optional<Publication> SqlitePublicationRepository::findById(int id) const
{
    m_lastError.clear();

    QSqlQuery abfrage(m_database.connection());
    abfrage.prepare(QStringLiteral("SELECT %1 FROM publication WHERE id = :id").arg(kColumns));
    abfrage.bindValue(QStringLiteral(":id"), id);

    if (!abfrage.exec()) {
        m_lastError = abfrage.lastError().text();
        return std::nullopt;
    }
    return abfrage.next() ? std::optional<Publication>(fromQuery(abfrage)) : std::nullopt;
}

std::optional<Publication> SqlitePublicationRepository::findByArxivId(const QString &arxivId) const
{
    m_lastError.clear();

    QSqlQuery abfrage(m_database.connection());
    abfrage.prepare(QStringLiteral("SELECT %1 FROM publication WHERE arxiv_id = :arxiv_id").arg(kColumns));
    abfrage.bindValue(QStringLiteral(":arxiv_id"), arxivId);

    if (!abfrage.exec()) {
        m_lastError = abfrage.lastError().text();
        return std::nullopt;
    }
    return abfrage.next() ? std::optional<Publication>(fromQuery(abfrage)) : std::nullopt;
}

QList<Publication> SqlitePublicationRepository::findByDiscipline(Discipline discipline, int maxAnzahl) const
{
    m_lastError.clear();
    QList<Publication> publications;

    const bool mitFilter = (discipline != Discipline::Alle);
    const bool mitGrenze = (maxAnzahl != kUnlimited);

    QString anweisung = QStringLiteral("SELECT %1 FROM publication ").arg(kColumns);
    if (mitFilter) {
        anweisung += QStringLiteral("WHERE discipline = :discipline ");
    }
    anweisung += kOrdering;
    if (mitGrenze) {
        anweisung += QStringLiteral(" LIMIT :max_count");
    }

    QSqlQuery abfrage(m_database.connection());
    abfrage.prepare(anweisung);
    if (mitFilter) {
        abfrage.bindValue(QStringLiteral(":discipline"), disciplineToText(discipline));
    }
    if (mitGrenze) {
        abfrage.bindValue(QStringLiteral(":max_count"), maxAnzahl);
    }

    if (!abfrage.exec()) {
        m_lastError = abfrage.lastError().text();
        return publications;
    }
    while (abfrage.next()) {
        publications.append(fromQuery(abfrage));
    }
    return publications;
}

bool SqlitePublicationRepository::save(Publication &publication)
{
    m_lastError.clear();

    if (!publication.isPersisted()) {
        const std::optional<Publication> bereitsBekannt = findByArxivId(publication.arxivId());
        if (bereitsBekannt.has_value()) {
            publication.setzeId(bereitsBekannt->id());
        }
    }

    return publication.isPersisted() ? update(publication) : insert(publication);
}

bool SqlitePublicationRepository::insert(Publication &publication)
{
    QSqlQuery abfrage(m_database.connection());
    abfrage.prepare(QStringLiteral(
        "INSERT INTO publication "
        "(arxiv_id, title, authors, summary, arxiv_kategorie, discipline, veroeffentlicht_am, url) "
        "VALUES (:arxiv_id, :title, :authors, :summary, :arxiv_kategorie, :discipline, :veroeffentlicht_am, :url)"));

    abfrage.bindValue(QStringLiteral(":arxiv_id"),        publication.arxivId());
    abfrage.bindValue(QStringLiteral(":title"),           publication.title());
    abfrage.bindValue(QStringLiteral(":authors"),         publication.authors().join(kAuthorSeparator));
    abfrage.bindValue(QStringLiteral(":summary"), publication.summary());
    abfrage.bindValue(QStringLiteral(":arxiv_kategorie"), publication.arxivCategory());
    abfrage.bindValue(QStringLiteral(":discipline"),       disciplineToText(publication.discipline()));
    abfrage.bindValue(QStringLiteral(":veroeffentlicht_am"), publication.publishedAt().toUTC().toString(Qt::ISODate));
    abfrage.bindValue(QStringLiteral(":url"),             publication.url());

    if (!abfrage.exec()) {
        m_lastError = QStringLiteral("Die Publication \"%1\" konnte nicht gespeichert werden: %2")
                              .arg(publication.arxivId(), abfrage.lastError().text());
        return false;
    }

    publication.setzeId(abfrage.lastInsertId().toInt());
    return true;
}

bool SqlitePublicationRepository::update(const Publication &publication)
{
    QSqlQuery abfrage(m_database.connection());
    abfrage.prepare(QStringLiteral(
        "UPDATE publication SET title              = :title, "
        "                             authors            = :authors, "
        "                             summary    = :summary, "
        "                             arxiv_kategorie    = :arxiv_kategorie, "
        "                             discipline          = :discipline, "
        "                             veroeffentlicht_am = :veroeffentlicht_am, "
        "                             url                = :url "
        "WHERE id = :id"));

    abfrage.bindValue(QStringLiteral(":title"),           publication.title());
    abfrage.bindValue(QStringLiteral(":authors"),         publication.authors().join(kAuthorSeparator));
    abfrage.bindValue(QStringLiteral(":summary"), publication.summary());
    abfrage.bindValue(QStringLiteral(":arxiv_kategorie"), publication.arxivCategory());
    abfrage.bindValue(QStringLiteral(":discipline"),       disciplineToText(publication.discipline()));
    abfrage.bindValue(QStringLiteral(":veroeffentlicht_am"), publication.publishedAt().toUTC().toString(Qt::ISODate));
    abfrage.bindValue(QStringLiteral(":url"),             publication.url());
    abfrage.bindValue(QStringLiteral(":id"),              publication.id());

    if (!abfrage.exec()) {
        m_lastError = QStringLiteral("Die Publication \"%1\" konnte nicht geaendert werden: %2")
                              .arg(publication.arxivId(), abfrage.lastError().text());
        return false;
    }
    return true;
}

int SqlitePublicationRepository::count() const
{
    m_lastError.clear();

    QSqlQuery abfrage(m_database.connection());
    if (!abfrage.exec(QStringLiteral("SELECT COUNT(*) FROM publication")) || !abfrage.next()) {
        m_lastError = abfrage.lastError().text();
        return 0;
    }
    return abfrage.value(0).toInt();
}

QString SqlitePublicationRepository::lastError() const
{
    return m_lastError;
}
