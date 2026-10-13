# Full-scale test session (off-screen contract, issue #71)

A runbook for a long Guerrilla session on the **full official Remaster**,
once without mods (Malden) and once with `@LoBo` (Sinai), that checks the
rules in [OFFSCREEN-CONTRACT.md](OFFSCREEN-CONTRACT.md) and the live-entity
and frame budgets. Part A is the human session. Part B is the automated
version and its recorded results. The session feeds the #31 Gate-Zero
evidence; it does not replace the 43-case suite in
[HUMAN-TESTS.md](HUMAN-TESTS.md).

## A. Human session

### A1. Prepare the data (once per machine)

Never install into the Steam folder when you only want to test: build a
shadow copy and install into that. From `arma_CWR`, in PowerShell 7:

```powershell
$R = 'D:/SteamLibrary/steamapps/common/ARMA Cold War Assault/Remastered'
$S = 'D:/Arma_CWA/tmp/remaster-shadow-67'
pwsh -File tools/runtime/new-remaster-shadow.ps1 -Source $R -Destination $S `
     -LoBoSource "$R/@LoBo"
pwsh -File ./setup-guerrilla.ps1 -GameDir $S -LoBoDir "$S/@LoBo" `
     -Tools ./dist/x64-win-rwdi/PoseidonTools.exe
cmd /c mklink /J packages\RemasterUD D:\Arma_CWA\tmp\remaster-shadow-67
```

