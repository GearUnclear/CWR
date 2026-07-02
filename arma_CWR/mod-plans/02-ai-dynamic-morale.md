# Dynamic morale: courage that reacts to casualties and the wounded leader

*Make an AI squad's willingness to fight rise and fall with the battle instead of being frozen at "the leader's skill level" for the whole mission.*

## Summary

Today an AI group's `_courage` is computed once (whenever the leader changes) as a flat copy of the leader's ability rating. This mod turns courage into a *live* value that is recomputed every Think tick from the current battlefield state:

- **Courage decays as squadmates die** — a half-strength squad becomes visibly more willing to break and run.
- **Courage drops sharply when the leader is wounded** (unconscious) rather than the squad simply ignoring a downed leader as it does now.
- **Courage rises when friendly units are nearby** (optional second slice), so a lone fireteam is jumpier than a platoon fighting shoulder-to-shoulder.
- The existing `forceCourage` mission override and `AllowFleeing()` API still win, so missions that pin courage stay deterministic.

**Observable result:** put a 6-man enemy squad against the player. As you whittle them down, the survivors start fleeing *earlier* (at a higher remaining strength) than in stock, and shooting the squad leader into an unconscious state triggers a rout instead of business-as-usual. All of this is visible without art, just by watching AI behavior.

## Why it's interesting

The flee/rout system already exists and is wired into the FSM (`Flee()` / `Unflee()` in `AIGroupImplHealth.cpp:1585`), but its only dynamic input is `ActualStrength()` vs a *static* threshold. That makes routs feel mechanical: every squad of a given leader breaks at exactly the same strength fraction regardless of how the fight unfolded. Because all the hard parts (strength accounting, flee pathing, FSM transitions, network serialization of `_courage`) are already built, this is a high-leverage change: a few lines in one function convert a constant into a reactive morale curve and noticeably change firefight pacing. It is also a pure-logic AI mod, exactly the genre requested.

## Difficulty & prerequisites

**Intermediate.** You need to be comfortable editing one or two C++ functions in the AI layer and rebuilding the engine with the documented preset. No new systems, no new threads, no netcode design (the field already serializes).

**No art assets required.** This is entirely C++ engine logic plus optional config/SQF tuning. No models, textures, sounds, or animations are touched or needed.

Prereqs: the standard toolchain from `CLAUDE.md` (Clang, Ninja, vcpkg, ccache) and a configured `win-x64-clang-rwdi` build dir. To watch it in-game you need Demo game data (`OFPR_DATA_DIR`).

## Key files & entry points

All real, verified paths/symbols:

- **`engine/Poseidon/AI/AIGroupImplHealth.cpp`**
  - `AIGroup::CalculateCourage()` — **line 1566**. The whole mod's core lives here. Currently 12 lines; the entire body after the `_forceCourage` guard is `_courage = Leader()->GetAbility();` (line 1577).
  - `AIGroup::ActualStrength() const` — **line 1498**. Sums `armor * (1 - dammage) * InvSqrt(armor)` over each live unit (and its assigned vehicle). Reused as-is; good reference for the per-unit damage math.
  - `AIGroup::CalculateMaximalStrength()` — **line 1537**. Same sum without the damage factor; sets `_maxStrength`.
- **`engine/Poseidon/AI/AIGroup.cpp`**
  - `AIGroup::Think(...)` flee decision — **lines 1218-1248**. `leaderAlive` is computed at 1218; the flee test `strength < (1.0 - _courage) * _maxStrength` is at 1232-1233, gated on `leaderAlive` at 1230.
  - `AIGroup::SelectLeader(AIUnit*)` — **line 1050**, calls `CalculateCourage()` at **1059**. This is currently the *only* caller of `CalculateCourage()`, which is why courage is effectively static.
  - Field init: `_maxStrength = 0; _forceCourage = -1; _courage = 1.0;` at **lines 74-76**.
  - Serialization of `maxStrength` / `forceCourage` / `courage` at **lines 284-286** (network/save) — no change needed; `_courage` already travels.
