# 06 — Sprint Log

*Mandatory requirement 1.5*

## Sprint 1 — Foundation
**Implemented:** Project skeleton (CMake + Ninja + MinGW), users/roles/password
hashing/authentication, SQLite schema and connection, user repository, publication +
discipline model, arXiv Atom parser, asynchronous HTTP fetch, publication repository
with duplicate protection, discipline filtering.
**Problems:** None outside the normal TDD cycle.
**Adjustments:** None.
**Open:** Everything scheduled for Sprint 2.

## Sprint 2 — Process, Roles, Views
**Implemented:** Reading list entry + status state machine, permission service (three
roles), reading list repository, login screen, main window with navigation, overview
page, publications list view, detail view, "My Reading List" view, rating/note form,
"Approvals" view, user management, consistent error messages, acceptance test with
negative cases, final documentation.
**Problems:**
- `NOT NULL constraint failed: readingListEntry.note` — a default-constructed `QString`
  binds as SQL `NULL` even with a `DEFAULT ''` column, because Qt distinguishes a null
  string from an empty one. Fixed with a small helper that substitutes an empty string
  for a null `QString` before binding.
- A crash on logout (`QStackedWidget` double free, exit code `-1073741819`) — page
  widgets were being destroyed twice because ownership was ambiguous. Fixed by
  documenting and enforcing that all page widgets are heap-allocated with `new` and
  handed to `MainWindow`, which then owns them unconditionally.
**Adjustments:** None to scope.
**Open:** B-28 (CSV export), B-29 (dark theme) — both `Could`, deferred.

## Sprint — B-27 Column-Header Sorting (2026-08-16)
**Implemented:** Sortable columns on the publications, reading list, and user tables
via a `SortableTableItem` that stores a typed sort key separately from the display
text.
**Problems:** Selecting a row, sorting the table, then acting on "the selected row"
operated on the wrong entry — the code tracked the selection by row index, and sorting
changes which row a given index points to.
**Adjustments:** Every sortable table now stores the underlying database ID on the
first cell of each row (a custom item-data role) and looks the row up by ID instead of
index whenever the user acts on a selection. Applied consistently across all three
sortable tables.
**Open:** None.

## Sprint — B-30 Automatic Daily Refresh (2026-08-21)
**Implemented:** `ArxivScheduler`, which checks once a minute (and immediately on
start) whether local time has passed 07:00 since the last automatic fetch that day; if
so it emits a signal that triggers the same controller path as the manual "Refresh"
button. Composition root wires the scheduler's signal to
`PublicationController::fetchRequested()` — from the controller's perspective an
automatic and a manual fetch are indistinguishable.
**Context:** Requested after the original project folder had disappeared without a
recoverable trace; the project was rebuilt from the full conversation history, and this
feature was added during the rebuild rather than as a separate later change.
**Problems:** None — the pure due/not-due logic (`ArxivScheduler::isDue`) is tested with
fabricated `QDateTime` values; the `QTimer`-driven wall-clock wiring is deliberately not
unit-tested and is instead covered once, end-to-end, in the acceptance test.
**Open:** Fetching while the application is closed is out of scope (see
`08_technology_justification.md`, Section 5).

## Sprint — Full English Rewrite (2026-08-21)
**Implemented:** The entire codebase (identifiers, comments, UI strings, tests,
documentation) rewritten from German to English at the user's request, including
renaming every source, header, and test file. GitHub repository set up at
`zzname55/arxiv-knowledge-management`.
**Problems:** A first-pass automated translation script used unguarded substring
replacement, which corrupted identifiers that contained a translated word as a
substring of a longer word (e.g. `Controller` → `Controler` via a `rolle` → `role`
replacement inside "Cont-rolle-r"). Fixed by requiring word boundaries in the
replacement regex and re-running from a clean copy of the German source, since nothing
had been committed yet.
A second-pass translation of local variable/parameter names also touched a hardcoded
test-fixture string comparison (`tst_arxivatomparser.cpp`) that was meant to match a
title string still written in German inside the corresponding `.xml` test data file,
briefly breaking that one test; caught and fixed immediately via the compiler/test run
before anything was committed.
**Open:** None — all 16 test suites pass on the English tree.
