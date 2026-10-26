# Local work archive, 2026-10

This orphan branch (`archive/local-work-2026-10`) is a frozen snapshot of local-only text work taken on 2026-10-26, just before the scratch worktrees and temporary folders it came from were deleted. It is text only (patches, notes, scripts, small data files) and it is never meant to be merged into `main` or any other branch. Nothing here builds on its own; patches apply to the base commit named next to them.

Path rule: any folder that was named `tmp` or `packages` at the source is stored as `tmp_` or `packages_` here.

Total: 7453 files, 96.6 MB.

## lanes/ (1027 files, 23.9 MB)

One folder per scratch worktree under `D:/Arma_CWA_worktrees/` (prefixes gfx-, fix-, fix2-, gr-, look-). Each holds `lane.patch` (`git diff HEAD`: staged and unstaged changes to tracked files), `base.txt` (HEAD commit, subject and the origin branches that contain it), `untracked/` (untracked text files) and `ignored/` (text from the ignored `tmp/` and `arma_CWR/tests/rendering/research/` folders). Patches apply with `git apply` on the base commit.

| Lane | Source path | Base commit | Date | Description |
|---|---|---|---|---|
| fix-bush-instancing | `D:/Arma_CWA_worktrees/fix-bush-instancing` | `3c53141a86` tests: capture runs take their own data junction and a shade | 2026-10-17 | lighting bug-fix lane, first pass (bug: bush-instancing); 4 files in lane.patch, 3 ignored |
| fix-capture-harness | `D:/Arma_CWA_worktrees/fix-capture-harness` | `3c53141a86` tests: capture runs take their own data junction and a shade | 2026-10-17 | lighting bug-fix lane, first pass (bug: capture-harness); 16 files in lane.patch, 11 ignored |
| fix-clear-sky-clouds | `D:/Arma_CWA_worktrees/fix-clear-sky-clouds` | `3c53141a86` tests: capture runs take their own data junction and a shade | 2026-10-17 | lighting bug-fix lane, first pass (bug: clear-sky-clouds); 4 files in lane.patch, 5 ignored |
| fix-crown-nan | `D:/Arma_CWA_worktrees/fix-crown-nan` | `3c53141a86` tests: capture runs take their own data junction and a shade | 2026-10-17 | lighting bug-fix lane, first pass (bug: crown-nan); 5 files in lane.patch, 3 ignored |
| fix-msaa-default | `D:/Arma_CWA_worktrees/fix-msaa-default` | `3c53141a86` tests: capture runs take their own data junction and a shade | 2026-10-17 | lighting bug-fix lane, first pass (bug: msaa-default); 8 files in lane.patch, 5 ignored |
| fix-night-bright | `D:/Arma_CWA_worktrees/fix-night-bright` | `3c53141a86` tests: capture runs take their own data junction and a shade | 2026-10-17 | lighting bug-fix lane, first pass (bug: night-bright); 7 files in lane.patch, 16 ignored |
| fix-object-shadows | `D:/Arma_CWA_worktrees/fix-object-shadows` | `3c53141a86` tests: capture runs take their own data junction and a shade | 2026-10-17 | lighting bug-fix lane, first pass (bug: object-shadows); 6 files in lane.patch, 5 ignored |
| fix-shadow-sampler | `D:/Arma_CWA_worktrees/fix-shadow-sampler` | `3c53141a86` tests: capture runs take their own data junction and a shade | 2026-10-17 | lighting bug-fix lane, first pass (bug: shadow-sampler); 3 files in lane.patch, 3 ignored |
| fix-shaft-veil | `D:/Arma_CWA_worktrees/fix-shaft-veil` | `3c53141a86` tests: capture runs take their own data junction and a shade | 2026-10-17 | lighting bug-fix lane, first pass (bug: shaft-veil); 7 files in lane.patch, 5 ignored |
| fix-spawned-vehicles | `D:/Arma_CWA_worktrees/fix-spawned-vehicles` | `3c53141a86` tests: capture runs take their own data junction and a shade | 2026-10-17 | lighting bug-fix lane, first pass (bug: spawned-vehicles); 6 files in lane.patch, 13 ignored |
| fix-streetlamp-crash | `D:/Arma_CWA_worktrees/fix-streetlamp-crash` | `3c53141a86` tests: capture runs take their own data junction and a shade | 2026-10-17 | lighting bug-fix lane, first pass (bug: streetlamp-crash); 4 files in lane.patch, 3 ignored |
| fix2-msaa-default | `D:/Arma_CWA_worktrees/fix2-msaa-default` | `84408f849d` roadmap: remastered lighting bug fixes landed | 2026-10-17 | lighting bug-fix lane, second pass (bug: msaa-default); 8 files in lane.patch, 5 ignored |
| fix2-object-shadows | `D:/Arma_CWA_worktrees/fix2-object-shadows` | `84408f849d` roadmap: remastered lighting bug fixes landed | 2026-10-17 | lighting bug-fix lane, second pass (bug: object-shadows); 6 files in lane.patch |
| gfx-aa | `D:/Arma_CWA_worktrees/gfx-aa` | `3c53141a86` tests: capture runs take their own data junction and a shade | 2026-10-17 | remastered-lighting research lane (feature: aa); 3 files in lane.patch, 19 ignored |
| gfx-atmosphere | `D:/Arma_CWA_worktrees/gfx-atmosphere` | `3c53141a86` tests: capture runs take their own data junction and a shade | 2026-10-17 | remastered-lighting research lane (feature: atmosphere); 10 files in lane.patch, 22 ignored |
| gfx-clouds | `D:/Arma_CWA_worktrees/gfx-clouds` | `3c53141a86` tests: capture runs take their own data junction and a shade | 2026-10-17 | remastered-lighting research lane (feature: clouds); 12 files in lane.patch, 11 ignored |
| gfx-combat | `D:/Arma_CWA_worktrees/gfx-combat` | `3c53141a86` tests: capture runs take their own data junction and a shade | 2026-10-17 | remastered-lighting research lane (feature: combat); 20 files in lane.patch, 2 untracked, 19 ignored |
| gfx-exposure | `D:/Arma_CWA_worktrees/gfx-exposure` | `3c53141a86` tests: capture runs take their own data junction and a shade | 2026-10-17 | remastered-lighting research lane (feature: exposure); 7 files in lane.patch, 13 ignored |
| gfx-foliage | `D:/Arma_CWA_worktrees/gfx-foliage` | `3c53141a86` tests: capture runs take their own data junction and a shade | 2026-10-17 | remastered-lighting research lane (feature: foliage); 7 files in lane.patch, 43 ignored |
| gfx-gbuffer | `D:/Arma_CWA_worktrees/gfx-gbuffer` | `3c53141a86` tests: capture runs take their own data junction and a shade | 2026-10-17 | remastered-lighting research lane (feature: gbuffer); 12 files in lane.patch, 37 ignored |
| gfx-haze | `D:/Arma_CWA_worktrees/gfx-haze` | `3c53141a86` tests: capture runs take their own data junction and a shade | 2026-10-17 | remastered-lighting research lane (feature: haze); 5 files in lane.patch, 41 ignored |
| gfx-hfrt | `D:/Arma_CWA_worktrees/gfx-hfrt` | `3c53141a86` tests: capture runs take their own data junction and a shade | 2026-10-17 | remastered-lighting research lane (feature: hfrt); 6 files in lane.patch, 37 ignored |
| gfx-materials | `D:/Arma_CWA_worktrees/gfx-materials` | `3c53141a86` tests: capture runs take their own data junction and a shade | 2026-10-17 | remastered-lighting research lane (feature: materials); 9 files in lane.patch, 45 ignored |
| gfx-night | `D:/Arma_CWA_worktrees/gfx-night` | `3c53141a86` tests: capture runs take their own data junction and a shade | 2026-10-17 | remastered-lighting research lane (feature: night); 9 files in lane.patch, 22 ignored |
| gfx-normals | `D:/Arma_CWA_worktrees/gfx-normals` | `3c53141a86` tests: capture runs take their own data junction and a shade | 2026-10-17 | remastered-lighting research lane (feature: normals); 8 files in lane.patch, 39 ignored |
| gfx-perf | `D:/Arma_CWA_worktrees/gfx-perf` | `3c53141a86` tests: capture runs take their own data junction and a shade | 2026-10-17 | remastered-lighting research lane (feature: perf); 11 files in lane.patch, 21 ignored |
| gfx-shade | `D:/Arma_CWA_worktrees/gfx-shade` | `3c53141a86` tests: capture runs take their own data junction and a shade | 2026-10-17 | remastered-lighting research lane (feature: shade); 3 files in lane.patch, 49 ignored |
| gfx-shadows | `D:/Arma_CWA_worktrees/gfx-shadows` | `3c53141a86` tests: capture runs take their own data junction and a shade | 2026-10-17 | remastered-lighting research lane (feature: shadows); 8 files in lane.patch, 27 ignored |
| gfx-ssgi | `D:/Arma_CWA_worktrees/gfx-ssgi` | `3c53141a86` tests: capture runs take their own data junction and a shade | 2026-10-17 | remastered-lighting research lane (feature: ssgi); 5 files in lane.patch, 25 ignored |
| gfx-ssr | `D:/Arma_CWA_worktrees/gfx-ssr` | `3c53141a86` tests: capture runs take their own data junction and a shade | 2026-10-17 | remastered-lighting research lane (feature: ssr); 11 files in lane.patch, 13 ignored |
| gfx-temporal | `D:/Arma_CWA_worktrees/gfx-temporal` | `3c53141a86` tests: capture runs take their own data junction and a shade | 2026-10-17 | remastered-lighting research lane (feature: temporal); 9 files in lane.patch, 31 ignored |
| gfx-texsharp | `D:/Arma_CWA_worktrees/gfx-texsharp` | `3c53141a86` tests: capture runs take their own data junction and a shade | 2026-10-17 | remastered-lighting research lane (feature: texsharp); 9 files in lane.patch, 79 ignored |
| gfx-volumetric | `D:/Arma_CWA_worktrees/gfx-volumetric` | `3c53141a86` tests: capture runs take their own data junction and a shade | 2026-10-17 | remastered-lighting research lane (feature: volumetric); 4 files in lane.patch, 10 ignored |
| gfx-wildcard | `D:/Arma_CWA_worktrees/gfx-wildcard` | `3c53141a86` tests: capture runs take their own data junction and a shade | 2026-10-17 | remastered-lighting research lane (feature: wildcard); 9 files in lane.patch, 41 ignored |
| gr-A4 | `D:/Arma_CWA_worktrees/gr-A4` | `ced77aeef3` graphics: the post pass runs as a list of stages | 2026-10-18 | graphics-remaster integration lane A4; 33 files in lane.patch |
| gr-A5 | `D:/Arma_CWA_worktrees/gr-A5` | `ced77aeef3` graphics: the post pass runs as a list of stages | 2026-10-18 | graphics-remaster integration lane A5; 28 files in lane.patch |
| gr-A6 | `D:/Arma_CWA_worktrees/gr-A6` | `c584185e8d` graphics: long views draw the same frame from a fraction of  | 2026-10-18 | graphics-remaster integration lane A6; 30 files in lane.patch |
| gr-A7 | `D:/Arma_CWA_worktrees/gr-A7` | `ced77aeef3` graphics: the post pass runs as a list of stages | 2026-10-18 | graphics-remaster integration lane A7; 30 files in lane.patch |
| gr-B1 | `D:/Arma_CWA_worktrees/gr-B1` | `193c7354cd` graphics: temporal anti-aliasing steadies foliage and cleans | 2026-10-18 | graphics-remaster integration lane B1; 15 files in lane.patch |
| gr-B10 | `D:/Arma_CWA_worktrees/gr-B10` | `5d7e7aed6d` graphics: a physically based sky table lights the dome and t | 2026-10-18 | graphics-remaster integration lane B10; 22 files in lane.patch |
| gr-B11 | `D:/Arma_CWA_worktrees/gr-B11` | `193c7354cd` graphics: temporal anti-aliasing steadies foliage and cleans | 2026-10-18 | graphics-remaster integration lane B11; 23 files in lane.patch |
| gr-B11-rebase | `D:/Arma_CWA_worktrees/gr-B11-rebase` | `5d7e7aed6d` graphics: a physically based sky table lights the dome and t | 2026-10-18 | graphics-remaster integration lane B11-rebase; 23 files in lane.patch |
| gr-B2 | `D:/Arma_CWA_worktrees/gr-B2` | `807ce4bf17` graphics: trees and buildings cast soft, contact-hardening s | 2026-10-18 | graphics-remaster integration lane B2; 11 files in lane.patch |
| gr-B3 | `D:/Arma_CWA_worktrees/gr-B3` | `7b7e3e61b6` graphics: every tree casts a long golden-hour shadow at any  | 2026-10-18 | graphics-remaster integration lane B3; 27 files in lane.patch |
| gr-B4 | `D:/Arma_CWA_worktrees/gr-B4` | `706b9a30b3` graphics: one tiled light list feeds every local light | 2026-10-18 | graphics-remaster integration lane B4; 48 files in lane.patch |
| gr-B4-rebase | `D:/Arma_CWA_worktrees/gr-B4-rebase` | `cb3dd0d813` graphics: rain wets cloth and stone, fills puddles and strea | 2026-10-18 | graphics-remaster integration lane B4-rebase; 51 files in lane.patch |
| gr-B5 | `D:/Arma_CWA_worktrees/gr-B5` | `5d7e7aed6d` graphics: a physically based sky table lights the dome and t | 2026-10-18 | graphics-remaster integration lane B5; 26 files in lane.patch |
| gr-B6 | `D:/Arma_CWA_worktrees/gr-B6` | `e91b3f3a97` graphics: the sea gets wave relief and a broken, glittering  | 2026-10-18 | graphics-remaster integration lane B6; 30 files in lane.patch |
| gr-B7 | `D:/Arma_CWA_worktrees/gr-B7` | `c36ee4ba6d` graphics: calm water and wet streets reflect what is on scre | 2026-10-18 | graphics-remaster integration lane B7; 35 files in lane.patch |
| gr-B8 | `D:/Arma_CWA_worktrees/gr-B8` | `807ce4bf17` graphics: trees and buildings cast soft, contact-hardening s | 2026-10-18 | graphics-remaster integration lane B8; 24 files in lane.patch |
| gr-B9 | `D:/Arma_CWA_worktrees/gr-B9` | `807ce4bf17` graphics: trees and buildings cast soft, contact-hardening s | 2026-10-18 | graphics-remaster integration lane B9; 28 files in lane.patch |
| look-director | `D:/Arma_CWA_worktrees/look-director` | `fec10759df` graphics: fires, flares and shell bursts light the men and c | 2026-10-23 | look-director work lane (director); 120 files in lane.patch, 33 untracked, 59 ignored |
| look-fix-fire | `D:/Arma_CWA_worktrees/look-fix-fire` | `fec10759df` graphics: fires, flares and shell bursts light the men and c | 2026-10-19 | look-director work lane (fix-fire); 121 files in lane.patch |
| look-fix-horizon | `D:/Arma_CWA_worktrees/look-fix-horizon` | `fec10759df` graphics: fires, flares and shell bursts light the men and c | 2026-10-19 | look-director work lane (fix-horizon); 115 files in lane.patch |
| look-fix-plate | `D:/Arma_CWA_worktrees/look-fix-plate` | `fec10759df` graphics: fires, flares and shell bursts light the men and c | 2026-10-19 | look-director work lane (fix-plate); 123 files in lane.patch |
| look-fix-seam | `D:/Arma_CWA_worktrees/look-fix-seam` | `fec10759df` graphics: fires, flares and shell bursts light the men and c | 2026-10-19 | look-director work lane (fix-seam); 116 files in lane.patch |
| look-fix-shine | `D:/Arma_CWA_worktrees/look-fix-shine` | `fec10759df` graphics: fires, flares and shell bursts light the men and c | 2026-10-19 | look-director work lane (fix-shine); 109 files in lane.patch |
| look-fix-veil | `D:/Arma_CWA_worktrees/look-fix-veil` | `fec10759df` graphics: fires, flares and shell bursts light the men and c | 2026-10-19 | look-director work lane (fix-veil); 117 files in lane.patch |
| look-fog | `D:/Arma_CWA_worktrees/look-fog` | `7b7e3e61b6` graphics: every tree casts a long golden-hour shadow at any  | 2026-10-18 | look-director work lane (fog); 47 files in lane.patch |
| look-light | `D:/Arma_CWA_worktrees/look-light` | `c36ee4ba6d` graphics: calm water and wet streets reflect what is on scre | 2026-10-18 | look-director work lane (light); 53 files in lane.patch |
| look-puddle-1 | `D:/Arma_CWA_worktrees/look-puddle-1` | `fec10759df` graphics: fires, flares and shell bursts light the men and c | 2026-10-19 | look-director work lane (puddle-1); 124 files in lane.patch |
| look-puddle-2 | `D:/Arma_CWA_worktrees/look-puddle-2` | `fec10759df` graphics: fires, flares and shell bursts light the men and c | 2026-10-19 | look-director work lane (puddle-2); 128 files in lane.patch |
| look-puddle-3 | `D:/Arma_CWA_worktrees/look-puddle-3` | `fec10759df` graphics: fires, flares and shell bursts light the men and c | 2026-10-19 | look-director work lane (puddle-3); 124 files in lane.patch |
| look-puddle-4 | `D:/Arma_CWA_worktrees/look-puddle-4` | `fec10759df` graphics: fires, flares and shell bursts light the men and c | 2026-10-19 | look-director work lane (puddle-4); 122 files in lane.patch |
| look-puddle-5 | `D:/Arma_CWA_worktrees/look-puddle-5` | `fec10759df` graphics: fires, flares and shell bursts light the men and c | 2026-10-19 | look-director work lane (puddle-5); 125 files in lane.patch |
| look-puddle-6 | `D:/Arma_CWA_worktrees/look-puddle-6` | `fec10759df` graphics: fires, flares and shell bursts light the men and c | 2026-10-19 | look-director work lane (puddle-6); 119 files in lane.patch, 6 untracked |
| look-ref | `D:/Arma_CWA_worktrees/look-ref` | `fec10759df` graphics: fires, flares and shell bursts light the men and c | 2026-10-19 | look-director work lane (ref); 96 files in lane.patch, 10 untracked |
| look-surface | `D:/Arma_CWA_worktrees/look-surface` | `fec10759df` graphics: fires, flares and shell bursts light the men and c | 2026-10-18 | look-director work lane (surface); 33 files in lane.patch, 60 untracked |

