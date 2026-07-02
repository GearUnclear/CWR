# knownTargets: expose a group's AI target knowledge as an array

*Turn the engine's hidden per-group enemy "blackboard" into a first-class SQF array so missions and scripts can read what the AI actually knows.*

## Summary

Today SQF can ask a group only one yes/no-ish question: `grp knowsAbout obj` returns a single fading accuracy scalar for **one named object** you already have a handle to. But internally every `AIGroup` maintains a full `TargetList` of `Target` structs describing every contact the group has perceived — position, side, type, last-seen time, and several fading "how sure are we" accuracies.

This mod adds a new **unary** SQF command `knownTargets group` that walks the group's live `TargetList` and returns an array of sub-arrays, one per known contact:

```sqf
_targets = knownTargets group player;
// _targets => [ [<object>, <side>, <knowledge 0..1>, [x,y,z]], ... ]
```

Observable result: a mission script (or the debug console) can dump, in real time, exactly which enemies an AI group has detected, which side it thinks they are, how confident it is, and where it last believed they were — without the script having to already hold a reference to each enemy. This is the building block for "the AI knows X" mission logic, reactive triggers, dynamic objectives, and debugging AI perception.

## Why it's interesting

- It exposes a rich AI subsystem that is otherwise completely invisible to scripting. `knowsAbout` only works if you *already* enumerated every candidate object and polled them one at a time; `knownTargets` flips that around and returns the group's own contact list directly.
- It is a pure read/query command: no gameplay balance changes, no new entities, and it cannot desync state because it only reads.
- It is a strong base for AI mods: "spawn reinforcements once the enemy squad has spotted the player," "mark known contacts on the map," "make a group forget a target," AI behavior debugging overlays, etc.
- Low risk, high payoff: a single new command function plus one registration line, modeled almost exactly on existing code (`GrpUnits`, `GrpKnowsAbout`).

## Difficulty & prerequisites

**Intermediate.** You need to:
- Build the engine with a configured preset (`win-x64-clang-rwdi`).
- Understand the `GameFunction(...)` registration tables in `GameStateExt.cpp` and the `GameValue`/`GameArrayType` helpers.
- Be comfortable reading the AI `Target`/`TargetList` structures.

**No art assets required.** This is C++ engine + SQF only. No models, textures, sounds, or config classes are added. Confirmed: the entire change lives in `engine/Poseidon/Game/Commands/` and is verifiable from the in-game debug console.

## Key files & entry points

All paths under repo root `C:/dev/arma_CWR`.

- `engine/Poseidon/Game/Commands/GameStateExtGrp.cpp`
  - `GrpKnowsAbout` at **line 1024** — the existing single-object query; the model for "get a group, pull a `Target`, return its accuracy." It calls `grp->FindTarget(veh)` and returns `target->FadingSideAccuracy()` (line 1045).
  - `ObjGetPos` at **line 1532** — the model for building a 3-element position `GameArray` (`CreateGameValue(GameArray)`, `array.Resize(3)`, fill `X/Z/Y`).
  - This is the natural home for the new `KnownTargets` function (group-domain commands live here).
- `engine/Poseidon/Game/Commands/GameStateExtUi.cpp`
  - `GrpUnits` at **line 802** — the canonical model for building an **array of objects** from a group: `state->CreateGameValue(GameArray)`, `GameArrayType& array = value`, `array.Add(GameValueExt(veh))`.
  - `CreateGameSide(...)` usages at lines 2033–2088 — confirms `CreateGameSide(TargetSide)` is the helper to wrap a side as a `GameValue`.
- `engine/Poseidon/Game/Commands/GameStateExt.cpp`
  - Forward declarations region (`GrpUnits` declared at **line 494**, `GrpKnowsAbout` at **line 672**) — add the new function's prototype here.
  - `knowsAbout` registration at **line 1263**: `GameOperator(GameScalar, "knowsAbout", function, GrpKnowsAbout, GameObjectOrGroup, GameObject)`.
  - The unary `GameFunction(GameArray, "units", GrpUnits, GameGroup)` / `... GameObject)` pair at **lines 977–978** — the exact pattern to copy for a new unary `GameArray`-returning command on a group.
  - `CreateGameObject` defined at **line 291** (`return GameValueExt(obj);`), declared in `GameStateExtCommon.hpp:65`. `CreateGameSide` is an inline in `GameStateExt.hpp:168` (`return GameValueExt(side);`).
