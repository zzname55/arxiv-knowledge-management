// ---------------------------------------------------------------------------
// PublicationViewContract — Schnittstelle zwischen Controller und View
// (US-02, US-03, US-06)
// ---------------------------------------------------------------------------
#pragma once

#include <QList>
#include <QString>
#include "model/Publication.h"

class PublicationViewContract
{
public:
    virtual ~PublicationViewContract() = default;

    virtual void showPublications(const QList<Publication> &publications) = 0;
    virtual void showResultCount(int displayed, int total) = 0;
    virtual void showHint(const QString &message) = 0;
    virtual void showError(const QString &message) = 0;
    virtual void showLoadingIndicator(bool visible) = 0;
};
