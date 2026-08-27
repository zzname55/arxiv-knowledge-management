// ---------------------------------------------------------------------------
// SqliteReadingListRepository — MVC layer: MODEL
// Verbindet mit user/publication (JOIN), um DisplayName und Title
// gleich mitzuliefern statt pro row nachzufragen.
// ---------------------------------------------------------------------------
#pragma once

#include <QString>
#include "model/ReadingListRepository.h"

class Database;
class QSqlQuery;

class SqliteReadingListRepository : public ReadingListRepository
{
public:
    explicit SqliteReadingListRepository(Database &database);

    std::optional<ReadingListEntry> findById(int id) const override;
    QList<ReadingListEntry>         findForUser(int userId) const override;
    QList<ReadingListEntry>         findAll() const override;
    QList<ReadingListEntry>         findByStatus(ReadingStatus status) const override;
    bool                            entryExists(int userId, int publicationId) const override;
    bool                            save(ReadingListEntry &entry) override;
    bool                            remove(int id) override;

    QString lastError() const;

private:
    static ReadingListEntry fromQuery(const QSqlQuery &query);
    QList<ReadingListEntry> readList(QSqlQuery &query) const;
    bool insert(ReadingListEntry &entry);
    bool update(const ReadingListEntry &entry);

    Database      &m_database;
    mutable QString m_lastError;
};