- **`engine/Poseidon/AI/AIGroup.hpp`**
  - Fields `_maxStrength` (424), `_forceCourage` (425), `_courage` (426 area; declared 423-425).
  - `void AllowFleeing(float allow = 1.0f) {_courage = _forceCourage = 1.0f - allow;}` — **line 687**. The override path you must not break.
  - Declarations `ActualStrength()` (724), `CalculateCourage()` (725).
- **`engine/Poseidon/AI/AIUnit.hpp` / `AIUnit.cpp`**
  - `enum LifeState { LSAlive, LSDead, LSDeadInRespawn, LSAsleep, LSUnconscious, NLifeStates }` — `AIUnit.hpp:95-103`. **`LSUnconscious` is the "wounded leader" signal.**
  - `float AIUnit::GetAbility() const` — `AIUnit.cpp:1367`, returns `_ability` in `[0.2, 1]` (or `1` for players / `DTUltraAI`).
  - `person->GetTotalDammage()` (used in `ActualStrength`, returns `0..1`) is the per-unit wound fraction to read for the leader.

## How it works today

1. When a leader is assigned (`SelectLeader`, `AIGroup.cpp:1050`), `CalculateCourage()` runs.
2. `CalculateCourage()` (`AIGroupImplHealth.cpp:1566`):
   - If `_forceCourage >= 0` (mission set it, or `AllowFleeing()` was called), `_courage = _forceCourage` and return.
   - Else if there is a leader, `_courage = Leader()->GetAbility()` — a constant in `[0.2, 1]`.
3. `_courage` then never changes again until the leader changes — casualties, a wounded leader, and nearby reinforcements have **zero** effect on it.
4. Each Think tick (`AIGroup.cpp:1218-1248`), for a non-player group **whose leader is `LSAlive`**: it compares live `ActualStrength()` to the fixed threshold `(1 - _courage) * _maxStrength`. Below it → `Flee()`; above it → `Unflee()`.
5. Note two consequences of the current design:
   - A squad with a *wounded/unconscious* leader (`LSUnconscious`) skips the flee block entirely (gated on `leaderAlive`), so it neither flees nor rallies based on strength.
   - Because the threshold is static, every squad of equal leader ability and `_maxStrength` breaks at the same strength fraction, regardless of how it got there.

## Implementation approach

The cleanest design keeps `_courage` as the single tunable that Think already consumes, and simply makes `CalculateCourage()` reactive plus calls it every tick.

1. **Call `CalculateCourage()` per tick.** In `AIGroup::Think` (`AIGroup.cpp`), just before the flee block at line ~1228 (inside the `else` that handles non-player groups, or unconditionally right after `leaderAlive` is computed at 1218), add a call to `CalculateCourage();`. This is cheap (a handful of loop iterations over `MAX_UNITS_PER_GROUP`) and runs on the local-update path only. Keep the `IsAnyPlayerGroup()` split intact.

2. **Rewrite `CalculateCourage()`** (`AIGroupImplHealth.cpp:1566`) to build courage from a base plus reactive modifiers, while preserving the override:
   ```cpp
   void AIGroup::CalculateCourage()
   {
       if (_forceCourage >= 0) { _courage = _forceCourage; return; }   // unchanged: respect override / AllowFleeing()
       AIUnit* leader = Leader();
       if (!leader) return;

       float courage = leader->GetAbility();          // base: leader skill, as today

       // (a) casualty decay: scale by surviving strength fraction
       float frac = (_maxStrength > 0.0f) ? (ActualStrength() / _maxStrength) : 1.0f;
       // frac in [0,1]; bias so a near-full squad keeps ~full courage, a gutted one loses morale
       courage *= (0.5f + 0.5f * frac);               // tune constants

       // (b) wounded-leader penalty
       if (leader->GetLifeState() == AIUnit::LSUnconscious)
           courage *= 0.4f;                            // tune
       else
       {
           float ldrDmg = leader->GetPerson() ? leader->GetPerson()->GetTotalDammage() : 0.0f;
           courage *= (1.0f - 0.5f * ldrDmg);          // graded penalty for a bloodied leader
       }

       // (c) clamp to the legacy range so threshold math stays sane
       _courage = Clamp(courage, 0.0f, 1.0f);
   }
   ```
   - `ActualStrength()` and `_maxStrength` are already members, computed exactly the way the threshold expects, so casualty decay reuses the engine's own accounting.
   - Verify the clamp helper name in Foundation math before using `Clamp` (search `engine/Poseidon/Foundation/Math`); if absent, inline `min/max`.