`look-director` is the largest lane: its `lane.patch` covers 120 changed files (1.2 MB). Its 51 untracked files are 33 text files kept under `untracked/` plus 18 generated capture tests (headers say "Generated, do not edit"; their generator `gen_look_director.py` is in the patch).

## graphics/

| Folder | Source path | Base commit | Date | Description |
|---|---|---|---|---|
| `graphics/lighting/` (1539 files, 8.2 MB) | `D:/Arma_CWA_worktrees/lighting (ignored tmp/ and arma_CWR/tests/rendering/research/)` | branch claude/youthful-lovelace-8imbdh @ 026133b5 | 2026-10-17 | Remastered-lighting research: per-feature research notes, MENU.md, scripts, prototype and bug-fix patches, capture test sources |
| `graphics/graphics-remaster-tmp/` (2957 files, 37.1 MB) | `D:/Arma_CWA_worktrees/graphics-remaster/tmp (copy; the folder stays)` | branch graphics-remaster @ 58bfb569 | 2026-10-23 | Integration lanes A0-B11, look-director rounds (look/), trailer video work (video/: brief, VO script, clips, runbook, tools, fogwind, mist, heads) |
| `graphics/scratchpad-0cfe8c80/` (894 files, 21.5 MB) | `C:/Users/Dane/AppData/Local/Temp/claude/D--Arma-CWA/0cfe8c80-fc33-4f8e-a86e-52fa1175350d/scratchpad` | various (see patch headers) | 2026-10-19 | Session scratchpad of the lighting/graphics orchestration: lane patches, review diffs, rebase files, commit messages, analysis scripts |

