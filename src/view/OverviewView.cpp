#include "view/OverviewView.h"

#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

OverviewView::OverviewView(QWidget *parentWidget)
    : QWidget(parentWidget)
{
    buildUi();
}

QLabel *OverviewView::buildTile(const QString &beschriftung, QWidget *parentWidget)
{
    auto *rahmen = new QFrame(parentWidget);
    rahmen->setFrameShape(QFrame::StyledPanel);
    rahmen->setMinimumHeight(110);

    auto *beschriftungAnzeige = new QLabel(beschriftung, rahmen);
    beschriftungAnzeige->setAlignment(Qt::AlignCenter);
    beschriftungAnzeige->setWordWrap(true);

    auto *zahlAnzeige = new QLabel(QStringLiteral("—"), rahmen);
    zahlAnzeige->setAlignment(Qt::AlignCenter);
    QFont zahlSchrift = zahlAnzeige->font();
    zahlSchrift.setPointSize(zahlSchrift.pointSize() + 12);
    zahlSchrift.setBold(true);
    zahlAnzeige->setFont(zahlSchrift);

    auto *kachelLayout = new QVBoxLayout(rahmen);
    kachelLayout->addWidget(beschriftungAnzeige);
    kachelLayout->addWidget(zahlAnzeige);

    parentWidget->layout()->addWidget(rahmen);

    return zahlAnzeige;
}

void OverviewView::buildUi()
{
    auto *heading = new QLabel(tr("Overview"), this);
    QFont ueberschriftSchrift = heading->font();
    ueberschriftSchrift.setPointSize(ueberschriftSchrift.pointSize() + 2);
    ueberschriftSchrift.setBold(true);
    heading->setFont(ueberschriftSchrift);

    m_greeting = new QLabel(this);

    auto *tileArea = new QWidget(this);
    auto *kachelLayout  = new QHBoxLayout(tileArea);
    kachelLayout->setContentsMargins(0, 0, 0, 0);

    m_publicationCount = buildTile(tr("Publications"),       tileArea);
    m_readingListCount           = buildTile(tr("On my reading list"),     tileArea);
    m_approvedCount         = buildTile(tr("Approved for training"), tileArea);

    m_lastFetchLabel = new QLabel(tr("Last fetched: never"), this);
    m_lastFetchLabel->setStyleSheet(QStringLiteral("color: #777777;"));

    auto *hinweisAutomatik = new QLabel(
        tr("The application also fetches automatically once a day at 7:00 AM, as long as it is running."), this);
    hinweisAutomatik->setStyleSheet(QStringLiteral("color: #777777;"));
    hinweisAutomatik->setWordWrap(true);

    auto *fetchButton = new QPushButton(tr("Refresh from arXiv now"), this);

    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(heading);
    mainLayout->addWidget(m_greeting);
    mainLayout->addSpacing(12);
    mainLayout->addWidget(tileArea);
    mainLayout->addSpacing(12);
    mainLayout->addWidget(m_lastFetchLabel);
    mainLayout->addWidget(hinweisAutomatik);
    mainLayout->addWidget(fetchButton, 0, Qt::AlignLeft);
    mainLayout->addStretch();

    connect(fetchButton, &QPushButton::clicked, this, &OverviewView::fetchRequested);
}

void OverviewView::setMetrics(int publications, int aufLeseliste, int freigegeben)
{
    m_publicationCount->setText(QString::number(publications));
    m_readingListCount->setText(QString::number(aufLeseliste));
    m_approvedCount->setText(QString::number(freigegeben));
}

void OverviewView::setLastFetch(const QString &timestamp)
{
    m_lastFetchLabel->setText(tr("Last fetched: %1").arg(timestamp));
}

void OverviewView::setGreeting(const QString &displayName)
{
    m_greeting->setText(tr("Welcome, %1.").arg(displayName));
}
