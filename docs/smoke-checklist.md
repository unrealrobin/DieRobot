# Die Robot — Smoke Checklist

Ten minutes, by hand, pass/fail per line. Run it before and after anything
structural (engine upgrade, pathing rework, GAS migration) so "did I break
something?" has an answer.

Record the result in the table at the bottom.

---

## 1. Boot & menu

| # | Check | Pass |
|---|---|---|
| 1.1 | Editor opens `StartUp.umap` with no errors in the log | |
| 1.2 | PIE from `StartUp` reaches the main menu | |
| 1.3 | Start menu music plays | |
| 1.4 | EOS login completes (log shows the online subsystem coming up) | |
| 1.5 | Leaderboard panel opens and shows entries | |

## 2. Save / load

| # | Check | Pass |
|---|---|---|
| 2.1 | New Game creates a save slot | |
| 2.2 | Load menu lists existing slots with correct wave numbers | |
| 2.3 | Loading a slot restores inventory and wave number | |
| 2.4 | Saving mid-run then reloading round-trips cleanly | |

## 3. Level transition

| # | Check | Pass |
|---|---|---|
| 3.1 | Pressing Play travels `StartUp` → `TheLab` **without hanging** | |
| 3.2 | Transition completes in a reasonable time — record it: ______ | |
| 3.3 | No `Recreating dtNavMesh` warnings in the log | |
| 3.4 | Player spawns with control and camera working | |

> 3.1–3.3 are the DIE-6/7 regression. A full navmesh rebuild at travel time is
> what froze the editor; if these fail, check the nav origin alignment first.

## 4. Build system

| # | Check | Pass |
|---|---|---|
| 4.1 | Build menu opens; icons render | |
| 4.2 | Place a wall; it snaps and deducts resources | |
| 4.3 | Place a floor | |
| 4.4 | Place a ramp | |
| 4.5 | Place each trap: Spike, Burn, Frost, Corrosion, ElectroStatic Pulse | |
| 4.6 | Place a Power Plate | |
| 4.7 | Place a Teleporter (both ends) | |
| 4.8 | Invalid placement shows red and is refused | |
| 4.9 | Delete a placed buildable; resources refund as expected | |
| 4.10 | Placing a wall visibly reroutes enemy pathing | |

## 5. Combat

| # | Check | Pass |
|---|---|---|
| 5.1 | Melee equips; primary attack swings | |
| 5.2 | Melee kills a Grunt | |
| 5.3 | Ranged equips; fires; ammo decrements | |
| 5.4 | Reload works | |
| 5.5 | Ranged kills a Shooter | |
| 5.6 | Charged Swing (hold-and-release) fires | |
| 5.7 | Knockback Blast fires and displaces enemies | |
| 5.8 | Damage numbers appear over damaged enemies | |
| 5.9 | Player takes damage; vignette responds | |
| 5.10 | Player death triggers the death UI | |

## 6. Status effects & synergies

| # | Check | Pass |
|---|---|---|
| 6.1 | A trap applies its status effect **on first use of a fresh session** | |
| 6.2 | The effect lasts its configured duration, then expires | |
| 6.3 | Effect icon appears above the enemy (hold Tab for the data cluster) | |
| 6.4 | A DOT effect ticks damage over time | |
| 6.5 | A slow visibly reduces enemy movement speed | |
| 6.6 | Two overlapping slows stay slowed until **both** expire | |
| 6.7 | A synergy triggers from a tag combination (e.g. Frost + Arc) | |

> 6.1 is the DIE-13 regression — the bug only ever showed on the *first*
> application of a given effect per session, so a warm session will hide it.
> 6.6 is DIE-19.

## 7. Waves

| # | Check | Pass |
|---|---|---|
| 7.1 | A wave starts; enemies spawn from doors | |
| 7.2 | Enemies path from every spawn point to Seeda | |
| 7.3 | Enemies traverse waypoints smoothly — no skipping or jitter | |
| 7.4 | Killing one enemy decrements the counter by exactly one | |
| 7.5 | One enemy death drops one set of loot | |
| 7.6 | Loot can be picked up and lands in inventory | |
| 7.7 | Wave completes and the next wave timer starts | |
| 7.8 | Early-start lever works | |
| 7.9 | Seeda takes damage and the fail state fires when destroyed | |

> 7.3 is DIE-14. 7.4 and 7.5 are DIE-16.

## 8. Bosses

| # | Check | Pass |
|---|---|---|
| 8.1 | Boss door opens on the boss wave | |
| 8.2 | Lola spawns | |
| 8.3 | **Lola's health decreases when damaged** | |
| 8.4 | Lola can be killed | |
| 8.5 | Lola's drones spawn and can be destroyed | |
| 8.6 | Bruiser spawns and can be killed | |
| 8.7 | Boss health bar tracks actual health | |

> 8.3–8.4 are the DIE-12 regression. Before that fix, Lola took zero damage.

## 9. Missions & UI

| # | Check | Pass |
|---|---|---|
| 9.1 | Mission display shows active objectives | |
| 9.2 | A kill objective increments by one per kill | |
| 9.3 | A build objective increments on placement | |
| 9.4 | Completing a mission grants its reward | |
| 9.5 | Settings panel opens; changes apply and persist | |

## 10. Log hygiene

| # | Check | Pass |
|---|---|---|
| 10.1 | No `Error:` lines during a normal wave | |
| 10.2 | No `Fatal` / assertion | |
| 10.3 | No `Resave recommended` on a fresh launch | |
| 10.4 | No Blueprint compile errors on startup | |

---

## Results

| Date | Build / commit | Result | Notes |
|---|---|---|---|
| | | | |

**Known-failing lines should be filed as issues, not left in this table.**
