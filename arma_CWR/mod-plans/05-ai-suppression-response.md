# Working suppression: incoming and near-miss fire forces cover even in Combat mode

*Make bullets that whip past a soldier's head actually scare it — even when it is already fighting — by extending the engine's existing `_dangerUntil` "danger window" to fire in `CMCombat` and feeding it from a per-bullet near-miss test.*

## Summary

The engine already has a complete "I am in danger" reaction system built around a short
time window (`AIUnit::_dangerUntil`). When that window is open, `IsDanger()` returns true and
several systems already respond: soldiers force a prone stance, the head switches to the
"danger" mimic, and detection bumps side-accuracy. The problem is twofold:

1. `AIUnit::SetDanger()` deliberately **does nothing when the unit is in `CMCombat`**
   (`AIUnitImpl.cpp:2196-2212`), so a unit that is already fighting never registers suppression.
2. Nothing sets danger from a *miss*. Danger is only raised from detection events
   (`AIGroup::ReactToEnemyDetected`, `AIUnit::Disclose`, `Target.cpp` sensor-fire) — i.e. when
   the AI already *knows about* a shooter. A bullet that cracks 1 m past an unaware soldier's
   ear produces no reaction at all.

This mod merges the two suppression ideas:

- **Extend `SetDanger`** to also open a *dampened* (shorter) danger window in `CMCombat`, with a
  cooldown so it cannot be spammed every frame.
