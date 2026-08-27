#include "controller/PublicationController.h"

#include "controller/PublicationViewContract.h"
#include "model/ArxivClient.h"
#include "model/PublicationRepository.h"

const QString PublicationController::kHintNoResults = QStringLiteral("No publications found for this discipline.");
const QString PublicationController::kMessageFetchSuccessful = QStringLiteral("Publications loaded from arXiv.");

PublicationController::PublicationController(ArxivClient &arxivClient, PublicationRepository &repository,
                                                         PublicationViewContract &ansicht, QObject *parentObject)
    : QObject(parentObject)
    , m_arxivClient(arxivClient)
    , m_repository(repository)
    , m_view(ansicht)
{
    connect(&m_arxivClient, &ArxivClient::publicationsReceived, this, &PublicationController::publicationsReceived);
    connect(&m_arxivClient, &ArxivClient::fetchFailed, this, &PublicationController::fetchFailed);
}

void PublicationController::refreshView()
{
    const QList<Publication> found = m_repository.findByDiscipline(m_selectedDiscipline);

    m_view.showPublications(found);
    m_view.showResultCount(static_cast<int>(found.size()), m_repository.count());
    m_view.showHint(found.isEmpty() ? kHintNoResults : QString());
}

void PublicationController::disciplineSelected(Discipline discipline)
{
    m_selectedDiscipline = discipline;
    refreshView();
}

void PublicationController::fetchRequested()
{
    m_view.showLoadingIndicator(true);
    m_arxivClient.fetch(m_selectedDiscipline, kDefaultResultCount);
}

void PublicationController::publicationsReceived(const QList<Publication> &publications)
{
    m_view.showLoadingIndicator(false);

    for (const Publication &empfangene : publications) {
        Publication zuSpeichern = empfangene;
        if (!m_repository.save(zuSpeichern)) {
            m_view.showError(QStringLiteral("The fetched publications could not be saved."));
            return;
        }
    }

    refreshView();

    if (!publications.isEmpty()) {
        m_view.showHint(kMessageFetchSuccessful);
    }
}

void PublicationController::fetchFailed(const QString &message)
{
    m_view.showLoadingIndicator(false);
    m_view.showError(message);
}
