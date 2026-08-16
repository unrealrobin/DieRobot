# CLAUDE.md

Guidance for Claude Code when working in this repository.

Die Robot is a third-person wave-defense and base-building game in Unreal Engine 5.8.
~29k lines of C++ across ~150 classes in a single module, plus a large Blueprint and
content layer.

---

## Read this first

Two documents outrank everything else at the start of a session:

1. **`.claude/WORKFLOW.md`** — the order things happen. Setup, a change loop that repeats
   once per logical change and ends in a reviewed commit, and a close-out run once per PR.
2. **`~/Documents/RLOV/DieRobot/02 Technical/Current State.md`** — where the project
   actually is right now: engine paths, toolchain traps, open bugs, and the gotchas that
   have already cost hours.

`.claude/rules/` holds the conventions. `WORKFLOW.md` holds the sequence. They are
different files on purpose — do not look for conventions in the workflow.

---

## Git Commit Preferences

Every commit follows this format exactly — subject line plus a structured body. The body is
**required**, not optional.

```
DIE-XX: imperative subject line under 72 characters

## What changed
- Concrete bullet of each file/module-level change
- Be specific: name the file and what was done to it

## Why
Clear explanation of the motivation. What problem does this solve?
What was broken, missing, or wrong before? Why this approach over
alternatives? Include context that won't be obvious from reading
the diff alone.
```

**Full example:**

```
DIE-24: clear buildable and mission arrays before repopulating them

## What changed
- Source/DieRobot/Private/Subsystems/SaveLoad/SaveLoadSubsystem.cpp —
  SaveBuildableData() now calls BuildingComponentsArray.Empty() before
  the append loop
- Source/DieRobot/Private/Subsystems/SaveLoad/SaveLoadSubsystem.cpp —
  same treatment for PlayerData.CompletedMissionList

## Why
SaveCurrentGame() loads the existing save into SaveGameInstance before
calling SaveBuildableData(), so the array arrived already populated and
the append doubled it. InitializeSaveLoadSession() runs LoadGame() then
SaveCurrentGame() on every level entry, so the doubling compounded once
per visit: a 9-piece base reached 20,736 entries in eight loads, in a
48 MB save file that froze the editor for minutes on load.

Emptying is correct rather than de-duplicating because the in-memory
array is the authority at save time — the file is a stale copy of it,
not a second source of truth.
```

**Rules:**

- Do NOT include "Co-Authored-By: Claude" or any similar attribution line
- Issue ID is always the first token: `DIE-XX:`
- Use an imperative verb: "add", "fix", "move", "remove", "refactor", "extract", "update"
- Keep the subject line under 72 characters
- One logical change per commit — don't bundle unrelated work
- Both `## What changed` and `## Why` are required on every commit

Full branching, push and PR rules live in `.claude/rules/git-workflow.md`.

---

## Build & Development Commands

The engine is at **`D:\UnrealEngine\UE_5.8`** — a source/custom install, *not* the default
Launcher path. Every command below needs the leading `&`: in PowerShell a quoted path at the
start of a line is a string literal, not a command.

```powershell
# Compile the editor target
& "D:\UnrealEngine\UE_5.8\Engine\Build\BatchFiles\Build.bat" DieRobotEditor Win64 Development -Project="C:\Users\Robin Lifshitz\Documents\Unreal Projects\DieRobot\DieRobot.uproject" -WaitMutex

# Regenerate project files (after adding or moving source files)
& "D:\UnrealEngine\UE_5.8\Engine\Build\BatchFiles\Build.bat" -projectfiles -project="C:\Users\Robin Lifshitz\Documents\Unreal Projects\DieRobot\DieRobot.uproject" -game -rocket -progress

# Launch the editor directly, bypassing Rider
& "D:\UnrealEngine\UE_5.8\Engine\Binaries\Win64\UnrealEditor.exe" "C:\Users\Robin Lifshitz\Documents\Unreal Projects\DieRobot\DieRobot.uproject"

# Compile every Blueprint headlessly — catches BP breakage a C++ change caused
& "D:\UnrealEngine\UE_5.8\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" "C:\Users\Robin Lifshitz\Documents\Unreal Projects\DieRobot\DieRobot.uproject" -run=CompileAllBlueprints -unattended -nopause
```

> **The editor must be closed before compiling.** UBT cannot replace a loaded DLL. A build
> that appears to hang is usually waiting on `-WaitMutex` for an editor that is still open.

> **The .NET 10 trap.** The engine ships its own .NET at
> `Engine/Binaries/ThirdParty/DotNet/10.0/win-x64/` and `Build.bat` finds it via
> `GetDotnetPath.bat`. **Rider and Visual Studio call the system `dotnet.exe` instead** and
> fail with *"You must install or update .NET"* until a system-wide SDK 10 is installed.
> Already resolved on this machine — but it is the first thing a fresh machine will hit.

---

## Architecture Overview

One C++ module, `DieRobot`, using Unreal's standard `Public/` (headers) and `Private/`
(implementation) split. Full detail and the layering rules are in
`.claude/rules/architecture.md`.

