# 04 — User Interface Design

*Mandatory requirement 1.3*

## Required Areas

| Area | Implementation |
|---|---|
| Overview page | "Overview" tab with metrics, "Last fetched", quick access |
| Input forms | Login screen, "Complete reading", "Create user" |
| List view | Publications table, reading list table (sortable by column header) |
| Detail view | To the right of the list |
| Navigation | Fixed tab bar on the left, filtered by role |

## Wireframes (short version, see the sprint log for the full version)

```
W1  Login screen:       Username / Password / Error line / Login
W2  Main window:        Header (name, role, logout) | Navigation | Content
W3  Publications:        Discipline filter, Refresh button, table, detail on the right
W4  My Reading List:     Table with status, rating, action buttons
W5  Complete reading:    Rating 1-5, note, Save/Cancel
W6  Approvals:           Table of all reading lists, Approve/Archive
W7  Users:               Account table, new-account form, change role/lock
```

## New: Feedback for the Automatic Refresh

The overview page shows the timestamp of the last fetch below the metrics. When an
automatic fetch runs at 7 AM, this line updates by itself, exactly as it does after a
manual click — the user does not need to tell which path produced the current data.

## Design Principles

Usability, clarity, consistency, sensible navigation, understandable labels,
appropriate error messages, input validation — as established in the original design
and carried over unchanged.