3. **Wounded-leader rout (optional but high-value).** Because the flee block is gated on `leaderAlive` (`AIGroup.cpp:1230`), an `LSUnconscious` leader currently freezes morale. To let a wounded leader's squad actually break, widen the gate, e.g. treat `LSUnconscious` as still-eligible-to-flee:
   ```cpp
   bool leaderActive = leader->GetLifeState() == AIUnit::LSAlive
                    || leader->GetLifeState() == AIUnit::LSUnconscious;
   ```
   Keep the existing `leaderAlive` for any other use; only the flee test needs the widened condition. This is the change that makes "shoot the leader → rout" observable.

4. **Friendlies-nearby bonus (second slice).** If you want courage to *rise* with support, after step 2 query nearby friendly groups. The center/stats layer already tracks groups (`AICenterImpl.cpp` iterates `_supportGroups` / `_guardingGroups`); a simpler first cut is to count live units in the leader's own center within a radius using existing world queries. Add a small bounded bonus (`courage *= 1.0f + min(0.3f, 0.05f * nFriends)`). Defer this until slices 1-3 are verified, since it touches more subsystems.

5. **Keep serialization untouched.** `_courage` already serializes (`AIGroup.cpp:286`); since it is now recomputed every tick on the local owner, MP clients stay consistent through the normal update path. Do **not** add new serialized fields unless you introduce persistent state (you don't need to).

## Config / data / SQF touchpoints

- **`forceCourage` already exists as mission data.** It is parsed into the group (`AIGroup.cpp:443`, `format.Add("forceCourage", NDTFloat, NCTFloatM1ToP1, DEFVALUE(float,-1), ...)`) and gates everything via the `_forceCourage >= 0` branch. Missions that set `forceCourage` continue to fully override the new dynamic curve — document this so designers know the escape hatch is intact.
- **SQF `allowFleeing`** maps to `AllowFleeing()` (`AIGroup.hpp:687`), which sets `_forceCourage`. Test that calling it still pins behavior after your change (it should, because step 2 returns early on `_forceCourage >= 0`).
- **Tuning constants** (the `0.5`, `0.4`, radius, friend bonus) are the only knobs. Consider lifting them to named `static const float` at file scope in `AIGroupImplHealth.cpp` so they are easy to find and adjust; optionally read them from a config class entry if you want per-difficulty tuning, but a hardcoded first pass is fine for a visible win.
- No new `.cpp`/`.hpp` files, no new config classes are *required*.

## Risks & gotchas

- **Per-tick cost.** `CalculateCourage()` now runs every Think instead of only on leader change. `ActualStrength()` loops `MAX_UNITS_PER_GROUP` (12) with a few float ops and an `InvSqrt` — negligible, but don't add expensive world scans in slice 1; the friendlies bonus (step 4) is where cost can creep, so bound its radius/count.
- **Don't break the override contract.** The early `return` on `_forceCourage >= 0` must stay first. `AllowFleeing(0)` sets `_forceCourage = 1.0` (never flee) and `AllowFleeing(1)` sets it to `0`; both must keep working unchanged.
- **`GetAbility()` returns 1 for players / `DTUltraAI`** (`AIUnit.cpp:1370`). Player-led groups already skip the flee block (`IsAnyPlayerGroup()` branch, `AIGroup.cpp:1220`), so this is fine, but don't assume `GetAbility() < 1` for the leader.
- **Wounded-leader gate widening (step 3) changes existing behavior** for any squad whose leader is unconscious. Confirm `LSUnconscious` is reachable in your data (some unit types may go straight to `LSDead`); if a squad never enters that state, the rout-on-wound effect won't show — verify with a soldier the player can wound non-lethally.
- **Repo conventions:**
  - Heavy, intentional warning suppression (~50 classes disabled globally) means the build won't warn you about, e.g., float/int mixing in the threshold math — reason about it yourself.
  - The `ClassIsMovableZeroed` memcpy/memmove pattern: `AIGroup` is a refcounted object moved via that pattern in places; don't add members with nontrivial construction to it. You aren't adding members here (good), so this is mainly a "don't" reminder.
  - `__FILE__` asymmetry (clang-cl absolute vs GNU-driver repo-relative) only matters if you add asserts/logs with paths; the AI `AI_ERROR` macro already handles this.
  - PCH is per-target PRIVATE; you're editing existing TUs in the Poseidon target so the PCH already covers your includes — no new include of `AIUnit.hpp` should be needed (it's already used here), but if you add one, expect a rebuild of the PCH-consuming target.
  - `FileSize` target warns at >3000 lines / errors at >5000. `AIGroup.cpp` and `AIGroupImplHealth.cpp` are large; check current line counts before adding much — keep the new logic compact (it is) and prefer editing `CalculateCourage()` in place rather than pasting a big new block.

