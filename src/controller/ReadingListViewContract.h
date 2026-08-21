// ---------------------------------------------------------------------------
// ReadingListViewContract — Schnittstelle zwischen Controller und View
// (US-04, US-05, US-07). Dient beiden Listen-Ansichten (eigene + all).
// ---------------------------------------------------------------------------
#pragma once

#include <QList>
#include <QString>
#include "model/ReadingListEntry.h"

class ReadingListViewContract
{
public:
    virtual ~ReadingListViewContract() = default;

    virtual void showEntries(const QList<ReadingListEntry> &entries) = 0;
    virtual void showMessage(const QString &message) = 0;
    virtual void showError(const QString &message) = 0;
};
