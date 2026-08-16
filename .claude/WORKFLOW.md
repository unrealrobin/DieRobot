# Die Robot — Claude Workflow

The sequence every change follows. Conventions live in `.claude/rules/` — this file is *the
order things happen*, not the conventions themselves.

Three parts:

1. **Setup** — once, at the start of a task
2. **The change loop** — repeats once per logical change, ending in a commit
3. **Close-out** — once, when the branch is ready to become a PR

A step marked **[BLOCKING]** means work stops on failure. Do not proceed until resolved.

---

## The three lanes

A game repo does not contain one kind of change. This workflow recognises three, and the
lane a change is in determines which steps it takes.

| Lane | What it is | How you know |
|---|---|---|
| **Code** | C++ under `Source/`, build files, `Config/*.ini` | The default. Takes every step. |
| **Content** | Blueprints, maps, DataAssets, art, audio under `Content/` | Binary `.uasset`. `git diff` shows nothing. See `rules/content-changes.md`. |
| **EXP** | A design experiment being tried, not shipped | Branch `exp/*`. Exempt from tests and review. **Never exempt from process.** |

Most real branches are **mixed** — a C++ change plus the Blueprint that calls it. A mixed
change takes the union of both lanes' requirements, never the cheaper one.

---

## A note on the two code reviews

They are **different mechanisms** and the distinction is load-bearing:

| Review | Mechanism | Who runs it |
|---|---|---|
| Per-change review, full-PR review | The **`code-review` skill**, invoked via the Skill tool | Claude |
| Ultra review | The **`/code-review ultra` slash command** | **User only** |

`/code-review ultra` is user-triggered and billed. **Claude cannot launch it** — not via
Bash, not via the Skill tool. Any step calling for ultra is necessarily a stop-and-hand-off.

---

## Documentation-only changes

Some changes contain no behavior — a rules edit, a checklist update. Running `test-writer`
on prose produces nothing, and a performance review of a markdown diff is theatre.

**A change qualifies as documentation-only when every file it touches is markdown or plain
prose, and none of them alters behavior.**

| File | Qualifies? |
|---|---|
| `*.md` anywhere | Yes |
| Anything under `Source/` | **No** |
| Anything under `Content/` | **No — content lane** |
| `Config/*.ini`, `*.uproject`, `*.Build.cs`, `*.Target.cs` | **No — these are behavior** |
| `.github/workflows/*`, `.gitattributes`, `.gitignore` | **No — behavioral** |

A qualifying change may skip **L.2** (tests), **L.3** (performance), **L.4** (code review),
**C.1**, **C.2** (smoke) and **C.4**. It still takes S.1–S.3, L.1, L.5, C.3, C.5 and C.6 —
the task still needs an issue, a branch, a commit, a check against what was asked for, a
vault answer, and a PR that meets the standard.

**State the exemption in the commit body when you use it.** An exemption taken without
writing it down is how a rule quietly stops meaning anything.

The exemption is for **prose, not for small diffs**. When a change is mostly prose but
touches one behavioral file, the honest options are to run the steps, or to split the
behavioral file into its own commit that takes them.

---

# Setup

## S.1 — Confirm rules are loaded

This session must have read every file in `.claude/rules/` at least once:

- `architecture.md`
- `unreal-style.md`
- `content-changes.md`
- `git-workflow.md`
- `linear-issues.md`

If already read in this session, skip.

## S.2 — Read the relevant vault docs

The vault at `~/Documents/RLOV/DieRobot/` is the source of truth for *how Die Robot is
supposed to work*. It is not in git and will not appear in any repo search — you must read
it deliberately.

**Always, every session:** `02 Technical/Current State.md`. It carries the engine paths, the
toolchain traps, the open bug list, and the gotchas that have already cost hours. Skipping
it is how the same hour gets burned twice.

Then, by area:

| If the task touches... | Read in vault... |
|---|---|
| Any system's real state, or a bug | Systems Inventory |
| Enemy behavior, wave pacing, player verbs, difficulty | Pillars & Decisions |
| Something the design hasn't settled | Open Questions & Graveyard |
| Why a past technical choice was made | Decision Log |
| Build, CI, test infrastructure | Setup Brief |
| A term you don't recognize | Glossary |

The plan you propose must be consistent with what the vault describes. **Where it conflicts,
say so out loud rather than silently picking one** — a design decision the code violates is
a finding, not an inconvenience. `ScaleHealth()` inflating enemy health against decision D4
is the standing example.