- **Add a near-miss test in `ShotBullet`**: each simulated frame a bullet sweeps a segment
  `_beg → _end`; if a living AI's aim point lies within ~1–3 m perpendicular of that segment,
  call `SetDanger()` on it (and disclose the shooter's rough direction so cover makes sense).
- **Consume `IsDanger()` in the aim/FSM path** so suppressed units (a) acquire targets more
  slowly, (b) drop stance, (c) pause their advance, and (d) route into the existing
  `FindHideBehind` cover code.

Observable result: walk a squad into a firefight and put rounds *near* but not *on* them.
Today they keep standing and shooting back at full effectiveness. After the mod they flinch —
go prone, slow their return fire, and scuttle to the nearest cover object — and they do it even
when they are already in Combat behaviour, which is exactly when suppression matters.

## Why it's interesting

OFP/ACWA AI is famously "all or nothing": until a unit is suppressed-to-death it fires with
full composure. Real suppression — making troops keep their heads down without killing them — is
the single biggest lever for believable infantry combat, and almost every modern milsim mod
re-implements it badly in SQF on top of the engine. Here the engine *already* has the reaction
plumbing (`IsDanger()` is read in three places); we are unlocking it for the one combat mode
where it is currently suppressed, and wiring a real stimulus (near-miss) to it. Small, surgical,
and the payoff is immediately visible in any firefight. No new assets, pure AI/engine work.

## Difficulty & prerequisites

**Advanced.** You touch the bullet simulation hot path, AI danger state, and the aim/FSM read
sites, and you must respect networking (`IsLocal()` gating) and serialization of `_dangerUntil`.

- C++20 / Clang toolchain per `CLAUDE.md` (clang-cl on Windows, Ninja, vcpkg, ccache).
- Familiarity with the `AIUnit` / `EntityAI` / `Man` / `Soldier` hierarchy.
- **No art assets required.** Reuses the existing `"danger"` head mimic, the existing prone
  animation states, and the existing `FindHideBehind` cover search. Nothing new in 3D, textures,
  or config is strictly needed; the optional config knobs in §8 reuse existing CfgAmmo fields.

## Key files & entry points

All paths absolute under `C:/dev/arma_CWR`.

- `engine/Poseidon/AI/AIUnitImpl.cpp`
  - `AIUnit::IsDanger() const` — `2185-2188`, reads `Glob.time <= _dangerUntil`.
  - `AIUnit::SetDanger(float until)` — `2190-2213`, the function to extend; currently `switch`
    only handles `CMSafe`/`CMAware` and explicitly comments "don't set danger in Combat mode".
- `engine/Poseidon/AI/AIUnit.hpp`
  - `_dangerUntil` member — `126` (`Foundation::Time`).
  - `bool IsDanger() const;` / `void SetDanger(float until = -1.0);` declarations — `254-255`.
- `engine/Poseidon/AI/AIUnit.cpp`
  - reset to "long ago": `_dangerUntil = Glob.time - 60.0f;` — `408`.
  - serialization: `ar.Serialize("dangerUntil", _dangerUntil, 1, Time(0))` — `611`.
- `engine/Poseidon/World/Entities/Weapons/Shots.cpp`
  - `ShotBullet::Simulate(float, SimulationImportance)` — `793-797` (calls `SetEnd(Position())`).
  - `ShotBullet::SetBeg/SetEnd/StartFrame` — `1467-1488` (define the `_beg → _end` segment).
  - `Shot::SetParent` / `_parent` — `62` (the shooter, used to skip friendly self-fire and to
    derive a hide direction).
- `engine/Poseidon/World/Detection/TargetFire.cpp`
  - `EntityAI::FindHideBehind(Vector3 pos, float maxDist)` — `696`; `FindHideBehind()` — `895`;
    `BegHide()` — `928`; `HideThink()` — `940`. The existing cover-search path to reuse.
- `engine/Poseidon/AI/VehicleAI.cpp`
  - `EntityAI::OnDanger()` base no-op — `1836`.
  - `EntityAI::IsCautiousOrDanger() const` — `2369-2381`, already OR-s `IsDanger()` with cautious
    modes; used by `TransportCore.cpp:1632`.
- `engine/Poseidon/World/Entities/Infantry/SoldierOldSim.cpp`
  - `Man::OnDanger()` — `1018-1025`, shortens `_lookForwardTimeLeft` (the flinch hook).
  - mimic switch reading `IsDanger()` — `158-161`.
- `engine/Poseidon/World/Entities/Infantry/SoldierOldAI.cpp`
  - stance: `if (unit->IsDanger() && !_inBuilding) maxPos = ManPosLying;` — `1798-1801`.
  - `Man::GetAimed(int weapon, Target*) const` — `1192-1402`; `aimPrecision` computed at `1308` —
    the acquisition-tolerance knob to detune under danger.
  - `Soldier::AIFire(float deltaT)` — `1417` (the firing loop).
- `engine/Poseidon/AI/AISubgroupFSM.cpp`
  - `CheckHideInit` → `HideThink()` — `1110-1116`; `HideEnter` → `BegHide()` — `1118-1138`.
- `engine/Poseidon/AI/VehicleAIPilot.cpp`
  - `enableHide = commander->IsFreeSoldier() && commander->GetCombatMode() >= CMCombat;` — `796`,
    the existing gate that already lets cover-seeking run in Combat (good news: the hide path is
    *already* Combat-enabled; we just need the danger trigger).
- `engine/Poseidon/AI/Path/ArcadeWaypoint.hpp`
  - `enum CombatMode { CMUnchanged, CMCareless, CMSafe, CMAware, CMCombat, CMStealth }` — `96-104`.

## How it works today

**Danger window.** `_dangerUntil` is a `Foundation::Time`. `IsDanger()` is simply
`Glob.time <= _dangerUntil`. `SetDanger(until)` (default `until = -1`):

```cpp
void AIUnit::SetDanger(float until) {
    if (IsPlayer()) return;
    switch (GetCombatMode()) {
        case CMSafe:
        case CMAware:
            if (until < 0) until = GRandGen.PlusMinus(5.0f, 1.0f); // ~4–6 s window
            _dangerUntil = Glob.time + until;
            EntityAI* veh = GetVehicle();
            if (veh) veh->OnDanger();
            break;
        // don't set danger in Combat mode  <-- falls through to nothing for CMCombat/CMStealth
    }
}
```

So in `CMCombat` (and `CMStealth`, `CMCareless`) the call is a no-op. Callers today:
`AIGroup::ReactToEnemyDetected` (`AIGroupImpl.cpp:428`), `AIUnit::Disclose`
(`AIUnitImpl.cpp:94,109`), and `Target.cpp:1041-1047` which *itself* guards
`unit->GetCombatMode() < CMCombat` before calling — i.e. the detection layer already assumes
danger is meaningless in Combat.

**Who reacts to `IsDanger()`** (the reaction plumbing that already exists):
- `SoldierOldSim.cpp:158` — head mimic → `"danger"`.
- `SoldierOldAI.cpp:1798` — forces `ManPosLying` (prone) when `_unitPos == UPAuto`.
- `VehicleAI.cpp:2376` — `IsCautiousOrDanger()` returns true, consumed by transport logic.
- `Man::OnDanger()` (`SoldierOldSim.cpp:1018`) — clamps `_lookForwardTimeLeft` to 0.3 s, a small
  "look toward threat / flinch" effect, fired via `EntityAI::OnDanger()` from `SetDanger`.

**Bullets.** `ShotBullet` is a `ShotShell`. `ShotShell::Simulate` (`Shots.cpp:647`) integrates
motion, does object/landscape collision and applies hit damage. `ShotBullet::Simulate`
(`Shots.cpp:793`) calls `base::Simulate` then `SetEnd(Position())`. `StartFrame`
(`Shots.cpp:1481`) sets `_beg = _end` each frame, so within a frame the pair `_beg → _end` is the
line segment the round traversed. **Nothing in this path notifies nearby units of a miss** — only
direct hits (`DirectLocalHit` / `ExplosionDammage`) have any AI consequence.

**Cover search.** `FindHideBehind` scans landscape object cells for a static object bigger than
the unit, between the unit and `HideFrom()` (derived from `_hideTarget`), and stores `_hideBehind`.
The subgroup FSM drives it (`AISubgroupFSM.cpp` Hide state → `HideThink`/`BegHide`), and
`VehicleAIPilot.cpp:796` already enables it for free soldiers at `>= CMCombat`. So the cover
machinery runs in Combat — it just is not being *triggered by suppression* today.

## Implementation approach

### 1. Extend `SetDanger` to fire (dampened) in Combat — `AIUnitImpl.cpp:2190`

Add `CMCombat` (and optionally `CMStealth`) handling with a shorter window and a re-arm cooldown
so the per-bullet stimulus in step 3 cannot extend the window indefinitely or thrash `OnDanger`:

```cpp
void AIUnit::SetDanger(float until) {
    if (IsPlayer()) return;
    CombatMode mode = GetCombatMode();
    switch (mode) {
        case CMSafe:
        case CMAware:
            if (until < 0) until = GRandGen.PlusMinus(5.0f, 1.0f);
            break;
        case CMCombat:
        case CMStealth:
            // dampened: shorter window, and only re-arm OnDanger occasionally
            if (until < 0) until = GRandGen.PlusMinus(2.5f, 0.5f);
            break;
        default: // CMCareless, CMUnchanged
            return;
    }
    Time newUntil = Glob.time + until;
    if (newUntil <= _dangerUntil) return;     // never shorten an existing window
    bool wasDanger = IsDanger();
    _dangerUntil = newUntil;
    if (!wasDanger) {                         // edge-trigger OnDanger (the flinch)
        EntityAI* veh = GetVehicle();
        if (veh) veh->OnDanger();
    }
}
```

Rationale: the `newUntil <= _dangerUntil` guard and the `!wasDanger` edge-trigger keep the hot
near-miss path cheap and prevent the head from re-flinching every frame. Keep the `IsPlayer()`
early-out unchanged (players are never AI-suppressed).

Optionally relax `Target.cpp:1041` (`GetCombatMode() < CMCombat`) so sensor-fire can also raise
the dampened window in Combat — but the near-miss stimulus in step 3 is the primary path, so this
is secondary and can be left alone for the first slice.

### 2. Add a near-miss helper — `Shots.cpp` (file-local) + a small AI entry

Add a file-static helper in `Shots.cpp` that, given a segment `beg → end` and the shooter
`_parent`, finds living infantry whose aim point is within a perpendicular threshold and raises
danger. Keep it cheap by querying only the landscape object cells the segment crosses (mirror the
cell-iteration style already used in `TargetFire.cpp:778` / `Shots.cpp` collision code) rather
than the whole world.

```cpp
static void SuppressNearMiss(EntityAI* parent, Vector3Val beg, Vector3Val end) {
    Vector3 seg = end - beg;
    float segLen2 = seg.SquareSize();
    if (segLen2 < 1e-4f) return;
    // iterate candidates in the landscape cells the segment touches (ObjRadiusRectangle style)
    // for each candidate EntityAI* veh:
    //   if (!veh || veh == parent) continue;
    //   if (veh->IsDammageDestroyed()) continue;
    //   AIUnit* u = veh->PilotUnit(); if (!u || u->IsPlayer()) continue;
    //   Vector3 p = veh->AimingPosition() - beg;          // soldier torso/head point
    //   float t = (p * seg) / segLen2;                    // projection param
    //   if (t < -0.0f || t > 1.0f) continue;              // miss is alongside the path
    //   Vector3 perp = p - seg * t;                       // perpendicular offset
    //   float d2 = perp.SquareSize();
    //   if (d2 > Square(NEAR_MISS_RADIUS)) continue;      // ~1–3 m
    //   u->SetDanger();                                   // dampened window in Combat
    //   if (parent) u->GetVehicle()->FindHideBehind(parent->Position(), 50.f); // bias cover
}
```

Notes:
- Use `AimingPosition()` (already used by `FindHideBehind` at `TargetFire.cpp:742`) as the body
  point; do not test against feet.
- `NEAR_MISS_RADIUS` ~2.0 m default. Optionally scale by ammo "scare" (see §8).
- Biasing cover with `FindHideBehind(parent->Position(), ...)` makes the unit hide *from the
  shooter's direction* even before it has a `Target` for him; if `parent` is null (deleted
  shooter) skip the hide bias and let the FSM pick.

