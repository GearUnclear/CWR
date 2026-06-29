# Smarter retreat: flee toward cover, not back to spawn

*Make panicked AI squads break contact toward low-exposure ground away from the enemy, instead of sprinting across open terrain back to waypoint 0.*

## Summary

When an AI group decides to flee (the `ArcadeFlee` FSM state), it asks
`FindFleePoint()` for a destination. Today that function only considers two
kinds of targets: friendly **supply** vehicles to run to, or — failing that —
**waypoint 0**, which is the group's *initial spawn position*. The result is
the well-known Operation Flashpoint behavior where a broken squad turns its back
on the firefight and jogs in a straight line to where the mission placed it,
often directly across open fields and frequently *toward* the enemy.

This mod rewrites `FindFleePoint()` so that retreat is driven by the engine's
existing **exposure cost field**. We sample a ring of candidate points fanned out
*away* from the nearest known enemy (the enemy target list is already iterated in
this very function), score each candidate with
`AICenter::GetExposurePessimistic()` plus a small distance penalty, and pick the
lowest-cost point. Supply-vehicle flee behavior is preserved as one more
candidate, not the only option.

**Observable result:** a suppressed/routed squad now peels off toward dead
ground — behind a hill, into a treeline edge, away from the muzzle flashes —
rather than making a beeline for spawn. You can watch this in the editor by
placing an inferior squad near a strong enemy and triggering a rout: the flee
vector points away from the threat and tends to end on covered/low-visibility
terrain.

## Why it's interesting

- It fixes one of the most immersion-breaking AI behaviors in the base game with
  a *localized* change to a single function, reusing infrastructure that already
  exists (the exposure map, the target list, `GetExposurePessimistic`).
- It is a pure tactics/AI change — exactly the modder's area of interest — with
  no new content. The payoff is visible within seconds of testing.
- The exposure field already powers pathfinding cover decisions
  (`PathPlanner.cpp:1409`, `AIUnitImpl.cpp:2559`, `AISubgroup.cpp:2359`), so
  reusing it for the flee destination makes retreat *consistent* with how units
  already choose covered routes.

## Difficulty & prerequisites

**Intermediate.** You need to be comfortable reading C++ inside the AI subsystem
and reasoning about the exposure cost field, but the edit is confined to one
function (plus an optional small helper). No new systems.

**No art assets required.** This is C++ engine logic only. There are no new
models, textures, sounds, configs, or missions. Everything reuses existing
runtime data (the per-center exposure map and target list). Testing uses
existing editor placements / Demo data.

## Key files & entry points

- **`engine/Poseidon/AI/AIArcadeActions.inc`**
  - `Vector3 FindFleePoint(AIGroup* group, bool& forced)` — **line 1815**. The
    function to rewrite. Currently it:
    - iterates `center->NTargets()` (line 1830),
    - filters to civilian/friendly supply vehicles only (lines 1837-1846),
    - scores them with `center->GetExposurePessimistic(info._realPos)` + distance
      (lines 1903-1908),
    - compares against waypoint 0's cost (lines 1916-1933) and returns either the
      best supply vehicle position or `group->GetWaypoint(0).position`.
  - `static void ArcadeFlee(AIGroupContext* context)` — **line 1936**. The caller.
    It takes `mission->_destination = FindFleePoint(...)` (line 1947), runs
    `leader->FindFreePosition(mission->_destination, normal)` (line 1967), then
    `group->Move(..., mission->_destination, ...)` (line 1972) at
    `SpeedFull`. We do **not** need to change this; it consumes whatever point we
    return.
- **`engine/Poseidon/AI/AICenter.hpp`**
  - `class AITargetInfo` — **line 17**. Fields we use: `_idExact` (line 20),
    `_side` (`TargetSide`, line 21), `_type` (line 22), `_realPos` (`Point3`,
    line 24), plus `_destroyed`/`_vanished` (lines 39-40) to skip stale targets.
  - `int NTargets()` (line 252), `const AITargetInfo& GetTarget(int)` (line 253).
  - `float GetExposurePessimistic(int x, int z)` (line 311) and the
    `Vector3Par` overload (line 315) — the cost field we sample.
  - `bool IsEnemy(TargetSide)` (line 271) / `bool IsFriendly(TargetSide)`
    (line 269).
