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

void ArxivClient::fetch(Discipline discipline, int maxTreffer)
{
    if (m_activeRequest != nullptr) {
        m_activeRequest->abort();
        m_activeRequest = nullptr;
    }

    QNetworkRequest anfrage(buildArxivQueryUrl(discipline, maxTreffer));
    anfrage.setHeader(QNetworkRequest::UserAgentHeader, kUserAgent);
    anfrage.setTransferTimeout(kTimeoutInMilliseconds);
    anfrage.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);

    m_activeRequest = m_network->get(anfrage);

    connect(m_activeRequest, &QNetworkReply::finished, this, [this]() {
        QNetworkReply *antwort = m_activeRequest;
        m_activeRequest      = nullptr;
        if (antwort == nullptr) {
            return;
        }
        handleResponse(antwort);
        antwort->deleteLater();
    });
}

bool ArxivClient::isFetching() const
{
    return m_activeRequest != nullptr;
}

void ArxivClient::handleResponse(QNetworkReply *antwort)
{
    if (antwort->error() == QNetworkReply::OperationCanceledError) {
        return;
    }
    if (antwort->error() != QNetworkReply::NoError) {
        emit fehlerAufgetreten(userMessageFor(antwort));
        return;
    }

    QString parserfehler;
    const QList<Publication> publications = ArxivAtomParser::parse(antwort->readAll(), &parserfehler);

    if (!parserfehler.isEmpty()) {
        emit fehlerAufgetreten(parserfehler);
        return;
    }
    emit publicationsReceived(publications);
}

QString ArxivClient::userMessageFor(QNetworkReply *antwort)
{
    switch (antwort->error()) {
    case QNetworkReply::HostNotFoundError:
    case QNetworkReply::UnknownNetworkError:
        return QStringLiteral("arXiv ist nicht erreichbar. Bitte die Internetverbindung pruefen.");
    case QNetworkReply::TimeoutError:
    case QNetworkReply::OperationCanceledError:
        return QStringLiteral("arXiv hat nicht rechtzeitig geantwortet. Bitte spaeter erneut versuchen.");
    case QNetworkReply::ContentNotFoundError:
        return QStringLiteral("Die Abfrage wurde von arXiv nicht verstanden.");
    case QNetworkReply::ServiceUnavailableError:
    case QNetworkReply::InternalServerError:
        return QStringLiteral("arXiv meldet eine Stoerung. Bitte spaeter erneut versuchen.");
    default:
        return QStringLiteral("Der Abruf von arXiv ist fehlgeschlagen: %1").arg(antwort->errorString());
    }
}
