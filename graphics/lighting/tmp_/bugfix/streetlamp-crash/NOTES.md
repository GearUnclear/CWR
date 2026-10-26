# Bugfix: streetlamp-crash (`switchLight` on map objects crashes the game)

Scratch worktree: `D:\Arma_CWA_worktrees\fix-streetlamp-crash` (detached at
`claude/youthful-lovelace-8imbdh` = 3c53141). Nothing committed, no branch made.

## Root cause

`switchLight` and `lightIsOn` find their lamp with `dyn_cast<StreetLamp>`:

- `engine/Poseidon/Game/Commands/GameStateExtUi.cpp:748` (`ObjSwitchLight`)
- `engine/Poseidon/Game/Commands/GameStateExtUi.cpp:800` (`ObjLightSwitched`)

`dyn_cast<To>` (`engine/Poseidon/World/Scene/Object.hpp:819-829`) asks
`from->IsClassId(&To::_classId)`. Each class that should be told apart must
declare `USE_CASTING(base)` and `DEFINE_CASTING(Type)` (the warning comment at
`Object.hpp:90-93` says exactly this). `StreetLamp` (and its parent `EntityHit`)
declared neither (`engine/Poseidon/World/Scene/ObjectClasses.hpp:88-125` at the
branch tip), so `StreetLamp::_classId` named `Entity::_classId`
(`Vehicle.hpp:522`, `Simul.cpp:359`). The cast therefore answered "yes, a lamp"
for every `Entity`: every house (`Building` derives from `VehicleSupply` ->
`Transport` -> `EntityAI` -> `Entity`, `House.hpp:159`), every soldier and every
vehicle. Trees and other plain `Object`s were correctly rejected.

`ObjSwitchLight` then `static_cast`s that object to `StreetLamp` and calls
`StreetLamp::SwitchLight` (`ObjectClasses.cpp:221-234`), which:

1. writes `_lightState` into the object's memory at the `StreetLamp` offset,
2. runs `SimulateSwitch` (`ObjectClasses.cpp:267-269`), which reads
   `Type()->_bulbHit` through the wrong type layout and indexes `EntityHit::_hit`
   with that garbage index (`EntityHit::GetHit`, `ObjectClasses.cpp:92-104`),
3. runs `CreateLight` (`ObjectClasses.cpp:243-265`), which reads and may
   `Free()` a `Ref<LightPointVisible>` that is really some other member.

Outcomes seen: an immediate access violation in `EntityHit::GetHit` (the ssgi
track, `runs/v9_crash2`), and, in this run, a soldier whose memory was
overwritten and which crashed one draw later in `EntityAI::GetHitCont` from
`Man::WoundsAnimation` (see the evidence below). Even when nothing crashes, the
command silently corrupts the target object. `lightIsOn` on a non-lamp reads the
same wrong offset and returns "OFF", "ON" or "AUTO" by chance instead of "ERROR".

The real lamps were never the problem: switching the six Saint Louis lamps by
object id worked in the ssgi run and works here.

### Stock or ours?

Stock. `ObjectClasses.hpp`/`.cpp` are unchanged since the public source import
(`git diff ea77e57:engine/... HEAD:arma_CWR/engine/...` is empty for both), and
Bohemia's `upstream/main` (commit ffc6183, "3.05", 2026-08-18) has the same
`StreetLamp` without `USE_CASTING` (only `Road` and `ForestPlain` in that header
have it) and the same `dyn_cast<StreetLamp>` in `ObjSwitchLight` and
`ObjLightSwitched`. I did not run the Demo's `PoseidonGameDemo.exe`: the repro
needs Abel and the tri harness, and the source match is conclusive.

## The fix

`fix.patch` (42 added lines, 4 files):

- `ObjectClasses.hpp`: `USE_CASTING(base)` at the end of `class StreetLamp`,
  with a two-line comment on why.
- `ObjectClasses.cpp`: `DEFINE_CASTING(StreetLamp)` before the constructor.
- New unit test `tests/unit/engine/Poseidon/World/Scene/test_streetlamp_casting.cpp`,
  registered in `tests/unit/engine/Poseidon/CMakeLists.txt`.

