// ---------------------------------------------------------------------------
// SortableTableItem — MVC layer: VIEW
// Tabellenzelle mit eigenem Sortierschluessel statt Textvergleich (B-27).
// Erste Zelle jeder row traegt zusaetzlich die Database-ID des Datensatzes,
// damit eine row nach dem Sortieren ueber die ID und nicht ueber ihre
// (dann veraenderte) Position wiedergefunden wird.
// ---------------------------------------------------------------------------
#pragma once

#include <QDateTime>
#include <QTableWidgetItem>
#include <QVariant>

inline constexpr int kSortierRolle     = Qt::UserRole + 1;
inline constexpr int kDatensatzIdRolle = Qt::UserRole;

class SortableTableItem : public QTableWidgetItem
{
public:
    SortableTableItem(const QString &anzeigetext, const QVariant &sortierschluessel = QVariant())
        : QTableWidgetItem(anzeigetext)
    {
        if (sortierschluessel.isValid()) {
            setData(kSortierRolle, sortierschluessel);
        }
    }

    bool operator<(const QTableWidgetItem &anderesElement) const override
    {
        const QVariant eigenerSchluessel = data(kSortierRolle);
        const QVariant fremderSchluessel = anderesElement.data(kSortierRolle);

        if (!eigenerSchluessel.isValid() || !fremderSchluessel.isValid()) {
            return QTableWidgetItem::operator<(anderesElement);
        }

        switch (eigenerSchluessel.typeId()) {
        case QMetaType::Int:
            return eigenerSchluessel.toInt() < fremderSchluessel.toInt();
        case QMetaType::QDateTime:
            return eigenerSchluessel.toDateTime() < fremderSchluessel.toDateTime();
        default:
            return QString::localeAwareCompare(eigenerSchluessel.toString(), fremderSchluessel.toString()) < 0;
        }
    }
};
