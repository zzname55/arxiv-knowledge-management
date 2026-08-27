#include "model/ArxivClient.h"

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>

#include "model/ArxivAtomParser.h"

namespace {
const QByteArray kUserAgent = QByteArrayLiteral("ArxivKnowledgeManagement/1.1 (school project, educational use)");
} // namespace

ArxivClient::ArxivClient(QObject *parentObject)
    : QObject(parentObject)
    , m_network(new QNetworkAccessManager(this))
{
}

void ArxivClient::fetch(Discipline discipline, int maxResults)
{
    if (m_activeRequest != nullptr) {
        m_activeRequest->abort();
        m_activeRequest = nullptr;
    }

    QNetworkRequest request(buildArxivQueryUrl(discipline, maxResults));
    request.setHeader(QNetworkRequest::UserAgentHeader, kUserAgent);
    request.setTransferTimeout(kTimeoutInMilliseconds);
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);

    m_activeRequest = m_network->get(request);

    connect(m_activeRequest, &QNetworkReply::finished, this, [this]() {
        QNetworkReply *reply = m_activeRequest;
        m_activeRequest      = nullptr;
        if (reply == nullptr) {
            return;
        }
        handleResponse(reply);
        reply->deleteLater();
    });
}

bool ArxivClient::isFetching() const
{
    return m_activeRequest != nullptr;
}

void ArxivClient::handleResponse(QNetworkReply *reply)
{
    if (reply->error() == QNetworkReply::OperationCanceledError) {
        return;
    }
    if (reply->error() != QNetworkReply::NoError) {
        emit fetchFailed(userMessageFor(reply));
        return;
    }

    QString parseError;
    const QList<Publication> publications = ArxivAtomParser::parse(reply->readAll(), &parseError);

    if (!parseError.isEmpty()) {
        emit fetchFailed(parseError);
        return;
    }
    emit publicationsReceived(publications);
}

QString ArxivClient::userMessageFor(QNetworkReply *reply)
{
    switch (reply->error()) {
    case QNetworkReply::HostNotFoundError:
    case QNetworkReply::UnknownNetworkError:
        return QStringLiteral("arXiv cannot be reached. Please check your internet connection.");
    case QNetworkReply::TimeoutError:
    case QNetworkReply::OperationCanceledError:
        return QStringLiteral("arXiv did not respond in time. Please try again later.");
    case QNetworkReply::ContentNotFoundError:
        return QStringLiteral("arXiv did not understand the query.");
    case QNetworkReply::ServiceUnavailableError:
    case QNetworkReply::InternalServerError:
        return QStringLiteral("arXiv reports a service problem. Please try again later.");
    default:
        return QStringLiteral("Fetching from arXiv failed: %1").arg(reply->errorString());
    }
}
