# Memory and instruction-file audit, 2026-10-18

Scope: all 71 memory files under `C:\Users\Dane\.claude\projects\D--Arma-CWA\memory\`, the index `MEMORY.md`, root `CLAUDE.md`, `arma_CWR/CLAUDE.md`, `AGENTS.md`, and `agent_guidelines/*.md`, checked against `main` (16259cc), `occupier-forces` (bd831e7), `graphics-remaster` (fec1075), `remaster-only-runtime` (2a6dc6f), `claude/youthful-lovelace-8imbdh` (lighting) and `claude/remaster-water-research`. Five Opus reviewers each took a file group and verified every claim with git, grep, ls or `gh`. The ten highest-impact claims were re-run by the orchestrator. Nothing was edited.

Verdict key: CURRENT (keep), STALE (facts moved on), WRONG (would mislead a session into a bad action), SUPERSEDED (another file now owns the topic), REDUNDANT (CLAUDE.md or another memory already says it).

## 1. Items that would actively mislead a session (fix first)

| # | File | What is wrong | What is true now |
|---|------|---------------|------------------|
| 1 | `upstream-sync-subtree-merge.md` | Recipe is `git merge -Xsubtree=arma_CWR main` and `git show main:<path>`. Local `main` is now the fork's dev branch, so this merges main into itself. | Use `git fetch upstream && git merge -Xsubtree=arma_CWR upstream/main` from main; `git show upstream/main:<path>`. upstream/main (3.05, ffc6183) is already an ancestor of main. |
| 2 | `arma_CWR/CLAUDE.md` on main, graphics-remaster, lighting, water-research (line 167) | Prescribes `Get-Process PoseidonGame \| Stop-Process -Force`, the global kill that `engine-run-coordination.md` and `agent_guidelines/scoped-game-cleanup.md` forbid. | The scoped `tools/stop-owned-games.ps1` and `tri --global-slots` exist only on occupier-forces and remaster-only-runtime (fd510d1, 898df3e). main has no `stop-owned-games.ps1`. The graphics branches are the ones running capture jobs. |
| 3 | `trident-full-data-setup.md` | Says no `packages/` or Demo dataset exists; quotes `.trident.env` as `'D:\Arma_CWA\ARMA Cold War Assault'` (missing `[Classic]`); describes hand-installed font/remaster shims; LoBo mount `--mods-dir D:\Arma_CWA`; treats Classic as the primary lane. | `arma_CWR/packages/{Demo,Game,RemasterUD}` junctions exist; `.trident.env` is `'...[Classic]'`; shims are tracked under `guerrilla-mode/runtime/classic` and `thirdparty/guerrilla-fonts`; mod-lane tomls use `--mods-dir "."` since e6bff81 (occupier-forces/remaster-only-runtime); Remaster shadow via `packages/RemasterUD` is the target. |
| 4 | `arma_CWR/CLAUDE.md` on occupier-forces | Says `full_cwa`/`lobo` lanes run on Classic via `.trident.env`. | Since e6bff81, 30 unpinned tests mount `<data dir>\@LoBo`. `[Classic]\@LoBo` does not exist, `tmp/remaster-shadow-67/@LoBo` does. `FRESH-INSTALL.md:203-205` on the same branch says Classic no longer satisfies them. Also: the LoBo precondition tells agents to repair `D:\Arma_CWA\@LoBo`, but tests mount the shadow's own copy, which `setup-guerrilla.ps1` already repairs. |
| 5 | `guerrilla-parity-tests-and-sqs-gotchas.md` | "Fix core scripts in the CANONICAL copy, then re-copy to all five mission dirs"; says #4/#5 engine bugs are "NOT fixed", gives the `format ["%1", bool]` workaround. | Single `guerrilla-mode/core` installed as `\gmcore` since 9d006ca; `test_mission_script_core.cpp` asserts no mission carries `scripts/`. #4 and #5 fixed in 193a280 (2026-07-03). Also no frontmatter. (roadmap.yaml line 1413 repeats the stale re-copy wording.) |
| 6 | `phase15-mission-migration-landed.md` | Same five-copies model; `@lobofixup` gen-patched-pbos described as required. | Single core; 1970c75 says the patched pbos are optional, `mod doctor --fix` repairs @LoBo itself. |
| 7 | `rmb-zoom-ads-tap-bindings.md` | "Landed on branch `controls/rmb-zoom-ads`, not pushed"; v3->v4 migration, "v5 profile treated as unknown". | Branch gone; 04e977c + 3ddca19 on main (PR #63). `kContextControlsVersion = 5` since ca9bd9d. World.hpp clang-format caveat still worth keeping. |
| 8 | `remaster-4wvr-object-load-bug.md` | "Draft PR #98 to CWR-CE"; "our engine likely shares the bug; Guerrilla-on-Sinai runs probably had no buildings"; worktree `mod_compat`. | PR ofpisnotdead-com/CWR-CE#98 MERGED 2026-07-22. af5b3bb fixed it on main. `mod_compat` worktree is gone. |
| 9 | `lebanon-playtest-uncommitted.md` (and its index line) | Installer "deletes the template unless `-IncludeWorld Sinai,Lebanon80`"; "@lobofixup lacks Hizballah". | Both fixed in 7694bf2 / 8dbbf4c (2026-09-02): `install-missions.ps1` reads pbo entry tables; `lobo-factions.hpp` lists Hizballah. Keep only: Lebanon80 landed 4ff55be, Names is a theater map so `seedCities = 0`. |
| 10 | `vehicle-controls-no-mouse-steer.md` | Options strings live only in gitignored `[Classic]/bin/STRINGTABLE_OPTIONS.utf8.csv`, "must be re-added after reinstall". | Tracked at `arma_CWR/thirdparty/classic-ui/STRINGTABLE_OPTIONS.utf8.csv` since de36aab; Remaster installs get `STRINGTABLE_UD.utf8.csv`. |
| 11 | `journal-diary-restyle-and-shadow-datadir.md` | Presents `D:\Arma_CWA_worktrees\classic-shadow` + a `.trident.env` pointing at it as the isolation recipe. | Per-agent Remaster shadows (`tmp/remaster-shadow-67`, `-b1..b4`, `-of1`) and `packages/RemasterUD` are the standard. |
| 12 | `dossier-change2-landed.md` | "`lobo-factions.hpp` authors no displayName, so LoBo factions reach prose as class names". Quotes prose constructions that no longer exist. | Every class authors a displayName since dbb7654. Prose rewritten in 7b496c3 and dbb7654; third rewrite designed in `guerrilla-mode/LEGENDS_PROSE_REWRITE.md`. |
| 13 | `engine-crash-issues-4-5-6-diagnosis.md` | ".Demo integration tests can't boot on this machine (no Demo dataset)". | `packages/Demo` junction since 2026-09-02; 76 tomls pin it. Delete that paragraph; rest is fine. |
| 14 | `remastered-lighting-rtx-vision.md` | "Keep all work on branch `claude/youthful-lovelace-8imbdh`". | `graphics-remaster` (fec1075) contains the whole lighting branch and is 21 commits ahead; it is the integration branch. Issue #215 closed. Neither lighting memory mentions `origin/graphics/lighting-improvements` (c03f12d, PR #210, branched straight off main, shares nothing with graphics-remaster). |

## 2. Memory policy contradictions

- **Agent-model policy is split across four files.** `feedback-careful-agent-usage-sonnet.md` says "omit the model override (session model is Opus) or set opus"; `feedback-opus-for-reading-agents.md` says "Always set `model: 'opus'` explicitly". The Sonnet filename and the index title ("Agent model default: Opus") say opposite things. The Sonnet file also lacks a `metadata` block. Fold the one history line (2026-10-11 morning Sonnet request, reverted 13:15) into the Opus file, delete the Sonnet file, and consider renaming the Opus file since it covers all agents, not just readers. `feedback-always-use-agents.md` and `feedback-agent-delegation-and-handoff.md` can merge into the same single agent-policy memory.
- **Workflow memories disagree.** `workflow-usage-limit-resume.md` says "Resume, don't rewrite"; the newer `workflow-resume-relayed-user-message.md` says a resume in a later turn re-runs everything, so launch a small new workflow. Reword the older bullet to "resume only in the same turn with unchanged prompts". `feedback-no-sendmessage-to-workflow-agents.md` repeats the same advice in its last paragraph.
- **Traffic memories disagree.** `traffic-modulation-landed.md` says day-window floats are unvalidated; `traffic-complaints-book-landed.md` says values > 1 are read as hours (true, `Traffic.cpp:277-281`, bfaf236).
- **`feedback-all-progress-on-github.md`** says "always work on main". Current work runs on user-authorized long-lived branches (occupier-forces is 96 commits ahead with PR #95 open; graphics-remaster 45 ahead). Add one sentence: authorized branches land through a PR to main; main is the destination, not the only place work happens.

## 3. Stale "still open / uncommitted / not pushed" claims

| File | Claim | Reality |
|------|-------|---------|
| `mouselook-directlook-decision.md` | fix "applied, uncommitted as of 2026-07-03" | committed 5a289cd (2026-07-06) |
| `guerrilla-hq-market-landed.md` | "NOT pushed yet" | 8a9a877, fe95a9f on main |
| `guerrilla-journal-and-pch-gotcha.md` | title says "uncommitted at hand-off"; page list Notes/Plan/GM_LOG | landed b595283; page map superseded by dossier-change1. No frontmatter. |
| `deep-undercover-system-landed.md` | fragility issues #19-#23 all open; site undercover page "deliberately left uncommitted"; "on guerrilla-mode" | #19, #22 CLOSED (#20, #21, #23 open); `site/undercover.html` + `uc-*.jpg` tracked (a619b68); branch gone, commit on main |
| `character-outfit-select-landed.md` | "Deferred (still on the open issue)" #25; Lebanon outfit keys in "UNCOMMITTED dir" | #25 CLOSED 2026-08-05; keys landed 4ff55be |
| `ambient-traffic-landed.md` | "#33 worldLocations half stays open"; "patrol traffic feeds no AlertMachine"; link `[[civilian-ambience]]` | #33 CLOSED 2026-10-11; a2d16fc feeds AlertMachine; link target does not exist |
| `traffic-complaints-book-landed.md` | panic-priority "planned"; two Trident tests "never run" | `traffic-panic-priority` done (e5f9ba5, PR #59); `guerrilla_traffic_danger` ran green under #66; convoy test still unverified |
| `dossier-change1/2/3-4` | work "on `claude/journal-feature-s31cuu` (PR #61)" | PR #61 MERGED 2026-09-22, all hashes on main. Worktree `D:\Arma_CWA_worktrees\journal-61` is a dead leftover at 113e68d. |
| `roadmap-system.md` | "audit facts as of 2026-07-06": #1 and #7 open | #1 closed 2026-07-08, #7 closed 2026-10-11 |
| `mod-plans-are-ai-drafts.md` | ARCHITECTURE.md calls itself a "binding spec"; `setVariable` absent; `UIMap.cpp:999` | phrase removed in be55dd8; `setVariable` at GameStateExt.cpp:1261; line is now 1004 |
| `cwa-overhaul-direction.md` | to-do: add setVariable/isNil/nearestObjects; sequencing via Demo | all three landed; sequencing is finished history; Demo framing conflicts with current policy |
| `mod-intake-54-session-gotchas.md` | End-state: rosters "ONLY in `@lobofixup/bin/config.cpp`"; "first GitHub run of ci.yml (branch never pushed)" | rosters in `guerrilla-mode/config/lobo-factions.hpp` + `install-lobo-factions.ps1`; CI green since 2026-09-03; #54/#56 closed; `CMakeLists.txt:~168` is now `DIST_PRESET_NAME` at 170-194 |
| `steam-full-remastered-package.md` | "repo docs do not mention" the path; "candidate primary OFPR_DATA_DIR"; resource-extra override worry; 37 GB log | `arma_CWR/CLAUDE.md:63` names it as the target; rule is never point tests at it, use the shadow; solved f9e7c99 (RscDisplayMainUD); log now 7.2 GB |
| `full-retail-remaster-install.md` | asset policy "needs revisiting"; "textureBlack backfilling is safe" | policy already updated; a Classic shim backfill is now out of scope. Duplicates the Steam memory (35 vs 37 GB disagreement). |
| `codex-era-work-sept-oct-2026.md` | #65 "bears on" the data-dir decision | decision taken 2026-10-11 (Remaster target); #65 still open on GitHub |
| `occupier-forces-branch-and-orchestration.md` | stops at gate 25 / 37d1c6b; "native_capture needed a retry"; first "State 2026-10-17" paragraph and "owner call: ground launch below floor" | tip is bd831e7 (retires the boot companion, 25/25 at `--retries 0`); round 6 reserve floor a2810b9 settled the owner call. Index line points at the Oct 13 prompt; live brief is `OCCUPIER-FORCES-NEXT.md`, `tmp/occupier-forces-handoff-2.md`, `tmp/of-agent-common-rules.md`. |
| `remastered-lighting-branch-setup.md` | "RESUME IT at once" and "never resume the lane run again"; VO is Breeze TTS and also shimmer/OpenRouter | run log, self-contradictory; video not done (`tmp/video` has only VOICE.txt + samples); 54 detached scratch worktrees (gfx-* 21, gr-* 17, fix-* 11, fix2-* 2, look-* 3) with no cleanup note; `stash@{0}` "A4 WIP" still exists |
| `screenshots-branch-maintained.md` | `clever-diffie-56b48c` worktree "detached on upstream 3.03 tag" | now on `claude/agitated-chandrasekhar-35b24d` at dcde1ac. Filename contradicts content (branch folded into main). |
| `feedback-no-em-dashes.md` | `site/` files "use CRLF"; link `[[site-portal-overview]]` | all 8 site/*.html are LF; link target does not exist |
| `feedback-legends-prose-lived-in-world.md` | (missing update) | dbb7654 rewrote Legends names/bios after the memory was written; user verdict on it not recorded; #57 open |
| `workflow-resume-relayed-user-message.md` | "Found 2026-10-19" | UTC date; local date 2026-10-18 |
| `feedback-timer-means-wait.md` | "one-shot CronCreate job" | this session's scheduler is `scheduled-tasks` with `fireAt`; CronCreate is deferred here (unverified which is right across session types) |
| `repo-rerooted-at-arma-cwa.md` | `.gitignore` excludes `/ARMA Cold War Assault/`; active branch `guerrilla-mode` | entry is `/ARMA Cold War Assault \[Classic\]/`; branch gone |
| `phase15-engine-rewrite-landed.md`, `vehicle-controls`, `engine-crash`, `upstream-sync` | name branch `guerrilla-mode` | branch no longer exists locally or on origin; work is on main |

## 4. Redundant with CLAUDE.md (delete or shrink)

- `project-name-uslu-dur.md`, `remaster-is-target.md`, `repo-rerooted-at-arma-cwa.md`, `mod-plans-are-ai-drafts.md`, `roadmap-system.md`, `cwa-overhaul-direction.md`: root CLAUDE.md already says each. Keep at most the one-line "why" (CWR name collision; upstream locked; cite engine source or Dane, not plan docs; pointer list of Classic-era memories plus the `remaster-only-runtime` escalation).
- `feedback-dont-restore-unexplained-deletions.md`: CLAUDE.md Conventions has the rule; keep only the incident and "ask the user if still ambiguous". Says "This repo (`arma_CWR`)".
- `agent-guidelines-folder.md`: CLAUDE.md describes the folder; keep only the flashing-menu / `addOns[]` tip.
- `cwr-unit-test-portability-fixes.md`: FormatTime fix landed 0e77283; `arma_CWR/CLAUDE.md:235` has a newer breakdown and issue #90. Shrink to one line.
- `cwr-build-env.md`: keep only the PATH snippet (the one thing CLAUDE.md lacks); replace the game-data sentence (names the pre-rename `ARMA Cold War Assault` folder).
- `lobo-mod-test-bed.md`: SUPERSEDED by `lobo-family-package.md` and the root CLAUDE.md @LoBo section; wrong game-dir path; cites #3 (closed 2026-07-03). Delete or fold one line.
- `poseidon-startup-stall-diagnosis.md`: duplicates roadmap item `startup-stall-diagnosis` (#30 open). Keep the ranked hypotheses or a pointer.
- `menu-e2e-and-concurrent-kill-gotchas.md`: canary-wait bullet replaced by `stop-owned-games.ps1` / `tri --global-slots` (occupier-forces only). Keep triEndTest, sub-second-pass and probe bullets.
- `issue-56-wrapup-gotchas.md`: junction bullet duplicates `arma_CWR/CLAUDE.md:213-215`; count "57 tomls" is now 76, and the RemasterUD lane (92 tomls) is missing.
- tri `.test.sqf` vs `.toml` gotcha appears in `guerrilla-hq-market-landed.md`, `traffic-threat-choice-66-landed.md` and the screenshots index line. Keep once, ideally as an `agent_guidelines` note.
- triAssert/`player fire` crash facts appear in `guerrilla-parity-tests` (stale) and `engine-crash-issues-4-5-6` (correct).

## 5. Merge proposals

- **Traffic, 6 to 1:** `ambient-traffic`, `traffic-modulation`, `traffic-park-lifecycle`, `traffic-perception-gate`, `traffic-complaints-book`, `traffic-threat-choice-66`. Keep: the doMove/IssueCommand idiom and NativeMoveIn; headlights and probe/spawn PickRoute coupling; GetIn re-board and steal-watch revoke; perception semantics and the ambient-test staging trap; dead units stay group members / FreeSpentGroup and the Disclose rule; TrafficEncounter seams; gmTraffic* probe list; park-test ValidateOutVehicle red; pointer to `agent_guidelines/seated-man-deletion.md` (occupier-forces) for the seated-crew leak. Drop landed-hash narrative (roadmap holds it) and resolved gaps.
- **Journal/dossier, 5 to 1:** `guerrilla-journal-and-pch-gotcha`, `journal-diary-restyle-and-shadow-datadir`, `dossier-change1`, `dossier-change2`, `dossier-changes-3-4`. Keep page map, Legend contracts, GM_HIST_EV naming, waypoint-slot-0 and guards-crewed bugs, pre-gamma capture, notepad/cwrpen font facts, `Log` macro, ccache PCH fix, gate recipe, the 5 `[ui]` reds needing `packages/Remaster`. Mark journal-61 worktree dead. Point at `LEGENDS_PROSE_REWRITE.md`.
- **Steam Remaster, 2 to 1:** `steam-full-remastered-package` + `full-retail-remaster-install`. Keep facts, "never point tests at the Steam folder, use the shadow", the open damage/glass item (#11, #13 open; roadmap line 2106).
- **Agent policy, 4 to 1:** see section 2.
- **LoBo, 2 to 1:** fold `lobo-mod-test-bed` into `lobo-family-package`.
- **Lighting, 2 to 1:** rewrite as a current-state note (graphics-remaster tip and its relation to the lighting branch and to `graphics/lighting-improvements`; open items: video + final check `wf_6190b6a1-1f1`, overhaul `wf_311066ec-3bc` awaiting the owner's three-way pick; VO = Breeze TTS; scratch-worktree cleanup status).
- **Occupier forces:** rewrite `occupier-forces-branch-and-orchestration` as one current-state block (tip bd831e7, gate 25 green, native_capture flake fixed, stop rule met; open: owner calls, #98, #99, Malden playtest; PR #95 open, PR #213 stacked). Mark `live-events-decide-outcomes`, `engine-run-coordination` and `issue-87` as "behaviour/tooling on occupier-forces until #95 merges". `issue-79-blockers-landed`: add that the counter-attack resolution model changes on occupier-forces and the `.trident.env` default fix is pending on remaster-only-runtime.

## 6. Index (`MEMORY.md`)

- `feedback-interview-not-knob-pages.md` has no index line and nothing links to it. Content is current (2026-10-13 feedback: interview in chat, no knob pages).
- 24 index lines use ` — ` as separator, against the no-em-dash rule; the rest use ` - ` or `: `.
- Index lines repeating stale claims: lebanon-playtest (IncludeWorld, Hizballah), rmb-zoom (branch name), occupier-forces (Oct 13 prompt as "the brief"), screenshots (title), careful-agent-usage-sonnet (title vs filename).
- Dead wikilinks: `[[civilian-ambience]]` (ambient-traffic), `[[site-portal-overview]]` (no-em-dashes).
- Files without frontmatter: `guerrilla-journal-and-pch-gotcha.md`, `guerrilla-parity-tests-and-sqs-gotchas.md`; `feedback-careful-agent-usage-sonnet.md` lacks `metadata`.

## 7. Checked-in instruction files

### Root `CLAUDE.md` (identical on all six branches)
- Every path it names exists.
- The @LoBo bullet names `fix-lobo-scope.ps1` / `gen-patched-pbos.ps1` as the tracked repair artifacts. `fix-lobo-scope.ps1` is a 58-line wrapper around `mod doctor`; the repair that runs is `setup-guerrilla.ps1` calling `install-lobo-factions.ps1` (line 341) then `mod doctor --fix` (line 347), plus the LoBo-Reloaded packaging path.
- No em-dash rule (9 em dashes here, 29 in arma_CWR/CLAUDE.md); agents copy the style.
- The repo `.ignore` comment still calls @LoBo "read-only reference".

### `arma_CWR/CLAUDE.md`
Passed: toolchain versions, presets, C++17 overrides, warning suppressions, DIST naming, ci.yml matrix, UDDataDir resolution order, launcher parameters, TridentCTest default, global-slot env vars and ledger path, unit suite names, Cargo members, junction targets. Issues #88, #90, #91 still open as stated. Problems:
1. Line 167 global kill on main and the three graphics branches (section 1, item 2).
2. `lobo`/`full_cwa` lanes "run on Classic" is false on occupier-forces (section 1, item 4). LoBo preconditions repair the wrong @LoBo copy.
3. Mods-dir description omits tri's `${repo_root}` token (23 tomls use it, 13 still use `../../tests/fixtures`; documented at `engine/Trident/src/scenarios/integration.rs:19`).
4. Pin counts stale on occupier-forces: Demo 76 (31+45) not 78; RemasterUD 92 not 25, with 59 tagged `occupierforces`. Correct on main.
5. Working map stale on occupier-forces: `Game/Guerrilla/` has 99 files not ~76; OccupierForces, OccupierAir, OccupierForcesLedger, OccupierForcesLevers, ForcePackages, DirectorLog, RoadRoute, BaseDetect/Generate/Props, CounterAttackAir/March/Orders not listed; `OccupierAirCommands.cpp`, `OccupierForcesCommands.cpp` missing from the Commands list; `guerrilla-mode/OCCUPIER-FORCES.md` (on main since f9aa158) missing from the design-doc list. `CounterAttack.cpp` is 5,601 lines, over the FileSize target's 5,000-line error limit (`cmake/CheckFileSize.cmake:21`); Traffic.cpp 4,683; OccupierForces.cpp 4,675.
6. "`Graphics/` effectively frozen" is false on graphics-remaster (34 Graphics / 30 GL33 commits, 121 files), lighting (13/12) and water-research (6/7). All three carry main's unchanged CLAUDE.md telling their agents not to touch the code they are rewriting.
7. Example `tri test -j6 --retries 2` conflicts with "low -j" and the default slot cap of 4; `--retries` already defaults to 3 (`main.rs:89-91`).
8. Non-preset build dir list lacks `build/lean-rwdi`, `build/options-rwdi`.
9. Linked overview `guerrilla-mode/README.md:274-282` still says "Acquire the (free) Demo game data" and calls Classic the default `OFPR_DATA_DIR`; line 314 says `install-missions.ps1` defaults to `[Classic]`. Real default is the Remaster shadow via UDDataDir.psm1.

### `AGENTS.md` (identical on all six branches)
- Accurate on main and occupier-forces. On remaster-only-runtime the root CLAUDE.md says "Classic 1.99 is unsupported... setup refuses Classic data" and the code throws (`setup-guerrilla.ps1:106`, `UDDataDir.psm1:20`), but AGENTS.md still says "being dropped, legacy awaiting retirement".
- Omits the never-kill-PoseidonGame-globally rule, the most damaging cross-session hazard (two codex worktrees also run on this machine).

### `agent_guidelines/`
- On main only `README.md` and `flashing-main-menu.md` exist. `scoped-game-cleanup.md`, `seated-man-deletion.md`, `tri-splits-lines-on-semicolons.md` and their index rows are on occupier-forces and remaster-only-runtime only.
- `README.md`: the tri-splits entry was appended as a bullet below the format notes, not as a table row.
- `tri-splits-lines-on-semicolons.md` breaks the README's rules: no "verified on" date, "around line 740" instead of `path:line`, no observed/inferred split. Mechanism is correct (`integration.rs:740`).
- `scoped-game-cleanup.md`, `seated-man-deletion.md`: accurate (one line drift, `WorldImpl.cpp:1684` is now 1685).
- `flashing-main-menu.md`: content true, but line citations pinned to a5daeb2 have drifted 10 to 170 lines (WorldImpl could-not-boot 2750 -> 2920; integration.rs wait_for_condition 857 -> 951; expected=46_or_47 1692 -> 1731; triWaitFrames 2151 -> 2161; WorldInit.cpp 833 -> 855; CleanUpInit 1140 -> 1149; ActivateTemplateAddons 566 -> 579; FrameFunctions.cpp 55 -> 80).

### Branch divergence of instruction files
- graphics-remaster, lighting, water-research: no divergence from main, no uncommitted edits. So they carry the global-kill rule, have no `stop-owned-games.ps1`, no `tri slots`, one guideline note.
- occupier-forces: 4 commits touch instruction files (fd510d1, 898df3e, 2524c3a, 2c23d60): scoped cleanup + per-agent shadow/build-dir rule + `tri slots` paragraph in arma_CWR/CLAUDE.md; three new guideline notes. Root CLAUDE.md and AGENTS.md unchanged.
- remaster-only-runtime: stacked on occupier-forces at 4b9e4b7, plus root CLAUDE.md "full Remaster only / Classic unsupported", arma_CWR/CLAUDE.md drops the Classic fallback, `.trident.env.example` becomes `packages/RemasterUD`. AGENTS.md not updated.
- No textual conflicts found. Risk is ordering: if a graphics branch lands first, main keeps the global-kill rule until #95 merges.
- Stray nested copies that Claude Code will load when a session reads files there: `tmp/local-dossier-portraits/CLAUDE.md` (codex worktree, f17e3d4, 2026-09-17) and `tmp/runtime-clean-clone/CLAUDE.md` (Sep 19 copy). Both predate the Target-platform section.

### Related (verified, outside instruction text)
- Steam `Remastered/@LoBo` has a `_ud-orig` folder, so `mod doctor` ran against the Steam copy on 2026-10-11, against the "never run setup against the Steam folder" rule. `new-remaster-shadow.ps1:24` and FRESH-INSTALL.md use `-LoBoSource '...Remastered/@LoBo'` as the example, while D:\LoBo-Reloaded is the declared LoBo source of truth.
- `.trident.env` on this machine still points at `[Classic]`; main's `.trident.env.example` says `packages/Demo`; remaster-only-runtime's says `packages/RemasterUD`. `Add-Island.ps1:97` still defaults to Classic on all three branches.

## 8. Open PRs on GearUnclear/CWR at audit time
#214 journal-humanizer-pass -> main; #213 remaster-only-runtime -> occupier-forces (stacked on #95); #210 graphics/lighting-improvements -> main; #95 occupier-forces -> main; #85, #84, #83 older fix/codex branches -> main.

## 9. Memories verified CURRENT with no changes needed
`feedback-agent-delegation-and-handoff`, `feedback-always-use-agents`, `feedback-no-ai-attribution-external`, `feedback-interview-not-knob-pages` (index only), `tri-hidden-desktop-wrapper` (add: local-only, tmp/ gitignored), `trident-staging-probe-techniques`, `lobo-family-package`, `issues-67-71-remaster-offscreen-landed`, `lean-tilt-rework-landed`, `traffic-park-lifecycle-landed`, `traffic-perception-gate-landed`, `traffic-threat-choice-66-landed`, `dossier-changes-3-4-landed`, `issue-87-checkpoint-audit-and-decisions` (GitHub retitled #87; add roadmap id `roadblock-checkpoint-inspections`), `live-events-decide-outcomes` (add branch scope), `mega-nc-mods-site` (ssh not re-verified; permission is per session), `codex-era-work-sept-oct-2026` (one sentence).
