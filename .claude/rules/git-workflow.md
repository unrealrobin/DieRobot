# Git Workflow Rules — Die Robot

## Never Push to Main

**All work lands on `main` through pull requests.** Never push directly to `main`.

This is a solo project, so the PR is not a gate against a careless colleague — it is a gate
against yourself at 1am. It creates one place where the diff, the smoke result, and the
reasoning sit together before anything becomes permanent. Self-merging is expected; skipping
the PR is not.

- The user reviews and merges every PR — Claude never merges.
- Lane 1 CI must be green before merge (see `WORKFLOW.md` → CI). Until Lane 1 exists, the
  L.1 compile gate and the C.2 smoke pass stand in for it.

**The one exception, stated so it is not improvised:** a repository-level emergency where
`main` itself is broken — a bad merge, a corrupted LFS pointer, a wrong `.gitattributes`.
Fix it directly, then say so in the commit body. This does not extend to "the change is
small."

## Branching Strategy

1. Every task or task group gets a branch off `main`.
2. Each branch gets one PR.
3. After merge, delete the branch.

**Naming:**

| Kind | Pattern | Example |
|---|---|---|
| Single issue | `die-XX` | `die-27` |
| Grouped issues | `die-XX-short-description` | `die-27-corridor-path-rewrite` |
| Experiment | `exp/<short-name>` | `exp/primed-threshold` |

Linear auto-links a branch when it finds a full `DIE-XX` identifier with the hyphen. Bare
numbers and concatenated forms like `die-27-28-29` do not link. When a branch covers several
issues, name it after the primary one — the rest link via the PR body.

**`exp/*` branches never merge to `main`.** See `WORKFLOW.md` → The EXP lane.

**Claude worktree branches (`claude/*`) are not work branches.** They cannot be linked in
Linear. If you find yourself on one at S.3, stop and say so.

### Check the branch point

Before starting work, confirm the branch is based on current `origin/main`:

```bash
git log --oneline -1 origin/main
git merge-base --is-ancestor origin/main HEAD && echo "up to date" || echo "STALE — rebase first"
```

This repo has already had a branch sitting eight months behind `main` while work continued
on it, and a `git checkout main` that silently reverted the entire working directory. With
8.8 GB of binary content under LFS, a stale branch point is not a style preference.

## Commit Message Format

Every commit follows this structure. The body is **required**.

```
DIE-XX: imperative subject line under 72 characters

## What changed
- Concrete bullet of each file/module-level change
- Be specific: name the file and what was done to it

## Why
Clear explanation of the motivation. What problem does this solve?
Include context that won't be obvious from reading the diff.
```

- Issue ID first, then `: `, then an imperative verb — "add", "fix", "move", "remove",
  "refactor", "extract", "update"
- Subject line under 72 characters
- **No "Co-Authored-By" or any Claude/Anthropic attribution line, anywhere**
- One logical change per commit
- Both `## What changed` and `## Why` required

### Content commits carry their own burden

For a commit touching `Content/`, `## What changed` cannot lean on the diff, because there
is no readable diff. Name each asset and state what changed inside it:

```
## What changed
- Content/Blueprints/Traps/BP_SpikeTrap.uasset — damage 25 → 18; added
  the Corrosion tag to the applied effect container
- Content/Maps/L_Lab.umap — moved RecastNavMesh-Regular to world origin
```

"Updated the spike trap" is not a description of a change nobody can read. See
`rules/content-changes.md`.

### When an exemption is taken

`WORKFLOW.md` allows documentation-only changes and the EXP lane to skip steps. **State the
exemption in the commit body**, naming which steps were skipped and why the change
qualifies. An exemption taken silently is how a rule stops meaning anything.

## Every Commit Is Reviewed Before It Lands

**Before each commit**, invoke the **`code-review` skill** on the uncommitted diff. Fix every
finding, or accept it explicitly in-code with a comment giving the reasoning. Then commit.

The order matters. Reviewing before the commit makes the commit that lands *the reviewed
artifact*. Reviewing after leaves the reviewed object and the landed object as two different
things, reconciled only by a follow-up commit or rewritten history.

This is step L.4 of `.claude/WORKFLOW.md`, restated here because it is a commit rule.

Note the mechanism: the **`code-review` skill**, via the Skill tool. Not `/code-review
ultra` — that is a user-triggered, billed slash command Claude cannot launch, and it appears
exactly once, at close-out, behind the risk bar.

## Pre-Push Checks

Run these **before every `git push`**. All must pass.