### 3. Call the near-miss test from the bullet hot path — `Shots.cpp:793`

```cpp
void ShotBullet::Simulate(float deltaT, SimulationImportance prec) {
    base::Simulate(deltaT, prec);   // may set _delete on impact
    SetEnd(Position());
    if (IsLocal() && !_delete && _beg.Distance2(_end) > Square(0.5f))
        SuppressNearMiss(_parent, _beg, _end);
}
```

Gate on `IsLocal()` (consistent with the damage code in `ShotShell::Simulate`) so only the host
that owns the bullet raises danger; danger then propagates via the units' normal AI/network
state. Skip when the round already impacted (`_delete`) to avoid double-processing the final
segment. The `> 0.5 m` guard avoids work on near-stationary tracers.

### 4. Detune acquisition under danger — `SoldierOldAI.cpp` `Man::GetAimed` (`~1308`)

Make a suppressed unit slower to consider itself "on target". `aimPrecision` at line 1308 widens
the acceptable error; under danger, inflate it (and/or the `tgtSize` window at 1338-1341) so the
unit needs to settle longer before `GetAimed`/`AIFire` will commit:

```cpp
float aimPrecision = (GetInvAbility() - 1) * 1.5f + 1;
AIUnit* u = CommanderUnit();
if (u && u->IsDanger()) aimPrecision *= 1.5f;   // shakier aim while suppressed
```

