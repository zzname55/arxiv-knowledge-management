// ---------------------------------------------------------------------------
// SqlitePublicationRepository — MVC layer: MODEL
// Authors werden mit " | " als Trennzeichen in einer column abgelegt (kommt
// in Personennamen nicht vor, ein Komma dagegen schon).
// ---------------------------------------------------------------------------
#pragma once

#include <QString>
#include "model/PublicationRepository.h"

class Database;
class QSqlQuery;

class SqlitePublicationRepository : public PublicationRepository
{
public:
    explicit SqlitePublicationRepository(Database &database);

    std::optional<Publication> findById(int id) const override;
    std::optional<Publication> findByArxivId(const QString &arxivId) const override;
    QList<Publication>         findByDiscipline(Discipline discipline, int maxCount = kUnlimited) const override;
    bool                             save(Publication &publication) override;
    int                              count() const override;

    /// Repairs rows whose stored discipline label can no longer be resolved.
    /// Such legacy rows appear whenever the discipline naming changes -- for
    /// instance when the former catch-all "Physics" was split into six real
    /// arXiv subfields. The assignment is recomputed from the still-present
    /// "arxiv_category" column, so no network request is required.
    ///
    /// The call is idempotent: a second run finds nothing left to correct.
    /// Returns the number of corrected rows, or -1 on an SQL failure (details
    /// are then available through lastError()).
    int migrateOutdatedDisciplines();

    QString lastError() const;

    static const QString kAuthorSeparator;

private:
    static Publication fromQuery(const QSqlQuery &query);
    bool insert(Publication &publication);
    bool update(const Publication &publication);

    Database      &m_database;
    mutable QString m_lastError;
};
