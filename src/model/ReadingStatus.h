// ---------------------------------------------------------------------------
// ReadingStatus — MVC layer: MODEL
// Statusautomat des Prozesses (mandatory requirement 1.1):
//   VORGEMERKT -> WIRD_GELESEN -> GELESEN -> FUER_SCHULUNG_FREIGEGEBEN -> ARCHIVIERT
// Laeuft in genau eine Richtung: kein Sprung, kein Rueckschritt.
// ---------------------------------------------------------------------------
#pragma once

#include <QList>
#include <QString>
#include <optional>

enum class ReadingStatus {
    Noted,
    InProgress,
    Gelesen,
    ApprovedForTraining,
    Archived
};

QString readingStatusToText(ReadingStatus status);
std::optional<ReadingStatus> readingStatusFromText(const QString &text);
QList<ReadingStatus> allReadingStatuses();

/// Erlaubt ist ausschliesslich der jeweils naechste Schritt.
bool isAllowedTransition(ReadingStatus von, ReadingStatus nach);

/// std::nullopt im Endzustand.
std::optional<ReadingStatus> nextStatus(ReadingStatus status);

bool isFinalState(ReadingStatus status);

/// true fuer GELESEN: dort sind Rating und Note Pflicht (AK-05.3).
bool requiresRating(ReadingStatus status);
