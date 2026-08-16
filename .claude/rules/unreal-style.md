# Unreal C++ Style — Die Robot

Conventions for C++ in this module. Where a rule below differs from what existing code does,
the rule wins for **new** code — but do not sweep the codebase to conform. Opportunistic
cleanup inside a file you're already editing is welcome; a rename-everything commit is not.

---

## File conventions

- **Header comment:** `// Property of Paracosm.` as line 1 of every file.
- **Indentation is tabs**, matching the existing tree and Epic's convention.
- **Includes are module-relative from `Public/`**, never relative paths:

  ```cpp
  #include "Components/Combat/CombatComponent.h"   // yes
  #include "../../Combat/CombatComponent.h"        // no
  ```

- Include order in a `.cpp`: own header first, blank line, then everything else.
- `#pragma once` at the top of every header, then `CoreMinimal.h`, then the generated header
  **last** — `.generated.h` must be the final include or UHT fails.

## Naming

Epic's prefixes are not optional — the reflection system depends on them.

| Prefix | For |
|---|---|
| `A` | Actors — `ADieRobotPlayableCharacter` |
| `U` | UObjects, components, subsystems — `UCombatComponent` |
| `F` | Plain structs — `FAbilityContext`, `FBuildableData` |
| `E` | Enums — `EOwnerWeaponState` |
| `I` | Interfaces |
| `b` | Booleans — `bCanEverTick`, `bIsPrimaryAbility` |

- Types and functions: `PascalCase`. Locals and parameters: `PascalCase` too (Epic style,
  not camelCase).
- **New gameplay types take the `DieRobot` prefix**, not `Timber`. `Timber` is the pre-June
  2026 module name. It survives in some Blueprint asset names (`GM_TimberGameModeBase`) —
  harmless, rename opportunistically, never as a task.
- Enum values get `UMETA(DisplayName = "...")` where the designer-facing name should differ.

## UPROPERTY and UFUNCTION

Every `UObject*` or `AActor*` member held by a UObject **must** be a `UPROPERTY`, even when
it is not exposed:

```cpp
UPROPERTY()
TObjectPtr<AActor> CachedTarget = nullptr;
```

Without it the garbage collector does not know about the reference, and the pointer dangles
after a collection. This is the single most common source of "it crashed after twenty
minutes" in Unreal.

- **Prefer `TObjectPtr<T>` over raw `T*`** for new UPROPERTY members. Existing raw pointers
  are fine where they are.
- **Always initialize members at declaration** — `= nullptr`, `= 0.f`,
  `= FVector::ZeroVector`. `FAbilityContext` does this correctly throughout; copy it.
- Specifier intent:

  | Specifier | Means |
  |---|---|
  | `EditDefaultsOnly` | Tuned on the Blueprint class. **The default for tuning values.** |
  | `EditAnywhere` | Also tunable per placed instance. Use only when per-instance really varies. |
  | `VisibleAnywhere` | Runtime state a designer should see but never set |
  | `BlueprintReadOnly` | BP may read it. Prefer over `BlueprintReadWrite`. |
  | `BlueprintReadWrite` | BP may write it — every write becomes un-reviewable. Justify it. |

- Every `UPROPERTY` exposed to the editor gets a `Category`. Uncategorized properties land in
  a heap at the bottom of the details panel.

> **Changing any of these breaks Blueprints silently.** Renaming a `UPROPERTY`, changing its
> type, or renaming a `UFUNCTION` invalidates every BP node referencing it, and C++ compiles
> clean regardless. Run `CompileAllBlueprints` (WORKFLOW L.1, second gate) and add a
> `CoreRedirect` when a rename is unavoidable.

## Never mutate a UDataAsset at runtime

A DataAsset is **shared state**. Every actor using it holds the same instance.

```cpp
// NO — this writes through into the shared asset
FStatusEffect& Effect = EffectAsset->Effect;
Effect.TimeRemaining = Effect.Duration;

// Yes — copy, then mutate the copy
FStatusEffect Effect = EffectAsset->Effect;
Effect.TimeRemaining = Effect.Duration;
```

