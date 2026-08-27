// ---------------------------------------------------------------------------
// OverviewView — MVC layer: VIEW (W2)
// Kennzahlen + timestamp des letzten Abrufs (manuell oder automatisch, US-09).
// ---------------------------------------------------------------------------
#pragma once

#include <QWidget>

class QLabel;
class QPushButton;

class OverviewView : public QWidget
{
    Q_OBJECT

public:
    explicit OverviewView(QWidget *parentWidget = nullptr);

    void setMetrics(int publications, int aufLeseliste, int freigegeben);
    void setLastFetch(const QString &timestamp);
    void setGreeting(const QString &displayName);

    /// While a fetch is running (manual or automatic) the button is disabled
    /// and relabeled, so a click on the Overview page visibly does something
    /// even if the user never switches to the Publications tab.
    void setLoading(bool loading);

    /// Feedback after a fetch completes (success or failure). Without this,
    /// the Overview page had no feedback at all -- a network error stayed
    /// invisible here because it was only reported to the (possibly not
    /// currently visible) Publications view.
    void showMessage(const QString &message, bool isError);

signals:
    void fetchRequested();

private:
    void buildUi();
    QLabel *buildTile(const QString &beschriftung, QWidget *parentWidget);

    QLabel      *m_greeting             = nullptr;
    QLabel      *m_publicationCount = nullptr;
    QLabel      *m_readingListCount           = nullptr;
    QLabel      *m_approvedCount         = nullptr;
    QLabel      *m_lastFetchLabel            = nullptr;
    QPushButton *m_fetchButton              = nullptr;
    QLabel      *m_messageLabel             = nullptr;
};