- `engine/Poseidon/AI/AIGroup.hpp`
  - `TargetList _targetList;` member at **line 408**.
  - `const TargetList &GetTargetList() const {return _targetList;}` accessor at **line 512** — the read entry point we iterate.
- `engine/Poseidon/AI/VehicleAI.hpp`
  - `struct Target` at **line 256**: fields `position` (258), `posReported` (261), `side` (`TargetSide`, 263), `type` (`const EntityAIType*`, 265), `lastSeen` (273), `isKnown` (280), `vanished` (281), `destroyed` (282), `idExact` (`TargetId`, 284).
  - Methods `FadingSideAccuracy()` / `FadingAccuracy()` (294–295), `IsKnown()` (302).
  - `class TargetList : public RefArray<Target>` at **line 331** — so iteration is `Size()` + `operator[]` over `Ref<Target>` entries.
- `engine/Poseidon/AI/TargetId.hpp`
  - `typedef EntityAI TargetType;` (line 15) and `typedef LLink<TargetType> TargetId;` (line 16) — so `Target::idExact` is an `LLink<EntityAI>` that converts to `EntityAI*` (as used in `LinkTarget::IdExact()`, VehicleAI.hpp:324-328). `EntityAI` derives from `Object`, so it feeds straight into `CreateGameObject`/`GameValueExt`.

## How it works today

`GrpKnowsAbout` (GameStateExtGrp.cpp:1024) is the only scripting window into AI perception:

1. Resolve `grp` from operand 1 via `GetGroup`.
2. Resolve a concrete `EntityAI* veh` from operand 2 via `GetObject` + `dyn_cast<EntityAI>`.
3. `Target* target = grp->FindTarget(veh);` — a lookup keyed by the object you already hold.
4. Return `target->FadingSideAccuracy()` (a 0..1-ish fading confidence), or `0.0f` if no target/group.

It is registered as a binary operator: `knowsAbout` (GameStateExt.cpp:1263). The limitation is structural: the caller must enumerate candidate objects itself and poll one at a time. There is **no** way to ask "what does this group know about?" — even though the answer already exists as `AIGroup::_targetList` (AIGroup.hpp:408), populated by the AI detection system (`AddTarget`, AIGroup.hpp:513). Each `Target` already carries everything we want to expose: the object (`idExact`), the perceived `side`, the fading accuracy, and the last reported position (`posReported`).

## Implementation approach

1. **Add the function** in `engine/Poseidon/Game/Commands/GameStateExtGrp.cpp`, next to `GrpKnowsAbout`. Model it on `GrpUnits` (array build) + `GrpKnowsAbout` (group resolution) + `ObjGetPos` (position sub-array). Sketch:

   ```cpp
   GameValue KnownTargets(const GameState* state, GameValuePar oper1)
   {
       GameValue value = state->CreateGameValue(GameArray);
       GameArrayType& out = value;

       AIGroup* grp = GetGroup(oper1);
       if (!grp)
       {
           return value; // empty array, matches GrpUnits' graceful-empty behavior
       }

       const TargetList& list = grp->GetTargetList();
       out.Realloc(list.Size());
       for (int i = 0; i < list.Size(); i++)
       {
           const Target* tgt = list[i];
           if (!tgt || !tgt->IsKnown())   // skip purely potential/forgotten contacts
           {
               continue;
           }

           EntityAI* obj = tgt->idExact;  // LLink<EntityAI> -> EntityAI*
           if (!obj)
           {
               continue;                  // contact object no longer exists
           }

           // last reported position as SQF [x, y, z] (engine X, Z, Y)
           GameValue posVal = state->CreateGameValue(GameArray);
           GameArrayType& pos = posVal;
           pos.Resize(3);
           Vector3Val p = tgt->posReported;
           pos[0] = p.X();
           pos[1] = p.Z();
           pos[2] = p.Y();

           GameValue entryVal = state->CreateGameValue(GameArray);
           GameArrayType& entry = entryVal;
           entry.Realloc(4);
           entry.Add(CreateGameObject(obj));
           entry.Add(CreateGameSide(tgt->side));
           entry.Add(tgt->FadingSideAccuracy());
           entry.Add(posVal);

           out.Add(entryVal);
       }
       return value;
   }
   ```

   Notes:
   - Use `tgt->posReported` for "last reported position" (the position the group last *believed*), which is the script-meaningful value; `position` is the internal best estimate. Pick one and document it; `posReported` matches the "knowledge" framing.
   - Use the **same** accuracy `GrpKnowsAbout` returns (`FadingSideAccuracy()`) so `knownTargets` and `knowsAbout` agree on the "knowledge" number.
   - Filtering: `IsKnown()` (VehicleAI.hpp:302) is the right gate; optionally also skip `tgt->destroyed`/`tgt->vanished` if you want only live, present contacts. Decide and document the contract.
   - The engine's `Y` is up; SQF positions are `[x, y, z]` where SQF `z` is engine `Y` (height). This mirrors `ObjGetPos` (GameStateExtGrp.cpp:1540-1542 fills `[X, Z, ...]`).

