# Tunable AI memory decay (fog-of-war forgetting)

*Turn the engine's three hardcoded "forgetting" curves into config knobs, so AI groups can range from "never lose track for 10 minutes" to "go blind the instant they lose line-of-sight."*

## Summary

Every AI group keeps a list of `Target` records — its working memory of who it has seen, where, how sure it is of the type/side, and how "spotable" the contact still is. That memory does not stay sharp forever: it decays on time-based curves so that, after a grace window, a contact the group can no longer observe slowly fades out of awareness. Today those curves are **hardcoded constants** in `Target.cpp`.

This mod exposes the three decay slopes and the grace window (`ValidTime`) as engine-read config values with defaults equal to the current numbers, so behavior is byte-for-byte identical until a mission/config overrides them. A mission maker (or a difficulty preset) can then dial in:

- **Sticky memory** — slow slopes / long grace: an alerted patrol hunts a contact for minutes after losing sight, suppressing the last-known position relentlessly.
- **Goldfish memory** — steep slopes / near-zero grace: break line-of-sight for a few seconds and the AI genuinely loses the plot, enabling stealth gameplay.

**Observable result:** with a fast-decay config, `group knowsAbout target` (the SQF query backed by `FadingSideAccuracy()`) drops toward 0 within seconds of an LOS break; with a slow-decay config it stays high far longer. In a `_ENABLE_CHEATS` build the on-screen cursor debug readout (`uncertainity`, `spotability`) visibly decays at the configured rate. Both are directly testable.

## Why it's interesting

This is one of the highest-leverage AI tuning points in the whole engine, and it is currently three magic numbers. Memory decay governs how long the AI suppresses a position you fled, how quickly a flanking maneuver "resets" enemy awareness, and whether stealth missions are even viable. Exposing it lets a beginner ship multiple dramatically different AI feels (aggressive bloodhounds vs. forgetful conscripts) from pure config, with zero new assets and a tiny, well-contained C++ change. It is the canonical "small surface, big behavioral payoff" mod.

## Difficulty & prerequisites

**Beginner.** The core change is ~30 lines in one `.cpp`, plus a config class. Prerequisites:

- A working `win-x64-clang-rwdi` build (see `CLAUDE.md`).
- Comfort reading C++ and the engine's `Pars >> "Class"` config idiom.
- Understanding of the engine `ParamEntry::ReadValue<T>(name, default)` helper.

**No art assets required.** No models, textures, sounds, or UI art. This is pure C++ + config. Confirmed off-the-table items (3D models / textures) are not touched.

## Key files & entry points

- **`C:/dev/arma_CWR/engine/Poseidon/World/Detection/Target.cpp`** — the heart of the change.
  - `const float ValidTime = 5.0f;` (line 43) — the grace window added in all three functions.
  - `Target::FadingSpotability() const` (lines 47–57) — uses slope `1.0f / 30`.
  - `Target::FadingAccuracy() const` (lines 59–64) — uses slope `1.0f / 240`.
  - `Target::FadingSideAccuracy() const` (lines 66–71) — uses slope `1.0f / 240`.
- **`C:/dev/arma_CWR/engine/Poseidon/AI/VehicleAI.hpp`** — `struct Target` declaration (lines 256–322); the three `Fading*` decls are at lines 294–296. Member fields involved: `spotability`/`spotabilityTime`, `accuracy`/`accuracyTime`, `sideAccuracy`/`sideAccuracyTime` (lines 267–272). **Do not add per-Target fields here** (see Risks).
- **`C:/dev/arma_CWR/engine/Poseidon/Game/Commands/GameStateExt.cpp`** — `knowsAbout` operator registered at line 1263 (`GameOperator(GameScalar, "knowsAbout", function, GrpKnowsAbout, ...)`); forward decl line 672. This is the SQF observable surface.
- **`C:/dev/arma_CWR/engine/Poseidon/Game/Commands/GameStateExtGrp.cpp`** — `GrpKnowsAbout` (lines 1024–1046) returns `target->FadingSideAccuracy()`. Your config change flows straight into this command — no edits needed here, it's the test hook.
- **`C:/dev/arma_CWR/engine/Poseidon/IO/ParamFile/ParamFile.hpp`** — `ParamEntry::ReadValue<Type>(const char* name, const Type& defVal)` (lines 161–173). The clean "read with default" helper you will use.
- Reference for the config-read idiom: `Pars >> "CfgWorlds"` etc. throughout `World/WorldInit.cpp`; the `GET_PAR(x)` macro pattern in `AI/VehicleAI.cpp` (lines 312, 359, 392–420).

