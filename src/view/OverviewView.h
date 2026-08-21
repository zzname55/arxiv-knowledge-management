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

signals:
    void fetchRequested();

private:
    void buildUi();
    QLabel *buildTile(const QString &beschriftung, QWidget *parentWidget);

    QLabel *m_greeting             = nullptr;
    QLabel *m_publicationCount = nullptr;
    QLabel *m_readingListCount           = nullptr;
    QLabel *m_approvedCount         = nullptr;
    QLabel *m_lastFetchLabel            = nullptr;
};