Why it is correct: this is the engine's own mechanism, used the same way by
`Road`, `ForestPlain`, `Building`, `Fireplace` and about forty other classes.
`StreetLamp` now owns a distinct `_classId`, its `IsClassId` override answers
for itself and chains to `base::IsClassId`, so `dyn_cast<StreetLamp>` succeeds
only for real lamps while `dyn_cast<Entity>` of a lamp still works. Non-lamps
make `switchLight` a no-op and `lightIsOn` return "ERROR", which is what the
command code already intended (`if (lamp)` / `return "ERROR"`). No data member
changes, so object layout, `Serialize` and savegames are unaffected; the only
runtime cost is one more virtual `IsClassId` level for lamps.

I fixed the cause rather than adding a bounds check in `EntityHit::GetHit`: a
guard there would hide the crash but leave `SwitchLight` writing lamp state into
houses and soldiers.

## Tests

New: `test_streetlamp_casting.cpp`, tags `[scene][streetlamp][casting]`, no GPU.

- "StreetLamp - declares its own casting id": the member pointer type of
  `&StreetLamp::IsClassId` names `StreetLamp`, and `&StreetLamp::_classId !=
  &Entity::_classId`.
- "StreetLamp - dyn_cast does not take a plain entity for a lamp": builds a real
  `Entity` (with `new EntityType(nullptr)`, no shape) and checks
  `dyn_cast<Entity>` returns it and `dyn_cast<StreetLamp>` returns null.

Results (PoseidonTests.exe from `build/fix-streetlamp-crash-rwdi`, run from D:):

| | `[streetlamp]` |
|---|---|
| without the fix (`unit-before.log`, rerun in `unit-before-rerun.log`) | 2 test cases, 2 failed; 3 of 4 assertions failed (`false`; `0x...8214 != 0x...8214`; `0x...0e00 == nullptr`) |
| with the fix (`unit-after.log`) | All tests passed (4 assertions in 2 test cases) |

Required suites with the fix (`unit-suites.log`): `[remaster]` 13 cases pass,
`[gpu]` 3, `[GL33]` 104, `[DebugGroup]` 6, `[ShutdownOrder]` 8,
`[UBORestoreGuard]` 1, `[StateOwnership]` 4, `[StateCacheAudit]` 5,
`[BufferUsageAudit]` 4, `[IboBindAudit]` 4, `[Conventions]` 7, `[scene]` 18,
`[vehicles]` 13, `[house]` 6: all pass. `[graphics]`: 351 of 352 pass; the one
failure is `DDSConverter: generate DDS fixture from synthetic_dxt5 DXT5`
(`REQUIRE( dds2.width == 64 )`, `32 == 64`), a hidden `[.fixture]` test that
fails the same way with the fix reverted (`unit-dds-without-fix.log`), so it is
pre-existing and unrelated.

Format: `clang-format --dry-run --Werror` exits 0 for `ObjectClasses.cpp` and
the new test. `ObjectClasses.hpp` is not clang-format clean at the branch tip
(tab-indented legacy header, 864 lines of `--dry-run` output before my edit); the three new
lines follow the file's tab style, and clang-format's own output for them
differs only in turning the tabs into spaces.

Build: `cmake --preset win-x64-clang-rwdi -B build/fix-streetlamp-crash-rwdi`,
`cmake --build ... -j 6`, exit 0 (`build-after.log`, `build-after-2.log`).

## In-game evidence

Research-only Trident test (git-excluded, copied to `research-test/`):
`tests/rendering/research/streetlamp-crash/lamp_switch.test.sqf`. It uses the
`town_night` view of the capture set (Saint Louis, Abel, 22:00, Remastered
lighting), spawns a Jeep and a SoldierWB, waits for the lamps' AUTO step,
shoots `lamps_auto`, counts objects whose `lightIsOn` is not "ERROR" within
200 m of the lamp cluster, then calls `switchLight "OFF"` on the soldier, then
the Jeep, then all 666 objects within 200 m (each step behind its own log line),
shoots `lamps_off`, switches all to "ON", shoots `lamps_on`, and asserts that
the Jeep and soldier read "ERROR" and every counted lamp followed the switch.
Run with `run-captures.ps1 -Tests ... -DataJunction RemasterUD-fix-streetlamp-crash -Retries 0`.