Callers that consume the faded values (no edits needed, but useful to understand reach):
`AI/AIGroupImpl.cpp` (target ranking, lines 626–634, 782–798), `AI/AICenterImplPreview.cpp` (lines 764–874), `UI/InGame/InGameUIDrawCursor.cpp` (debug readout, lines 632–638), `UI/Map/UIMapMain.cpp` (map marker visibility), `UI/InGame/InGameUIMenuSim.cpp` (command-menu target listing).

## How it works today

`Glob.time` is the global sim clock. Each `Target` stamps the time of its last good observation into `spotabilityTime` / `accuracyTime` / `sideAccuracyTime`. The `Fading*` accessors compute how stale that stamp is and scale the stored value down:

```cpp
const float ValidTime = 5.0f;              // grace window, shared by all three

float Target::FadingSpotability() const
{
    float old = Glob.time - spotabilityTime - ValidTime - 1;   // seconds past grace
    if (old < 0) return spotability;                           // still fresh -> full value
    float ret = spotability - old * (1.0f / 30);               // linear decay, 1/30 per s
    saturate(ret, 0, 4);                                       // clamp to [0,4]
    return ret;
}

float Target::FadingAccuracy() const
{
    float fade = 1 - (Glob.time - accuracyTime - ValidTime - 1) * (1.0f / 240); // ramp 0..1
    saturate(fade, 0, 1);
    return accuracy * fade;                                    // full decay over 240 s
}

float Target::FadingSideAccuracy() const  // identical, on sideAccuracyTime
{
    float fade = 1 - (Glob.time - sideAccuracyTime - ValidTime - 1) * (1.0f / 240);
    saturate(fade, 0, 1);
    return sideAccuracy * fade;
}
```

Key behaviors observed:

- The effective grace is `ValidTime + 1` = **6 seconds** (note the extra `- 1` literal). Within that window the value is returned undecayed.
- **Spotability** decays as a *subtractive* slope (`spotability - old/30`): at full `spotability` it takes ~`spotability * 30` seconds to hit zero. It clamps to `[0,4]`.
- **Accuracy** and **SideAccuracy** decay as a *multiplicative* ramp that reaches 0 exactly **240 s** after the grace window, then clamps. `FadingSideAccuracy()` is what `knowsAbout` reports.
- `MinVisibleFire = 0.63f` (line 45) is a separate threshold, unrelated to decay; leave it alone.

So today there is exactly one global behavior for every unit, side, and difficulty. There is **no** `CfgAISkill` consumed by the engine (a grep for `CfgAISkill` across `engine/` finds zero references) — the classic OFP skill-config class is absent here, so this mod introduces its own config class rather than extending a non-existent one.

## Implementation approach

The cleanest design keeps the per-`Target` data layout untouched and pulls the tunables from a small global struct populated once at config load. Reading `Pars >> ...` on every `Fading*` call would be wasteful (these run in AI hot loops), so cache.

1. **Define a decay-params struct + global accessor.** In `Target.cpp` (top, after the includes / namespace open), add:
   ```cpp
   struct TargetDecayParams
   {
       float validTime         = 5.0f;        // matches const ValidTime
       float spotabilitySlope  = 1.0f / 30;   // per second
       float accuracySlope     = 1.0f / 240;  // per second
       float sideAccuracySlope = 1.0f / 240;  // per second
   };
   static TargetDecayParams GTargetDecay;     // defaults == legacy behavior
   ```
   Defaults are exactly the current literals, so an unmodded run is identical.

2. **Add a one-time loader** that reads the config class with safe fallbacks via `ReadValue`:
   ```cpp
   void LoadTargetDecayParams()
   {
       const ParamEntry& root = Pars;
       if (root.FindEntry("CfgAIMemory"))
       {
           const ParamEntry& c = root >> "CfgAIMemory";
           GTargetDecay.validTime         = c.ReadValue("validTime",         5.0f);
           GTargetDecay.spotabilitySlope  = c.ReadValue("spotabilitySlope",  1.0f / 30);
           GTargetDecay.accuracySlope     = c.ReadValue("accuracySlope",     1.0f / 240);
           GTargetDecay.sideAccuracySlope = c.ReadValue("sideAccuracySlope", 1.0f / 240);
       }
       else
       {
           GTargetDecay = TargetDecayParams{};  // reset to legacy defaults
       }
   }
   ```
   Confirm the exact `ParamEntry` API at `ParamFile.hpp:161` (`ReadValue<Type>`) and the `Pars` global / `FindEntry` usage as seen in `World/WorldInit.cpp:843`. Provide a header declaration for `LoadTargetDecayParams()` (e.g. in `AI/VehicleAI.hpp` near the `Target` struct, or a small new header) so the call site can reach it.

