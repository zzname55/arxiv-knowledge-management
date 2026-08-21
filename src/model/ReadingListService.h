// ---------------------------------------------------------------------------
// ReadingListService — MVC layer: MODEL
// Fuehrt den betrieblichen Prozess: ReadingStatus (Ablauf) + PermissionService
// (Berechtigung) + ReadingListRepository (Ablage).
// ---------------------------------------------------------------------------
#pragma once

#include <QList>
#include <QString>
#include "model/User.h"
#include "model/ReadingListEntry.h"
#include "model/OperationResult.h"

class ReadingListRepository;
class PublicationRepository;

class ReadingListService
{
public:
    ReadingListService(ReadingListRepository &leselisteRepository, PublicationRepository &veroeffentlichungRepository);

    OperationResult addToReadingList(const User &user, int publicationId);
    OperationResult changeStatus(const User &user, int entryId, ReadingStatus targetStatus);
    OperationResult completeReading(const User &user, int entryId, int rating, const QString &note);
    OperationResult discard(const User &user, int entryId);

    QList<ReadingListEntry> ownList(const User &user) const;
    QList<ReadingListEntry> allReadingLists(const User &user, OperationResult *result = nullptr) const;
    QList<ReadingListEntry> approvedForTraining() const;

    static const QString kMessageAlreadyOnReadingList;
    static const QString kMessagePublicationUnknown;
    static const QString kMessageEntryUnknown;
    static const QString kMessageInvalidTransition;
    static const QString kMessageInvalidRating;
    static const QString kMessageNoteMissing;
    static const QString kMessageRatingRequired;
    static const QString kMessageSaveFailed;

    static constexpr int kRatingMinimum = 1;
    static constexpr int kRatingMaximum = 5;

private:
    std::optional<ReadingListEntry> fetchAndCheckAccess(const User &user, int entryId, OperationResult &result) const;

    ReadingListRepository         &m_leselisteRepository;
    PublicationRepository &m_veroeffentlichungRepository;
};
