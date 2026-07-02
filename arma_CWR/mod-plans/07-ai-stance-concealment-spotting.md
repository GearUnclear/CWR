# Stance- and concealment-weighted infantry spotting

*Make a stationary prone soldier in cover dramatically harder for AI to detect by amplifying the existing stance and concealment terms in the spotting math.*

## Summary

The OFP/Poseidon detection pipeline already feeds two stance-relevant signals into the
AI's "how spottable is this target" calculation: a per-move `visibleSize` weight (small
for prone/crouch, large for standing/running) and a `GetHidden()` concealment factor that
samples nearby cover. This mod re-weights those signals so that going prone, holding
still, and using cover compound into a much larger detection penalty for the observer.

Observable result: an AI rifleman that today picks out a prone, motionless enemy in
bushes at ~150 m will instead take noticeably longer (or fail) to acquire that target,
while a standing or moving target is detected about as fast as before. Crouching gives a
partial benefit between the two. The change is purely in the perception math — no new
animations, models, or art.

## Why it's interesting

- **Tactical depth for an AI-focused modder.** Stance and cover become genuinely
  meaningful against AI, not just against human players. "Stop, drop, and hide" turns
  into a survivable tactic, which changes how firefights and ambushes play out.
- **Pure engine/AI work.** It lives entirely in two C++ functions plus optional config,
  exactly the kind of change favored here (no assets, no SQF required to ship).
- **Self-contained and tunable.** The whole effect is a handful of multipliers gated on
  stance and distance; easy to expose as config so the balance can be iterated without
  recompiling.

## Difficulty & prerequisites

**Intermediate.** You need to be comfortable reading the legacy soldier/perception code
and reasoning about a multiplicative spotting chain, but the edit surface is tiny and
localized.

**No art assets required.** This reuses the existing per-move `visibleSize` config values
and the existing `GetHidden()` cover sampling. Nothing in this plan adds or modifies a
model, texture, animation, or sound. Confirmed against the source below.

## Key files & entry points

- `C:/dev/arma_CWR/engine/Poseidon/World/Entities/Infantry/SoldierOldActions.cpp`
  - `Man::VisibleMovement()` — lines 2270-2290. Multiplies `base::VisibleMovement()` by
    the primary move's `GetVisibleSize()` and by `(1 - _hideBody)`.
  - `Man::GetHidden()` — lines 2308-2311. Returns `1 - _surround.Track(this, Position(), 50, 0.5f)`
    (1 = fully visible, lower = more concealed).
  - `Man::IsDown()` — lines 2145-2148, delegating to free function `::IsDown(int pos)`
    at lines 2136-2139 (true for `ManPosLying`, `ManPosBinocLying`, `ManPosLyingNoWeapon`,
    `ManPosHandGunLying`).
  - `Man::VisibleSize()` — lines 2313-2325 (parallel size term; same `visibleSize`
    weighting, useful to keep consistent if you touch the size path).
- `C:/dev/arma_CWR/engine/Poseidon/World/Detection/Target.cpp`
  - The spotting block at lines 956-1011. `movement = ai->VisibleMovement()` is read at
    line 824; `hidden = ai->GetHidden()` at line 971; the close-range camouflage falloff
    is lines 972-982; `spotCoef` is assembled at lines 987-1011 (optics branch 992-998,
    naked-eye branch 1000-1006).
- `C:/dev/arma_CWR/engine/Poseidon/AI/VehicleAIPilot.cpp`
  - `EntityAI::VisibleMovement()` — lines 2292-2306, the camouflage-based base that
    `Man::VisibleMovement()` calls via `base::VisibleMovement()`.
- `C:/dev/arma_CWR/engine/Poseidon/World/Entities/Infantry/SoldierOld.hpp`
  - `MoveInfo` class (line 138); `_visibleSize` field (line 162); accessor
    `GetVisibleSize()` (line 226). `ManPos*` stance enum at lines 458-476
    (`ManPosLying`, `ManPosCrouch`, `ManPosCombat`, `ManPosStand`, ...).
