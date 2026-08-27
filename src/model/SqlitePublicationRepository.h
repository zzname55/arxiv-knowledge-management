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

    QString lastError() const;

    static const QString kAuthorSeparator;

private:
    static Publication fromQuery(const QSqlQuery &query);
    bool insert(Publication &publication);
    bool update(const Publication &publication);

    Database      &m_database;
    mutable QString m_lastError;
};
