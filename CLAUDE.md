# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## What this is

This repository is **Uslu dur!** (short: **UD**) — a **total game overhaul** of *Arma: Cold War Assault* (originally *Operation Flashpoint*, 2001). "Uslu dur!" is the project's public brand; the rename is branding-only, so folder names, build targets, and the `cwr` / `CWR` codenames throughout the source are unchanged and stay as-is. Both names refer to the same project.

The overhaul is not just an engine port — it reworks AI tactics/perception and adds **Guerrilla Mode**, a new persistent open-world insurgency game mode with islands and factions swappable at new-game. Engine C++ changes are a **first-class tool** here, alongside SQF mission scripting — the old "avoid C++, script everything" posture is retired.

## Target platform: the Remaster (Classic compatibility is being dropped)

**As of 2026-10-11, UD targets the full official *Arma: Cold War Assault* Remaster.** Compatibility with the Classic 1.99 install is being dropped. New engine, content, and test work should assume Remaster data and the Remaster's `remaster.cpp` contract. Do not add new Classic-only shims, fallbacks, or test lanes, and do not spend effort keeping a change working on Classic. The Classic shims and the `[Classic]` data dir still exist while the migration is under way; treat them as legacy to be retired, not as something to extend. Where older docs or memory notes say "Classic 1.99 is the target", this section supersedes them.

## Repository layout (git root = `D:\Arma_CWA`)

The git root was rerooted up from `arma_CWR/` to `D:\Arma_CWA` so the web portal and mod test bed are version-controlled alongside the engine. `origin` is the `GearUnclear/CWR` fork. Top-level contents:

- **`arma_CWR/`** — the engine + overhaul design docs. This is the main development area and has its own detailed [`arma_CWR/CLAUDE.md`](arma_CWR/CLAUDE.md) covering toolchain, build, tests, and architecture. **Read that file before doing any engine work.** The `arma_CWR` folder name is legacy from before the reroot/rename; don't "fix" it.
- **`site/`** — the public web portal / field manual (static HTML: `index.html`, `about.html`, `gameplay.html`, `tech.html`, `designloop.html`, `downloads.html`, `media.html`, `undercover.html`, plus `assets/`). Branded "Uslu dur!". Pure front-end, no build step — edit the HTML/CSS/asset files directly and open in a browser to verify.
- **`@LoBo/`** — the *Lost Brothers* mod (Sinai island + IDF/Egypt/Syria/Jordan factions), used as a **test bed** for the Guerrilla Mode island/faction swappability work, and as of 2026-08-08 **ours to modify**: fixing a content defect at source in @LoBo is now preferred over layering a runtime overlay on top of it. (This reverses the earlier "third-party, read-only" stance.) Two caveats stay: the directory is **gitignored as game data**, so an edit made here exists only on the machine that made it, and the tracked artifact must be a **regeneration script** that can redo the repair after a reinstall; and the pbo bytes are APL-SA third-party data that must never be committed to this GPL repo. The repair that actually runs is `arma_CWR/setup-guerrilla.ps1`: it calls `arma_CWR/tools/lobo/install-lobo-factions.ps1` on its `-LoBoDir`, then `PoseidonTools mod doctor <LoBoDir> --fix`. `arma_CWR/tools/lobo/fix-lobo-scope.ps1` is a thin wrapper around `mod doctor`, and the fixture regenerator `arma_CWR/tests/fixtures/mods-lobo/@lobofixup/gen-patched-pbos.ps1` is optional. `D:\LoBo-Reloaded` (outside the repo, family use only) is the LoBo source of truth and where a reinstall comes from: verified builds in its `dist\` (`LoBo-Reloaded-<date>.7z`), unpacked under `modfolders\<date>\`; see its `README.txt`.
- **`ARMA Cold War Assault [Classic]/`** — original v1.99 game data, ignored by Git. **Legacy: Classic compatibility is being dropped in favour of the Remaster (see above).** Until it is retired: `arma_CWR/setup-guerrilla.ps1` prepares the folder passed as `-GameDir` in place (no copy is made; files it replaces are backed up under that folder's `.ud-backups`; see `arma_CWR/guerrilla-mode/FRESH-INSTALL.md`). Classic compatibility/menu shims now live in `arma_CWR/guerrilla-mode/runtime/classic`, Options text resources under separately licensed `arma_CWR/thirdparty/classic-ui`, and OFL fonts under `arma_CWR/thirdparty/guerrilla-fonts`. Edit those tracked sources and reinstall; do not rely on old local-only overrides or renamed Windows fonts. Base game PBOs remain external.
- **`Arma Cold War Assault Demo [Remaster]/`** — the official remaster Demo package (Steam app 4819000). **Read-only reference — never edit** (this one really is off limits; `@LoBo` no longer is). **Asset-targeting policy (2026-07-14): the Demo's asset set is NOT a gameplay target.** Guerrilla Mode content targets the full Remaster + `@LoBo` asset sets (superseding the earlier "Classic 1.99 + `@LoBo` only" wording); the engine's descriptor-resolution pass (plan 15) keeps missing-class situations non-fatal. Do not spend effort making Guerrilla content or tests run on the Demo package's roster. The package's surviving roles: (1) contract source — its `BIN/remaster.cpp` inventories every `Remaster >>` config key the engine reads (the full Remaster ships its own complete `remaster.cpp`, so this is now a cross-check rather than a Classic-shim backfill list); (2) reference engine build — `PoseidonGameDemo.exe` is the official binary of this repo's remaster engine, for "ours or upstream's?" behaviour diffs (#11/#13); (3) occasional `demo`-lane data source for engine-level (non-content) tests.
- **`AGENTS.md`**: short cross-agent summary of the key rules (Remaster target, no new branches) for agents that are not Claude; it points back here.
- **`agent_guidelines/`**: one-topic notes on repo findings (what goes wrong, why, what to do). Its `README.md` indexes them and sets the format rules for adding one.
- **`.github/workflows/`**: CI. `ci.yml` builds and runs the unit suites and mod-intake tool lanes against the synthetic fixtures (no game data); `lobo-repair.yml` runs the offline LoBo repair tests.
- **`tmp/`**: local scratch, gitignored. Holds the Remaster shadows (`tmp/remaster-shadow-67` and siblings) that the launchers default to.

## Where work happens

- **Engine / overhaul mechanics** → `arma_CWR/` (C++ engine under `engine/`, apps under `apps/`, design docs in `arma_CWR/mod-plans/` and `arma_CWR/guerrilla-mode/`). See `arma_CWR/CLAUDE.md`.
- **Website** → `site/`. Static; no toolchain.
- The **`mod-plans/`** and **`guerrilla-mode/`** documents are living, AI-drafted design docs subject to correction — treat them as design intent, not settled hard specs. Verify their claims against the actual code before relying on them.

## Internal roadmap (keep it updated)

`roadmap/roadmap.yaml` is the machine-parsable source of truth for what is done, in flight, and planned, with dependencies and GitHub-issue links. `roadmap/generate.py` validates it, syncs against GitHub issues, and renders `roadmap/index.html` + `status.json`; a post-commit hook regenerates them (one-time setup per clone: `git config core.hooksPath .githooks`). **When you land, start, or plan a meaningful chunk of work, update the matching item in `roadmap.yaml`** (status, commits, a `log` line for problems hit); the generator warns when a GitHub issue is not linked to any roadmap item. GitHub issues live on the fork: always `gh -R GearUnclear/CWR` (plain `gh` resolves to the upstream Bohemia repo). See `roadmap/README.md`.

## Conventions

- **Make no new branches unless the user grants permission.** Work on the current branch (normally `main`). Do not run `git branch`, `git checkout -b`, `git switch -c`, `git worktree add -b`, or push a new branch to any remote without the user's explicit OK in chat for that branch. Approval for one branch does not extend to the next. This applies to subagents too.

- The public brand is **Uslu dur! / UD** in user-facing text (the site, docs written for players). Internal code, folders, and build targets keep the `cwr`/`CWR`/`Poseidon` names — do not mass-rename them.
- Multiple concurrent sessions touch this repo. If a file appears to be missing or deleted, check whether the removal was intentional before "restoring" it.
- Prose in docs, the site, commit messages and agent notes uses no em dashes.