- **`engine/Poseidon/AI/AICenterStats.cpp`**
  - `AICenter::IsEnemy()` — **line 1394** (returns true for `TEnemy`, or
    `_friends[side] < 0.6`). This is how we classify "the enemy" when scanning the
    target list.
- **`engine/Poseidon/AI/AICenterImplPreview.cpp`**
  - `AICenter::GetExposurePessimistic(int x, int z)` — **line 891**. Reads
    `_map->_exposureEnemy(x,z)` + `_map->_exposureUnknown(x,z)` and returns
    `EXPOSURE_COEF * 0.33 * (exposureE + exposureU)`. Returns `0` if `!_map` or
    out of range (lines 893-900). The `Vector3Par` overload (line 938) converts
    world position to grid cell via `InvLandGrid`.
- **`engine/Poseidon/World/Terrain/Landscape.hpp`**
  - `float SurfaceY(float x, float z)` — **line 707**, and
    `SurfaceYAboveWater` (line 708) for snapping a sampled XZ point to terrain
    height. `InvLandGrid`/`LandGrid` macros at lines 23-24 (resolve through the
    global `GLandscape`).
- **Reference only — do not edit** (other consumers of the same exposure field,
  useful for confirming sign/scale conventions):
  - `engine/Poseidon/AI/AIGroupImplHealth.cpp:224` and `:975` — supply/repair
    target selection that already skips targets where
    `GetExposurePessimistic(info._realPos) > 0` (i.e. higher = more dangerous).
  - `engine/Poseidon/AI/AIGroupImpl.cpp:1640`, `AISubgroup.cpp:2359`,
    `AIUnitImpl.cpp:2559`, `Path/PathPlanner.cpp:1409`.

## How it works today

`FindFleePoint()` (AIArcadeActions.inc:1815-1934):

1. Sets `forced = false`, captures the leader position `posL`, and the center.
2. Loops every center target. It **skips** anything that is not a civilian or
   friendly (`!(info._side == TCivilian) && !center->IsFriendly(info._side)`,
   line 1837) and anything that is not a `Transport`/`VehicleSupply`
   (`dyn_cast<Transport, Object>`, line 1842), and skips the group's own vehicle.
3. For surviving supply vehicles it computes a `coef` reflecting how useful the
   supply type is (attendant, repair, ammo, magazine, fuel — lines 1856-1901),
   then `cost = GetExposurePessimistic(info._realPos) + dist*0.1`, multiplied by
   `coef` (lines 1903-1908). It tracks the minimum-cost supply vehicle.
4. It then computes the cost of **waypoint 0** with a heavy penalty
   `coef = 1/0.1 = 10` (lines 1916-1923).
5. If the best supply vehicle beats penalized waypoint 0, it returns that vehicle
   position and sets `forced = true`; otherwise it returns `wInfo.position` — the
   **spawn waypoint** (lines 1925-1933).

So in the common case (no friendly supply vehicle nearby) the function
unconditionally returns the spawn point. The enemy target positions are *iterated
right here* but never used to influence direction — they are simply filtered out.
`GetExposurePessimistic` is already wired up and proven: higher value = more
exposed/dangerous, `0` when the map is unavailable or the cell is out of range.

## Implementation approach

All steps are inside `FindFleePoint()` in `AIArcadeActions.inc`. Keep the
existing supply-vehicle scan; **add** an enemy-aware sampling pass and fold both
into the final selection.

1. **Find the nearest known enemy direction.** Add a second pass (or reuse the
   existing loop) over `center->NTargets()`. For each `info`:
   - skip `info._destroyed || info._vanished` and `!info._idExact`,
   - keep only `center->IsEnemy(info._side)` (AICenterStats.cpp:1394),
   - track the closest enemy `_realPos` to `posL` by `SquareSizeXZ()` (same
     distance idiom used at AIGroupImplHealth.cpp:228).
   Compute `Vector3 away = (posL - nearestEnemyPos)` flattened to XZ; if its
   `SizeXZ()` is below an epsilon (enemy basically on top of us, or no enemy
   known) fall back to the current waypoint-0 behavior so we never regress.