2. **Forward-declare** the function in `engine/Poseidon/Game/Commands/GameStateExt.cpp`, in the declaration block alongside `GrpUnits` (line 494) / `GrpKnowsAbout` (line 672):

   ```cpp
   GameValue KnownTargets(const GameState* state, GameValuePar oper1);
   ```

3. **Register** it as a unary command in the `GameFunction` table in `GameStateExt.cpp`. Place it near the group queries (e.g. after the `units` pair at lines 977–978). Mirror `units` so it accepts both a group and an object (resolving the object's group), since `GetGroup`/`GrpUnits` already handle both:

   ```cpp
   GameFunction(GameArray, "knownTargets", KnownTargets, GameGroup),
   GameFunction(GameArray, "knownTargets", KnownTargets, GameObject),
   ```

   If `GetGroup` does not already accept an object operand the way `GrpUnits` does, drop the `GameObject` overload and keep only `GameGroup`. Verify against `GetGroup`'s implementation before committing.

4. **Build & smoke test** (see Testing). No header changes are required: `VehicleAI.hpp` and `AIGroup.hpp` are already included by `GameStateExtGrp.cpp` (lines 16-17), and `CreateGameSide`/`CreateGameObject` are visible via `GameStateExt.hpp` (line 11 include).

## Config / data / SQF touchpoints

No config classes, no `.pbo` data, no stringtable entries. The only new surface is the SQF command:

- `knownTargets <group>` → array of `[object, side, knowledge, lastReportedPos]`.
- `knownTargets <object>` (if the `GameObject` overload is kept) → same, for the object's own group.

Optional documentation touchpoint: if the repo keeps an SQF command reference list, add `knownTargets` there for discoverability. (Search the repo for where `knowsAbout` is documented, if anywhere; none is required for the command to function.)

## Risks & gotchas

- **`LLink` lifetime.** `Target::idExact` is an `LLink<EntityAI>` (TargetId.hpp:16). A target can outlive its object; always null-check after `EntityAI* obj = tgt->idExact;` (the `LinkTarget::IdExact()` accessor at VehicleAI.hpp:324-328 does exactly this). Returning a `GameValueExt` for a dangling object would be a crash risk.
- **`RefArray<Target>` entries can be null / stale.** `TargetList` derives from `RefArray<Target>` (VehicleAI.hpp:331). Null-check each `list[i]` and use `IsKnown()` to avoid exposing purely "potential" contacts the AI hasn't actually confirmed.
- **Don't mutate.** `GetTargetList()` returns `const TargetList&` (AIGroup.hpp:512). Keep everything `const`; this is a query, not a command. The AI thread/detection code owns target lifetime.
- **Heavy intentional warning suppression.** Per `CLAUDE.md`, ~50 warning classes are disabled globally. Do not "fix" surrounding patterns you see while editing (e.g. tautological compares, intentionally incomplete `switch` in AI code). Match the file's existing style.
- **`ClassIsMovableZeroed` / memcpy on movable types.** `Target` uses `USE_FAST_ALLOCATOR` and is managed via `Ref`/`RefArray`. Never copy a `Target` by value or `memcpy` it; only read its fields. The bitwise-move pattern documented in `CLAUDE.md` is about engine containers — leave it alone.
- **clang-cl vs GNU-driver `__FILE__`.** Irrelevant to runtime logic here, but if you add asserts/logs with `__FILE__`, remember the asymmetry noted in `CLAUDE.md` (absolute on clang-cl, repo-relative on GNU clang).
- **Per-target PRIVATE PCH.** `GameStateExtGrp.cpp` already compiles under the existing PCH config; you are not adding a new TU, so no `POSEIDON_PCH_TARGETS` change is needed.
- **`FileSize` target.** `GameStateExtGrp.cpp` is large. Adding ~40 lines is fine, but if it is already near the 3000-line warn threshold, be aware the `FileSize` target may warn (it errors only above 5000). Check with `cmake --build ... --target FileSize` if concerned.
- **Side semantics.** `Target::side` is a `TargetSide` (a perceived/merged side that may be `TSideUnknown` / `TSideEnemy` etc., not necessarily a clean East/West). That is the *correct* value to expose — it reflects what the AI believes — but document that it can be "unknown."

## Testing

1. **Build:**
   ```sh
   cmake --preset win-x64-clang-rwdi
   cmake --build build/win-x64-clang-rwdi
   ```
   Confirm `GameStateExtGrp.cpp` and `GameStateExt.cpp` recompile cleanly.

2. **Unit (Catch2, `tests/unit/`).** The evaluator/command-table suites live under `PoseidonTests` / `PoseidonEvaluatorTests`. If there is an existing test that exercises a unary `GameArray` command (search the unit tests for `"units"` or `knowsAbout`), add a sibling case parsing `knownTargets grpNull` and asserting it returns an empty `GameArray` (the graceful no-group path). Run:
   ```sh
   ctest --test-dir build/win-x64-clang-rwdi -R "PoseidonEvaluatorTests|PoseidonTests" --output-on-failure
   ```
   A parse/registration-only test (command resolves to the right return type and arity) is valuable even without live AI state.

3. **Trident integration (`tests/integration/`, needs game data).** Author an SQF scenario: spawn an East squad and a West player in line of sight, wait for detection, then assert `count (knownTargets group player) > 0` and that element `[_t select 0]` is the expected enemy and `[_t select 2]` (knowledge) rises over time / equals `(group player) knowsAbout (_t select 0)`. Build and run:
   ```sh
   cargo build --manifest-path engine/Trident/Cargo.toml
   tri test -j6 --retries 2 tests/integration
   ```
   This cross-checks the new command against the existing `knowsAbout` for the same object — the strongest correctness signal.

4. **In-game manual (fastest visual win).** Launch the `win-x64-clang-rwdi` build with Demo data, place the player near an enemy squad, and from the debug console / a radio-trigger script:
   ```sqf
   hint str (knownTargets group player)
   ```
   You should see contacts appear (with rising knowledge values and plausible positions) as the AI spots enemies, and disappear/decay as targets are lost. Compare a single entry's knowledge against `(group player) knowsAbout (_x select 0)`.

## Scope estimate

- **Core engine work:** ~40–60 lines in `GameStateExtGrp.cpp`, 1 forward declaration, 1–2 registration lines in `GameStateExt.cpp`. Roughly **half a day** including a clean build and console smoke test.
- **Tests:** +1–2 hours for a Catch2 empty-array case; +2–4 hours for a Trident detection scenario (most of that is data setup).

**Minimal first slice for a visible win:** implement only the `GameGroup` overload returning `[object, knowledge]` per known target (skip side and position initially). That is the smallest change that compiles, registers, and produces a non-trivial `hint str (knownTargets group player)` in-game. Once that displays live contacts, extend each entry to the full `[object, side, knowledge, lastReportedPos]` shape and add the `GameObject` overload.