BEFORE (branch tip build `D:\Arma_CWA_worktrees\lighting\arma_CWR\dist\lighting-rwdi`, output `before/`):

- census: `near=666;lamps=31;jeep=OFF;man=OFF` (the cast took the Jeep, the
  soldier and other entities for lamps).
- `event=switch;target=soldier` and `event=switched;target=soldier` were logged,
  then about 0.35 s later: `UNHANDLED EXCEPTION 0xC0000005` in
  `EntityAI::GetHitCont+0x10` <- `Man::WoundsAnimation` <- `Man::BasicAnimation`
  <- `Man::Animate` <- `Object::Draw` <- `EntityAI::Draw`
  (`before/tri/lamp_switch/game_stderr.log`). tri: `FAIL lamp_switch -- exit
  code -1073741819, 23.6s`. Only `lamps_auto` was shot.

AFTER (`dist/fix-streetlamp-crash-rwdi`, output `after/`):

- census: `near=666;lamps=6;jeep=ERROR;man=ERROR` (exactly the six lampazel).
- all three switch steps returned, `state;want=OFF;lamps=6;off=6`,
  `state;want=ON;lamps=6;on=6;jeep=ERROR;man=ERROR`, every assert passed, tri
  `passed 1/1 (32.0s)`, no exception in the log.
- Images: `after_lamps_off.png` shows the town unlit (mean luma 14.0) and
  `after_lamps_on.png` matches `after_lamps_auto.png` (mean luma 115.17 each,
  max channel difference 4/255).
- `before_lamps_auto.png` vs `after_lamps_auto.png`: max channel difference
  4/255, mean 0.015, confined to the top 369 rows (sky and tree crown), so the
  fix changes nothing in normal rendering.
- `evidence_before_after.png`: the four shots side by side with labels.

Note: the test asks `triSimFrames 1200` before the first shot; the engine log
reports `simulated 600 frames` for that call. The lamps were already lit at the
shot, so it did not matter here.

## Risks and follow-ups

- Behaviour change only for scripts that call `switchLight` / `lightIsOn` on a
  non-lamp: before, undefined behaviour; now a no-op and "ERROR". Lamps behave
  as before.
- **The same defect exists for eight more classes** (no own casting id, so
  `dyn_cast` falls through to an ancestor's id). Not fixed here because they are
  outside this bug and each changes behaviour elsewhere; worth one follow-up,
  ideally with a compile-time check in `dyn_cast` such as
  `static_assert(std::is_same_v<decltype(&To::IsClassId), bool (To::*)(const int*) const>)`
  (that would refuse to compile until all of them are fixed):
  - `Airplane` (falls to `Transport`): `GameStateExtUi.cpp:116`, inside
    `setDammage` / `setDamage` with value 0: `plane->RepairGear()` writes
    `_gearDammage = false` (`Airplane.hpp:189`) into ANY Transport, houses
    included. A very common mission command silently corrupts memory. Highest
    priority.
  - `Car` (falls to `Transport` via `TankOrCar`): `scudState`,
    `GameStateExtUi.cpp:813`; Guerrilla `HornSound`, `GuerrillaBase.cpp:924`
    (UD code; currently true for any Transport).
  - `Motorcycle` (-> `Transport`): `GuerrillaBase.cpp:924`.
  - `Tank` (-> `Transport`): `getTurretDirAndElev`, `GameStateExtGrp.cpp:1887`.
  - `HelicopterAuto` (-> `Transport`): `HelicopterAI.cpp:2068` (network create).
  - `Explosion`, `ObjectDestructed` (-> `Entity`): `Smokes.cpp:2002`,
    `Smokes.cpp:575` (network create).
  - `SeaGullAuto` (-> `SeaGull`): `SoldierOldMove.cpp:968`, harmless in practice.
- Not addressed (seen while reading, no crash): `EntityHitType::InitShape`
  appends `_bulbHit` to `_hitPoints` on every shape reload without clearing the
  list (`EntityAIType` clears it at `VehicleAI.cpp:882`), so the list grows by
  one stale entry each time the lamp type is reloaded. Indices stay in range
  because `_hit` is sized from the list at construction.