```
DieRobot/
├── Source/DieRobot/
│   ├── Public/                   # Headers, mirrored folder-for-folder with Private/
│   └── Private/
│       ├── AI/                   # Controllers, behavior tree tasks & services
│       ├── BuildSystem/          # Buildables: walls, ramps, traps, constructs
│       ├── Character/            # Player and enemy characters
│       ├── Components/           # Combat, build, inventory, mission, nav, status effect
│       ├── Data/                 # DataAssets & DataTable row types
│       ├── Subsystems/           # SaveLoad, Wave, Synergy, Events, Music, SFX, Online
│       ├── Synergy/              # Element/tier effect handlers
│       ├── UI/                   # Widgets (MVVM)
│       └── ViewModels/           # MVVM view models
├── Content/                      # Blueprints, assets, maps — LFS-tracked binary
├── Config/                       # DefaultEngine.ini, DefaultGame.ini, Tags/
└── docs/                         # ⚠ PUBLISHED GITHUB PAGES SITE — see below
```

### The systems that matter

| System | Entry point | State |
|---|---|---|
| Build system | `Components/BuildSystem/BuildSystemManagerComponent.cpp` | Works, unarmored. Largest file in the project. |
| Trap synergy | `Subsystems/SynergySystem/SynergySystem.cpp` | Five elements × three tiers + five emergent effects. One confirmed bug (DIE-13). |
| Player abilities | `Components/Combat/CombatComponent.cpp` | A hand-rolled GAS. The GAS migration maps it nearly one-to-one. |
| Missions | `Components/MissionDelivery/MissionDeliveryComponent.cpp` | The best-designed system here. Declarative GameplayTag subscriptions. |
| AI & pathing | `AI/DieRobotAiControllerBase.cpp`, `Behavior/Tasks/Task_MoveThroughCorridorPathV2.cpp` | Recast corridor extraction. Owns its own Linear project. |
| Waves | `Subsystems/Wave/WaveGameInstanceSubsystem.cpp` | Works. `ScaleHealth()` violates design decision D4. |
| Save/load | `Subsystems/SaveLoad/SaveLoadSubsystem.cpp` | Works after DIE-24. Highest-blast-radius system in the game. |

For honest per-system state — what is built, what is broken, and why — read
`~/Documents/RLOV/DieRobot/02 Technical/Systems Inventory.md` rather than inferring from
the code.

---

## Hard facts that bite

**`docs/` is a published GitHub Pages site.** It has a `_config.yml` and serves
`Portfolio/` and `RobinLifshitz/`. Anything placed there is **public**, not merely
committed. Never put working documentation in it. Its only legitimate project content is
`docs/smoke-checklist.md`, which is a test artifact.

**`git status` clean does not mean backed up.** Use:

```bash
git log --branches --not --remotes --oneline
```

That is branch-agnostic and catches unpushed work anywhere. Eight months of work once sat
unpushed because "clean" was read as "safe."

**`Content/` is LFS-tracked and permanent.** ~14.8 GB stored against a 10 GB allowance.
Every asset committed is stored forever — gitignoring something later caps growth but
reclaims nothing. Think before adding bulk art.

**Sentry is disabled and crash reporting is off.** The bundled plugin targeted engine 5.4
and did not survive the 5.8 upgrade. `sentry.properties` remains but is inert. The disabled
plugin entry in the uproject is a deliberate marker, not an oversight.

**CoreRedirects in `Config/DefaultEngine.ini` are load-bearing for saves only.** After the
full content resave, no asset references the old `timbermvp` module. Deleting the redirects
now breaks any save written by an older build, and nothing else. Class redirects resolve
**before** property and function redirects — any entry keyed on a `Timber*` class can never
fire.

---

## Unreal MCP

The editor hosts an MCP server (UE 5.8, Experimental). `.mcp.json` points Claude Code at
`http://127.0.0.1:8000/mcp`, so **with the editor open you get live editor tools natively** —
no headless round-trips.

`bEnableToolSearch` is on, so `tools/list` returns only three meta-tools:
`list_toolsets`, `describe_toolset`, `call_tool`. Everything else is discovered through
them. A short tool list is the designed behavior, not a failure.

What's worth knowing is registered:

| Toolset | Use |
|---|---|
| `AutomationTestToolset` | Run the Settled suite in-editor — WORKFLOW L.2's fast path |
| `editor_toolset…ObjectTools` | `list_properties` / `get_properties` / `set_properties` on live objects |
| `editor_toolset…DataAssetTools`, `DataTableTools` | The DIE-13 surface |
| `EditorToolset.EditorAppToolset` | `StartPIE`, `StopPIE`, `CaptureViewport`, `SearchCVars` |
| `editor_toolset…SceneTools` | Place and remove actors in the loaded level |
| `EditorToolset.LogsToolset` | Read the output log, set category verbosity |
| `GASToolsets` | AttributeSet, AbilitySystemInspector, GameplayCue — for the GAS Migration |
| `aimodule_toolset…BehaviorTreeTools` | Inspect BT assets — for the Pathing Rework |
| `GameplayTagsToolset` | Read and manage the tag vocabulary |