## occupier/

| Folder | Source path | Base commit | Date | Description |
|---|---|---|---|---|
| `occupier/` (195 files, 2.0 MB) | `D:/Arma_CWA/tmp` (top-level of-*, occupier-forces-* and listed notes; of-director-video*/, of-held-tests/ was empty) | occupier-forces @ a37990ec | 2026-10-18 | Occupier forces orchestration: handoffs, progress log, reviews, assessments, issue drafts, commit messages, gate and staging scripts, patches, director-video scenario sources and render scripts |

## lobo/

Only our own scripts and reports (md, txt, py, ps1, json) from each `D:/Arma_CWA/tmp/lobo-*` folder. No extracted LoBo content, configs (in any form, JSON included), scripts, readmes, file listings or scraped indexes.

| Folder | Source path | Base commit | Date | Description |
|---|---|---|---|---|
| `lobo/lobo-audit-20261011/` (13 files, 287 KB) | `D:/Arma_CWA/tmp/lobo-audit-20261011` | none | 2026-10-11 | LoBo repair, audit and validation work: our scripts, specs, reports and tool outputs |
| `lobo/lobo-family-wf/` (39 files, 82 KB) | `D:/Arma_CWA/tmp/lobo-family-wf` | none | 2026-10-11 | LoBo repair, audit and validation work: our scripts, specs, reports and tool outputs |
| `lobo/lobo-issue69-validation/` (3 files, 22 KB) | `D:/Arma_CWA/tmp/lobo-issue69-validation` | none | 2026-10-11 | LoBo repair, audit and validation work: our scripts, specs, reports and tool outputs |
| `lobo/lobo-missing-2/` (11 files, 115 KB) | `D:/Arma_CWA/tmp/lobo-missing-2` | none | 2026-10-12 | LoBo repair, audit and validation work: our scripts, specs, reports and tool outputs |
| `lobo/lobo-ownership-null-x/` (2 files, 4 KB) | `D:/Arma_CWA/tmp/lobo-ownership-null-x` | none | 2026-10-11 | LoBo repair, audit and validation work: our scripts, specs, reports and tool outputs |
| `lobo/lobo-repair-3/` (157 files, 1.0 MB) | `D:/Arma_CWA/tmp/lobo-repair-3` | none | 2026-10-11 | LoBo repair, audit and validation work: our scripts, specs, reports and tool outputs |
| `lobo/lobo-repair-4/` (81 files, 358 KB) | `D:/Arma_CWA/tmp/lobo-repair-4` | none | 2026-10-12 | LoBo repair, audit and validation work: our scripts, specs, reports and tool outputs |
| `lobo/lobo-runtime-acceptance-20261011/` (20 files, 89 KB) | `D:/Arma_CWA/tmp/lobo-runtime-acceptance-20261011` | none | 2026-10-11 | LoBo repair, audit and validation work: our scripts, specs, reports and tool outputs |
| `lobo/lobo-runtime-tracker-20261011/` (3 files, 11 KB) | `D:/Arma_CWA/tmp/lobo-runtime-tracker-20261011` | none | 2026-10-11 | LoBo repair, audit and validation work: our scripts, specs, reports and tool outputs |
| `lobo/lobo-script-repair-20261011/` (1 files, 0 KB) | `D:/Arma_CWA/tmp/lobo-script-repair-20261011` | none | 2026-10-11 | LoBo repair, audit and validation work: our scripts, specs, reports and tool outputs |
| `lobo/lobo-tracker/` (3 files, 30 KB) | `D:/Arma_CWA/tmp/lobo-tracker` | none | 2026-10-12 | LoBo repair, audit and validation work: our scripts, specs, reports and tool outputs |
| `lobo/lobo-wound-recolor/` (8 files, 33 KB) | `D:/Arma_CWA/tmp/lobo-wound-recolor` | none | 2026-10-12 | LoBo repair, audit and validation work: our scripts, specs, reports and tool outputs |
| `lobo/lobo-wound-review/` (6 files, 30 KB) | `D:/Arma_CWA/tmp/lobo-wound-review` | none | 2026-10-12 | LoBo repair, audit and validation work: our scripts, specs, reports and tool outputs |

