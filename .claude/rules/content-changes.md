# Content Change Rules — Die Robot

Everything under `Content/` is a binary `.uasset` or `.umap`. **`git diff` shows nothing.**
`git log -p` shows nothing. The `code-review` skill cannot read it. A reviewer opening the
PR sees a filename and a byte count.

This is the single largest structural difference between working in this repo and working in
a normal codebase, and every rule here follows from it.

---

## The core rule

**A content change is only reviewable if the person who made it describes it.** There is no
fallback, no tooling that recovers the information, and no way to reconstruct it later. The
description *is* the diff.

Nobody — including you, in three weeks — can otherwise answer "what changed in
`BP_SpikeTrap` in that commit?"

---

## The evidence package

Every commit touching `Content/` carries:

### 1. A per-asset statement of what changed

Not "updated the trap." Name each asset and what changed inside it:

```
## What changed
- Content/Blueprints/Traps/BP_SpikeTrap.uasset — base damage 25 → 18;
  added Effect.Corrosion.Erode to the applied effect container; retriggerable
  cooldown 1.2s → 0.8s
- Content/Data/DT_WaveComposition.uasset — wave 7 row: Bruiser count 4 → 6
- Content/Maps/L_Lab.umap — moved RecastNavMesh-Regular to world origin
```

Numbers matter. "Reduced the damage" is not recoverable; `25 → 18` is.

### 2. Visual evidence for anything visible

A screenshot or a short clip, attached to the PR, for any change to:

- Materials, VFX, Niagara systems, lighting
- Level geometry or layout
- UI widgets
- Animation or montages

Prose cannot carry these. A screenshot is the only artifact that answers "does this look
right."

### 3. A smoke result for anything functional

A Blueprint graph change, a DataAsset value change, a DataTable row — these have no visual
and no unit test. Their verification is running the game and saying what happened. State
which smoke sections you ran.

**A content commit with none of the three is not a reviewed commit.** It is a byte count
with a hopeful message.

---

## Never commit content you cannot describe

If you did not make the change and cannot say what is inside it, do not commit it. Two cases
recur:

**The editor touched files you didn't edit.** Opening a map or asset can re-save it —
recomputed lighting, a rebuilt navmesh, a bumped asset version. These show as modified with
no intent behind them. Check `git status` before staging and **revert what you did not mean
to change**. Committing an editor-incidental resave hides a real change in the noise the
next time.

**A resave or migration commandlet ran.** That is legitimate, but it is its own commit with
its own message stating what ran and why — never bundled with a gameplay change.

---

## LFS is permanent

`Content/` is LFS-tracked. The repo sits at ~14.8 GB against a 10 GB Pro allowance, metered
at $0.07/GB/month.

- **Every version of every asset is stored forever.** A 200 MB asset re-saved ten times is
  2 GB stored, permanently.
- Gitignoring something later **caps future growth and reclaims nothing.**
- `Content/SourcedAssets/` (~6.1 GB) is purchased marketplace art, re-downloadable from Fab.
  It is the main growth driver and the first thing to think twice about.

**Run `git lfs status` before pushing** and look at what is in the list. A surprise there is
a bill, not a mistake you can undo.

---

## Blueprint and C++ move together

A C++ change that alters a `UPROPERTY`, `UFUNCTION`, class name, or any reflected signature
can silently break Blueprints that reference it. **The C++ compiler will not tell you.**

- Run `CompileAllBlueprints` after any such change — it is step L.1's second gate.
- When a rename is unavoidable, add a `CoreRedirect` in `Config/DefaultEngine.ini` rather
  than leaving assets to fail. Note that **class redirects resolve before property and
  function redirects** — a property redirect keyed on an old class name can never fire.
- If a C++ change requires a Blueprint change, they belong in the **same commit**. Split
  across two, the repo has a commit where the game does not run.

---

## Merge conflicts in content are unresolvable

There is no three-way merge for a `.uasset`. Git will offer you "ours" or "theirs" and both
mean discarding someone's work.

Solo, this is mostly avoidable by keeping content branches short and rebasing before
touching an asset another branch also touched. **When a content conflict does happen, do not
resolve it mechanically** — pick one version deliberately, re-apply the other change by
hand in the editor, and say so in the commit body.

Long-lived content branches are the thing that turns this from rare into routine. Keep them
short.

---

## What belongs in Blueprint at all

From CLAUDE.md → Key Principles: **Blueprint is a leaf, not a layer.**

| Belongs in Blueprint | Belongs in C++ |
|---|---|
| Composition — which components, which meshes | Logic other systems depend on |
| Tuning values and curves | Anything with a state machine |
| Presentation, animation hookup, VFX triggers | Anything on a per-frame path |
| Widget layout | Anything worth a test |

The reason is entirely this file's subject: **C++ can be diffed, reviewed, and tested.
Blueprint can be none of the three.** Logic in a Blueprint is logic that leaves no trace in
the history and cannot be regression-tested. Put it there only when the alternative is
worse.
