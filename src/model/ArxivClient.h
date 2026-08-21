// ---------------------------------------------------------------------------
// ArxivClient — MVC layer: MODEL
// Holt die Atom-Antwort von der arXiv-API asynchron (AK-02.4: Oberflaeche
// darf nicht einfrieren) und reicht die ausgewerteten Arbeiten per Signal weiter.
// ---------------------------------------------------------------------------
#pragma once

#include <QObject>
#include <QList>
#include <QString>
#include "model/Discipline.h"
#include "model/Publication.h"

class QNetworkAccessManager;
class QNetworkReply;

class ArxivClient : public QObject
{
    Q_OBJECT

public:
    explicit ArxivClient(QObject *parentObject = nullptr);

    /// Startet den Abruf. Ein laufender Abruf wird zuvor abgebrochen.
    void fetch(Discipline discipline, int maxTreffer = kDefaultResultCount);

    bool isFetching() const;

    static constexpr int kTimeoutInMilliseconds = 15000;

signals:
    void publicationsReceived(const QList<Publication> &publications);
    void fehlerAufgetreten(const QString &message);

private:
    void handleResponse(QNetworkReply *antwort);
    static QString userMessageFor(QNetworkReply *antwort);

    QNetworkAccessManager *m_network        = nullptr;
    QNetworkReply         *m_activeRequest = nullptr;
};
