// ---------------------------------------------------------------------------
// ReadingListRouter — MVC layer: VIEW (Adapter)
// Leitet Rueckmeldungen eines ReadingListController an zwei Stellen: die
// eigentliche Leselisten-Ansicht (im Hintergrund) und dorthin, wo der
// User gerade hinsieht (z. B. der Reiter publications), falls
// dort "Auf readingList setzen" triggered wurde (AK-04.4 muss visible sein).
// ---------------------------------------------------------------------------
#pragma once

#include <QString>
#include <functional>
#include "controller/ReadingListViewContract.h"

class ReadingListRouter : public ReadingListViewContract
{
public:
    ReadingListRouter(ReadingListViewContract &listTarget, std::function<void(const QString &, bool)> messageTarget)
        : m_listenziel(listTarget)
        , m_meldungsziel(std::move(messageTarget))
    {
    }

    void showEntries(const QList<ReadingListEntry> &entries) override
    {
        m_listenziel.showEntries(entries);
    }

    void showMessage(const QString &message) override
    {
        m_listenziel.showMessage(message);
        if (m_meldungsziel) {
            m_meldungsziel(message, false);
        }
    }

    void showError(const QString &message) override
    {
        m_listenziel.showError(message);
        if (m_meldungsziel) {
            m_meldungsziel(message, true);
        }
    }

private:
    ReadingListViewContract                          &m_listenziel;
    std::function<void(const QString &, bool)>  m_meldungsziel;
};