## ce/

| File | Source path | Base commit | Date | Description |
|---|---|---|---|---|
| `ce/ce-304-fix.patch, ce/base.txt` | `D:/Arma_CWA/tmp/ce-304-fix` | CWR-CE aa52258 | 2026-10 | Uncommitted CWR-CE issue #304 fix (git diff HEAD, incl. 3 staged new files) |
| `ce/FIX-304-REPORT.txt` | `D:/Arma_CWA/tmp/ce-304-verified/FIX-304-REPORT.txt` | CWR-CE aa52258 | 2026-10-08 | Verification report for the #304 fix |
| `ce/ce-latest.patch, ce/ce-latest-untracked.patch, ce/ce-latest-base.txt` | `D:/Arma_CWA/tmp/ce-latest` | CWR-CE aa52258 | 2026-10 | Uncommitted OptionsUI change plus untracked menu-return tests and fixtures |
| `ce/community-audit-rendering-report.md` | `D:/Arma_CWA/tmp/community-audit/rendering-report.md` | none | 2026-10-07 | Community rendering audit report |
| `ce/cwrce/` | `D:/Arma_CWA_worktrees/cwrce` | CWR-CE branch fix-4wvr-empty-islands | 2026-10 | 2 local-only commits as git format-patch (4WVR empty-island fix); no uncommitted changes |

