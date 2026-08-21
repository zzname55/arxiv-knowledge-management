#include "model/ArxivScheduler.h"

ArxivScheduler::ArxivScheduler(QTime fetchTime, QObject *parentObject)
    : QObject(parentObject)
    , m_fetchTime(fetchTime)
{
    connect(&m_timer, &QTimer::timeout, this, [this]() {
        checkNow(QDateTime::currentDateTime());
    });
}

void ArxivScheduler::start()
{
    // Sofortpruefung: wird die Anwendung erst um 9 Uhr geoeffnet, soll der
    // automatische Abruf trotzdem noch today stattfinden, nicht erst morgen.
    checkNow(QDateTime::currentDateTime());
    m_timer.start(kCheckIntervalInMilliseconds);
}

bool ArxivScheduler::checkNow(const QDateTime &now)
{
    if (!istFaelig(now, m_fetchTime, m_lastFetchDay)) {
        return false;
    }

    m_lastFetchDay = now.date();
    emit automaticFetchDue();
    return true;
}

bool ArxivScheduler::istFaelig(const QDateTime &now, const QTime &fetchTime,
                               const std::optional<QDate> &lastFetchDay)
{
    if (now.time() < fetchTime) {
        return false;
    }
    // An diesem Kalendertag ist bereits automatisch abgerufen worden.
    if (lastFetchDay.has_value() && *lastFetchDay >= now.date()) {
        return false;
    }
    return true;
}
