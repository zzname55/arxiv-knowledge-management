#include "model/ReadingStatus.h"

QString readingStatusToText(ReadingStatus status)
{
    switch (status) {
    case ReadingStatus::Noted:              return QStringLiteral("Noted");
    case ReadingStatus::InProgress:             return QStringLiteral("Wird gelesen");
    case ReadingStatus::Gelesen:                 return QStringLiteral("Gelesen");
    case ReadingStatus::ApprovedForTraining: return QStringLiteral("Für Schulung freigegeben");
    case ReadingStatus::Archived:              return QStringLiteral("Archived");
    }
    return QString();
}

std::optional<ReadingStatus> readingStatusFromText(const QString &text)
{
    for (const ReadingStatus status : allReadingStatuses()) {
        if (readingStatusToText(status) == text) {
            return status;
        }
    }
    return std::nullopt;
}

QList<ReadingStatus> allReadingStatuses()
{
    return { ReadingStatus::Noted, ReadingStatus::InProgress, ReadingStatus::Gelesen,
             ReadingStatus::ApprovedForTraining, ReadingStatus::Archived };
}

std::optional<ReadingStatus> nextStatus(ReadingStatus status)
{
    const QList<ReadingStatus> prozess  = allReadingStatuses();
    const qsizetype         position = prozess.indexOf(status);

    if (position < 0 || position + 1 >= prozess.size()) {
        return std::nullopt;
    }
    return prozess.at(position + 1);
}

bool isAllowedTransition(ReadingStatus von, ReadingStatus nach)
{
    const std::optional<ReadingStatus> folgezustand = nextStatus(von);
    return folgezustand.has_value() && *folgezustand == nach;
}

bool isFinalState(ReadingStatus status)
{
    return !nextStatus(status).has_value();
}

bool requiresRating(ReadingStatus status)
{
    return status == ReadingStatus::Gelesen;
}