If the vault doesn't cover the area you're touching, flag it as a gap — close-out step C.4
may add a page.

## S.3 — Linear & branch

- Confirm an issue exists in team **DIE** under the **Die Robot** initiative, in the right
  project. Create one if not, following `rules/linear-issues.md`.
- Link the issue to the project and set it to **In Progress** in a single `save_issue` call.
- Create a `die-XX` branch off `main` per `rules/git-workflow.md`.

**[BLOCKING]** If the current branch is a Claude worktree branch (`claude/*`) rather than
`die-XX`, stop and say so. Linear cannot link a `claude/*` branch.

**[BLOCKING] Confirm the branch point is current.** Run `git log --oneline -1 origin/main`
and confirm the branch is based on it. A worktree created earlier in the session may sit on
a stale commit — this repo has already had a branch silently eight months behind `main`, and
with 8.8 GB of binary content that is not a preference, it is a trap.

---

# The Change Loop

**Repeats once per logical change.** A PR is N passes through this loop, producing N
commits. Each commit that lands has already been reviewed.

## L.1 — Implement

Write the code. Follow:

- `rules/architecture.md` for module layout and layer order
- `rules/unreal-style.md` for UE idioms, naming, and ownership
- `rules/content-changes.md` if the change touches `Content/`

**Gate — the editor must be closed, then the editor target compiles:**

```powershell
& "D:\UnrealEngine\UE_5.8\Engine\Build\BatchFiles\Build.bat" DieRobotEditor Win64 Development -Project="C:\Users\Robin Lifshitz\Documents\Unreal Projects\DieRobot\DieRobot.uproject" -WaitMutex
```

**Second gate, whenever a change alters a `UPROPERTY`, `UFUNCTION`, class name, or any
reflected signature:** Blueprints referencing it may now be silently broken, because C++
compiling proves nothing about the BP layer.

```powershell
& "D:\UnrealEngine\UE_5.8\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" "C:\Users\Robin Lifshitz\Documents\Unreal Projects\DieRobot\DieRobot.uproject" -run=CompileAllBlueprints -unattended -nopause
```

This is the game-dev step Harbor has no analogue for. A pure-C++ refactor that compiles
clean can still break thirty Blueprints, and nothing in the C++ toolchain will tell you.

## L.2 — Tests

Invoke the **`test-writer`** agent on the code that just landed.

**Gate:** the `DieRobot.Settled.*` suite passes.

> **Currently aspirational.** The suite does not exist yet — Setup Brief Phase 2.3 lands it.
> `Private/Tests/TestObj.cpp` is **not** a test; it is a shipped interactable lever that
> early-starts a wave. Do not mistake it for coverage.
>
> **Until the suite exists**, this step still runs — `test-writer` writes the first tests
> for the system being touched, and they become the seed of the suite. The gate is: the
> tests you just wrote pass. A change to a system with no tests is the opportunity to give
> it some, not a reason to skip.

**Exemption:** documentation-only changes and the EXP lane skip this step.

**Content-lane note:** a Blueprint or DataAsset change is usually not unit-testable. Its
verification is the smoke pass at L.3 plus the evidence required by
`rules/content-changes.md` — not a skipped step.

## L.3 — Performance review [BLOCKING]

Invoke the **`performance-reviewer`** agent.

**Why this and not a security audit:** Die Robot is a single-player offline game. There is
no auth, no PII, no server, no untrusted input. The hazard is not a breach — it is a frame
time regression that makes wave 12 unplayable, and those are invisible in a diff.

**Scope — this step is conditional:**

- **Runs** when the change touches anything on a per-frame or per-enemy path: `Tick`,
  behavior tree tasks and services, `AI/`, `Components/Combat/`, `Components/Navigation/`,
  `Components/StatusEffect/`, `Subsystems/Wave/`, `Subsystems/SynergySystem/`, spawn or
  destroy paths, or anything on the ultra risk bar (see C.4).
- **Skipped otherwise, with an explicit one-line note saying so.** A UI text change does not
  need a performance review — but the skip is stated, never silent.
- **Runs at least once before close-out** on any branch containing code. If every loop pass
  skipped it, run it at C.1.

**Gate:** PASS, or every finding addressed.

