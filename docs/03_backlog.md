# 03 — Product Backlog and Sprint Planning

*Mandatory requirement 1.5*

Approach: Scrum-oriented with a Kanban board, prioritized using MoSCoW.

| ID | Backlog Item | Priority | Sprint |
|----|-----------------|-----------|--------|
| B-01 | Project skeleton, CMake, test infrastructure | Must | 1 |
| B-02 | User + role data model | Must | 1 |
| B-03 | Salted password hashing | Must | 1 |
| B-04 | Authentication service | Must | 1 |
| B-05 | SQLite schema and database connection | Must | 1 |
| B-06 | User repository | Must | 1 |
| B-07 | Publication + discipline data model | Must | 1 |
| B-08 | arXiv Atom response parser | Must | 1 |
| B-09 | HTTP fetch of the arXiv API (asynchronous) | Must | 1 |
| B-10 | Publication repository incl. duplicate protection | Must | 1 |
| B-11 | Filtering by discipline | Must | 1 |
| B-12 | Reading list entry + status state machine | Must | 2 |
| B-13 | Permission service (three roles) | Must | 2 |
| B-14 | Reading list entry repository | Must | 2 |
| B-15 | Login screen (view) | Must | 2 |
| B-16 | Main window with navigation | Must | 2 |
| B-17 | Overview page | Must | 2 |
| B-18 | Publications list view | Must | 2 |
| B-19 | Detail view | Must | 2 |
| B-20 | "My Reading List" view | Must | 2 |
| B-21 | Rating + note input form | Must | 2 |
| B-22 | "Approvals" view | Must | 2 |
| B-23 | User management | Must | 2 |
| B-24 | Consistent error messages | Should | 2 |
| B-25 | Acceptance test incl. negative test cases | Must | 2 |
| B-26 | Final documentation | Must | 2 |
| B-27 | Column-header sorting of lists | Could | added 2026-08-16 |
| B-28 | Export training list as CSV | Could | open |
| B-29 | Dark color scheme | Could | open |
| B-30 | **Automatic daily refresh at 7 AM** | **Must (requested later)** | **added 2026-08-21** |

**Note 2026-08-21:** After the complete loss of the source folder, the project was
rebuilt from scratch. In the process B-30 was added: an `ArxivScheduler` that fetches
automatically once a day at 7:00 AM, in addition to the manual "Refresh" click, which
continues to work as before.
