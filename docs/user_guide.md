# User Guide

## Logging In

When you start the application you see a login screen. Enter your username and
password and click "Login". If the username or password is wrong, you'll see a
message — it won't tell you which one, for security reasons. If your account has been
deactivated by an administrator, login is refused with a message as well.

Three test accounts exist out of the box (starting password `start1234` for all):

| Username | Role |
|---|---|
| `ma01` | Employee |
| `wm01` | Knowledge Manager |
| `admin01` | Administrator |

## Overview

After logging in you land on the Overview tab. It shows:

- how many publications are in the system,
- how many entries are on your own reading list,
- how many papers have been approved for training,
- when publications were last fetched from arXiv (whether that fetch happened
  automatically at 7 AM or because someone clicked "Refresh" — you don't need to know
  which).

There's also a "Refresh" button here, identical to the one on the Publications tab.

## Publications

The Publications tab lists the newest papers from arXiv. Use the discipline dropdown
to narrow the list down; pick "All disciplines" to see everything again. Click a row to
see the full abstract and details on the right. Click "Add to reading list" to note a
paper for yourself — you'll see a confirmation, and it immediately shows up on "My
Reading List".

New publications are fetched automatically once a day, at 7 AM, as long as the
application is running. You can also click "Refresh" at any time to fetch the newest
publications on demand — both ways work independently of each other.

## My Reading List

This is your personal list, moving through a fixed process:

```
NOTED → IN_PROGRESS → READ
```

- **Start reading** moves a NOTED entry to IN_PROGRESS.
- **Mark as read** opens a small dialog asking for a rating (1–5) and an optional note,
  then moves the entry to READ.
- **Discard** removes an entry you decided not to pursue (available for NOTED and
  IN_PROGRESS entries).

You only ever see your own entries here — nobody else can see or edit your reading
list, and you can't see anyone else's.

Once a paper reaches READ, you can't do anything further with it yourself — approving
it for training is a separate step that only a Knowledge Manager can take (see below).

## Approvals (Knowledge Manager and Administrator only)

This tab is not shown to employees. Here you see every reading list entry from every
user. Once an entry has been read and rated, you can:

- **Approve for training** — marks it as cleared for internal training sessions.
- **Archive** — moves an approved entry into the archive once the training has
  happened.

## Users (Administrator only)

Only administrators see this tab. From here you can:

- create a new account (username, display name, password, role),
- change a user's role,
- deactivate or reactivate an account.

A deactivated account can no longer log in, but its existing reading list data is kept.

## Logging Out

Click "Logout" in the top bar at any time. This returns you to the login screen without
closing the application.