- A **BLOCK** finding halts the loop. Return to L.1, fix, re-run from L.2.
- An **EXPAND** finding is not a blocker but a strategic signal — a cost is growing in a way
  the current scope doesn't cover. Document it, decide whether to expand scope now or in a
  follow-up, and carry it into C.4.

## L.4 — Code review

Invoke the **`code-review` skill** on the **uncommitted** diff.

Reviewing before the commit is deliberate: the commit that lands is then the reviewed
artifact. Reviewing after would make the reviewed object and the landed object different
things.

**Gate:** every finding fixed, or explicitly accepted in-code with a comment giving the
reasoning. An accepted finding with no written reason is not accepted — it's forgotten.

**Content-lane substitution.** The `code-review` skill cannot read a `.uasset`. For a
content-only change, the substitute is the evidence package in `rules/content-changes.md`:
a written statement of what changed and why, plus a screenshot or clip. **This is a
substitution, not an exemption** — the change is still reviewed, by a human, against a
description that a human wrote.

**Exemption:** documentation-only changes and the EXP lane skip this step.

## L.5 — Commit

Commit per `rules/git-workflow.md` — `DIE-XX:` subject under 72 characters, `## What
changed` and `## Why` both required.

Then either return to L.1 for the next logical change, or proceed to close-out.

---

# Close-Out

**Runs once**, when every logical change is committed and the branch is ready to become a
PR.

## C.1 — Deferred performance review

If every pass through L.3 was skipped, run **`performance-reviewer`** now over the whole
branch.

Skip only when the agent already ran at least once during the loop, **or** the branch is
documentation-only.

## C.2 — Smoke pass [BLOCKING]

Run the manual pass in `docs/smoke-checklist.md`.

**This is the step that has no Harbor equivalent and the one most worth defending.** Three
of the bugs found during the 5.8 recovery were invisible to code review and only findable by
running the thing. DIE-24 in particular survived eight sessions and would have been caught
by a single save/load round-trip.

**Minimum bar:** one full wave, start to finish, from the main menu — not from PIE dropped
into the level. Transition, build, combat, wave completion, save, reload, verify the base
came back the same size.

The full checklist is required when the branch touches save/load, the build system,
navigation, or wave logic. For a narrow change, run the sections the change can plausibly
reach and **state which sections you ran and which you didn't.**

**Record the result in the PR's `## Test plan`.** "Smoke passed" is not a result. Which
wave, what was built, what the save round-trip showed.

## C.3 — Task review

Invoke the **`linear-task-reviewer`** agent against the Linear issue and the full branch
diff.

**Gate:** every Success Criterion met. Drift resolved in both directions — by updating the
issue, filing a new one, or removing the work. Never left unnoted.

## C.4 — Full-PR code review

Invoke the **`code-review` skill** over `merge-base..HEAD`.

The per-change reviews covered each commit in isolation; this pass looks for what only
appears across commits — a guard added in commit 2 that commit 5 routes around, a helper
introduced twice, a signature that drifted between its definition and its callers.

**Exemption:** documentation-only branches skip this step. The risk bar below still applies
to any branch containing code or content.

### The ultra risk bar

Ultra is required when the PR touches any of:

- **Save/load** — `Subsystems/SaveLoad/`, `USaveLoadStruct`, `FBuildableData`, or any struct
  serialized into a save. *A bad write is discovered by a player, weeks later, as a lost
  base — and there is no schema version to migrate from.*
- **The build system** — `BuildSystemManagerComponent`, `ABuildableBase`, placement,
  snapping, or attachment GUIDs. *This is the core verb of the game and it persists.*
- **Navigation** — `AI/`, corridor pathing, `NavigationHelperComponent`, navmesh config, or
  `RecastNavMesh` actor properties. *Already the known-weakest system, and the failure mode
  is enemies walking through walls or standing still.*
- **Damage and attributes** — `CombatComponent`, `TakeDamage` overrides, `ScaleHealth`,
  weapon damage math, status effect application. *Every balance decision the design makes
  rests on these being right, and three of them are currently wrong.*
- **The GAS migration** — any commit in that project, without exception. *It replaces four
  of the above at once.*
- **Build configuration** — `*.Build.cs`, `*.Target.cs`, `DieRobot.uproject`, or the
  `[CoreRedirects]` block in `DefaultEngine.ini`. *These are how the 5.8 upgrade nearly
  failed, and a wrong redirect breaks saves silently.*