2. **Generate candidate flee points.** Build a small fixed set of candidates by
   fanning the normalized `away` vector across a spread (e.g. base bearing ±60°
   in a few steps) at a couple of radii (e.g. 50 m and 100 m). A 3-angle × 2-range
   grid is ~6 candidates — cheap, runs only on the flee transition. For each
   candidate XZ, snap Y with `GLandscape->SurfaceYAboveWater(x, z)`
   (Landscape.hpp:708) so the point sits on terrain. Keep candidate count small;
   the cost field lookup is O(1) per point.

3. **Score candidates with the existing cost field.** For each candidate `c`:
   `cost = center->GetExposurePessimistic(c) + (posL - c).SizeXZ() * costPerDist`.
   Reuse the existing `const float costPerDist = 0.1f` (line 1828). Lower is
   better — consistent with the existing supply scoring and with
   AIGroupImplHealth.cpp:224's "skip if exposure > 0" convention. Optionally add a
   small bonus for candidates whose bearing is closest to pure `away` so ties
   resolve toward "directly away from the enemy."

4. **Unify selection.** Keep the supply-vehicle `minCost`/`bestInfo` result from
   the original loop. Compare it against the best sampled-cover candidate and the
   (penalized) waypoint-0 cost from lines 1916-1923. Pick the global minimum:
   - if a supply vehicle wins → return `bestInfo->_realPos`, `forced = true`
     (unchanged semantics);
   - if a sampled cover point wins → return that point (leave `forced = false`,
     since it is a tactical reposition, not a hard objective);
   - else → return `wInfo.position` (waypoint 0), preserving the original
     last-resort fallback.

5. **Preserve the `AI_ERROR` guards** at the top (lines 1819-1823) and the
   `NWaypoints() > 0` precondition — waypoint 0 must remain a valid fallback.

6. **Keep the signature unchanged** (`Vector3 FindFleePoint(AIGroup*, bool&)`)
   so `ArcadeFlee` (line 1947) needs no edit. `ArcadeFlee` already calls
   `leader->FindFreePosition(mission->_destination, normal)` (line 1967) which
   nudges the returned point to a free spot, so minor candidate overlap with
   obstacles is tolerated.

7. **Tunables** (declare as `static const` near the top of the function so the
   `FileSize` budget and readability stay sane): spread angle, ring radii, number
   of angle steps, and an optional `awayBias` weight. Start conservative
   (radii 50/100 m, ±60°, 3 steps) and tune in the editor.

Optional refinement (defer to a second pass): instead of nearest single enemy,
average the direction to the few nearest enemies weighted by inverse distance, so
a squad caught between two threats flees along the bisector. The `AIEnemyList` /
`AIEnemyInfo` aggregation (AICenter.hpp:64-72) may already cluster enemies and is
worth checking before hand-rolling this.

## Config / data / SQF touchpoints

None strictly required — this is self-contained engine logic, and that keeps the
slice small.

If you want the behavior tunable without recompiling, the natural place is the
existing AI/CfgAISkill-style config rather than a new class. Check
`engine/Poseidon/AI` for where flee/courage parameters are already read (search
for `courage`, `GetCombatExperience`, or skill scalars) and route the spread/
radius constants through an existing scalar instead of inventing a new config
node. Only do this if you find an existing knob; do **not** add new config schema
for a first slice.

There is no SQF surface that needs to change. Mission scripts that issue a manual
`flee`/`allowFleeing`-style command still funnel through the same FSM state, so
they benefit automatically.

## Risks & gotchas

- **Cost-field sign/scale.** `GetExposurePessimistic` returns
  `EXPOSURE_COEF(50) * 0.33 * (enemy+unknown exposure)` and **0** when `_map` is
  null or the cell is out of range (AICenterImplPreview.cpp:893-905). A sampled
  point off the map therefore scores artificially *safe* (cost 0). Clamp/skip
  candidates whose grid cell `!InRange` (or that fall outside the island) so the
  squad doesn't try to flee off the world edge.