Setup must print `Validated: official Remaster data` (Remaster mode, issue
#67): Bohemia's files stay byte-identical, UD adds `gmcore`, the missions, the
faction library, the fonts and the UD menu. Rerun setup after pulling changes
to `guerrilla-mode/core` or the templates. The `packages\RemasterUD` junction
is what the automated lane uses; a human session can use the shadow path
directly. (A player who wants UD on the Steam install itself follows
[FRESH-INSTALL.md](FRESH-INSTALL.md) instead.)

### A2. Build and launch

```powershell
cmake --build build/win-x64-clang-rwdi --target PoseidonGame PoseidonTools
# S1: Malden, no mods
pwsh -File ./play-guerrilla.ps1 -GameDir $S -GameExe ./dist/x64-win-rwdi/PoseidonGame.exe
# S2: Sinai with @LoBo
pwsh -File ./play-guerrilla.ps1 -GameDir $S -LoBoDir "$S/@LoBo" -GameExe ./dist/x64-win-rwdi/PoseidonGame.exe
```

Main menu, GUERRILLA, pick the island (S2: Sinai, defaults IDF occupier and
EgyptFrontier resistance), START. Run at 1920x1080 with the default view
distance. `play-guerrilla.ps1` writes the engine log, stdout and stderr to
`logs\game-<ts>.log`, `.stdout.log` and `.stderr.log` (crash traces land in
stderr) and prints their paths and the PID; `-NoLog` turns that off. Add
`-Dev` for the dev panel (Ctrl+`) with its Console and Profile tabs. Stop
the PID you started when you are done.

Record at the top of your notes: the build commit, the data dir, the mods,
the view distance, the resolution and the hardware.

### A3. What to do and what to observe

Each session runs at least 2 hours of game time. The dev panel's time
multiplier (x4) is allowed only in step 4.

| Step | Do | Pass when |
|---|---|---|
| 1. Boot | Start the campaign. | No script error, no crash, a `[gmsim]` line in the log within a minute. |
| 2. Hub scene | Stand in the starting town with a garrison inside 800 m, civilians and traffic around. After 60 s run `gmSimDiag` in the Console and read the Profile tab. | Live bodies <= 100 (130 hard cap), groups per side <= 40, frame p95 <= 25 ms, the `gm` row avg <= 0.5 ms. |
| 3. Leave troops (scenario a) | Clear a post, order 2 or 3 recruits to hold it (Stop), walk or drive more than 1 km away. Then do the same in a town with one fighter. | The journal's Places page shows capture rising and then the zone liberated while you are away; "Last seen" does not move; the town's support climbs from afar and the town rises. A post that still has its stored garrison stays contested and does not move. |
| 4. Wait in an untouched region (scenario b) | Drive to an empty part of the island and wait 20 minutes (x4 allowed). If you bled a post earlier and left it calm, note its garrison (`gmZone <i> select 3`) before and after. | Heat cools, income hints arrive, nothing else changes owner or support, the bled post regains 1 soldier per 10 minutes and never exceeds its authored strength. |
| 5. Convoy (scenario c) | Ambush a convoy (Sinai has natural ones; on Malden use `gmTrafficForceSpawn ["convoy", <outpost>]` near the Outpost), leave for 3 minutes, come back. | The zone went RED and back to GREEN, the QRF stood down, no second convoy or stale QRF appears, the post's garrison is the same size. |
| 6. Save and reload (scenario d) | Save next to a live garrison you have bled, quit the process, relaunch, load; repeat with a save made far away; walk back in each time. | The same number of soldiers every time: no healing, no duplicates. |
| 7. Transit | Drive between two regions for 10 minutes. | `[gmsim]` lines keep coming; no budget WARN; frame time stays within budget. |

### A4. Reading the output

```powershell
Select-String '\[gmsim\]|GarrisonCache|garrison at|Traffic:|Error in expression|WARN' logs\game-<ts>.log
```

A `[gmsim]` line reads:

```
[gmsim] zones bubble=3 remote=1 live occ=16 res=5 civ=18 total=39
        groups occ=4 res=2 civ=6 maxSide=6 garrison zones=1 bodies=6
        traffic civ=2 parked=0 patrol=1 convoy=0 gm=0.01/0.01/0.12ms
        frame=7.13/8.25/10.24ms
```

`bubble` zones are within 800 m; `remote` is how many zones the last zone tick
advanced from outside the bubble (D1); `total` is every live AI body on every
side, player included; `maxSide` is the largest group count on one side;
`gm` and `frame` are avg/p95/max in ms over the last 256 frames. A budget
breach adds a WARN line starting `[gmsim] budget:`. D2 recoveries log
`Guerrilla: garrison at <zone> recovered 1 (reserve n of m)`.

### A5. Pass and fail

**Pass:** no script error and no crash (the `.stderr.log` holds no crash
trace); every observation in A3 holds; no `[gmsim] budget:` WARN; hub frame p95
within budget; every reload restores the counts of step 6.

**Fail:** a duplicated or healed garrison (except D2 steps), a zone flipping
without resistance presence, a distant town losing support with no ledger
cause, a budget breach, or a crash.

Record the session in [HUMAN-TEST-RUN.md](HUMAN-TEST-RUN.md) (rows OFF-a to
OFF-d plus the budget table) and keep the saves and screenshots.

## B. Automated session and results

### B1. How to run it

```powershell
tri test --game-dir dist/x64-win-rwdi --retries 2 -j1 `
  tests/integration/scripting/guerrilla_offscreen_capture.test.sqf `
  tests/integration/scripting/guerrilla_offscreen_idle.test.sqf `
  tests/integration/scripting/guerrilla_offscreen_convoy.test.sqf `
  tests/integration/scripting/guerrilla_offscreen_save_reload.seq `
  tests/integration/ui/remaster/remaster_ud_offscreen_budget_abel.test.sqf `
  tests/integration/ui/remaster/remaster_ud_offscreen_budget_sinai_lobo.test.sqf
```

(or `--tag offscreen`). The four scenario tests use the fixture mission
`tests/integration/missions/guerrilla_offscreen.abel`. The two budget soaks
launch the shipped `Guerrilla.Abel` and `Guerrilla.Sinai` templates through
the GUERRILLA menu with their stock tuning (D1 on, D2 at 600 s), visit the
start position, every non-town zone and the first four towns, let each stop
settle for 400 frames, then log one `triPerfStats` and one `[gmsim]` line.
They assert at most 130 live bodies and 40 groups per side at every stop.
Each soak passes `--focus`: a menu-launched session pauses when another window
takes focus, which froze an earlier soak on a shared desktop. The engine log
of a run is `tmp/tri/<id>/game_stdout.log`.

### B2. Results, 2026-10-11

Build: commits `ea9754c` (engine) and `e740e79` (tests), `build/agent71-rwdi`
RelWithDebInfo. Data: the Remaster shadow `D:\Arma_CWA\tmp\remaster-shadow-67`
prepared by setup in Remaster mode; S2 mounts the shadow's repaired `@LoBo`.
Hardware: Intel i7-14700KF, NVIDIA RTX 4060 Ti, 96 GB. The Trident window is
800x600, not the 1080p of the human hub budget, so frame numbers are a lower
bound for the human session, not a substitute.

| Test | Result | Wall time |
|---|---|---|
| `guerrilla_offscreen_capture` (a) | pass | 104 s |
| `guerrilla_offscreen_idle` (b) | pass | 82 s |
| `guerrilla_offscreen_convoy` (c) | pass | 68 s |
| `guerrilla_offscreen_save_reload.seq` (d, 3 processes) | pass | 38 s |
| `remaster_ud_offscreen_budget_abel` | pass | 105 s |
| `remaster_ud_offscreen_budget_sinai_lobo` | pass | 109 s |

Budget numbers from the soaks (worst value over all stops):

| Metric | Budget | Malden, no mods | Sinai + @LoBo |
|---|---|---|---|
| Live AI bodies (all sides) | <= 100 typical, 130 cap | 45 | 61 |
| Groups on one side | <= 40 | 11 (civilian) | 16 (civilian) |
| Frame avg / p95 | p95 <= 25 ms | 8.7 / 13.6 ms | 9.4 / 11.9 ms |
| `gm` phase avg / p95 | 0.5 / 1.5 ms | 0.03 / 0.01 ms | 0.30 / 0.01 ms |
| `gm` phase worst single frame (logged, not gated) | | 5.9 ms | 75.8 ms |
| `[gmsim] budget:` WARN | none | none | none |

Readings:

- Every stop on both islands is far inside the live-entity budget; civilians
  are the largest group consumer (Sinai's 26 dealers plus town civilians).
- The `gm` phase is negligible on average. Its worst frames are spawn bursts
  (a garrison, patrol or convoy materializing in one frame): 75.8 ms once on
  Sinai, at the stop where the player arrived at an occupier post and its
  garrison and a patrol materialized. The
  contract logs these and does not gate them; spreading spawns over frames is
  a candidate follow-up if the human session feels the hitch.
- The `remote` count stayed 0 in the soaks because no resistance troops were
  left in the field there; scenario (a) is what exercises it.

Still to do by a human (Part A): the 1080p hub-scene frame budget and the
2-hour S1/S2 sessions.