This is precisely DIE-13. `AddStatusEffectToComponent` takes a non-const reference into a
`UDataAsset` and stores the effect *before* initialising `TimeRemaining`. The first
application of each effect type per session silently fails; afterwards the asset is "primed"
and it works. **Order-dependent, session-dependent, and near-impossible to reproduce in
isolation** — which is why the rule is absolute rather than case-by-case.

Take `const&` or by value from a DataAsset. If you need a mutable copy, say so by copying.

## Null-check what you cast

```cpp
// The existing pattern in CombatComponent::BeginPlay — do not copy it
OwningCharacter = Cast<ACharacter>(GetOwner());
UE_LOG(LogTemp, Warning, TEXT("Owner: %s"), *OwningCharacter->GetName());  // ← crash

// New code
OwningCharacter = Cast<ACharacter>(GetOwner());
if (!OwningCharacter)
{
    UE_LOG(LogDieRobot, Error, TEXT("%hs: owner is not a Character"), __FUNCTION__);
    return;
}
```

`Cast<>` returns null on failure — that is its contract. `CastChecked<>` is available when
failure genuinely means a programming error and you want the assert.

**Prefer an Interface to a concrete cast** when a component needs something from its owner.
See `rules/architecture.md` → sideways dependencies for why: one `Cast<ADieRobotEnemyCharacter>`
is what confines the entire status-effect system to enemies.

## Logging

Existing code uses `UE_LOG(LogTemp, Warning, ...)` almost everywhere. **`LogTemp` is not a
log category, it is the absence of one** — it makes filtered log reading impossible and turns
every shipping log into noise.

New code declares and uses a project category:

```cpp
DECLARE_LOG_CATEGORY_EXTERN(LogDieRobot, Log, All);   // in a shared header
DEFINE_LOG_CATEGORY(LogDieRobot);                     // in one .cpp
```

Levels mean things: `Error` for a broken invariant, `Warning` for a recoverable surprise,
`Log` for lifecycle, `Verbose` for per-frame detail. **`Warning` for normal operation is
noise**, and noise is why the real warning gets missed.

Never log per-frame at `Log` or above. A string format on every tick is real cost.

## Tick is opt-in and expensive

```cpp
PrimaryComponentTick.bCanEverTick = false;   // the default you should be writing
```

`UCombatComponent` currently sets `bCanEverTick = true` and its `TickComponent` does nothing
but call `Super`. That is a per-frame virtual call and a tick-registration for no work, on
every character in the level.

Before enabling tick, ask whether the work is event-driven instead:

- A timer (`GetWorldTimerManager().SetTimer`) for anything periodic but not per-frame
- A delegate or the event subsystem for anything reactive
- `SetComponentTickInterval()` when it must tick but not every frame

If it must tick, keep the body allocation-free: no `TArray` construction, no string
formatting, no `GetAllActorsOfClass`, no `FindComponentByClass`. Cache in `BeginPlay`.

## Const-correctness

- Getters are `const`. `GetAbilityInputRequirement(bool) const` is the existing good example.
- Loop over containers by `const&`: `for (const FBuildableData& Data : Buildables)`. Copying
  `FBuildableData` in a loop is not free — it carries ten directional GUIDs.
- Take `const FString&` / `const TArray<T>&` parameters, not by value.

## Containers and allocation

- `Reserve()` before a loop whose size you know.
- `TArray::Add` in a loop over thousands of items reallocates repeatedly — that plus
  `SpawnActor`'s overlap sweep is DIE-26's O(n²).
- Prefer `TMap` lookup to a linear `TArray` search on any path that runs more than once a
  frame.
- `MoveTemp()` when handing off ownership of a container rather than copying it.

## Actors, spawning and destruction

- Guard `Destroy()` paths against re-entrancy. DIE-16 is a missing death guard: without it a
  single enemy can drop loot twice and drift the wave counter, because death can be signalled
  from more than one path in the same frame.