## stash/

| File | Source | Base commit | Date | Description |
|---|---|---|---|---|
| `stash/stash-0-97dc5ad3.patch` | `D:/Arma_CWA` stash@{0} | 97dc5ad3 | 2026-10-18 | On graphics-remaster: A4 WIP (perf) moved to scratch gr-A4 |
| `stash/stash-1-40519b3c.patch` | `D:/Arma_CWA` stash@{1} | 40519b3c | 2026-10-11 | On main: leftovers identical to origin/main (2026-10-11 sync) |

`stash/README.txt` lists each message and SHA. Patches are `git stash show -p --include-untracked`.

## misc/

| Folder | Source path | Base commit | Date | Description |
|---|---|---|---|---|
| `misc/issue-42/` (1 files, 44 KB) | `git format-patch -1 158bb350 (remote branch origin/claude/issue-42-solution-plan-bx4akl)` | 158bb350 |  | Unmerged issue #42 solution plan for unassociated assailants |
| `misc/local-dossier-portraits-arma_CWR-tmp/` (217 files, 833 KB) | `D:/Arma_CWA/tmp/local-dossier-portraits/arma_CWR/tmp` | codex/local-dossier-portraits @ f17e3d4a | 2026-09-17 | Portrait feature test sources, Linux build scripts, install-test mission copies |
| `misc/root-arma_CWR-tmp/` (133 files, 221 KB) | `D:/Arma_CWA/arma_CWR/tmp` | n/a | 2026-10-15 | Root checkout scratch: render-diagnosis repair patches, acceptance results, probe tests, helper scripts |
| `misc/narrative-rewrite/` (130 files, 465 KB) | `D:/Arma_CWA/tmp/narrative-rewrite` | n/a | 2026-10-08 | Legends narrative rewrite: briefs, research, drafts, cards, review data and assembly scripts |

