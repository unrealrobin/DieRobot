# Architecture Rules — Die Robot

One C++ module, `DieRobot`, plus a large Blueprint and content layer. ~29k lines across
~150 classes.

For *what each system currently does and what state it's in*, read the vault's
`02 Technical/Systems Inventory.md`. This file is the **layout and layering contract** — where
new code goes and what may depend on what.

---

## Module layout

Standard Unreal `Public/` (headers) and `Private/` (implementation) split. **The two trees
mirror each other folder-for-folder.** A header at `Public/Components/Combat/CombatComponent.h`
has its implementation at `Private/Components/Combat/CombatComponent.cpp`. Breaking the mirror
is how files become unfindable.

```
Source/DieRobot/
├── DieRobot.Build.cs
├── Public/            # Headers — mirrors Private/ exactly
└── Private/
    ├── AI/            # Controllers, behavior tree Tasks/ and Services/
    ├── BuildSystem/   # Buildables: BuildableBase, BuildingComponents, Ramps, Traps, Constructs
    ├── Character/     # Player and Enemies/ (incl. Boss/)
    ├── Components/    # BuildSystem, Combat, Inventory, MissionDelivery, Navigation,
    │                  #   StatusEffect, Vignette — the behavior layer
    ├── Controller/    # PlayerController
    ├── Core/          # Module-wide plumbing
    ├── Data/          # DataAssets/ and DataTable row types
    ├── Environment/   # World actors
    ├── GameModes/     # GameMode, GameState
    ├── Interfaces/    # UINTERFACE contracts
    ├── Loot/          # Loot tables, drops
    ├── Settings/      # Developer settings
    ├── States/        # PlayerState
    ├── Subsystems/    # SaveLoad, Wave, SynergySystem, Events, Music, SFX, Online,
    │                  #   Dialogue, GameConfig — the service layer
    ├── Synergy/       # EffectLogic/DerivedHandlers/ — per-effect logic
    ├── Tests/         # ⚠ Currently contains no tests. See below.
    ├── Types/         # Shared enums and structs
    ├── UI/            # Widgets
    ├── ViewModels/    # MVVM view models
    └── Weapons/       # Weapons and Abilities/
```

> **`Private/Tests/TestObj.cpp` is not a test.** It is a shipped interactable lever that
> early-starts a wave. Do not read it as coverage, and do not put real tests beside it
> without moving it first — the folder name is currently a lie.

## Where new code goes

| You're adding... | It goes in... |
|---|---|
| Per-actor behavior attachable to more than one owner | `Components/` |
| A world-lifetime or game-lifetime service | `Subsystems/` |
| A placeable thing the player builds | `BuildSystem/` |
| A new element, tier, or emergent effect | `Synergy/EffectLogic/DerivedHandlers/` |
| Enemy decision-making | `AI/Behavior/Tasks/` or `Services/` |
| Tunable data (a trap, a wave, a loot table, a mission) | `Data/DataAssets/` — **not** a new class |
| A contract two unrelated classes share | `Interfaces/` |
| A struct or enum used across folders | `Types/` |

**The default answer for new content is a DataAsset, not a class.** Traps, waves, effects,
missions, loot and dialogue are already data-driven. If adding an enemy variant or a trap
requires a C++ edit, that is a design finding to raise — not a task to quietly complete.

---

## Where tests go

**`Private/Tests/`, inside this module** — Epic's own convention, used throughout the engine
(`Core/Private/Tests/`, `AIModule/Private/EnvironmentQuery/Tests/`, `CoreOnline/Private/Tests/`).

Not a separate test module, for three reasons:

- **Tests need private headers.** Nearly everything worth testing — `USaveLoadSubsystem`,
  `UBuildSystemManagerComponent`, the synergy handlers — is in `Private/`. A separate module
  cannot see them without exporting, and exporting production classes because the test tooling
  demands it lets the harness redesign the code.
- **Test code does not ship.** The automation macros are wrapped in `#if WITH_AUTOMATION_TESTS`
  (`Misc/AutomationTest.h`), which is 0 in Shipping and Test. They compile to nothing in a
  packaged build, so the usual reason to isolate tests does not apply.
- No extra `Build.cs`, `.uproject` module entry, or target to maintain.

**Mirror the source tree inside `Tests/`**, the same way `Public/` mirrors `Private/`:

```
Private/Tests/
  SaveLoad/SaveLoadSubsystem.spec.cpp
  Combat/DamageCalculation.spec.cpp
  Synergy/EffectCombination.spec.cpp
  Wave/WaveComposition.spec.cpp
```

There is no `Public/Tests/` — nothing includes a test.

### Two forms, both used by Epic

| Form | For |
|---|---|
| `*.spec.cpp` — `DEFINE_SPEC`, Describe/It | **The default.** Shared fixtures, nested structure. Epic uses it in `Json/Private/Tests/` and `BuildPatchServices/Private/Tests/Unit/`. |
| `IMPLEMENT_SIMPLE_AUTOMATION_TEST` in a plain `.cpp` | One-off invariant checks with no shared setup. |

Spec form is the default for `DieRobot.Settled.*`: the suite needs shared fixtures (a world, a
populated save), and the Describe nesting maps onto the `System.Behavior` naming directly.

### Functional tests are content, and they are committed

Tests needing a running world with real actors are Blueprint actors placed in a test map. Those
live in **`Content/Maps/Tests/`**, committed like any other content.

**They must not live in a gitignored `Content/Developers/` scratch area** — CI could never run
them. The Developers folder is for throwaway experiment assets; a committed test fixture is a
different thing.

The dividing line: **needs a world with actors → functional test in a map. Pure logic → C++ in
`Private/Tests/`.**

### Before the first real test lands