3. **Call the loader once after config is parsed.** Hook it into world/config init where other `Pars >>` reads happen — e.g. alongside the `CfgWorlds` reads in `World/WorldInit.cpp` (around lines 843–885) or wherever global AI config is first available. If a single guaranteed call site is awkward, a lazy `static bool loaded` guard inside the accessor is acceptable for a beginner slice (note: re-init on mission change then needs an explicit reset).

4. **Rewrite the three `Fading*` functions** to read from `GTargetDecay` instead of literals. Keep the `+ 1` (or fold it into `validTime` — your call; document whichever you pick):
   ```cpp
   float Target::FadingSpotability() const
   {
       float old = Glob.time - spotabilityTime - GTargetDecay.validTime - 1;
       if (old < 0) return spotability;
       float ret = spotability - old * GTargetDecay.spotabilitySlope;
       saturate(ret, 0, 4);
       return ret;
   }

   float Target::FadingAccuracy() const
   {
       float fade = 1 - (Glob.time - accuracyTime - GTargetDecay.validTime - 1)
                        * GTargetDecay.accuracySlope;
       saturate(fade, 0, 1);
       return accuracy * fade;
   }

   float Target::FadingSideAccuracy() const
   {
       float fade = 1 - (Glob.time - sideAccuracyTime - GTargetDecay.validTime - 1)
                        * GTargetDecay.sideAccuracySlope;
       saturate(fade, 0, 1);
       return sideAccuracy * fade;
   }
   ```
   Remove or keep `const float ValidTime = 5.0f;` (line 43) — if other code in `Target.cpp` still references `ValidTime`, leave the constant and just stop using it inside the three functions to avoid breaking those references. Grep `ValidTime` within the file before deleting.

5. **(Optional) A semantic helper:** convert slopes to designer-friendly "seconds to forget." A slope of `1/T` means full decay over `T` seconds, so let the config express `accuracyForgetTime = 240` and compute `slope = 1.0f / max(0.01f, T)`. This is far more intuitive for mission makers than raw slopes. Pick one representation and document it.

6. **(Optional, stretch) Per-side / per-difficulty variants.** Because `Target` knows its `group` (`Target::group`, `VehicleAI.hpp:287`) and `side`, you could select a different `TargetDecayParams` per side or per global difficulty. Keep this out of the first slice — it widens the config surface and the test matrix.

## Config / data / SQF touchpoints

- **New config class `CfgAIMemory`** (top-level, read via `Pars`). Example a mission's `description.ext` or a config addon could ship:
  ```cpp
  class CfgAIMemory
  {
      validTime         = 5.0;     // grace seconds before any decay
      spotabilitySlope  = 0.0333;  // 1/30; bigger = forgets faster
      accuracySlope     = 0.0042;  // 1/240
      sideAccuracySlope = 0.0042;  // 1/240
  };
  ```
  "Goldfish" preset: `validTime = 0.5; accuracySlope = 0.5; sideAccuracySlope = 0.5; spotabilitySlope = 1.0;` (forgets in ~1–2 s).
  "Bloodhound" preset: `validTime = 30; accuracySlope = 0.0017; sideAccuracySlope = 0.0017; spotabilitySlope = 0.005;` (~10 min memory).
- **SQF observable (no new command needed):** `group knowsAbout target` is registered at `GameStateExt.cpp:1263` and returns `FadingSideAccuracy()` (`GameStateExtGrp.cpp:1045`). This already exposes the decayed value to scripting — perfect for both gameplay and tests. Sampling `knowsAbout` over time *is* the decay curve.
- **No serialization surface change:** because we add only a static/global struct (not `Target` members), `Target::Serialize` / saved games are untouched.

## Risks & gotchas