- **Don't flee *into* the enemy.** Verify the direction sign: `away = posL -
  enemyPos` points from enemy to us; moving along `+away` increases distance.
  Getting this backwards sends squads straight into contact — the headline bug,
  inverted. Add a unit test that asserts the chosen point increases distance to
  the nearest enemy (see Testing).
- **Heavy intentional warning suppression.** Per `CLAUDE.md`, ~50 warning classes
  are disabled globally. Do not "tidy" surrounding patterns you didn't touch
  (e.g. the `dyn_cast` chains, tautological compares). Match the file's existing
  style.
- **No bitwise-move concerns here.** `Vector3`/`Point3` are value types used on
  the stack; this change doesn't `memcpy`/`memmove` anything, so the
  `ClassIsMovableZeroed` pattern is not in play. Don't introduce raw byte copies.
- **`__FILE__` asymmetry** (clang-cl absolute vs GNU-driver repo-relative) only
  matters if you add asserts/logs that print paths. Prefer `AI_ERROR(...)` /
  `LOG_DEBUG(AI, ...)` (already used in this file, e.g.
  AICenterImplPreview.cpp:880) which handle this consistently.
- **Per-target PRIVATE PCH.** `AIArcadeActions.inc` is an `.inc` compiled into the
  AI translation units; you should not need new includes (`AICenter.hpp`,
  `Landscape.hpp`, `Vector3` are already visible here). If you do add an include,
  remember PCH is per-target PRIVATE, not `REUSE_FROM`.
- **`FileSize` target.** It warns >3000 / errors >5000 lines. Check the current
  size of `AIArcadeActions.inc` before adding ~40-60 lines; if it's already near
  the warn threshold, factor the candidate-scoring into a small static helper
  rather than inlining a large block.
- **Performance.** Keep candidate count low (≤8). `FindFleePoint` runs on the
  flee *transition*, not every frame, but multiple routed groups can hit it in
  the same tick. Each `GetExposurePessimistic` is an O(1) grid read, so a handful
  is negligible.

## Testing

1. **Catch2 unit test** (`tests/unit/`, suite likely `PoseidonTests` or a new AI
   case). The hard dependency is `AICenter` + an exposure `_map`; if that's heavy
   to stand up, factor the *pure* geometry/scoring into a free helper, e.g.
   `int PickBestFleeCandidate(Vector3 leader, Vector3 enemy, ArrayOfCandidates,
   CostFn)` taking the cost field as a `std::function`/callback so the test can
   inject a synthetic field. Assert:
   - with a single enemy and a flat "all equal" cost field, the chosen point
     increases distance to the enemy (direction sign correct);
   - with a cost field that is cheap in one quadrant, the chosen point lands in
     that quadrant even if slightly off the pure-away bearing;
   - degenerate input (enemy coincident with leader / no enemy) returns the
     waypoint-0 fallback.
   Build + run:
   ```sh
   cmake --preset win-x64-clang-rwdi
   cmake --build build/win-x64-clang-rwdi
   ctest --test-dir build/win-x64-clang-rwdi -R PoseidonTests --output-on-failure
   ```
2. **Trident integration** (`tests/integration/`, needs Demo game data). Author an
   SQF scenario: place a weak squad in open ground with a strong enemy squad to
   one side and a hill/treeline to the opposite side, force a rout
   (`allowFleeing`/morale damage), then after N seconds assert the squad's
   leader position moved *away* from the enemy and onto lower-exposure terrain.
   ```sh
   cargo build --manifest-path engine/Trident/Cargo.toml
   tri test -j6 --retries 2 tests/integration -k flee   # adjust selector to the new test
   ```
3. **In-game (fastest qualitative check).** Run the `win-x64-clang-rwdi` client,
   open the editor, place a small squad next to a stronger enemy, start, and watch
   the rout: the flee marker/destination should point away from the enemy toward
   cover, not back toward the squad's start. Compare against an unmodified build to
   confirm the behavior change.

## Scope estimate

- **Core change:** ~40-80 lines inside `FindFleePoint()` plus an optional ~20-line
  static scoring helper. Roughly half a day to implement, half a day to tune
  radii/spread in the editor.
- **Unit test:** a few hours if you extract the pure scoring helper (recommended);
  longer if you try to stand up a full `AICenter`.
- **Integration test:** half a day to author a reliable SQF rout scenario.

**Minimal first slice for a fast visible win:** in `FindFleePoint`, after the
existing supply-vehicle loop, find the single nearest enemy, generate just
**three** candidates along `away` at one radius (~75 m), snap Y with
`SurfaceYAboveWater`, score with `GetExposurePessimistic + dist*0.1`, and return
the best of {best supply vehicle, best cover candidate, waypoint 0}. No config, no
multi-enemy averaging. This alone flips the headline behavior — squads flee away
from the enemy — and is editor-verifiable immediately. Layer ring radii, wider
spread, and multi-enemy bisector handling afterward.
