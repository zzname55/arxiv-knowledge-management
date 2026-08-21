// ---------------------------------------------------------------------------
// PublicationView — MVC layer: VIEW (W3, US-02/03/06)
// ---------------------------------------------------------------------------
#pragma once

#include <QList>
#include <QWidget>
#include "controller/PublicationViewContract.h"

class QComboBox;
class QLabel;
class QPushButton;
class QTableWidget;
class QTextBrowser;

class PublicationView : public QWidget, public PublicationViewContract
{
    Q_OBJECT

public:
    explicit PublicationView(QWidget *parentWidget = nullptr);

    void showPublications(const QList<Publication> &publications) override;
    void showResultCount(int displayed, int total) override;
    void showHint(const QString &message) override;
    void showError(const QString &message) override;
    void showLoadingIndicator(bool visible) override;

signals:
    void disciplineSelected(Discipline discipline);
    void fetchRequested();
    void addToReadingListRequested(int publicationId);

private:
    void buildUi();
    void selectionChanged();
    void clearDetailView();
    void showDetails(const Publication &publication);
    const Publication *selectedPublication() const;

    enum column { SpalteTitel = 0, SpalteDisziplin = 1, SpalteDatum = 2, SpaltenAnzahl = 3 };

    QComboBox    *m_disciplineSelector   = nullptr;
    QPushButton  *m_refreshButton = nullptr;
    QLabel       *m_resultCountLabel     = nullptr;
    QLabel       *m_hintLabel     = nullptr;
    QTableWidget *m_table            = nullptr;
    QTextBrowser *m_detailView      = nullptr;
    QPushButton  *m_addToReadingListButton     = nullptr;

    QList<Publication> m_displayedPublications;
};
