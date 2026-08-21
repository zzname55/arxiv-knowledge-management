// ---------------------------------------------------------------------------
// main.cpp — Composition Root
// The only translation unit that sees all three MVC layers at once.
// ---------------------------------------------------------------------------
#include <QApplication>
#include <QDateTime>
#include <QDir>
#include <QLocale>
#include <QMessageBox>
#include <QStandardPaths>

#include <functional>

#include "controller/LoginController.h"
#include "controller/UserManagementController.h"
#include "controller/ReadingListController.h"
#include "controller/PublicationController.h"

#include "model/ArxivClient.h"
#include "model/ArxivScheduler.h"
#include "model/AuthenticationService.h"
#include "model/UserManagementService.h"
#include "model/Database.h"
#include "model/ReadingListService.h"
#include "model/PermissionService.h"
#include "model/SqliteUserRepository.h"
#include "model/SqliteReadingListRepository.h"
#include "model/SqlitePublicationRepository.h"

#include "view/LoginView.h"
#include "view/UserManagementView.h"
#include "view/MainWindow.h"
#include "view/ReadingListView.h"
#include "view/ReadingListRouter.h"
#include "view/OverviewView.h"
#include "view/PublicationView.h"

namespace {

const QString kDatabaseFileName = QStringLiteral("knowledge.db");

QString resolveDatabasePath()
{
    const QString dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(dataDir);
    return QDir(dataDir).filePath(kDatabaseFileName);
}

void reportStartupError(const QString &message)
{
    QMessageBox::critical(nullptr, QObject::tr("ArxivKnowledgeManagement — Unable to Start"), message);
}

} // namespace

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QApplication::setApplicationName(QStringLiteral("ArxivKnowledgeManagement"));
    QApplication::setOrganizationName(QStringLiteral("TechnoLab"));

    Database database(QStringLiteral("app"));

    if (!database.open(resolveDatabasePath())) {
        reportStartupError(database.lastError());
        return 1;
    }
    if (!database.createSchema()) {
        reportStartupError(database.lastError());
        return 1;
    }

    SqliteUserRepository        userRepository(database);
    SqlitePublicationRepository publicationRepository(database);
    SqliteReadingListRepository readingListRepository(database);

    if (!userRepository.createDefaultUsers()) {
        reportStartupError(userRepository.lastError());
        return 1;
    }

    AuthenticationService  authentication(userRepository);
    ReadingListService     readingListService(readingListRepository, publicationRepository);
    UserManagementService  userManagementService(userRepository);
    ArxivClient             arxivClient;

    // US-09: fetches from arXiv once a day at 07:00, as long as the
    // application is running. start() checks immediately and then every minute.
    ArxivScheduler arxivScheduler;

    bool loginAgain = true;

    while (loginAgain) {
        loginAgain = false;

        LoginView       loginView;
        LoginController loginController(authentication, loginView);

        QObject::connect(&loginView, &LoginView::loginRequested, &loginView, [&loginController]() {
            loginController.loginRequested();
        });

        if (loginView.exec() != QDialog::Accepted) {
            return 0;
        }

        const User currentUser = loginView.currentUser();

        auto *overviewPage        = new OverviewView;
        auto *publicationPage     = new PublicationView;
        auto *ownReadingListPage  = new ReadingListView(ReadingListView::Mode::OwnList);
        auto *approvalsPage       = new ReadingListView(ReadingListView::Mode::AllLists);
        auto *userManagementPage  = new UserManagementView;

        OverviewView    &overviewView       = *overviewPage;
        PublicationView &publicationView    = *publicationPage;
        ReadingListView &ownReadingListView = *ownReadingListPage;
        ReadingListView &approvalsView      = *approvalsPage;
        UserManagementView &userManagementView = *userManagementPage;

        overviewView.setGreeting(currentUser.displayName());

        PublicationController publicationController(arxivClient, publicationRepository, publicationView);

        ReadingListController ownReadingListController(readingListService, authentication, ownReadingListView);
        ReadingListController approvalsController(readingListService, authentication, approvalsView);

        UserManagementController userManagementController(userManagementService, authentication, userManagementView);

        // Routes feedback from "add to reading list" (triggered from the
        // Publications tab) to both the reading list itself and to whatever
        // the user is currently looking at (AK-04.4 must be visible there too).
        ReadingListRouter readingListRouter(ownReadingListView, [&publicationView](const QString &message, bool isError) {
            if (isError) {
                publicationView.showError(message);
            } else {
                publicationView.showHint(message);
            }
        });
        ReadingListController addToListController(readingListService, authentication, readingListRouter);

        const auto refreshOverview = [&]() {
            overviewView.setMetrics(
                publicationRepository.count(),
                static_cast<int>(readingListService.ownList(currentUser).size()),
                static_cast<int>(readingListService.approvedForTraining().size()));
        };

        const auto showLastFetch = [&]() {
            overviewView.setLastFetch(QLocale::system().toString(QDateTime::currentDateTime(), QLocale::ShortFormat));
        };

        QObject::connect(&publicationView, &PublicationView::fetchRequested, &publicationView, [&]() {
            publicationController.fetchRequested();
        });
        QObject::connect(&publicationView, &PublicationView::disciplineSelected, &publicationView, [&](Discipline discipline) {
            publicationController.disciplineSelected(discipline);
        });
        QObject::connect(&publicationView, &PublicationView::addToReadingListRequested, &publicationView,
                         [&](int publicationId) {
                             addToListController.addToReadingList(publicationId);
                             refreshOverview();
                         });

        QObject::connect(&overviewView, &OverviewView::fetchRequested, &overviewView, [&]() {
            publicationController.fetchRequested();
        });

        // Automatic daily fetch (US-09): triggers the exact same controller
        // path as the manual button.
        QObject::connect(&arxivScheduler, &ArxivScheduler::automaticFetchDue, &publicationView, [&]() {
            publicationController.fetchRequested();
        });

        QObject::connect(&arxivClient, &ArxivClient::publicationsReceived, &overviewView, [&]() {
            showLastFetch();
            refreshOverview();
        });

        QObject::connect(&ownReadingListView, &ReadingListView::startReadingRequested, &ownReadingListView, [&](int entryId) {
            ownReadingListController.startReading(entryId);
        });
        QObject::connect(&ownReadingListView, &ReadingListView::completeReadingRequested, &ownReadingListView,
                         [&](int entryId, int rating, const QString &note) {
                             ownReadingListController.completeReading(entryId, rating, note);
                             refreshOverview();
                         });
        QObject::connect(&ownReadingListView, &ReadingListView::discardRequested, &ownReadingListView, [&](int entryId) {
            ownReadingListController.discard(entryId);
            refreshOverview();
        });

        QObject::connect(&approvalsView, &ReadingListView::approveRequested, &approvalsView, [&](int entryId) {
            approvalsController.approveForTraining(entryId);
            refreshOverview();
        });
        QObject::connect(&approvalsView, &ReadingListView::archiveRequested, &approvalsView, [&](int entryId) {
            approvalsController.archive(entryId);
            refreshOverview();
        });

        QObject::connect(&userManagementView, &UserManagementView::createRequested, &userManagementView,
                         [&](const QString &username, const QString &displayName, const QString &password, UserRole role) {
                             userManagementController.createUser(username, displayName, password, role);
                         });
        QObject::connect(&userManagementView, &UserManagementView::changeRoleRequested, &userManagementView,
                         [&](int userId, UserRole role) {
                             userManagementController.changeRole(userId, role);
                         });
        QObject::connect(&userManagementView, &UserManagementView::deactivateRequested, &userManagementView,
                         [&](int userId) {
                             userManagementController.deactivateAccount(userId);
                         });
        QObject::connect(&userManagementView, &UserManagementView::activateRequested, &userManagementView,
                         [&](int userId) {
                             userManagementController.activateAccount(userId);
                         });

        const UserRole role = currentUser.role();

        const QList<MainWindow::Page> pages = {
            { QObject::tr("Overview"),      &overviewView,       true },
            { QObject::tr("Publications"),  &publicationView,    true },
            { QObject::tr("My Reading List"), &ownReadingListView, true },
            { QObject::tr("Approvals"),     &approvalsView,      PermissionService::canViewAllReadingLists(role) },
            { QObject::tr("Users"),         &userManagementView, PermissionService::canManageUsers(role) }
        };

        QList<std::function<void()>> pageRefreshers;
        for (const MainWindow::Page &page : pages) {
            if (!page.visible) {
                continue;
            }
            if (page.widget == &overviewView) {
                pageRefreshers.append(refreshOverview);
            } else if (page.widget == &publicationView) {
                pageRefreshers.append([&]() { publicationController.refreshView(); });
            } else if (page.widget == &ownReadingListView) {
                pageRefreshers.append([&]() { ownReadingListController.refreshOwnList(); });
            } else if (page.widget == &approvalsView) {
                pageRefreshers.append([&]() { approvalsController.refreshAllLists(); });
            } else {
                pageRefreshers.append([&]() { userManagementController.refreshList(); });
            }
        }

        MainWindow mainWindow(currentUser, pages);

        QObject::connect(&mainWindow, &MainWindow::pageChanged, &mainWindow, [&](int pageIndex) {
            if (pageIndex >= 0 && pageIndex < pageRefreshers.size()) {
                pageRefreshers.at(pageIndex)();
            }
        });

        QObject::connect(&mainWindow, &MainWindow::logoutRequested, &mainWindow, [&]() {
            loginController.logoutRequested();
            loginAgain = true;
            mainWindow.close();
        });

        refreshOverview();
        publicationController.refreshView();

        mainWindow.show();

        // Start only now, so the scheduler cannot fire a fetch against a
        // view that does not exist yet while the login dialog is still open.
        arxivScheduler.start();

        app.exec();
    }

    return 0;
}
