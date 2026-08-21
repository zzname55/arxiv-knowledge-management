// ---------------------------------------------------------------------------
// ReadingListRepository — MVC layer: MODEL
// Rein virtuell. Prueft KEINE Berechtigungen, das macht der ReadingListService.
// ---------------------------------------------------------------------------
#pragma once

#include <QList>
#include <optional>
#include "model/ReadingListEntry.h"

class ReadingListRepository
{
public:
    virtual ~ReadingListRepository() = default;

    virtual std::optional<ReadingListEntry> findById(int id) const = 0;
    virtual QList<ReadingListEntry> findForUser(int userId) const = 0;
    virtual QList<ReadingListEntry> findAll() const = 0;
    virtual QList<ReadingListEntry> findByStatus(ReadingStatus status) const = 0;
    virtual bool entryExists(int userId, int publicationId) const = 0;
    virtual bool save(ReadingListEntry &entry) = 0;
    virtual bool remove(int id) = 0;
};