This is the "slow acquisition" lever and is observable as longer time-to-first-effective-shot
when rounds are landing nearby.

### 5. Pause advance + ensure cover routing — FSM read sites

- Prone is already handled at `SoldierOldAI.cpp:1798`; with step 1 it now triggers in Combat.
- The hide path is already Combat-enabled (`VehicleAIPilot.cpp:796`). The cover bias in step 2
  seeds `_hideBehind`; the existing `HideThink`/`BegHide` FSM (`AISubgroupFSM.cpp:1110-1138`) keeps
  it refreshed. Verify the subgroup actually enters the Hide state when a unit `IsDanger()` — if
  not, add `IsDanger()` to the Hide-state entry condition there.
- To pause advance, gate forward movement on `IsDanger()` near the speed/path-follow code that
  reads `_unitPos`/formation (same region as `SoldierOldAI.cpp:1796-1815`): clamp desired speed to
  0 (or a crawl) for the duration of the danger window when not already at a cover position.

### 6. Wire `OnDanger` flinch (free win)

`Man::OnDanger()` already exists (`SoldierOldSim.cpp:1018`) and now fires in Combat because of the
edge-trigger in step 1 — no extra work, you get the look-toward-threat flinch for free.

## Config / data / SQF touchpoints

Optional, all reusing existing surfaces — none required for the core feature:

