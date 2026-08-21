#include "controller/ReadingListController.h"

#include "controller/ReadingListViewContract.h"
#include "model/AuthenticationService.h"
#include "model/ReadingListService.h"

const QString ReadingListController::kMessageAddedToReadingList = QStringLiteral("Die Veröffentlichung steht now auf Ihrer readingList.");
const QString ReadingListController::kMessageStatusChanged = QStringLiteral("Der Bearbeitungsstand wurde geändert.");
const QString ReadingListController::kMessageDiscarded = QStringLiteral("Der entry wurde von der readingList entfernt.");
const QString ReadingListController::kMessageApproved = QStringLiteral("Die Veröffentlichung ist für Schulungen freigegeben.");
const QString ReadingListController::kMessageArchived = QStringLiteral("Der entry wurde archiviert.");

namespace {
const QString kMessageNotLoggedIn = QStringLiteral("Es ist kein User loggedIn.");
} // namespace

ReadingListController::ReadingListController(ReadingListService &leselisteService, AuthenticationService &authentifizierung, ReadingListViewContract &ansicht)
    : m_readingListService(leselisteService)
    , m_authentication(authentifizierung)
    , m_view(ansicht)
{
}

void ReadingListController::refreshOwnList()
{
    const std::optional<User> user = m_authentication.currentUser();
    if (!user.has_value()) {
        m_view.showError(kMessageNotLoggedIn);
        return;
    }
    m_showAllLists = false;
    m_view.showEntries(m_readingListService.ownList(*user));
}

void ReadingListController::refreshAllLists()
{
    const std::optional<User> user = m_authentication.currentUser();
    if (!user.has_value()) {
        m_view.showError(kMessageNotLoggedIn);
        return;
    }
    m_showAllLists = true;

    OperationResult              result;
    const QList<ReadingListEntry> entries = m_readingListService.allReadingLists(*user, &result);

    m_view.showEntries(entries);
    if (!result.successful) {
        m_view.showError(result.errorMessage);
    }
}

void ReadingListController::handleResult(bool successful, const QString &errorMessage, const QString &erfolgsmeldung)
{
    if (successful) {
        m_view.showMessage(erfolgsmeldung);
    } else {
        m_view.showError(errorMessage);
    }

    if (m_showAllLists) {
        refreshAllLists();
    } else {
        refreshOwnList();
    }
}

void ReadingListController::addToReadingList(int publicationId)
{
    const std::optional<User> user = m_authentication.currentUser();
    if (!user.has_value()) {
        m_view.showError(kMessageNotLoggedIn);
        return;
    }
    const OperationResult result = m_readingListService.addToReadingList(*user, publicationId);
    handleResult(result.successful, result.errorMessage, kMessageAddedToReadingList);
}

void ReadingListController::startReading(int entryId)
{
    const std::optional<User> user = m_authentication.currentUser();
    if (!user.has_value()) {
        m_view.showError(kMessageNotLoggedIn);
        return;
    }
    const OperationResult result = m_readingListService.changeStatus(*user, entryId, ReadingStatus::InProgress);
    handleResult(result.successful, result.errorMessage, kMessageStatusChanged);
}

void ReadingListController::completeReading(int entryId, int rating, const QString &note)
{
    const std::optional<User> user = m_authentication.currentUser();
    if (!user.has_value()) {
        m_view.showError(kMessageNotLoggedIn);
        return;
    }
    const OperationResult result = m_readingListService.completeReading(*user, entryId, rating, note);
    handleResult(result.successful, result.errorMessage, kMessageStatusChanged);
}

void ReadingListController::discard(int entryId)
{
    const std::optional<User> user = m_authentication.currentUser();
    if (!user.has_value()) {
        m_view.showError(kMessageNotLoggedIn);
        return;
    }
    const OperationResult result = m_readingListService.discard(*user, entryId);
    handleResult(result.successful, result.errorMessage, kMessageDiscarded);
}

void ReadingListController::approveForTraining(int entryId)
{
    const std::optional<User> user = m_authentication.currentUser();
    if (!user.has_value()) {
        m_view.showError(kMessageNotLoggedIn);
        return;
    }
    const OperationResult result = m_readingListService.changeStatus(*user, entryId, ReadingStatus::ApprovedForTraining);
    handleResult(result.successful, result.errorMessage, kMessageApproved);
}

void ReadingListController::archive(int entryId)
{
    const std::optional<User> user = m_authentication.currentUser();
    if (!user.has_value()) {
        m_view.showError(kMessageNotLoggedIn);
        return;
    }
    const OperationResult result = m_readingListService.changeStatus(*user, entryId, ReadingStatus::Archived);
    handleResult(result.successful, result.errorMessage, kMessageArchived);
}