`Private/Tests/TestObj.cpp` has to move to `Private/Debug/`. It is a shipped gameplay actor —
unguarded by `WITH_AUTOMATION_TESTS` — sitting in a folder named Tests. Its eventual neighbour
is the cheat manager the Setup Brief calls for. Its own small commit, not bundled with a test.

---

## Layer order

Dependencies point **downward only**. A layer may know about layers below it, never above.

```
UI / ViewModels          ← reads state, sends intent; owns no game logic
        ↓
Components               ← per-actor behavior (combat, build, mission, nav, status)
        ↓
Subsystems               ← world/game services (save, wave, synergy, events)
        ↓
Data / Types / Interfaces ← DataAssets, structs, enums, contracts. Depends on nothing.
```

Rules that follow from this:

- **A Subsystem never reaches up into a Component to drive it.** It emits an event or exposes
  state the Component reads. `MissionEventSubsystem` is the model to copy: combat broadcasts a
  tagged event, missions subscribe declaratively, and neither knows the other exists.
- **A Component may talk to a Subsystem** via `GetWorld()->GetSubsystem<T>()` or the game
  instance. That direction is fine.
- **UI never mutates game state directly.** It goes through a ViewModel to a Component or
  Subsystem. `ViewModels/` exists to make this the path of least resistance.
- **`Data/`, `Types/` and `Interfaces/` depend on nothing in the module.** If a DataAsset needs
  to include a Component header, the data and the behavior are tangled — that is the finding.

### Sideways dependencies are the real hazard

Component-to-component coupling is what makes systems un-testable and un-migratable. The
standing example is in `Components/StatusEffect/`:

> `UStatusEffectHandlerComponent` hard-casts its owner to `ADieRobotEnemyCharacter`. That single
> cast is why **status effects cannot apply to the player, the Seeda, or buildables** — no burning
> player, no frozen Seeda. A whole class of design space is closed by one `Cast<>`.

When a component needs something from its owner, express it as an **Interface** the owner
implements, not a concrete cast. A cast to a concrete class in a component is a design decision
about what that component can ever be attached to — make it deliberately or not at all.

---

## GameplayTags are the cross-system vocabulary

Tags are pervasive already, defined in `Config/Tags/` (see `SynergySystemTags.ini`). They are
how systems refer to each other's concepts without linking against each other's headers.

- **Prefer a tag to an `IsA` ladder.** The standing anti-example is
  `PopulateCombatEventContextTags`, which decides an enemy's event tag by walking a chain of
  `IsA` checks — so adding an enemy type requires editing C++. It should read a tag off the
  asset.
- Tag names are a public interface. Renaming one breaks Blueprints and DataAssets silently, and
  there is no compiler to catch it.
- This pervasiveness is why the GAS migration is well-positioned: GAS's whole coordination model
  is tags, and the vocabulary already exists.

---

## Events over polling

`Subsystems/Events/MissionEventSubsystem` is the best-designed thing in the codebase and the
pattern to reach for. Combat emits `Event.Combat.Damage` with a `FGameplayTagContainer` of
context tags; missions subscribe to what they care about. Neither side knows the other.

That is roughly 80% of what GAS's GameplayEvent layer does, already built. When two systems need
to coordinate, the question is "what event describes this?" before "what pointer do I need?"

---

## Subsystem lifetimes

Pick the narrowest that works, and know which you picked:

| Type | Lives for | Use when |
|---|---|---|
| `UGameInstanceSubsystem` | The whole process, across level travel | State must survive a level change (Wave, SaveLoad, Online) |
| `UWorldSubsystem` | One level | State is meaningless in another level |
| `ULocalPlayerSubsystem` | One local player | Per-player, e.g. input or HUD state |

> **Anything cached across level travel in a GameInstance subsystem is a latent bug** until
> proven otherwise. A `UAudioComponent` held by the music subsystem across travel is a known
> live example — the actor it belonged to is gone, the pointer is not.

---

## The Build.cs is a dependency contract

`DieRobot.Build.cs` lists `PublicDependencyModuleNames` only. Adding a module there is a real
architectural decision — it grows compile time for every translation unit and takes on an engine
API surface.

Current dependencies: Core, CoreUObject, Engine, InputCore, EnhancedInput, UMG, SlateCore, Slate,
Niagara, MetasoundEngine, GameplayTags, NavigationSystem, ModelViewViewModel, and the online
stack (OnlineServicesInterface, CoreOnline, OnlineServicesCommon, OnlineServicesEOS).

- **Prefer `PrivateDependencyModuleNames`** for anything not appearing in a public header. It
  keeps the dependency out of every downstream include.
- **`GameplayAbilities`, `GameplayTasks` and `GameplayAbilities`' friends are the GAS migration's
  first commit** — adding them is on the ultra risk bar.
- Both OSSv1 and OSSv2 plugins are currently enabled. Picking one is outstanding work, not a
  settled state.

---

## Blueprint is a leaf, not a layer

Restated from CLAUDE.md because it is an architectural rule, not a style preference:

**C++ can be diffed, reviewed and tested. Blueprint can be none of the three.** Logic in a
Blueprint leaves no trace in the history and cannot be regression-tested.

| Belongs in Blueprint | Belongs in C++ |
|---|---|
| Composition — which components, which meshes | Logic other systems depend on |
| Tuning values and curves | Anything with a state machine |
| Presentation, animation hookup, VFX triggers | Anything on a per-frame path |
| Widget layout | Anything worth a test |

A Blueprint may call into C++ freely. **C++ should not depend on logic that only exists in a
Blueprint** — that inverts the layering and puts the un-reviewable half underneath.

See `rules/content-changes.md` for what a Blueprint change owes a reviewer.