- **Per-ammo scare radius (reuse CfgAmmo).** CfgAmmo entries already carry `hit`, `indirectHit`,
  etc. (read via `Type()->hit` in `ShotShell::Simulate`). You can scale `NEAR_MISS_RADIUS` by an
  existing field (e.g. larger window for cannon/`explosive` rounds) without adding new config keys.
  If you want an explicit knob, add one CfgAmmo float (e.g. `aiSuppression`) defaulting to 1.0;
  this is a config addition, not an asset.
- **Difficulty / global toggle.** Gate the whole behaviour behind a build flag or an existing
  difficulty/skill scalar so it can be A/B tested. `GetInvAbility()` (already used at
  `SoldierOldAI.cpp:1308`) is a natural per-unit skill input to scale window length.
- **SQF observability.** `IsDanger()` has no script binding today; exposing a read-only
  `getSuppression`-style command (or reusing the existing `unitReady`/behaviour reporting) lets
  Trident SQF assert the state. Adding a scripting function is optional but makes the integration
  test in §10 far easier — wire it through the SQF `Evaluator` the way other unit getters are.

## Risks & gotchas

- **Hot path cost.** `ShotBullet::Simulate` runs for every bullet every frame. The near-miss scan
  must iterate only the few landscape cells the segment crosses (as the existing collision code
  does), never the global object list. Profile with many shooters; the `IsLocal()` + `_delete` +
  `>0.5 m` guards keep it off for spent/short tracers.
- **Don't shorten the window.** The `newUntil <= _dangerUntil` guard matters: without it the
  dampened Combat window could *truncate* a longer Aware window set elsewhere.
- **Edge-trigger `OnDanger`.** Calling `OnDanger()` every frame would re-clamp `_lookForwardTimeLeft`
  and visibly lock the head; only call on the rising edge (`!wasDanger`).
- **Networking / locality.** Only raise danger on the bullet's owner (`IsLocal()`); `_dangerUntil`
  is already serialized (`AIUnit.cpp:611`) so remote units pick up state through normal sync. Do
  not raise danger on non-local units directly.
- **Player exclusion.** Keep the `IsPlayer()` early-out; suppressing the human is a different
  (HUD/camera) feature and out of scope.
- **Repo warning conventions (`CLAUDE.md`).** The build globally suppresses ~50 clang warning
  classes; the intentionally-incomplete `switch` over `CombatMode` in `SetDanger` is exactly the
  kind of pattern the project keeps — preserve a `default:` that returns rather than "fixing" the
  switch into something the linter likes. Don't refactor surrounding `memcpy`/`memmove`
  `ClassIsMovableZeroed` code you may pass through in `Shots.cpp`.
