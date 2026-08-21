// ---------------------------------------------------------------------------
// ReadingListView — MVC layer: VIEW (W4/W6, US-04/05/07)
// Dient zwei Betriebsarten: eigene list oder all Listen (KnowledgeManager).
// ---------------------------------------------------------------------------
#pragma once

#include <QList>
#include <QWidget>
#include "controller/ReadingListViewContract.h"

class QLabel;
class QPushButton;
class QTableWidget;
class QTextBrowser;

class ReadingListView : public QWidget, public ReadingListViewContract
{
    Q_OBJECT

public:
    enum class Mode { OwnList, AllLists };

    explicit ReadingListView(Mode mode, QWidget *parentWidget = nullptr);

    void showEntries(const QList<ReadingListEntry> &entries) override;
    void showMessage(const QString &message) override;
    void showError(const QString &message) override;

signals:
    void startReadingRequested(int entryId);
    void completeReadingRequested(int entryId, int rating, const QString &note);
    void discardRequested(int entryId);
    void approveRequested(int entryId);
    void archiveRequested(int entryId);

private:
    void buildUi();
    void selectionChanged();
    void openCompletionDialog();
    const ReadingListEntry *selectedEntry() const;

    Mode m_mode;

    QTableWidget *m_table         = nullptr;
    QTextBrowser *m_noteView    = nullptr;
    QLabel       *m_messageLabel = nullptr;

    QPushButton *m_startReadingButton = nullptr;
    QPushButton *m_completeButton  = nullptr;
    QPushButton *m_discardButton     = nullptr;
    QPushButton *m_approveButton     = nullptr;
    QPushButton *m_archiveButton   = nullptr;

    QList<ReadingListEntry> m_displayedEntries;
};
