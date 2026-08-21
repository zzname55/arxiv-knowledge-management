# 05 — MVC Architecture

*Mandatory requirement 1.4*

## Layers

```
src/
  model/       Data + business logic. Knows Qt Core/Sql/Network, never Qt Widgets.
  view/        Presentation only. No business logic, no SQL, no network calls.
               Reports user actions exclusively via Qt signals.
  controller/  Receives view signals, calls the model, returns results to the view.
  app/         main.cpp — the composition root that wires model, view, and controller.
```

The separation is enforced at build time: `bws_model` and `bws_controller` are CMake
targets that link only against `Qt6::Core`, `Qt6::Sql`, and `Qt6::Network` — never
`Qt6::Widgets`. Any accidental `#include <QWidget>` (or similar) inside `src/model/`
fails to compile, because `bws_model` has no Widgets include path.

## One Traced Data Flow: "Add to Reading List"

1. **View** — the user clicks "Add to reading list" in `PublicationView`. The row's
   publication ID is read from the table and the widget emits
   `addToReadingListRequested(int publicationId)`. The view itself does not touch the
   database or the reading list service.
2. **Controller** — `main.cpp` connects that signal to a lambda that calls
   `ReadingListController::addToReadingList(publicationId)`. The controller asks
   `AuthenticationService` for the currently logged-in user and calls
   `ReadingListService::addToList(user, publicationId)`.
3. **Model** — `ReadingListService` first asks `PermissionService` whether this role is
   allowed to maintain a reading list at all, then checks
   `ReadingListRepository::findByUserAndPublication(...)` for a duplicate. If none
   exists, it creates a `ReadingListEntry` with status `Noted` and persists it through
   `SqliteReadingListRepository`, which issues the actual `INSERT` against SQLite.
4. **Model → Controller** — `ReadingListService` returns an `OperationResult` (success
   or a human-readable failure reason).
5. **Controller → View** — the controller calls `showMessage(...)` or `showError(...)`
   on the `ReadingListViewContract` interface it was given. In the composition root,
   that interface is implemented by `ReadingListRouter`, which forwards the message to
   both the (background) reading list view and to whichever view the user is currently
   looking at, so the feedback is visible immediately even though "My Reading List"
   might not be the active tab.

At no point does `PublicationView` or `ReadingListView` talk to a repository or a
service directly — every step passes through the controller, matching the hard rule in
`CLAUDE.md` Section 3.

## Why a Router Adapter for the Reading List View

`ReadingListRouter` implements `ReadingListViewContract` but is not a widget. It exists
because "add to reading list" can be triggered from a tab (Publications) that is not
the reading list tab itself. Rather than let the controller know about two views, or
let the publications view reach into the reading list view, the router hides that
fan-out behind the same interface the controller already depends on — the controller
still only knows one `ReadingListViewContract`.
