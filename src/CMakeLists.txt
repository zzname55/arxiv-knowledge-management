# ---------------------------------------------------------------------------
# Sources for the three MVC layers.
# ---------------------------------------------------------------------------

# --- MODEL layer ------------------------------------------------------------
add_library(bws_model STATIC
    model/UserRole.h
    model/UserRole.cpp
    model/User.h
    model/User.cpp
    model/PasswordHasher.h
    model/PasswordHasher.cpp
    model/UserRepository.h
    model/AuthenticationService.h
    model/AuthenticationService.cpp

    model/Database.h
    model/Database.cpp
    model/SqliteUserRepository.h
    model/SqliteUserRepository.cpp
    model/SqlitePublicationRepository.h
    model/SqlitePublicationRepository.cpp

    model/Discipline.h
    model/Discipline.cpp
    model/Publication.h
    model/Publication.cpp
    model/PublicationRepository.h
    model/ArxivAtomParser.h
    model/ArxivAtomParser.cpp
    model/ArxivClient.h
    model/ArxivClient.cpp
    model/ArxivScheduler.h
    model/ArxivScheduler.cpp

    model/ReadingStatus.h
    model/ReadingStatus.cpp
    model/ReadingListEntry.h
    model/ReadingListEntry.cpp
    model/PermissionService.h
    model/PermissionService.cpp
    model/ReadingListRepository.h
    model/SqliteReadingListRepository.h
    model/SqliteReadingListRepository.cpp
    model/ReadingListService.h
    model/ReadingListService.cpp

    model/OperationResult.h
    model/UserManagementService.h
    model/UserManagementService.cpp
)

target_include_directories(bws_model PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(bws_model PUBLIC
    Qt6::Core
    Qt6::Network
    Qt6::Sql
)

# --- CONTROLLER layer --------------------------------------------------------
add_library(bws_controller STATIC
    controller/LoginViewContract.h
    controller/LoginController.h
    controller/LoginController.cpp
    controller/PublicationViewContract.h
    controller/PublicationController.h
    controller/PublicationController.cpp
    controller/ReadingListViewContract.h
    controller/ReadingListController.h
    controller/ReadingListController.cpp
    controller/UserManagementViewContract.h
    controller/UserManagementController.h
    controller/UserManagementController.cpp
)

target_include_directories(bws_controller PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(bws_controller PUBLIC
    bws_model
    Qt6::Core
)

# --- VIEW layer ---------------------------------------------------------------
add_library(bws_view STATIC
    view/LoginView.h
    view/LoginView.cpp
    view/OverviewView.h
    view/OverviewView.cpp
    view/PublicationView.h
    view/PublicationView.cpp
    view/CompleteReadingDialog.h
    view/CompleteReadingDialog.cpp
    view/ReadingListView.h
    view/ReadingListView.cpp
    view/ReadingListRouter.h
    view/UserManagementView.h
    view/UserManagementView.cpp
    view/MainWindow.h
    view/MainWindow.cpp
    view/SortableTableItem.h
)

target_include_directories(bws_view PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(bws_view PUBLIC
    bws_controller
    Qt6::Widgets
)

# --- Executable ---------------------------------------------------------------
qt_add_executable(ArxivKnowledgeManagement
    app/main.cpp
)

target_link_libraries(ArxivKnowledgeManagement PRIVATE
    bws_view
)

set_target_properties(ArxivKnowledgeManagement PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE    TRUE
)
