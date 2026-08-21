# 01 — Process Analysis

*Mandatory requirement 1.1: implementation of a business process*

---

## 1. Starting Situation

*TechnoLab GmbH* is a mid-sized company with around 80 employees in software
development and data analysis. The development department depends on staying current
with the state of research. To do this, employees follow recent scientific
publications, mostly preprints on **arXiv.org**, the largest open publication server
for computer science, mathematics, physics, statistics, biology, economics, and
electrical engineering.

The internal training program draws on these publications: whatever is relevant and
understood gets passed on to the rest of the team in an internal training session.

---

## 2. Current State

### 2.1 Description

1. Employees open arXiv **manually in the browser** and search the category pages for
   new publications.
2. Interesting finds are saved as **browser bookmarks**, emailed **to themselves**, or
   noted in a **private notes file**.
3. Whether a paper was actually read is known only to that one person.
4. Relevant papers are **emailed to the whole team**, with no indication of whether
   they were actually reviewed.
5. Topics for internal training sessions are chosen **from memory** by department
   leadership during the monthly meeting.

### 2.2 Weakness Analysis

| No. | Weakness | Impact | Severity |
|-----|---------------|-----------|---------|
| S1 | No central storage | Knowledge cannot be found and is lost when staff leave | high |
| S2 | No visible processing state | No one knows whether a paper was read or just filed away | high |
| S3 | Duplicate effort | Multiple people research and read the same paper | medium |
| S4 | Manual research | High time cost, no cross-discipline view | medium |
| S5 | No quality control | Unreviewed content goes out to the whole distribution list | high |
| S6 | Arbitrary topic selection | No traceable basis for training sessions | medium |
| S7 | No access control | Anyone can forward anything, no accountability | high |
| S8 | No history | No way to trace what was approved, when, or why | low |
| S9 | No automatic fetching | New papers are noticed only when someone checks manually | medium |

---

## 3. Target Process

### 3.1 Objective

A central desktop application that

- fetches the newest publications from arXiv **automatically** (daily at 7 AM) and
  **on demand** (clicking "Refresh"),
- allows **filtering by discipline**,
- gives every employee a **personal reading list** with a traceable processing state,
- **requires approval from a responsible role** before content is distributed, and
- secures the whole process with **role-based access control**.

### 3.2 The Optimized Target Process

```
        ┌────────────────────────────────────────────────────┐
        │  arXiv API (export.arxiv.org/api/query)             │
        │  automatic daily at 07:00 + manual on click          │
        └────────────────────┬───────────────────────────────┘
                             v
┌──────────────────────────────────────────────────────────────┐
│  OVERVIEW: newest publications, filter: discipline            │
└────────────────────┬─────────────────────────────────────────┘
                     │  Employee: "Add to reading list"
                     v
        ╔════════════════════════╗
        ║  NOTED                 ║
        ╚═══════════╤════════════╝
                    │  Employee: "Start reading"
                    v
        ╔════════════════════════╗
        ║  IN_PROGRESS            ║
        ╚═══════════╤════════════╝
                    │  Employee: "Mark as read" + rating/note
                    v
        ╔════════════════════════╗
        ║  READ                   ║  ← boundary of employee permissions
        ╚═══════════╤════════════╝
                    │  ▲ only Knowledge Manager: "Approve for training"
                    v
        ╔════════════════════════╗
        ║  APPROVED_FOR_          ║
        ║  TRAINING               ║
        ╚═══════════╤════════════╝
                    │  ▲ only Knowledge Manager: "Archive"
                    v
        ╔════════════════════════╗
        ║  ARCHIVED               ║
        ╚════════════════════════╝
```

### 3.3 Weaknesses Resolved

Weaknesses S1 through S8 are resolved, as in the original version, through central
data storage, a status state machine, a role model, and an approval process.
Additionally:

| Weakness | Solution |
|---|---|
| S9 No automatic fetching | The application fetches automatically at 7 AM daily, in addition to being available on click at any time |

---

## 4. Scope Boundaries

Not part of this project:

- Full-text download of PDF files from arXiv (metadata and reference URL only)
- Multi-user operation via a database server (SQLite is single-machine)
- Fetching while the application is closed (no Windows service, no Task Scheduler entry)