- `C:/dev/arma_CWR/engine/Poseidon/World/Entities/Infantry/SoldierOldSimProxy.cpp`
  - `_visibleSize = entry >> "visibleSize";` at line 1260 — where the per-move config
    value is loaded (this is the existing data knob the stance weighting rides on).

## How it works today

1. **Per-target spottability** is computed in `Target.cpp`. Around line 824 it reads
   `movement = ai->VisibleMovement()`, the observed target's effective visual
   "loudness".
2. **`Man::VisibleMovement()`** (SoldierOldActions.cpp:2270) starts from
   `base::VisibleMovement()` (the camouflage/speed term in
   `EntityAI::VisibleMovement()`, VehicleAIPilot.cpp:2292) and then multiplies by:
   - `pri->GetVisibleSize()` — the primary move's `visibleSize` config value. Prone and
     crouch moves carry a small value; standing/running moves a larger one. This is the
     stance signal already present in the data.
   - `(1 - _hideBody)` when `_hideBody > 0` — a separate "lie flat behind a wall" hide
     factor (`HideBody()` sets `_hideBodyWanted = 1`, SoldierOld.hpp:642).
3. **`Man::GetHidden()`** (SoldierOldActions.cpp:2308) returns `1 - cover`, where `cover`
   comes from `_surround.Track(...)` sampling nearby occluders within 50 m. Higher cover
   → lower `hidden` → harder to see.
4. **`Target.cpp` folds both together** at lines 956-1011:
   - It applies a close-range camouflage falloff (lines 972-982): inside
     `patternFullDistance = 60 m`, concealment is weakened, and weakened *further* the
     faster the target is moving (`moveFactor` from `movement`). So camo/cover matters
     most at distance and when still.
   - In both the optics branch (992-998) and naked-eye branch (1000-1006), the final
     `spotCoef = movement * <sensitivity/optics/light> * hidden`. So `movement` (which
     already contains the `visibleSize` stance weight) and `hidden` multiply directly
     into spottability.

Net effect today: stance and cover *do* help, but the `visibleSize` deltas between
stances are modest and `hidden` contributes linearly with no extra distance emphasis, so
a prone hidden soldier is only somewhat harder to spot than a standing one.

## Implementation approach

The goal is to (a) widen the stance gap by down-weighting `visibleSize` for "down"
stances, and (b) make concealment bite harder at range. All multipliers should default to
the current behavior (1.0) so the mod is opt-in via config and easy to A/B.

1. **Add a stance-aware visibleSize weighting in `Man::VisibleMovement()`**
   (SoldierOldActions.cpp:2270-2290). After `vis *= pri->GetVisibleSize();` (line 2281),
   apply an extra stance multiplier based on `GetActUpDegree()`:
   - prone (`IsDown()` true, or explicitly `ManPosLying` family) → multiply by a small
     factor, e.g. `_stanceVisProne` (default ~0.45).
   - crouch (`ManPosCrouch` / `ManPosHandGunCrouch` / `ManPosCombat`) → an intermediate
     factor, e.g. `_stanceVisCrouch` (default ~0.7).
   - standing → 1.0 (unchanged).
   Reuse the existing `::IsDown(int)` helper (line 2136) for the prone test and compare
   `GetActUpDegree()` against the `ManPos*` enum (SoldierOld.hpp:458) for crouch. Keep
   the existing `_shootVisible > 1` early-out (lines 2272-2275) untouched so firing units
   still light up regardless of stance.

2. **Couple the stance weight to stillness.** The strongest tactical payoff is a
   *stationary* prone target. `base::VisibleMovement()` already raises visibility with
   speed (EntityAI::VisibleMovement, VehicleAIPilot.cpp:2298-2304), so a still soldier is
   near the camouflage floor. Apply the prone/crouch reduction only when the unit is at
   or near zero speed (read `Speed().SizeXZ()` as in `Man::Audible()`,
   SoldierOldActions.cpp:2300), lerping the stance factor back toward 1.0 as speed rises.
   This guarantees "crawl fast and you're still visible" while "freeze and you vanish".