**Live introspection is the point.** It reads what is actually loaded, not what is on disk —
which is the only way to verify order-dependent runtime state like DIE-13.

> **Security.** There is **no authentication layer**. It binds to loopback and rejects
> non-loopback `Origin` headers, and that is the entire protection. `bAutoStartServer=True`
> lives in `Saved/Config/WindowsEditor/EditorPerProjectUserSettings.ini`, which is
> gitignored — **deliberately not in committed project config**, because this repo is public
> and an auto-starting unauthenticated control port must not be the default for a clone.
> Both plugins are `TargetAllowList: ["Editor"]` and can never reach a shipping build.

**Not available:** `LiveCodingToolset` ships on disk but is absent from `AllToolsets`'
dependency list, so it never registers. The editor still has to be **closed to compile**.

## Documentation

The design vault lives in Obsidian at **`~/Documents/RLOV/DieRobot/`**. It is the source of
truth for *why the game is the way it is* — pillars, decisions, open questions, system
state, and the environment.

Read the relevant page before proposing a plan that touches its area:

| If you're touching... | Read in vault... |
|---|---|
| Anything, at session start | `02 Technical/Current State.md` |
| A system's actual state or a known bug | `02 Technical/Systems Inventory.md` |
| Enemy behavior, wave pacing, player verbs | `01 Design/Pillars & Decisions.md` |
| A question the design hasn't settled | `01 Design/Open Questions & Graveyard.md` |
| Why a technical choice was made | `03 Reference/Decision Log.md` |
| A term you don't recognize | `03 Reference/Glossary.md` |
| The setup and CI plan | `03 Reference/Setup Brief.md` |

**The vault is not in git and is not committed to this repo.** That is deliberate — see the
2026-08-14 Decision Log entry. This repo carries only agent configuration (`CLAUDE.md`,
`.claude/`) and test artifacts.

Vault drift is worse than no vault. When work changes something load-bearing, update the
vault — see `.claude/WORKFLOW.md` → C.4.

---

## Issue Tracking

Work is tracked in **Linear**, team **DIE**, under the **Die Robot** initiative:

| Project | State |
|---|---|
| Clean Up | In progress — recovery, upgrade, setup |
| Pathing Rework | Planned — DIE-27..38 |
| GAS Migration | Planned — DIE-39..49 |

When starting work:

- Confirm an issue exists in the right project. Create one if not, per
  `.claude/rules/linear-issues.md`.
- Move it through **Backlog → In Progress → In Review → Done**.
- Every commit references the issue ID (`DIE-XX: …`).

**The one Linear rule worth restating here, because it causes real damage:** never use `\n`
escape sequences in a Linear `description`. Linear does not render them — it stores the
literal characters `\` and `n`, and the issue reads as garbage. Use actual line breaks.
Pre-flight check before every `save_issue` / `save_project` call.

---

## Claude Setup

- **`.claude/WORKFLOW.md`** — the per-task sequence. Follow it.
- **`.claude/rules/`** — architecture, unreal-style, content-changes, git-workflow,
  linear-issues.
- **`.claude/agents/`** — `test-writer`, `performance-reviewer`, `linear-task-reviewer`.
  Invoked by step in WORKFLOW.md.

Code quality and style review is the **`code-review` skill**, invoked via the Skill tool
before every commit — not an agent. `/code-review ultra` is a separate, user-triggered and
billed slash command **Claude cannot launch**; it appears once, at close-out, behind the
risk bar in WORKFLOW.md.

There is no `security-auditor` here. Die Robot is a single-player offline game with no auth,
no PII, and no server surface — a security audit of a trap damage calculation is theatre.
The analogous blocking review is **`performance-reviewer`**, because in a game frame time is
a feature and a 3 ms regression on the enemy tick is a real defect.

---

## Key Principles

- **Frame time is a feature.** A change that is correct and costs 4 ms on the enemy tick is
  not done. Cost is part of the diff.
- **Data over code.** Traps, waves, effects, missions, loot and dialogue are already
  DataAssets and DataTables. A new enemy, trap or effect should not require a C++ edit. When
  it does, that is the finding.
- **GameplayTags are the vocabulary.** They are pervasive already, which is why the GAS
  migration is well-positioned. Prefer a tag to an `IsA` ladder — the `IsA` ladder in
  `PopulateCombatEventContextTags` is the standing example of what not to do.
- **Blueprint is a leaf, not a layer.** Logic that other systems depend on belongs in C++
  where it can be reviewed, tested, and diffed. Blueprints hold composition, tuning values,
  and presentation.
- **Never mutate a `UDataAsset` at runtime.** Taking a non-const reference into a shared
  asset and writing through it is exactly the bug in DIE-13: the first application per
  session silently fails, the asset is then "primed", and it becomes unreproducible.
- **The save file is the least forgiving surface in the project.** It is versioned by
  nothing, read by every level load, and a bad write is discovered by a player, weeks later,
  as a lost base. Treat every change to it as high risk.
- **The vault holds the "why", the code holds the runtime "how".** Don't let them drift.
