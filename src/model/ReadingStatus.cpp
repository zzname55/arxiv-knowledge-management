#include "model/ReadingStatus.h"

QString readingStatusToText(ReadingStatus status)
{
    switch (status) {
    case ReadingStatus::Noted:              return QStringLiteral("Noted");
    case ReadingStatus::InProgress:             return QStringLiteral("In Progress");
    case ReadingStatus::Read:                 return QStringLiteral("Read");
    case ReadingStatus::ApprovedForTraining: return QStringLiteral("Approved for Training");
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
    return { ReadingStatus::Noted, ReadingStatus::InProgress, ReadingStatus::Read,
             ReadingStatus::ApprovedForTraining, ReadingStatus::Archived };
}

std::optional<ReadingStatus> nextStatus(ReadingStatus status)
{
    const QList<ReadingStatus> process  = allReadingStatuses();
    const qsizetype         position = process.indexOf(status);

    if (position < 0 || position + 1 >= process.size()) {
        return std::nullopt;
    }
    return process.at(position + 1);
}

bool isAllowedTransition(ReadingStatus from, ReadingStatus to)
{
    const std::optional<ReadingStatus> nextState = nextStatus(from);
    return nextState.has_value() && *nextState == to;
}

bool isFinalState(ReadingStatus status)
{
    return !nextStatus(status).has_value();
}

bool requiresRating(ReadingStatus status)
{
    return status == ReadingStatus::Read;
}
