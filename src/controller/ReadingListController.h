// ---------------------------------------------------------------------------
// ReadingListController — MVC layer: CONTROLLER (US-04, US-05, US-07)
// ---------------------------------------------------------------------------
#pragma once

#include <QString>

class AuthenticationService;
class ReadingListService;
class ReadingListViewContract;

class ReadingListController
{
public:
    ReadingListController(ReadingListService &leselisteService, AuthenticationService &authentifizierung, ReadingListViewContract &ansicht);

    void refreshOwnList();
    void refreshAllLists();
    void addToReadingList(int publicationId);
    void startReading(int entryId);
    void completeReading(int entryId, int rating, const QString &note);
    void discard(int entryId);
    void approveForTraining(int entryId);
    void archive(int entryId);

    static const QString kMessageAddedToReadingList;
    static const QString kMessageStatusChanged;
    static const QString kMessageDiscarded;
    static const QString kMessageApproved;
    static const QString kMessageArchived;

private:
    void handleResult(bool successful, const QString &errorMessage, const QString &erfolgsmeldung);

    bool m_showAllLists = false;

    ReadingListService          &m_readingListService;
    AuthenticationService &m_authentication;
    ReadingListViewContract         &m_view;
};