- Never dereference an `AActor*` cached last frame without `IsValid()`.
- `SpawnActor` runs an overlap sweep against existing actors. In a loop, that is quadratic —
  use `FActorSpawnParameters` with `ESpawnActorCollisionHandlingMethod::AlwaysSpawn` when the
  placement is already known-good.

## Overriding engine functions

```cpp
// NO — this hides AActor::TakeDamage rather than overriding it
float TakeDamage(float Damage, ...);

// Yes
virtual float TakeDamage(float Damage, FDamageEvent const& DamageEvent,
                         AController* EventInstigator, AActor* DamageCauser) override;
```

DIE-18 is exactly this hiding, and DIE-12 is its consequence: Boss Lola takes zero damage
because the payload goes to a function that is never the one the engine calls. **Always write
`override`.** The compiler then tells you when the signature is wrong, which is the entire
point.

## Member initializers are not computed at runtime

```cpp
// NO — computed once at construction, then serialized separately.
// Editing BaseWeaponDamage on the Blueprint never updates this.
float TotalWeaponDamage = BaseWeaponDamage * DamageModifierValue;

// Yes — a function, computed when asked
float GetTotalDamage() const { return BaseWeaponDamage * DamageModifierValue; }
```

DIE-15. The two melee paths currently disagree about which value to read, which is what a
silently-stale derived member always eventually causes. **Derived values are functions, not
fields.**

## Comments

**`/** */` on declarations, `//` on implementations.** Header comments are the contract;
`.cpp` comments are the reasoning. A `.cpp` comment restating what the code does is
deletable — a header comment explaining internal mechanics belongs in the `.cpp`. Most
overgrown comments are one of those two sitting in the wrong file.

### Header comments on reflected members are shipped UI

UHT extracts the comment above a `UPROPERTY` or `UFUNCTION` into the `ToolTip` metadata. It
becomes **the text a designer reads** hovering that property in the Details panel, and
`@param` lines become the pin tooltips on the Blueprint node.

So a comment on a reflected member has an audience who will never open the header. Write it
for them. This is also the discipline that keeps comments short on its own — a six-line
ramble is obviously wrong once you picture it in a tooltip box.

### One line is the default

The multi-line block is the exception and has to earn it — with `@param`, `@see`, or a
genuine second idea.

```cpp
/** If true, the trap re-arms after its cooldown instead of being consumed. */
UPROPERTY(EditDefaultsOnly, Category = "Trap")
bool bRetriggerable = true;

/**
 * Applies an element at the given tier, combining with any tags already active.
 * @param Element  Which of the five elements to apply.
 * @param Tier     1-3, escalating. Tier 3 can trigger an emergent effect.
 * @see USynergySystem::EvaluateCombination()
 */
```

- **Booleans read `If true, …`** — the template forces the tight form. Epic uses it
  throughout `Actor.h`.
- **Prefer `@see` to prose.** Linking the related function beats re-explaining it, and the
  link stays correct when the other side changes. This is the single biggest cure for
  comments that sprawl.

### `//~` for anything that must not become a tooltip

The tilde suppresses doc extraction. `Actor.h` uses it 17 times:

```cpp
//~ Begin UActorComponent Interface
virtual void BeginPlay() override;
virtual void TickComponent(...) override;
//~ End UActorComponent Interface
```

Without the tilde, an organizational comment sitting above a `UFUNCTION` becomes that
function's tooltip — an interface marker shipped to a designer as documentation. Use `//~`
for section dividers and interface blocks, always.

### The rest

- Comment **why**, not what. `// Using a Character because a character will always be the
  owner of this component. Has Mesh.` is a good comment — it explains a choice.
- A TODO names the problem and its cost, not just "fix this". The existing codebase does this
  well; several TODOs correctly diagnose real design flaws before they bite.
- When you accept a code-review finding rather than fixing it, the reasoning goes **in the
  code**, at the site. An accepted finding with no written reason is a forgotten one.
