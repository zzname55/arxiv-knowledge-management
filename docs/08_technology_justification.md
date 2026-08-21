# 08 — Justification of the Technical Decision

*Mandatory requirement 3*

## 1. Chosen Technology
C++20 with Qt 6.10 (Qt Widgets), CMake/Ninja, SQLite via QtSql, Qt Test.

## 2. Assessment

| Criterion | Assessment |
|---|---|
| Technical fit | The signal/slot system enforces loose coupling between view and controller |
| Development effort | GUI, networking, XML, database, cryptography, and tests all from one source (Qt) |
| Extensibility | Repositories are purely virtual interfaces |
| Maintainability | Static typing, `const`-correctness, RAII |
| Usability | Native widgets on Windows |
| Availability | Qt 6.10 SDK incl. Qt Creator fully installed |

## 3. Alternatives Considered

| Alternative | Why not chosen |
|---|---|
| Java/JavaFX | Requires a runtime, more involved setup |
| C#/WPF | Tied to the .NET ecosystem |
| Python/PyQt | Errors only surface at runtime, cumbersome to distribute |
| PHP/JS in the browser | Requires a web server, not a desktop application |

## 4. Data Source: arXiv API Instead of HTML Scraping
arXiv's `robots.txt` disallows automated fetching of the HTML pages. The API at
`export.arxiv.org/api/query` is the way the operator intends the same data to be
accessed: a fixed Atom structure, cleanly separated fields, lower server load.

## 5. Automatic Daily Fetch Instead of a Background Service
For the automatic 7 AM refresh requested afterward, two approaches were considered: a
`QTimer` inside the running application, or a separate Windows Task Scheduler entry
with a console-capable second execution path. The `QTimer` approach (`ArxivScheduler`)
was chosen because it requires no additional installation steps on the target machine
and can be tested with the same tooling (Qt) as the rest of the application. The
limitation that the automatic fetch only takes effect while the application is running
is explicitly documented in `CLAUDE.md`, Section 9, and in the sprint log.