Also checked and found to hold nothing to archive: the worktrees under `D:/Arma_CWA/.claude/worktrees/` (busy-spence-466165, clever-diffie-56b48c, exciting-vaughan-6b6e8c, lobo-repair-4, stoic-hamilton-7da418; `lobo-repair-3` is an empty folder) and the two Codex worktrees under `C:/Users/Dane/.codex/worktrees/` (lobo-content-repairs, lobo-runtime-acceptance). None has tracked changes; their ignored files are only regenerable roadmap output, caches, build output, logs and screenshots. All their HEAD commits are on origin/main.

## What was deliberately left out

- Renders, screenshots, contact sheets and every other image (png, bmp, jpg, gif), plus video and audio (mp4, mkv, wav, mp3).
- Build output and binaries: build, dist and vcpkg trees, obj, exe, dll, pdb, cargo targets, and the scratchpad build and dist copies.
- Game data and third-party mod data: pbo, paa, pac, p3d, wrp, ogg and friends; game install shadows and junctions (never followed).
- De-binarized Bohemia and LoBo config dumps in any form (cpp, hpp, txt, and JSON renderings of configs), stringtables, extracted LoBo scripts and readmes, LoBo model and pbo inventories, and third-party file listings or scraped indexes.
- Save-game dumps (`version=` class dumps and `.fps` JSON renderings) and object lists dumped from islands.
- Logs and console or test-output captures: `*.log`, console, status, unit and tri outputs, UTF-16 PowerShell captures, raw per-frame `timings.json`/`timings.csv` from the capture harness, and director-video `trace`/`stats` JSON.
- Files whose header says "Generated, do not edit" (regenerable capture tests), files over 5 MB, and secrets or `.env` files.
- Two copies of the repo source tree under the scratchpad `ll/applycheck` and `ll/applycheck06` (empty git repos holding a copied `arma_CWR/` used to test patch application; the source is in git history and the patches are archived elsewhere).
- Files of other extensions (glsl shader dumps, sqm missions, cfg, tmpl and similar) unless they sit inside a patch.


## Addendum: lanes/look-surface/external-E-look-surface/

Text from `E:/look-lanes/look-surface`, the look-surface lane's external work folder (notes, round patches, tool and timing scripts, capture summaries), collected just before that folder was deleted. Build and dist output, images and logs were left out.