## Testing

1. **Build:** `cmake --build build/win-x64-clang-rwdi` (configure first with `cmake --preset win-x64-clang-rwdi` if needed).

2. **Catch2 unit (`tests/unit/engine/Poseidon/AI/test_ai.cpp`).** It currently only asserts `sizeof(AIGroup) > 0` plus an arcade-encoding case (`PoseidonTests`, tag `[ai]`). The reactive math is hard to unit-test without standing up a full group, but you can add a *pure-function* test if you refactor the courage formula into a free/static helper, e.g. `float ComputeCourage(float ability, float strengthFrac, bool leaderUnconscious, float leaderDmg)`, and call it from `CalculateCourage()`. Then add a `TEST_CASE("dynamic morale curve", "[ai][morale]")` asserting: full strength + healthy leader ≈ ability; half strength < full; unconscious leader < healthy; `forceCourage` path bypassed. Run:
   ```sh
   ctest --test-dir build/win-x64-clang-rwdi -R PoseidonTests --output-on-failure
   ```
   Filter with the Catch2 tag `[morale]` if you tag it.

3. **Trident integration (`tests/integration`).** Add an SQF scenario under `tests/integration/ingame` (or `scripting`) that spawns an enemy squad vs. a controlled threat, kills members one at a time, and asserts the group enters fleeing state earlier than a baseline. Read group flee state via the existing scripting surface (the `_flee` flag drives observable behavior); `allowFleeing`/`forceCourage` give deterministic control cases. Build and run:
   ```sh
   cargo build --manifest-path engine/Trident/Cargo.toml
   tri test -j6 --retries 2 tests/integration
   ```
   Needs `OFPR_GAME_DIR` + `OFPR_DATA_DIR` per `.trident.env`.

4. **In-game smoke test (the real payoff).** Launch the `win-x64-clang-rwdi` client with Demo data, place a 5-6 man enemy infantry squad and engage:
   - Kill 2-3 members and confirm survivors break/flee at a *higher* remaining strength than stock (compare against an unmodified build, or temporarily `forceCourage` to reproduce old behavior).
   - Wound the squad leader into the unconscious state and confirm the squad routs (requires step 3).
   - Set the group's `forceCourage` in the mission and confirm dynamic morale is fully overridden.

## Scope estimate

- **Slice 1 (fast visible win, ~1-2 hrs):** steps 1-2 only — call `CalculateCourage()` each tick and add casualty decay (`courage *= 0.5 + 0.5*frac`). Squads now break progressively earlier as they take losses. Pure edit of one function + one call site; no new files.
- **Slice 2 (~1 hr):** step 3 — widen the flee gate for `LSUnconscious` and add the wounded-leader penalty. Delivers the dramatic "shoot the leader → rout" moment.
- **Slice 3 (optional, ~2-4 hrs):** step 4 — friendlies-nearby bonus, which touches center/world queries and deserves its own profiling and tests.

Total core effort is small because every dependency (strength accounting, flee FSM, serialization, override path) already exists; the work is turning a constant into a curve and proving it on-screen. Start with Slice 1, verify in-game, then layer in 2 and 3.
