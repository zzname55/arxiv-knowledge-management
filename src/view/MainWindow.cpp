#include "view/MainWindow.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QPushButton>
#include <QStackedWidget>
#include <QStatusBar>
#include <QVBoxLayout>

MainWindow::MainWindow(const User &user, const QList<Page> &pages, QWidget *parentWidget)
    : QMainWindow(parentWidget)
{
    buildUi(user, pages);
}

void MainWindow::buildUi(const User &user, const QList<Page> &pages)
{
    setWindowTitle(tr("ArxivKnowledgeManagement"));
    resize(1100, 700);

    auto *userLabel = new QLabel(tr("Logged in: %1 (%2)").arg(user.displayName(), roleToText(user.role())), this);
    auto *logoutButton   = new QPushButton(tr("Logout"), this);

    auto *headerRow = new QHBoxLayout;
    headerRow->addWidget(userLabel);
    headerRow->addStretch();
    headerRow->addWidget(logoutButton);

    m_navigation = new QListWidget(this);
    m_navigation->setObjectName(QStringLiteral("navigation"));
    m_navigation->setMaximumWidth(190);

    m_pageStack = new QStackedWidget(this);

    for (const Page &page : pages) {
        if (page.widget == nullptr) {
            continue;
        }
        const int stapelIndex = m_pageStack->addWidget(page.widget);
        if (page.visible) {
            m_navigation->addItem(page.beschriftung);
            m_stackIndexPerNavRow.append(stapelIndex);
        }
    }

    if (m_navigation->count() > 0) {
        m_navigation->setCurrentRow(0);
    }

    auto *content = new QHBoxLayout;
    content->addWidget(m_navigation);
    content->addWidget(m_pageStack, 1);

    auto *zentralWidget = new QWidget(this);
    auto *mainLayout   = new QVBoxLayout(zentralWidget);
    mainLayout->addLayout(headerRow);
    mainLayout->addLayout(content, 1);

    setCentralWidget(zentralWidget);
    statusBar()->showMessage(tr("Ready."));

    connect(logoutButton, &QPushButton::clicked, this, &MainWindow::logoutRequested);

    connect(m_navigation, &QListWidget::currentRowChanged, this, [this](int row) {
        if (row < 0 || row >= m_stackIndexPerNavRow.size()) {
            return;
        }
        m_pageStack->setCurrentIndex(m_stackIndexPerNavRow.at(row));
        emit pageChanged(row);
    });
}

void MainWindow::showStatusMessage(const QString &message)
{
    statusBar()->showMessage(message);
}
