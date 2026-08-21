# 07 — Acceptance Test

*Mandatory requirement 6*

All rows below are automated in `tests/tst_acceptancetest.cpp`, driven against real
widgets via `QTest::mouseClick`/`QTest::keyClicks` — no mocked UI layer — plus the
scheduler test T21 described at the bottom. Run with:

```
ctest --test-dir build --output-on-failure -R tst_acceptancetest
```

## Positive Cases

| # | Scenario | Expected Result | Status |
|---|---|---|---|
| T01 | Log in as `ma01` with the correct password | Main window opens, name and role visible | Pass |
| T02 | Click "Refresh" on the Publications tab | 5 newest publications load, loading indicator shown then hidden | Pass |
| T03 | Select a discipline filter | List narrows, result count updates | Pass |
| T04 | Select "All disciplines" | Full list returns | Pass |
| T05 | Click "Add to reading list" | Entry appears in "My Reading List" with status NOTED | Pass |
| T06 | Click "Start reading" on a NOTED entry | Status changes to IN_PROGRESS | Pass |
| T07 | Click "Mark as read", enter rating + note, save | Status changes to READ, rating/note stored | Pass |
| T08 | Log in as `wm01`, open "Approvals" | All employees' reading lists are visible | Pass |
| T09 | Approve a READ entry for training | Status changes to APPROVED_FOR_TRAINING | Pass |
| T10 | Archive an approved entry | Status changes to ARCHIVED | Pass |
| T11 | Log in as `admin01`, create a user | New account appears in the user table | Pass |
| T12 | Change a user's role | Role updates immediately | Pass |
| T13 | Deactivate a user | Account marked inactive, cannot log in afterward (see T18) | Pass |
| T14 | Restart the application, log in again | All previously saved reading list entries persist | Pass |

## Negative Cases

| # | Scenario | Expected Result | Status |
|---|---|---|---|
| T15 | Log in with a wrong password | Generic error shown, main window stays closed, field not identified | Pass |
| T16 | Submit the login form with an empty username or password | Hint shown, no login attempt made | Pass |
| T17 | Employee (`ma01`) attempts to open "Approvals" or "Users" | Tabs are not shown, and `PermissionService` also denies the action if invoked directly (bypassing the UI), confirming the check is enforced in the model, not just hidden in the view | Pass |
| T18 | Log in with a deactivated account | Login is refused with an error message | Pass |
| T19 | Employee attempts to approve their own READ entry for training | Denied by `PermissionService`; the "Approve" action is only reachable by a Knowledge Manager or Administrator | Pass |
| T20 | Add the same publication to the reading list twice | Second attempt is rejected as a duplicate, no second entry created | Pass |

## Scheduler Coverage (US-09 / B-30)

| # | Scenario | Expected Result | Status |
|---|---|---|---|
| T21 | Wire a real `ArxivScheduler` exactly as in `main.cpp`; call `pruefeJetzt`/`checkNow` at 06:30 | No fetch triggered | Pass |
| T21 | Same scheduler, call again at 07:01 | Exactly one fetch triggered, through the same controller path as the manual button | Pass |
| T21 | Same scheduler, call again same day at 18:00 | No second fetch (already fired today) | Pass |

Additional pure-logic coverage for `ArxivScheduler::isDue` (11 cases: before/at/after
the fetch time, same-day suppression, next-day re-trigger, signal emission, custom
fetch times) lives in `tests/tst_arxivscheduler.cpp` and uses only fabricated
`QDateTime` values, never the real wall clock.