```powershell
# 1. Close the editor first — UBT cannot replace a loaded DLL.

# 2. Editor target compiles clean
& "D:\UnrealEngine\UE_5.8\Engine\Build\BatchFiles\Build.bat" DieRobotEditor Win64 Development -Project="C:\Users\Robin Lifshitz\Documents\Unreal Projects\DieRobot\DieRobot.uproject" -WaitMutex

# 3. Every Blueprint still compiles — required whenever a reflected
#    signature changed (UPROPERTY, UFUNCTION, class name)
& "D:\UnrealEngine\UE_5.8\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" "C:\Users\Robin Lifshitz\Documents\Unreal Projects\DieRobot\DieRobot.uproject" -run=CompileAllBlueprints -unattended -nopause

# 4. The Settled suite (once it exists — Setup Brief Phase 2.3)
```

Then check what is about to go up:

```bash
git status --short
git lfs status
```

**Look at the LFS list before pushing.** Storage is cumulative and permanent — an asset
pushed once is stored forever, and gitignoring it later caps growth without reclaiming
anything. The repo sits at ~14.8 GB against a 10 GB allowance. An accidental
`Content/SourcedAssets/` addition is a bill, not a mistake you can take back.

## Verify the Push Actually Landed

```bash
git log --branches --not --remotes --oneline
```

**Empty output is the only proof.** This is branch-agnostic — it catches unpushed work on
any branch, not just the current one.

Do not trust the tail of a push's output. A push to this repo has already failed on
`Connection to github.com closed by remote host` while printing output that read as success.
Large LFS pushes fail in ways that look like progress.

## PR Description Standard

Every PR body uses these headings:

```markdown
## Summary

## Test plan

## Linked Issues
```

The headings are the easy part. What makes a PR description worth reading is the **prose
under them**, and that is what this section specifies. All three headings with thin content
does not meet this standard.

### `## Summary`

**One bolded claim per logical change.** Each explains *the failure mode it fixes* — the
bug, the observable symptom, and why it mattered. Not a description of the diff; the diff is
already in the PR.

The model:

> **DIE-24 — clear the buildable array before repopulating it.** `SaveCurrentGame()` loads
> the existing save into the instance before `SaveBuildableData()` appends to it, so every
> save doubled the array. `InitializeSaveLoadSession()` runs load-then-save on every level
> entry, so it compounded per visit: a 9-piece base reached 20,736 entries in eight loads,
> in a 48 MB file that froze the editor for minutes. The symptom looked like a hang at
> startup with one core pegged, which is why it survived eight sessions.

Three things are present: what was wrong, what a person would have *seen*, and why the
wrongness had consequences. All three are required.

**Not this:**

> ~~- Fixed a save bug~~
> ~~- Updated the navmesh actor~~

A reviewer cannot tell from that whether the change is correct, or whether it matters.

Where a decision was deliberate and non-obvious — an ordering that must not be swapped, a
tuning value chosen by feel — say so and say why. Those are the paragraphs a reviewer most
needs and can least reconstruct.

### `## Test plan`

Actual commands and actual results. Never "tests pass."

> - Editor target — clean build, 0 warnings
> - `CompileAllBlueprints` — 412 assets, 0 failures
> - `DieRobot.Settled.SaveLoad.*` — 6 tests, 0 failures
> - **Smoke:** wave 1 from main menu, built 6 walls + 2 spike traps, saved, reloaded —
>   base restored at 8 pieces, save file 41 KB. Sections 1–6 run; section 9 (leaderboard)
>   not run, branch doesn't touch EOS.

Name the new tests and what they cover. For any branch touching save/load, the build system,
navigation, or waves, **the smoke result is not optional and "passed" is not a result** —
state what you built, what came back, and what size.

### `## Linked Issues`

Every issue in full `DIE-XX` form, so Linear links each one.

```markdown
## Linked Issues
DIE-XX, DIE-YY, DIE-ZZ
```

### Carry-up notes

Anything left open goes at the end of `## Summary`: judgment calls not settled, work
deliberately *not* done and why, decisions the reviewer should weigh in on.

> **No automated test for the navmesh origin fix.** Verifying tile alignment requires a
> loaded world and a built navmesh; the Settled suite currently has no world fixture.
> Building one is a decision to make deliberately rather than inside a bug fix. Covered by
> smoke section 4 instead.

The purpose is to distinguish *deliberate omission* from *oversight*. A reviewer who cannot
tell which they are looking at has to ask, and that round trip is what this prevents.

### Enforcement

Written standard plus a self-check before opening the PR. **No hook.** A hook can verify
headings exist, and missing headings has never been the failure mode — thin prose under
correct headings is, and that is not mechanically detectable.
