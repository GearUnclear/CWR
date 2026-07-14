# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## What this is

This repository is **Uslu dur!** (short: **UD**) — a **total game overhaul** of *Arma: Cold War Assault* (originally *Operation Flashpoint*, 2001). "Uslu dur!" is the project's public brand; the rename is branding-only, so folder names, build targets, and the `cwr` / `CWR` codenames throughout the source are unchanged and stay as-is. Both names refer to the same project.

The overhaul is not just an engine port — it reworks AI tactics/perception and adds **Guerrilla Mode**, a new persistent open-world insurgency game mode with islands and factions swappable at new-game. Engine C++ changes are a **first-class tool** here, alongside SQF mission scripting — the old "avoid C++, script everything" posture is retired.

## Repository layout (git root = `D:\Arma_CWA`)

The git root was rerooted up from `arma_CWR/` to `D:\Arma_CWA` so the web portal and mod test bed are version-controlled alongside the engine. `origin` is the `GearUnclear/CWR` fork. Top-level contents:

- **`arma_CWR/`** — the engine + overhaul design docs. This is the main development area and has its own detailed [`arma_CWR/CLAUDE.md`](arma_CWR/CLAUDE.md) covering toolchain, build, tests, and architecture. **Read that file before doing any engine work.** The `arma_CWR` folder name is legacy from before the reroot/rename; don't "fix" it.
- **`site/`** — the public web portal / field manual (static HTML: `index.html`, `about.html`, `gameplay.html`, `tech.html`, `designloop.html`, `downloads.html`, plus `assets/`). Branded "Uslu dur!". Pure front-end, no build step — edit the HTML/CSS/asset files directly and open in a browser to verify.
- **`@LoBo/`** — the *Lost Brothers* mod (Sinai island + IDF/Egypt/Syria/Jordan factions), used as a **test bed** for the Guerrilla Mode island/faction swappability work. Third-party mod content, not our source; treat as read-only reference data unless a task says otherwise.
- **`ARMA Cold War Assault [Classic]/`** — the original v1.99 full install (`ColdWarAssault.exe`, full Worlds/Campaigns/AddOns). **Primary data dir** (`OFPR_DATA_DIR`) for Guerrilla Mode and the `full_cwa`/`lobo` test lanes — the Demo lacks the full island/faction content and third-party-mod compatibility (#11). Game data, **not** tracked source; read-only except one deliberate exception — the local `bin/remaster.cpp` shim and UD-added `fonts/cwr_*.ttf` live inside it (#11/#13).
- **`Arma Cold War Assault Demo [Remaster]/`** — the official remaster Demo package (Steam app 4819000). **Read-only reference, like `@LoBo` — never edit.** **Asset-targeting policy (2026-07-14): the Demo's asset set is NOT a gameplay target.** Guerrilla Mode content targets the Classic 1.99 + `@LoBo` asset sets only; what we preserve toward the Demo/Remaster side is *future compatibility with the full Remaster's assets* — the engine's descriptor-resolution pass (plan 15) keeps missing-class situations non-fatal, and nothing may hard-require an asset the full Remaster is expected to lack. Do not spend effort making Guerrilla content or tests run on the Demo package's roster. The package's surviving roles: (1) contract source — its `BIN/remaster.cpp` inventories every `Remaster >>` config key the engine reads, the checklist for backfilling the Classic shim; (2) reference engine build — `PoseidonGameDemo.exe` is the official binary of this repo's remaster engine, for "ours or upstream's?" behaviour diffs (#11/#13); (3) occasional `demo`-lane data source for engine-level (non-content) tests.

## Where work happens

- **Engine / overhaul mechanics** → `arma_CWR/` (C++ engine under `engine/`, apps under `apps/`, design docs in `arma_CWR/mod-plans/` and `arma_CWR/guerrilla-mode/`). See `arma_CWR/CLAUDE.md`.
- **Website** → `site/`. Static; no toolchain.
- The **`mod-plans/`** and **`guerrilla-mode/`** documents are living, AI-drafted design docs subject to correction — treat them as design intent, not settled hard specs. Verify their claims against the actual code before relying on them.

## Internal roadmap (keep it updated)

`roadmap/roadmap.yaml` is the machine-parsable source of truth for what is done, in flight, and planned, with dependencies and GitHub-issue links. `roadmap/generate.py` validates it, syncs against GitHub issues, and renders `roadmap/index.html` + `status.json`; a post-commit hook regenerates them (one-time setup per clone: `git config core.hooksPath .githooks`). **When you land, start, or plan a meaningful chunk of work, update the matching item in `roadmap.yaml`** (status, commits, a `log` line for problems hit); the generator warns when a GitHub issue is not linked to any roadmap item. GitHub issues live on the fork: always `gh -R GearUnclear/CWR` (plain `gh` resolves to the upstream Bohemia repo). See `roadmap/README.md`.

## Conventions

- The public brand is **Uslu dur! / UD** in user-facing text (the site, docs written for players). Internal code, folders, and build targets keep the `cwr`/`CWR`/`Poseidon` names — do not mass-rename them.
- Multiple concurrent sessions touch this repo. If a file appears to be missing or deleted, check whether the removal was intentional before "restoring" it.