**On a hit: stop. Name which trigger fired. Hand off to the user**, who runs
`/code-review ultra` and brings back the findings. Claude cannot run this step.

## C.5 — Vault update check

Ask: **did anything we built or decided warrant a vault change?**

| If we did this... | Suggest updating... |
|---|---|
| Made a significant technical or process decision | Decision Log |
| Settled a design question, or killed one | Pillars & Decisions, Open Questions & Graveyard |
| Changed what a system does, or fixed a listed bug | Systems Inventory |
| Changed engine version, build commands, or toolchain | Current State |
| Introduced a new term, or retired one | Glossary |
| Landed a test that armors a system | Systems Inventory (the SETTLED claim), Current State |
| Found a gotcha that cost more than an hour | Current State → Gotchas |
| `performance-reviewer` returned an EXPAND finding | Decision Log + Systems Inventory |

When a trigger fires: state what should change and why, propose specific page edits, offer
to make them.

If nothing warranted a vault change, **say so explicitly. Silence is not an answer.**

**The vault drifting from reality is worse than no vault.** This step exists to catch drift
while context is fresh — and because the vault is outside git, nothing else will ever catch
it.

## C.6 — PR

Per `rules/git-workflow.md`:

1. Pre-push checks — see `rules/git-workflow.md` → Pre-Push Checks.
2. Push the branch. Then confirm it actually landed:
   `git log --branches --not --remotes --oneline` must be empty. **A push to this repo can
   fail on an LFS timeout and still print a plausible-looking tail** — that has happened.
3. Open a PR meeting the **PR Description Standard** in `rules/git-workflow.md` — not just
   the headings, the prose standard under them.
4. **Wait for review and merge by the user.** Claude does not merge.
5. After merge, move the Linear issue to **Done** and delete the branch.

---

## CI — two lanes

Game builds are slow. A full cook and package is tens of minutes and gigabytes; running that
per-PR would make CI something to route around, and CI that gets routed around is worse than
none.

| | Lane 1 | Lane 2 |
|---|---|---|
| **Runs** | Every PR | Weekly, and before any build handed to another person |
| **Does** | Compile the editor target, `CompileAllBlueprints`, `DieRobot.Settled.*` | Cook + package Development and Shipping, capture a perf trace on a scripted wave |
| **Budget** | Minutes | As long as it takes |
| **Blocks merge** | Yes | No — it opens issues |

Lane 1 is the merge gate. Lane 2 is the early-warning system: a cook that breaks is found
within the week rather than on the day a demo is due.

> **Currently aspirational.** There is no `.github/` in this repo. Setup Brief Phase 1 lands
> Lane 1. Until then, L.1's compile gate and C.2's smoke pass **are** the CI — run by hand,
> and the standard does not drop because the runner is a person.

---

## The EXP lane

Experimental code and content — a mechanic being tried, a tuning pass being felt out, a
prototype that exists to answer a design question.

**Exempt from:** L.2 (tests), L.3 (performance), L.4 (code review).

**Never exempt from:** having a Linear issue, a branch, a commit message that says what it
is, and an answer at the end.

Rules:

- Branch is named `exp/<short-name>`. Never `die-XX`.
- The commit body states the **question being answered**, not just the change.
- **An EXP branch never merges to `main`.** It is either graduated — reimplemented on a
  `die-XX` branch taking every step — or deleted. There is no third option, because the
  exemption is what makes it fast, and merging exempt code is how a codebase acquires a
  region nobody will touch.
- Every EXP branch gets a written answer in the vault's Open Questions & Graveyard. **An
  experiment with no recorded answer was not an experiment; it was a detour.**

The exemption exists because infrastructure must never be the reason a design question goes
unanswered. Setup Brief §8 states it directly: if an experiment is waiting, the cheat manager
and firing range outrank all CI work.

---

## Active Agents

| Agent | Step | Role |
|---|---|---|
| `test-writer` | L.2 | Writes UE Automation tests for code that just landed |
| `performance-reviewer` | L.3 [BLOCKING] | Audits frame-time and allocation cost; flags growth |
| `linear-task-reviewer` | C.3 | Verifies the work against the task it came from |

**Deliberately absent.** No `security-auditor` — see CLAUDE.md → Claude Setup for why a
security audit of a single-player offline game is theatre. If Die Robot ever ships
multiplayer, leaderboards that accept client-authored scores, or user-generated content,
that decision is reopened and this line is the reminder to reopen it.
