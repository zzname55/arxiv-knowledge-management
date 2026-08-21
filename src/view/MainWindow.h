// ---------------------------------------------------------------------------
// MainWindow — MVC layer: VIEW (W2)
//
// CAUTION -- ownership: the window takes over the given pages
// (auch die nicht sichtbaren). Sie muessen mit `new` angelegt sein, nicht auf
// dem stack und nicht in einem unique_ptr -- sonst werden sie beim Schliessen
// zweimal freigegeben. Dieser Fehler ist im Projekt bereits einmal aufgetreten
// (Absturz beim Logout) und wurde durch diesen hint dokumentiert.
// ---------------------------------------------------------------------------
#pragma once

#include <QMainWindow>
#include "model/User.h"

class QListWidget;
class QStackedWidget;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    struct Page {
        QString  beschriftung;
        QWidget *widget = nullptr;
        bool visible = true;
    };

    MainWindow(const User &user, const QList<Page> &pages, QWidget *parentWidget = nullptr);

    void showStatusMessage(const QString &message);

signals:
    void logoutRequested();
    void pageChanged(int pageIndex);

private:
    void buildUi(const User &user, const QList<Page> &pages);

    QListWidget    *m_navigation   = nullptr;
    QStackedWidget *m_pageStack = nullptr;
    QList<int>      m_stackIndexPerNavRow;
};
