# Flashing main menu during a `tri` mission run

Verified 2026-10-12 against engine commit `a5daeb2` (the engine is unchanged at
`0dbe248`), the Steam Remaster data dir (read-only) and the installed
`D:\Arma_CWA\@LoBo`. Paths are relative to the repo root, except the
`Repair4.Abel` probe files, which at the time lived only in the detached
worktree `.claude/worktrees/lobo-repair-4`. Line citations re-checked with
`grep -n` and refreshed on 2026-10-18 at commit `bd831e7` (branch
`occupier-forces`); the behaviour was not re-run.

## Symptom

A Trident test that boots a mission (`mission = ...` in its `.toml`) shows the
main menu instead of the mission. The window flickers between the menu and black
for roughly the test's `assert_timeout`. With `--retries N` Trident kills the
game, relaunches it and the flicker comes back, up to N more times.

## What the flicker means

Builds from commit `c39e49e` on no longer flicker: the run stops on the first
readiness poll and names the reason (see the follow-ups at the end). The rest
of this section describes older builds, and still explains why the boot failed.

The test mission never booted, and Trident is waiting for a player that does not
exist.

1. When the boot fails, `World::StartIntro` logs `StartAutoTest could not boot
   '<staged path>/mission.sqm'` and returns, leaving the main menu up
   (`arma_CWR/engine/Poseidon/World/WorldImpl.cpp:2920`). Only a
   `--check-init` smoke check exits at this point.
2. Trident then polls `triMissionPlayerReady` until `assert_timeout`
   (`arma_CWR/engine/Trident/src/scenarios/integration.rs:1641-1650`, loop in
   `wait_for_condition` at 951-1033). On the menu it returns
   `FAIL:display=<idd>,expected=46_or_47`
   (`arma_CWR/engine/Poseidon/Game/Commands/GameStateExtTest.cpp:1731`).
3. Between polls Trident evals `triWaitFrames 5`, with no sleep. That command
   clears the screen to black and presents five frames without drawing any UI
   (`GameStateExtTest.cpp:2161-2193`). The normal main loop draws the menu in
   between, so the window alternates black and menu about ten times a second.

Signature in the game stdout: a `could not boot` line followed by hundreds of
`[tri] triWaitFrames: pumped 5 frames` lines and no mission activity.

## Why a boot fails

- **The path does not resolve to a `mission.sqm`.** No flicker in the current
  build: the game exits at startup with code 44 and logs `Test mission path
  '...' does not resolve to a mission.sqm file`
  (`arma_CWR/apps/cwr/Game/GameApplication.cpp:1734-1740`).
- **Neither the mission nor its intro has any group.** A mission with zero
  groups falls back to its intro; when that is empty too, `StartAutoTest`
  returns false (`arma_CWR/engine/Poseidon/UI/DisplayUIMenus.cpp:2044-2049`)
  and the menu flickers.
- **The world suffix is not a world in the data dir.** The startup check only
  needs the folder and its `mission.sqm`, so this passes it and fails later
  inside the boot. Not reproduced; expect the flicker or worse.
- **The boot is rejected by the error latch.** This is the silent one.
  `World::InitVehicles` ends with `return GetMaxError() < EMError`
  (`arma_CWR/engine/Poseidon/World/WorldInit.cpp:855`). The latch is cleared by
  `CleanUpInit` (`WorldInit.cpp:1149`) when `StartAutoTest` calls
  `SwitchLandscape` (`DisplayUIMenus.cpp:2060`), before `ActivateAddons` and
  `InitVehicles`. Any `WarningMessage()` or `ErrorMessage(EMWarning/EMError)`
  after that point (world load, the mission's init, the ZoneRegistry, portraits,
  unit creation) sets it: `WarningMessageLevel` stores `level + 1`, so a plain
  warning already counts as `EMError`, and its text is kept in `WarningText`
  (`arma_CWR/engine/Poseidon/World/Simulation/Animation/FrameFunctions.cpp:80-113`).
  Before `c39e49e` that text was never logged and the log showed `could not
  boot` with no reason; now the `could not boot` line ends with it. `LOG_ERROR` lines such as
  `Animation ... not found or empty` do not set the latch.

The usual trigger of the last case is addon access. `CheckAccess`
(`arma_CWR/engine/Poseidon/World/Entities/Vehicles/VehicleTypes.cpp:461-480`)
raises that warning when a class that a placed unit needs (the unit's class, a
weapon in its `weapons[]`, a magazine in its `magazines[]`) belongs to an addon
missing from the mission's `addOns[]`. It is the one latch source that leaves a
trace, the WARN from `World::CheckAddon` (`WorldImpl.cpp:1182-1203`):

```
[WARN] [WORLD]   Access denied: <entry> - owner addon '<name>' is not activated (missing from the mission's addOns[]?)
```

Before `c39e49e`, other sources latched without any log line, for example the config reader's
`ErrorMessage(EMError, ...)` calls ("No section", "not a class", "not an array"
in `arma_CWR/engine/Poseidon/IO/ParamFile/ParamFile.cpp`) and `Cannot find
texture` in `arma_CWR/engine/Poseidon/World/Entities/Infantry/Head.cpp:733`.
On such a build, when `could not boot` appears without an `Access denied` line,
suspect those. Newer builds log each one as a `Warning Message:` WARN.