- **Do NOT add fields to `struct Target`.** `Target` uses `USE_FAST_ALLOCATOR` (`VehicleAI.hpp:321`) and has a hand-written `Serialize` / `SaveRef` / `LoadRef`. The engine's bitwise-movable (`ClassIsMovableZeroed`) `memcpy`/`memmove` pattern and the fast allocator mean blindly adding members risks both serialization drift and movability assumptions. Keeping tunables global sidesteps all of it.
- **Hot path cost:** `Fading*` are called in AI ranking loops (`AIGroupImpl.cpp:626–798`, `AICenterImplPreview.cpp`). Reading `Pars >>` per call would be expensive — that is why we cache in `GTargetDecay`. Don't reintroduce per-call config lookups.
- **Mission reload / config reparse:** ensure `LoadTargetDecayParams()` re-runs (or resets to defaults) when a new mission loads, otherwise a sticky-memory test pollutes the next mission. A `static bool loaded` lazy guard must be reset on world teardown.
- **The `- 1` literal** in all three functions silently extends the grace by 1 s. Decide whether to fold it into `validTime` or keep it; mismatching this between functions or vs. the config doc causes confusing off-by-one decay timing.
- **Intentional warning suppression:** the repo globally disables ~50 clang warning classes on purpose (see `CMakeLists.txt`). A tautological/float-compare pattern near `saturate` may look "wrong" but is deliberate — don't refactor surrounding code to silence warnings.
- **`__FILE__` asymmetry (clang-cl vs GNU):** only relevant if you add asserts/logging that print paths; the repo deliberately keeps `__FILE__` absolute on clang-cl. Not expected to bite here.
- **Per-target PRIVATE PCH:** `Target.cpp` builds under a target with a PRIVATE PCH baking compile defs; just rebuild via the preset, don't try to share PCH.
- **`FileSize` target:** `Target.cpp` is large — keep additions lean; the lint target warns >3000 and errors >5000 lines. Don't dump big tables here.
- **`saturate(ret, 0, 4)`** caps spotability at 4; if you let a config push raw `spotability` higher elsewhere, decay timing changes nonlinearly. Out of scope, but be aware.

## Testing

1. **Unit (Catch2, `tests/unit/`).** The three `Fading*` are pure functions of `Glob.time`, the time stamps, the stored values, and `GTargetDecay`. Add a small test (suite e.g. `PoseidonTests` under `tests/unit/engine/Poseidon`) that constructs a `Target`, sets `accuracy = 1`, `accuracyTime = T0`, sets `GTargetDecay.accuracySlope` to a known value and `validTime` to 0, advances a mocked `Glob.time`, and asserts `FadingAccuracy()` matches the closed-form `1 - (dt) * slope` (clamped). Repeat for spotability (subtractive) and side accuracy. Run:
   ```sh
   ctest --test-dir build/win-x64-clang-rwdi -R PoseidonTests --output-on-failure
   ```
   (Driving `Glob.time` may require the existing test harness's sim-time helper; if direct manipulation isn't exposed, fall back to the integration test below, which is the more faithful check.)

2. **Integration (Trident, `tests/integration/scripting/`).** This is the strongest signal because it exercises the real AI pipeline through `knowsAbout`. Add `ai_memory_decay.test.sqf` + `.test.toml` modeled on `event_handler_damage_repair.test.*`:
   - `.toml`: `binary = "PoseidonGame"`, a mission with an enemy AI group and a target, `extra_args = ["--dev"]`, a `CfgAIMemory` with a steep slope in the mission's `description.ext`.
   - `.sqf`: let the group spot the target, `triSimFrames` enough to stamp a high `knowsAbout`, break LOS (move/hide the target), advance time with `triSimFrames`, then `triAssertLt [(group player knowsAbout target), 0.1]` for the goldfish config. A second test with the bloodhound config asserts `triAssertGt` after the same elapsed time. Run:
   ```sh
   cargo build --manifest-path engine/Trident/Cargo.toml
   tri test -j6 --retries 2 tests/integration/scripting/ai_memory_decay
   ```
   (Requires `OFPR_GAME_DIR` / `OFPR_DATA_DIR` per `CLAUDE.md`.)

3. **In-game manual (cheats build).** Build `win-x64-clang-rwdi`, load a mission, enable the cursor debug overlay (`InGameUIDrawCursor.cpp:632–638` prints `uncertainity`/`spotability` under `_ENABLE_CHEATS`). Watch the numbers decay: with the goldfish `CfgAIMemory` they should crash to 0 within a couple seconds of breaking LOS; with the bloodhound config they should barely move. Confirm an unmodded mission (no `CfgAIMemory`) decays at exactly the legacy rate.

## Scope estimate

- **Core C++ change:** ~30–40 lines in `Target.cpp` (struct + loader + three rewrites), 1–2 lines for the loader declaration, 1 line at the init call site. Half a day including a build.
- **Config presets + docs:** ~1 hour.
- **Tests:** unit ~1–2 h, one integration pair ~2–3 h (mission setup dominates).

**Suggested minimal first slice (fast visible win):** Skip the config class entirely for v0. Add the `GTargetDecay` struct with default values, wire the three `Fading*` to read it, and temporarily hard-set `GTargetDecay.accuracySlope = 0.5f` etc. in the loader (or behind a `_ENABLE_CHEATS` toggle). Build `win-x64-clang-rwdi`, watch the cursor debug overlay forget a contact in ~2 s. That proves the plumbing end-to-end in an afternoon. Then layer in the `CfgAIMemory` read (`ReadValue` with legacy defaults) so the behavior becomes data-driven and the unmodded path is restored to identical, and finally add the Trident test.
