# CLAUDE.md — Binding Project Guidelines

This file is the single source of truth for how work is done in this project.
It is read at every session and must be followed. No `Roadmap.md`.

**Note 2026-08-21:** The entire project folder (src, tests, build, CMakeLists.txt,
CLAUDE.md) disappeared from the local OneDrive folder with no identifiable cause
(Windows Recycle Bin empty, no shadow copies, no File History). Only `docs/` survived.
The project was fully reconstructed from the conversation history. The automatic daily
arXiv refresh was added in the process (see Section 9).

**Note 2026-08-21 (2):** The entire codebase was rewritten from German to English at
the user's request — identifiers, comments, UI strings, tests, and documentation. See
Section 5 for the updated comment-language policy.

---

## 1. Project

**Name:** ArxivKnowledgeManagement
**Context:** School project (BWS) "Designing and Developing User Interfaces"
**Basis:** `docs/Anforderungen.pdf` (mandatory requirements 1.1–1.5, user stories, acceptance criteria, acceptance test)

**Core domain:** Internal knowledge management application. Scientific publications are
imported from arXiv, added to personal reading lists, go through a processing workflow,
and are approved for internal training sessions.

**Business process being modeled (mandatory requirement 1.1):**

```
Import from arXiv → NOTED → IN_PROGRESS → READ → APPROVED_FOR_TRAINING → ARCHIVED
```

---

## 2. Technology — Not Negotiable

| Item | Decision |
|---|---|
| Language | C++20 |
| Framework | Qt 6.10.0, Widgets (no QML) |
| IDE | Qt Creator (`C:\Qt\Tools\QtCreator\bin\qtcreator.exe`) |
| Kit | `mingw_64` (MinGW 13.1) |
| Build system | CMake + Ninja |
| Persistence | SQLite via `Qt6::Sql` (`QSQLITE`) |
| Network | `Qt6::Network` |
| XML | `QXmlStreamReader` (`Qt6::Core`) |
| Tests | `Qt6::Test` (QTest) |
| Data source | **arXiv API** `https://export.arxiv.org/api/query` — **no** HTML scraping (arXiv's `robots.txt` disallows crawling the HTML pages; the API is the interface the operator intends for the same data) |

---

## 3. Architecture — MVC (Mandatory Requirement 1.4)

Strict separation, enforced by the build system (`bws_model` and `bws_controller` do
not link against `Qt6::Widgets`).

```
src/
  model/       Data + business logic. Knows Qt Core/Sql/Network, NEVER Qt Widgets.
  view/        Presentation. Knows Widgets. Contains NO business logic, NO SQL,
               NO network calls. Reports user actions only via signals.
  controller/  Receives view signals, calls the model, returns results to the view.
  app/         main.cpp — wires up model, view, and controller (composition root).
```

**Hard rules**

1. `#include <QWidget>` (or any other Widgets header) must **never** appear in `src/model/`.
2. A view knows **no** repository and **no** service. Only its own widgets.
3. The controller is the **only** place where view and model meet.
4. Data flow is always: `View (signal) → Controller → Model → Controller → View (setter/slot)`.
5. Database access only through `*Repository` interfaces (purely virtual).
6. Permission checks live in the **model** (`PermissionService`), not in the view.

---

## 4. Approach — TDD in Phases

**Cycle:** RED (failing test) → GREEN (minimal code) → REFACTOR.
No production code without a test that failed first (exception: pure view classes).
The next phase starts only once the current one is green.

**Phase Plan**

| Phase | Content | Status |
|---|---|---|
| 0 | Process analysis, user stories, acceptance criteria, backlog, wireframes | **done** |
| 1 | Project skeleton, CMake, build, first test runs | **done** |
| 2 | Model: users, roles, password hashing, authentication | **done** |
| 3 | Persistence: SQLite schema, repositories | **done** |
| 4 | arXiv integration: Atom parser, HTTP client, discipline filter | **done** |
| 5 | Reading list process: status state machine, permissions | **done** |
| 6 | Controller layer | **done** |
| 7 | Views, navigation, validation, error messages | **done** |
| 8 | Acceptance test, negative test cases, final documentation | **done** |
| 9 | Automatic daily arXiv refresh at 7 AM | **done** |
| 10 | Full English rewrite of the codebase | **done** |

Build and verify:

```
cmake -S . -B build -G Ninja -DCMAKE_PREFIX_PATH=C:/Qt/6.10.0/mingw_64
cmake --build build
ctest --test-dir build --output-on-failure
```

---

## 5. Clean Code

- **Names say what is meant.** `approvePaperForTraining()` instead of `doIt()`.
- **One function, one job.** Rule of thumb: max. ~20 lines.
- **No magic values.** Name your constants.
- **No duplication.**
- **Don't swallow errors.** No empty `catch`, no ignored `bool`.
- **Consistent `const`**, pass complex types as `const T&`.
- **RAII**, no manual `new`/`delete`; Qt parent ownership or `std::unique_ptr`.

### Comments — Explicit Client Requirement

- **Every class** gets a header comment: purpose, MVC layer, how it interacts with others.
- **Every public method** gets a comment: what it does, parameters, return value, failure cases.
- **In the body**, comment the *why*, not the *what*.
- Comment language: **English**. Identifiers: **English** throughout, including domain
  terms (`readingList`, `discipline`, `approval`), consistent with the full rewrite in
  Section 0's note above. (Previously this project required German comments and German
  domain terms; that policy was superseded when the codebase was rewritten to English.)