3. **Strengthen `GetHidden()` at distance in `Target.cpp`** (lines 970-982). The current
   block *weakens* concealment at close range. Add a complementary term that *amplifies*
   concealment beyond `patternFullDistance` for low-`hidden` (well-covered) targets, e.g.
   raise `hidden` toward 0 with an exponent or extra gain past 60 m:
   - keep the existing close-range falloff exactly as-is (don't regress CQB),
   - for `dist2 >= Square(patternFullDistance)`, compute a distance gain
     `g = clamp((dist - 60) / farDistance, 0, 1)` and push concealed targets harder:
     `hidden = hidden * (1 - concealGain * g * (1 - hidden))` (only bites when `hidden < 1`,
     i.e. the target actually has cover).
   - gate the whole thing on the new `concealGain`/`farDistance` config so default builds
     are unchanged.

4. **Thread config through.** Add fields to the soldier/move type so the four numbers
   (`stanceVisProne`, `stanceVisCrouch`, `concealGain`, `concealFarDistance`) are
   data-driven rather than hardcoded. The natural home is alongside the existing
   `visibleSize` load in `SoldierOldSimProxy.cpp:1260` (per-move) or a CfgVehicles-level
   read for the global tuning constants. If you want a zero-config first slice, hardcode
   the defaults and skip this step (see Scope estimate).

5. **Keep `VisibleSize()` consistent (optional).** `Man::VisibleSize()`
   (SoldierOldActions.cpp:2313-2325) uses the same `visibleSize` weighting for the size
   path used elsewhere (e.g. aim/lead). If you want stance to also affect how big a target
   *reads* for tracking, apply the same stance multiplier there; otherwise leave it so the
   change is scoped strictly to detection.

## Config / data / SQF touchpoints

- **Existing data already drives stance**: each move's `visibleSize` is read at
  `SoldierOldSimProxy.cpp:1260` (`entry >> "visibleSize"`). You can get a milder version
  of this entire mod with *no code* by lowering `visibleSize` on prone/crouch move
  definitions in config — useful as a sanity check that the signal flows through to
  spotting before you touch C++.
- **New tuning constants** (step 4) are best surfaced as config so balance iterates
  without recompiles: e.g. `stanceVisProne`, `stanceVisCrouch` per move or per soldier
  class; `aiConcealGain`, `aiConcealFarDistance` as global/AI-type values near the other
  detection constants used in `Target.cpp`.
- **SQF observability**: no SQF change is required, but `knowsAbout`, `reveal`,
  `setUnitPos` ("UP"/"MIDDLE"/"DOWN"), and unit `setBehaviour`/`disableAI "MOVE"` are the
  natural scripting handles for building deterministic detection tests (see Testing).

## Risks & gotchas

- **Multiplicative chain is sensitive.** `spotCoef` in `Target.cpp` is a product
  (`movement * ... * hidden`). Pushing both `movement` (via stance) and `hidden` (via
  distance gain) down at once multiplies, so effects compound fast — start with mild
  defaults and tune. Watch `saturateMin(actSpotability, 4)` at Target.cpp:1025 and the
  ear-fallback at 1016-1023 (which forces `hidden = 1`): sound can still give a hidden
  unit away, which is realistic but means the mod won't make a noisy crawler invisible.
- **Don't regress close-range.** Leave the existing 0-60 m camouflage falloff
  (Target.cpp:972-982) intact; only add the far-distance amplification so CQB stays
  lethal.
- **Heavy intentional warning suppression.** Per CLAUDE.md, ~50 warning classes are off
  globally and many "odd" patterns are deliberate — don't refactor surrounding legacy
  code while you're in there; make the smallest possible edit.
- **`ClassIsMovableZeroed` / memcpy-movable types.** `Man`/`MoveInfo` and friends live in
  this legacy memory model. Add only plain scalar (`float`/`int`) fields for the new
  config knobs; do not introduce members with non-trivial constructors/destructors into
  these classes, and don't reorder existing members.
- **clang-cl vs GNU `__FILE__` asymmetry.** Irrelevant to the math, but if you add asserts
  or logging, remember test helpers walk an absolute `__FILE__` on clang-cl.
- **Per-target PRIVATE PCH.** `SoldierOld*.cpp` and `Target.cpp` are PCH targets; if you
  add a new header for config constants, make sure it's reachable through the existing PCH
  / includes rather than fighting the per-target PCH setup.
- **FileSize target.** `SoldierOldActions.cpp` is already large; the `FileSize` target
  warns >3000 and errors >5000 lines. Keep additions minimal and consider putting any
  helper in a small inline rather than bloating the file.
- **Determinism / MP.** Detection runs server-side per observer; keep the new terms pure
  functions of unit state (stance, speed, cover, distance) so behavior stays consistent
  and serialization (`SoldierOldAI.cpp` hideBody serialize at 2274) isn't affected — the
  new fields are config, not per-tick mutable state, so no extra netcode is needed.

## Testing

- **Unit (Catch2, `tests/unit`).** The detection suite today is a compile stub
  (`tests/unit/engine/Poseidon/World/Detection/test_detector.cpp`). Add a focused test
  that exercises the new math directly: construct/mocked inputs and assert that, holding
  everything else equal, a prone+still+covered configuration yields a strictly lower
  `spotCoef`/`VisibleMovement()` than standing+moving, and that with default config the
  output equals the legacy formula (regression guard). Tag it `[detection]` and run:
  ```sh
  ctest --test-dir build/win-x64-clang-rwdi -R detector --output-on-failure
  ```
  If `Man::VisibleMovement()` is hard to unit-test in isolation, factor the new
  stance/conceal weighting into a small free/inline function taking (stance, speed,
  hidden, distance) and unit-test that pure function.
- **Integration (Trident, `tests/integration`).** Add an SQF scenario under
  `tests/integration/scripting/` (mirror the `*.test.sqf` + `*.test.toml` pair, e.g.
  `event_handler_damage_repair.test.toml`: `binary = "PoseidonGame"`, a `.Demo` mission,
  `timeout`, `extra_args = ["--dev"]`). Spawn an observer and a target at fixed range,
  `disableAI "MOVE"` on both, `setUnitPos "DOWN"` the target near cover, then poll
  `observer knowsAbout target` over time and assert it stays below a threshold for longer
  than the same scenario with `setUnitPos "UP"`. Run:
  ```sh
  cargo build --manifest-path engine/Trident/Cargo.toml
  tri test -j6 --retries 2 tests/integration/scripting
  ```
- **In-game (manual, `win-x64-clang-rwdi`).** Build and run the client:
  ```sh
  cmake --preset win-x64-clang-rwdi
  cmake --build build/win-x64-clang-rwdi
  ```
  Place an enemy AI rifleman ~150 m away facing you; compare time-to-detection standing
  vs. crouched vs. prone-in-bushes-and-still. Use the camera/console
  `player knowsAbout enemy` readout to quantify. Verify a prone target that *crawls* is
  still detected promptly (step 2 stillness coupling).

## Scope estimate

- **Minimal first slice (fast visible win, ~half a day):** Hardcode the stance multiplier
  in `Man::VisibleMovement()` (step 1) only — prone ×0.45, crouch ×0.7, standing ×1.0,
  no config, no Target.cpp change. This alone widens the stance gap and is immediately
  observable via `knowsAbout` at range. Confirm with a quick in-game A/B.
- **Full feature (~1-2 days):** Add stillness coupling (step 2), the distance-amplified
  concealment in `Target.cpp` (step 3), config plumbing for the four constants (step 4),
  the Catch2 regression/behavior test, and one Trident scenario.
- **Risk-adjusted:** Budget extra time for balance tuning — because the terms multiply,
  the "feel" pass (finding values that help defenders without making AI blind) is likely
  the longest part. Keep defaults at legacy behavior so you can ship the code path dark
  and tune via config.