- **`__FILE__` asymmetry.** Irrelevant to this change unless you add asserts/logging that print
  paths — remember clang-cl leaves `__FILE__` absolute while GNU-driver clang rewrites it.
- **PCH is per-target PRIVATE.** Adding includes to `Shots.cpp` / `AIUnitImpl.cpp` is fine; if you
  add a new public header for the near-miss helper, remember targets bake their own PCH (see
  `POSEIDON_PCH_TARGETS`). Prefer a file-static helper to avoid header churn.
- **FileSize target.** `AIUnitImpl.cpp` and `Shots.cpp` are large; keep additions tight — the
  `FileSize` target warns >3000 lines and errors >5000. The near-miss helper is ~30 lines; fine.

## Testing

**Unit (Catch2, `tests/unit/`).** The pure logic in `SetDanger` and the segment math are unit-
testable without game data:
- Add cases to `PoseidonTests` (or `PoseidonCoreTests`) that construct an `AIUnit`, set combat
  mode to `CMCombat`, call `SetDanger()`, and assert `IsDanger()` is now true and that the window
  is the *dampened* length (set `Glob.time` deterministically). Assert `SetDanger` never shortens
  an existing longer window, and that a second same-frame call does not re-fire `OnDanger`.
- Add a small free-function test for the point-to-segment perpendicular distance used in
  `SuppressNearMiss` (factor it out so it is testable): points inside/outside the radius and the
  `t < 0 / t > 1` end-cap cases.
- Run: `ctest --test-dir build/win-x64-clang-rwdi -R PoseidonTests --output-on-failure`.

**Integration (Trident, `tests/integration/`).** Needs Demo game data (`OFPR_DATA_DIR`). Write an
SQF scenario: place an AI rifleman set to Combat (`setBehaviour "COMBAT"`), spawn a shooter that
fires deliberately wide (target an empty marker ~1.5 m to the side), then poll the unit's stance /
the suppression getter from §8. Assert the unit goes prone / `IsDanger()` becomes true within the
window, and that a control run with shots landing >5 m away does **not** trip it.
- Build runner: `cargo build --manifest-path engine/Trident/Cargo.toml`.
- Run: `tri test -j6 --retries 2 tests/integration` (binary `engine/Trident/target/debug/tri`).

**In-game (manual, fastest signal).** Build `cmake --preset win-x64-clang-rwdi` /
`cmake --build build/win-x64-clang-rwdi`, run the Demo client, drop an editor mission with one AI
in Combat, and fire bursts near (not at) it.
- Before: it stays upright and returns accurate fire immediately.
- After: it flinches (head/look snaps toward you), drops prone, return fire is delayed/shakier,
  and it relocates to a nearby wall/rock. Toggle the §8 build flag to A/B compare.

## Scope estimate

Roughly **2–4 days** for a confident, profiled implementation:

- Step 1 (extend `SetDanger`): ~1 hour, low risk.
- Steps 2–3 (near-miss scan + hot-path call): ~1 day including cell-iteration plumbing and
  profiling — the riskiest part.
- Steps 4–5 (aim detune + advance pause + verify FSM hide entry): ~1 day of tuning.
- Tests + config knob: ~0.5 day.

**Minimal first slice for a fast visible win:** do **only** step 1 plus a *throwaway* always-on
version of step 3 that calls `SetDanger()` on any AI within a fixed 2 m of `_beg→_end` (skip the
cell optimization, the aim detune, and config). Because the reaction plumbing already exists
(`SoldierOldAI.cpp:1798` prone, `SoldierOldSim.cpp:158` mimic, `Man::OnDanger` flinch), you will
*immediately* see Combat-mode soldiers hit the dirt when you shoot past them — proving the whole
idea — before you invest in the perpendicular-distance math, performance, and cover routing.
