# Linear Issue Rules — Die Robot

How every Linear issue and project description is structured. The goal is a description a
reader can **skim to the part they need** — not one they must read start to finish.

Applies to `save_issue` and `save_project` alike, and to editing an existing description: if
it lacks these sections, add them.

Team **DIE**, initiative **Die Robot**.

---

## The Six Sections

In this order. Every section is required except `## Technical Spec`.

```markdown
## Summary
## Intent
## Goals
## Technical Spec      ← the only omittable section
## Expected Result
## Success Criteria
```

Each section has a **distinct job**. The failure mode this structure prevents is three
sections all answering "what does done look like" in slightly different words — which makes
an issue longer without making it clearer. If two sections would say the same thing, one of
them is being written wrong.

---

### `## Summary`

Two sentences. A ten-year-old understands it. **Zero technical terms** — no file names, class
names, function names, acronyms, type names.

> Right now the boss takes no damage at all — you can hit her forever and nothing happens.
> This makes hitting the boss actually hurt her.

**Not this:**

> ~~`ADieRobotBoss::TakeDamage` hides `AActor::TakeDamage`, so the engine's damage dispatch
> never reaches the override and the payload is discarded.~~

That is true, and it belongs in the issue — under `Technical Spec`, not here.

**The test:** read only this section, then explain the issue out loud to someone who has
never seen the codebase. If you can't, it's still too technical. Rewrite it.

---

### `## Intent`

The implied result, placed in Die Robot's context. One short paragraph. Answers *what larger
thing is this part of* — the reason the work exists, not the work itself.

> Boss fights are the payoff at the end of a wave block, and a boss that cannot be hurt turns
> the game's biggest moment into a soft-lock. This is about the finale reading as a fight.

**Not this:**

> ~~Fix the TakeDamage override.~~

That's a task, not an intent. Intent survives even if the implementation approach changes
completely.

**Where a design pillar or decision applies, name it.** `Serves P1.` or `Violates D4 —
this issue removes the violation.` The vault's `Pillars & Decisions` is the reference, and an
issue that connects to it is one someone can prioritise.

---

### `## Goals`

Capabilities the game has afterward. Stated **without mechanism**.

> - Every enemy, boss included, takes damage from every player damage source
> - A new enemy type inherits working damage without extra wiring

**Not this:**

> ~~Add `override` to the TakeDamage declaration~~

That is mechanism. Mechanism belongs in `Technical Spec` or `Expected Result`. A Goal should
still read correctly if the implementation is rewritten from scratch.

---

### `## Technical Spec`

Prior design carried in from earlier work — data layouts, constraints, decisions already
settled elsewhere, links to a vault page. Anything the implementer would otherwise have to
rediscover.

**The only omittable section.** Many tasks carry no prior design. Omit the heading entirely
rather than writing "N/A" — an empty required section is noise, and noise is what this format
exists to remove.

---

### `## Expected Result`

The concrete artifacts that will exist when the task is done. Classes, functions, assets,
DataAssets, config entries, tests.

> - `ADieRobotBoss::TakeDamage` declared `virtual ... override`
> - `ABP_Boss_Lola` re-parented cleanly; its stale `BlackboardDataAsset` reference removed
> - `DieRobot.Settled.Combat.BossTakesDamage` automation test

**Not this:**

> ~~- The boss will take damage~~

That's a Goal in future tense. `Expected Result` answers *what will exist*, `Goals` answers
*what will be possible*.

---

### `## Success Criteria`

Binary, checkable gates. Checkbox list. Each item is something a reader can mark done or not
done **without judgment**.

> - [ ] Editor target compiles clean
> - [ ] `CompileAllBlueprints` — 0 failures
> - [ ] `DieRobot.Settled.Combat.*` green
> - [ ] **Playable:** Lola's health bar drops on a melee hit and on a projectile hit
> - [ ] **Playable:** wave 10 completes with the boss dying, from the main menu

**Not this:**

> ~~- [ ] Boss damage works correctly~~

Nobody can mark that done. If a criterion needs an opinion to evaluate, rewrite it as
something observable, or move it to `Goals`.

#### Gameplay issues need a playable criterion

This is the game-dev departure and it is not optional. **A compile-and-tests criterion set on
a gameplay issue is incomplete**, because the entire recovery history of this project says so:
three of the bugs found during the 5.8 upgrade were invisible to code review and only findable
by running the game. DIE-24 survived eight sessions and would have fallen to one save/load
round-trip.

State what a person must **see** for the issue to be done — which map, which wave, which
observable outcome. If you cannot write that sentence, the issue is not yet understood well
enough to start.

---

## Formatting Rules

**This has caused real damage before. Read before touching Linear.**

**Never use `\n` escape sequences.** Linear does not render them — it stores the literal
characters `\` and `n`, and every reader sees garbled text. The mistake happens when
constructing a `description` parameter and reaching for `\n` as a newline escape.

- Use actual line breaks (blank lines between paragraphs)
- Markdown headings (`##`), bullets (`-`), bold (`**text**`) for structure
- Code blocks use triple backticks with an actual newline after the opening fence
- Never `\n`, `\\n`, or any escape sequence for a newline — ever

**Pre-flight check** before calling `save_issue` or `save_project`: does the description
string contain `\n`? If yes, rewrite with real line breaks before sending.

---

## Projects

Every issue belongs to a project under the **Die Robot** initiative:

| Project | Covers |
|---|---|
| Clean Up | Recovery, engine upgrade, setup, CI, test infrastructure |
| Pathing Rework | DIE-27..38 — nav, corridor pathing, RVO, wall destruction |
| GAS Migration | DIE-39..49 — ASC, attributes, effects, abilities |

Bug issues found during other work still get filed to the project that owns the system, not
to whatever project was in progress when they were noticed.

## Labels

The label set that actually exists on team DIE:

- `Bug` — incorrect behavior, or a violation of a rule in `.claude/rules/`
- `Improvement` — code improvement without new behavior (refactors, cleanup)
- `Task` — maintenance, config, tooling, infrastructure
- `Feature` — new player-facing capability
- `Story` — a larger player-facing milestone spanning several issues

**Not yet created, and worth adding when the work volume justifies it:** `Content` (Blueprint,
asset, map, DataAsset work), `Design` (a question to answer rather than code to write), and
`Perf` (frame time, allocation, load time). Until they exist, use `Task` and say which kind it
is in the Summary — **do not label with something that isn't there**, and do not assume this
list is current without checking `list_issue_labels`.

## Status Flow

**Backlog → In Progress → In Review → Done.** Link the issue to its project and set status in
a single `save_issue` call at task start. Use state IDs, not names.

`In Review` means the PR is open. An issue does not reach `Done` until the PR is merged — see
`WORKFLOW.md` → C.6.

## Design questions are issues too

An open design question — the kind tracked in the vault's `Open Questions & Graveyard` — gets
a Linear issue when an **experiment** is going to answer it, labelled `Design`.

Its `Success Criteria` is not code. It is *the question is answered and the answer is written
down in the vault.* An experiment with no recorded answer was not an experiment; it was a
detour.
