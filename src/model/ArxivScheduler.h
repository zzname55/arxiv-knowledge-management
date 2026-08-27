// ---------------------------------------------------------------------------
// ArxivScheduler — MVC layer: MODEL
//
// Loest einmal taeglich um eine feste Uhrzeit (Standard: 07:00) automatisch
// einen arXiv-Abruf aus, solange die Anwendung laeuft (US-09, AK-09.1-09.4).
//
// Die eigentliche Zeitpruefung steht als reine, deterministische Funktion
// istFaelig() zur Verfuegung und ist damit ohne Wartezeit testbar: der Test
// uebergibt einen erfundenen timestamp statt auf die echte Uhr zu warten.
// start() verbindet diese Pruefung mit einem QTimer fuer den echten Betrieb
// und wird bewusst NICHT unit-getestet -- das haengt from der Wanduhr ab und
// laesst sich nur durch tatsaechliches Laufenlassen der Anwendung pruefen.
//
// Der manuelle Klick auf "Aktualisieren" bleibt davon unberuehrt: der
// Zeitplaner merkt sich nur, ob ER selbst an einem Kalendertag schon
// triggered hat, nicht ob ueberhaupt abgerufen wurde (AK-09.3).
// ---------------------------------------------------------------------------
#pragma once

#include <QDate>
#include <QDateTime>
#include <QObject>
#include <QTime>
#include <QTimer>
#include <optional>

class ArxivScheduler : public QObject
{
    Q_OBJECT

public:
    /// @param fetchTime Uhrzeit des taeglichen automatischen Abrufs (Ortszeit).
    explicit ArxivScheduler(QTime fetchTime = QTime(7, 0), QObject *parentObject = nullptr);

    /// Startet die minuetliche Pruefung ueber einen internen QTimer und prueft
    /// sofort einmal mit der aktuellen Zeit -- falls die Anwendung erst to
    /// 7 Uhr geoeffnet wird, soll der Abruf trotzdem nicht bis zum naechsten
    /// Tag warten.
    void start();

    /// Prueft einen konkreten timestamp und loest bei Faelligkeit
    /// automaticFetchDue() aus. Oeffentlich, damit Tests ohne echten
    /// Zeitablauf pruefen koennen.
    /// @return true, wenn triggered wurde.
    bool checkNow(const QDateTime &now);

    /// Reine Zeitlogik ohne Seiteneffekte: ist zum timestamp "now" ein
    /// automatischer Abruf faellig, wenn der letzte automatische Abruf am
    /// Tag "lastFetchDay" stattfand (oder noch nie)?
    static bool istFaelig(const QDateTime &now, const QTime &fetchTime,
                         const std::optional<QDate> &lastFetchDay);

    /// Kalendertag des letzten automatisch ausgeloesten Abrufs, falls vorhanden.
    std::optional<QDate> lastAutomaticFetchDay() const { return m_lastFetchDay; }

    /// Intervall der internen Pruefung in Millisekunden.
    static constexpr int kCheckIntervalInMilliseconds = 60000;

signals:
    /// Der Controller startet daraufhin denselben Abruf wie bei "Aktualisieren".
    void automaticFetchDue();

private:
    QTime                 m_fetchTime;
    std::optional<QDate>  m_lastFetchDay;
    QTimer                m_timer;
};