---

## 6. Documentation

| File | Content | Mandatory Requirement |
|---|---|---|
| `01_process_analysis.md` | Current state, process diagram, weaknesses, target process | 1.1 |
| `02_user_stories.md` | User stories + acceptance criteria | 4, 5 |
| `03_backlog.md` | Product backlog, prioritization, sprints | 1.5 |
| `04_ui_design.md` | Wireframes/mockups before implementation | 1.3 |
| `05_architecture_mvc.md` | MVC implementation + one fully traced data flow | 1.4 |
| `06_sprint_log.md` | Per sprint: implemented / problems / adjustments / open | 1.5 |
| `07_acceptance_test.md` | Acceptance table incl. negative test cases | 6 |
| `08_technology_justification.md` | Justification of C++/Qt against the alternatives | 3 |

---

## 7. Roles and Permissions (Mandatory Requirement 1.2)

| Role | Test user | May |
|---|---|---|
| Employee | `ma01` | Search/import papers, maintain **own** reading list, set status up to `READ` |
| Knowledge Manager | `wm01` | additionally: view **all** reading lists, approve papers for training, archive |
| Administrator | `admin01` | additionally: create/edit/deactivate users, assign roles |

Starting password for all three test accounts: `start1234`.

---

## 8. Working with the Assistant

- Before each phase: briefly say what's being built. Afterward: what's green, what's open.
- No unrequested extra features. The requirements catalog is the scope.
- If a requirement is ambiguous: state the assumption, keep going, don't block.
- Tests are actually run after every phase; the result is reported honestly.

---

## 9. Automatic Daily Refresh (requested afterward, 2026-08-21)

In addition to the manual "Refresh" click, the application fetches the arXiv API
**automatically once a day at 07:00 local time**, as long as it is running.

**Deliberate limitation:** This is a Qt Widgets desktop application, not a background
service. The automatic fetch only works while the application is open — it checks on
startup and then every minute whether the 7 AM mark has been passed since the last
fetch. Fetching while the application is closed would require a separate Windows Task
Scheduler entry; that is explicitly **not** part of this task and would require a
second, console-capable execution mode of the application.

The manual "Refresh" click remains available at any time alongside this and fetches
additionally, independent of the time of day — exactly as requested.
