#include "model/ReadingListService.h"

#include "model/ReadingListRepository.h"
#include "model/PermissionService.h"
#include "model/PublicationRepository.h"

const QString ReadingListService::kMessageAlreadyOnReadingList = QStringLiteral("This publication is already on your reading list.");
const QString ReadingListService::kMessagePublicationUnknown = QStringLiteral("This publication is unknown.");
const QString ReadingListService::kMessageEntryUnknown = QStringLiteral("This reading list entry does not exist.");
const QString ReadingListService::kMessageInvalidTransition = QStringLiteral("This step is not allowed in the process.");
const QString ReadingListService::kMessageInvalidRating = QStringLiteral("Please give a rating between 1 and 5.");
const QString ReadingListService::kMessageNoteMissing = QStringLiteral("Please enter a note.");
const QString ReadingListService::kMessageRatingRequired = QStringLiteral("A rating and a note are required to complete reading.");
const QString ReadingListService::kMessageSaveFailed = QStringLiteral("The change could not be saved.");

ReadingListService::ReadingListService(ReadingListRepository &leselisteRepository, PublicationRepository &veroeffentlichungRepository)
    : m_leselisteRepository(leselisteRepository)
    , m_veroeffentlichungRepository(veroeffentlichungRepository)
{
}

OperationResult ReadingListService::addToReadingList(const User &user, int publicationId)
{
    if (!PermissionService::canManageOwnReadingList(user.role())) {
        return OperationResult::failure(PermissionService::kMessageNoPermission);
    }
    if (!m_veroeffentlichungRepository.findById(publicationId).has_value()) {
        return OperationResult::failure(kMessagePublicationUnknown);
    }
    if (m_leselisteRepository.entryExists(user.id(), publicationId)) {
        return OperationResult::failure(kMessageAlreadyOnReadingList);
    }

    const QDateTime now = QDateTime::currentDateTimeUtc();

    ReadingListEntry entry;
    entry.setUserId(user.id());
    entry.setPublicationId(publicationId);
    entry.setStatus(ReadingStatus::Noted);
    entry.setCreatedAt(now);
    entry.setChangedAt(now);

    if (!m_leselisteRepository.save(entry)) {
        return OperationResult::failure(kMessageSaveFailed);
    }
    return OperationResult::success();
}

std::optional<ReadingListEntry> ReadingListService::fetchAndCheckAccess(const User &user, int entryId, OperationResult &result) const
{
    const std::optional<ReadingListEntry> entry = m_leselisteRepository.findById(entryId);
    if (!entry.has_value()) {
        result = OperationResult::failure(kMessageEntryUnknown);
        return std::nullopt;
    }
    if (!PermissionService::canEditEntry(user, *entry)) {
        result = OperationResult::failure(PermissionService::kMessageNoPermission);
        return std::nullopt;
    }
    return entry;
}

OperationResult ReadingListService::changeStatus(const User &user, int entryId, ReadingStatus targetStatus)
{
    if (!PermissionService::canSetStatus(user.role(), targetStatus)) {
        return OperationResult::failure(PermissionService::kMessageNoPermission);
    }

    OperationResult result;
    const std::optional<ReadingListEntry> entry = fetchAndCheckAccess(user, entryId, result);
    if (!entry.has_value()) {
        return result;
    }
    if (!isAllowedTransition(entry->status(), targetStatus)) {
        return OperationResult::failure(kMessageInvalidTransition);
    }
    if (requiresRating(targetStatus)) {
        return OperationResult::failure(kMessageRatingRequired);
    }

    ReadingListEntry geaenderterEintrag = *entry;
    geaenderterEintrag.setStatus(targetStatus);
    geaenderterEintrag.setChangedAt(QDateTime::currentDateTimeUtc());

    if (!m_leselisteRepository.save(geaenderterEintrag)) {
        return OperationResult::failure(kMessageSaveFailed);
    }
    return OperationResult::success();
}

OperationResult ReadingListService::completeReading(const User &user, int entryId, int rating, const QString &note)
{
    if (!PermissionService::canSetStatus(user.role(), ReadingStatus::Read)) {
        return OperationResult::failure(PermissionService::kMessageNoPermission);
    }

    OperationResult result;
    const std::optional<ReadingListEntry> entry = fetchAndCheckAccess(user, entryId, result);
    if (!entry.has_value()) {
        return result;
    }
    if (!isAllowedTransition(entry->status(), ReadingStatus::Read)) {
        return OperationResult::failure(kMessageInvalidTransition);
    }
    if (rating < kRatingMinimum || rating > kRatingMaximum) {
        return OperationResult::failure(kMessageInvalidRating);
    }

    const QString bereinigteNotiz = note.trimmed();
    if (bereinigteNotiz.isEmpty()) {
        return OperationResult::failure(kMessageNoteMissing);
    }

    ReadingListEntry geaenderterEintrag = *entry;
    geaenderterEintrag.setStatus(ReadingStatus::Read);
    geaenderterEintrag.setRating(rating);
    geaenderterEintrag.setNote(bereinigteNotiz);
    geaenderterEintrag.setChangedAt(QDateTime::currentDateTimeUtc());

    if (!m_leselisteRepository.save(geaenderterEintrag)) {
        return OperationResult::failure(kMessageSaveFailed);
    }
    return OperationResult::success();
}

OperationResult ReadingListService::discard(const User &user, int entryId)
{
    OperationResult result;
    const std::optional<ReadingListEntry> entry = fetchAndCheckAccess(user, entryId, result);
    if (!entry.has_value()) {
        return result;
    }
    if (entry->userId() != user.id()) {
        return OperationResult::failure(PermissionService::kMessageNoPermission);
    }
    if (entry->status() != ReadingStatus::Noted && entry->status() != ReadingStatus::InProgress) {
        return OperationResult::failure(kMessageInvalidTransition);
    }
    if (!m_leselisteRepository.remove(entryId)) {
        return OperationResult::failure(kMessageSaveFailed);
    }
    return OperationResult::success();
}

QList<ReadingListEntry> ReadingListService::ownList(const User &user) const
{
    return m_leselisteRepository.findForUser(user.id());
}

QList<ReadingListEntry> ReadingListService::allReadingLists(const User &user, OperationResult *result) const
{
    if (!PermissionService::canViewAllReadingLists(user.role())) {
        if (result != nullptr) {
            *result = OperationResult::failure(PermissionService::kMessageNoPermission);
        }
        return {};
    }
    if (result != nullptr) {
        *result = OperationResult::success();
    }
    return m_leselisteRepository.findAll();
}

QList<ReadingListEntry> ReadingListService::approvedForTraining() const
{
    return m_leselisteRepository.findByStatus(ReadingStatus::ApprovedForTraining);
}
