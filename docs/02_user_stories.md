# 02 — User Stories and Acceptance Criteria

*Mandatory requirement 4 (at least 5 user stories) and 5 (verifiable acceptance criteria)*

Format: **As a [role] I want [feature/goal], so that [benefit].**

## US-01 — Login
As an employee I want to log in with a username and password, so that only
authorized people have access.
- AC-01.1 The login screen appears before the main window.
- AC-01.2 Valid credentials open the main window.
- AC-01.3 Name and role stay visible at all times.
- AC-01.4 Wrong password → message shown, main window stays closed.
- AC-01.5 The message does not reveal which field was wrong.
- AC-01.6 Empty field → hint, no login attempt is made.
- AC-01.7 A deactivated account cannot log in.
- AC-01.8 The password is stored hashed (SHA-256 + salt).
- AC-01.9 "Logout" ends the session.

## US-02 — Fetch the Newest Publications
As an employee I want to fetch the five newest publications, so that I can stay
informed without manual research.
- AC-02.1 Clicking "Refresh" loads publications.
- AC-02.2 Exactly the 5 newest, sorted descending.
- AC-02.3 Title, authors, discipline, date, and arXiv ID are visible.
- AC-02.4 A loading indicator is shown during the fetch.
- AC-02.5 A clear error message is shown on a network failure.
- AC-02.6 Duplicate protection based on the arXiv ID.
- **AC-02.7 In addition, the application fetches automatically once a day at 07:00,
  as long as it is running, with no click required from the user.**

## US-03 — Filter by Discipline
As an employee I want to filter by discipline, so that I only see what is relevant.
- AC-03.1 At least 8 entries in the selector.
- AC-03.2 The filter narrows the list.
- AC-03.3 "All disciplines" shows everything again.
- AC-03.4 The result count is displayed.
- AC-03.5 No matches → a hint text instead of an empty area.

## US-04 — Add to the Reading List
As an employee I want to note a publication, so that I can find it again.
- AC-04.1 through AC-04.5 as before: status NOTED, immediately visible, no duplicates,
  only the user's own list is visible.

## US-05 — Maintain Processing State
As an employee I want to change the status and rate it on completion, so that it is
clear what has been read.
- AC-05.1 through AC-05.7 as before: enforced order, required fields at READ, boundary
  at READ, no access to other users' entries, persistence across restarts.

## US-06 — Detail View
As an employee I want to see all details including the abstract, so that I can decide
whether it's worth reading.

## US-07 — Approval for Internal Training
As a Knowledge Manager I want to approve papers that have been read and rated, so that
only reviewed content is distributed.

## US-08 — User Management
As an administrator I want to create users, assign roles, and lock accounts, so that
only authorized people have access.

## US-09 — Automatic Daily Refresh *(new, 2026-08-21)*
As an employee I want the application to look for new publications by itself every
morning at 7 AM, so that I already see up-to-date data the first time I open it,
without having to remember to check.
- AC-09.1 If the application is running past 7:00 AM local time, it fetches automatically.
- AC-09.2 The automatic fetch triggers at most once per calendar day.
- AC-09.3 The manual "Refresh" click continues to work independently at any time, and
  is not blocked by the automatic fetch.
- AC-09.4 After an automatic fetch, the timestamp is visible on the overview page
  ("Last fetched: ..."), without needing to distinguish whether it was automatic or manual.