Guerrilla templates are mostly protected: when the mission's `description.ext`
defines `CfgGuerrillaZones`, `Guerrilla::ActivateTemplateAddons`
(`WorldInit.cpp:579`, `arma_CWR/engine/Poseidon/Game/Guerrilla/AddonActivation.cpp`)
activates the owners of the placed units' `weapons[]` and `magazines[]` before
the units are created. Plain probe and test missions get no such help. The
ZoneRegistry's own additive activation ("activated N addon(s) the faction
descriptors need") runs after the units are created, too late for them, and
does not clear the latch.

## Case: `Repair4.Abel`, 2026-10-12

The probe `arma_CWR/tools/lobo/runtime/repair4/lobo_il28_tailgun.test.toml`
(uncommitted repair .4 tooling at the time) flickered the menu at 02:19 local.
The mission name and path were correct: `tools/lobo/runtime/Repair4.Abel` was
staged, world Abel (Malden) loaded and the mission's 16 addons activated.

The mission adds an EAST group led by a mission-placed `LoBo_Egypt_Pilot`. That
class has `LoBo_throw2` in its `weapons[]`, `LoBo_throw2` belongs to
`LoBoWeapNad`, `LoBo_Pilot` only requires `BIS_Resistance`, the mission's
`addOns[]` did not list `LoBoWeapNad`, and the mission has no
`description.ext`, so nothing activated it. From the game log:

```
02:19:04.928 [WARN] [WORLD]   Access denied: CONFIG.cpp/CfgWeapons/LoBo_throw2/ - owner addon 'loboweapnad' is not activated (missing from the mission's addOns[]?)
02:19:04.973 [INFO] [CORE]    ZoneRegistry: activated 20 addon(s) ... lobois, loboweapnad, ...
02:19:06.272 [DBUG] [CORE]    LOAD: InitVehicles total 1485ms
02:19:06.272 [ERRR] [CORE]    StartAutoTest could not boot '.../Missions/Repair4.Abel/mission.sqm'
```

The `InitVehicles total` line shows `InitVehicles` ran to its final
`GetMaxError()` check. 366 `triWaitFrames` polls followed, until the run was
interrupted at 02:19:41. `Repair3.Abel`, whose probes boot, places only a stock
`SoldierWB` and creates the LoBo classes from the test script after the boot.

The `Access denied` line is enough on its own to reject the boot and is the
most likely cause, but the log cannot rule out a second, silent latch source.
Adding `LoBoWeapNad` to `Repair4.Abel`'s `addOns[]` should fix it; as of this
writing the probe has not been rerun.

The rule of thumb "the menu flashes when a mission is named wrong" points the
right way: the flicker says the mission did not boot. The name is one reason;
an `addOns[]` gap is another, and before `c39e49e` it left at most one WARN behind.

## Before launching a mission probe

1. Run a new probe with `--retries 0` until it has booted once. Every retry
   relaunches the game and flickers again.
2. Check that the folder is `<Name>.<World>` with a world the data dir has
   (Abel is Malden, Eden is Everon, Cain is Kolgujev, Noe is Nogova), and that
   the toml's `mission =` path exists relative to `arma_CWR/`.
3. Outside Guerrilla templates, put into `addOns[]` the owner addon of every
   class placed in `mission.sqm` and of everything in its `weapons[]` and
   `magazines[]`. A pbo's `requiredAddons[]` does not cover its units' weapons.
   Simpler: place only stock units in `mission.sqm` and create mod classes from
   the test script after the boot.
4. Watch the game stdout during the first seconds. On `StartAutoTest could not
   boot`, read the reason at the end of that line. On builds before `c39e49e`,
   stop the run (only processes you started) and grep the same log for
   `Access denied` first.
5. `--log-level debug` also turns on GL33 per-draw logging. The 39 s run above
   wrote 193 MB, 99.5% of it `[GRAPHICS]` lines, and a four-minute run wrote
   1.5 GB. Grep with `-v GRAPHICS`. `--log-categories` does not help: the
   engine parses it but never consults it (`LoggingSystem::IsCategoryEnabled`
   in `arma_CWR/engine/Poseidon/Foundation/Logging/Logging.cpp` has no caller,
   checked 2026-10-12).

## Engine follow-ups (done 2026-10-12, commit `c39e49e`)

- The `could not boot` line now ends with the reason, for example
  `StartAutoTest could not boot '<path>': latched warning: Cannot load mission,
  missing addons: <addon>`, and the latched warning itself is logged once at
  WARN (`Warning Message: ...`) when it latches. WARN does not trip `--strict`.
  Other reasons: `neither the mission nor its intro has a group`, `no error
  latched` (InitVehicles failed without the latch) and `error latch set without
  a message`.
- After a failed boot `triMissionPlayerReady` returns
  `FATAL:boot_failed:<reason>`. Trident treats a `FATAL:` result as terminal, so
  the run ends on the first poll, with no flicker and the reason in the failure
  message. Under the default `--strict` the `could not boot` ERROR shuts the
  game down first; Trident then reads the same reason from the test's
  `game_stdout.log` and appends it to `timeout waiting for ready: harness
  connection closed`.
- The addon name in the reason is the owner `CheckAccess` reports, which is the
  addon's `CfgPatches` class name, lowercased (the pbo name can differ).
- A test can expect the failure with `expect_boot_failure = "<text>"` in its
  toml; see `arma_CWR/tests/integration/harness/boot_failure_reported.test.toml`
  and its `_strict` twin.
- Still open: `--retries` retries a boot failure like any other failure, although
  it is deterministic. Each retry now costs about a second instead of
  `assert_timeout`.
