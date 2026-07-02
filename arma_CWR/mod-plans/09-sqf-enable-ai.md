# enableAI / enableAIFeature: re-enable what disableAI turned off

*Give scripters a way to clear AI-disable bits, not just set them — restore MOVE, TARGET, AUTOTARGET, and ANIM behavior at runtime.*

## Summary

The engine exposes `unit disableAI "MOVE"` but has no inverse. `disableAI` only
**ORs** a bit into a per-unit disabled mask, so once a script disables a feature
there is no scripting path to turn it back on (short of deleting/recreating the
unit). This plan adds:

- `unit enableAI "MOVE"` — a binary operator that **clears** the corresponding
  bit (`SetAIDisabled(dai & ~s)`), mirroring `disableAI`.
- (optional) `unit checkAIFeature "MOVE"` — a binary getter returning a `Bool`
  for whether the feature is currently *enabled*, so scripts can branch on it.

Observable result: a previously frozen unit (`u disableAI "MOVE"`) starts moving
again after `u enableAI "MOVE"`; an AI that stopped engaging
(`u disableAI "AUTOTARGET"`) resumes target selection after
`u enableAI "AUTOTARGET"`. Fully testable from SQF and from a Catch2 unit test
that toggles the mask through the operator table.

## Why it's interesting

This is a high-value, low-risk AI-scripting primitive. Mission and cutscene
authors routinely freeze units for staging, then need to "wake them up." Today
they have to resort to hacks (e.g. `doStop`/`commandStop` which behave
differently, or `setBehaviour` juggling). A real `enableAI` makes the disable
state a proper toggle and brings the command set closer to later Arma titles,
which modders already expect. It is almost pure plumbing: one new function plus
one operator-table row, reusing the exact enum/string mapping the existing
`disableAI` already relies on.

## Difficulty & prerequisites

Beginner. Pure C++ engine change in the scripting command layer plus the SQF
operator registry. No new SQF parser work (the operator framework already
supports object–string binaries), no serialization changes (the mask field is
already serialized), and **no art assets of any kind** — no models, textures,
sounds, or configs are required. Reuses the existing `DisabledAI` enum and its
name table verbatim.

## Key files & entry points

- **`C:/dev/arma_CWR/engine/Poseidon/Game/Commands/GameStateExtUi.cpp`**
  - `ObjDisableAI(...)` at **line 2372** — the function to mirror. Resolves the
    object, casts to `EntityAI`, gets `CommanderUnit()`, maps the string via
    `GetEnumValue<AIUnit::DisabledAI>`, then `unit->SetAIDisabled(dai | s)`
    (line 2397). New `ObjEnableAI` (and optional `ObjCheckAIFeature`) go beside it.
- **`C:/dev/arma_CWR/engine/Poseidon/Game/Commands/GameStateExt.cpp`**
  - Forward declaration of `ObjDisableAI` at **line 719** — add the mirror decl(s)
    here.
  - Operator registration `GameOperator(GameNothing, "disableAI", function,
    ObjDisableAI, GameObject, GameString)` at **line 1307**, inside the
    `GetExtBinary` table that starts at **line 1217** — add the new row(s)
    immediately after.
- **`C:/dev/arma_CWR/engine/Poseidon/AI/AIUnit.hpp`**
  - `enum DisabledAI { DATarget=1, DAMove=2, DAAutoTarget=4, DAAnim=8 }` at
    **lines 104–111** (note the "serialized as value — only add values" comment).
  - `int _disabledAI;` field at **line 134**.
  - Accessors `int GetAIDisabled() const` (**line 327**) and
    `void SetAIDisabled(int state)` (**line 328**).
- **`C:/dev/arma_CWR/engine/Poseidon/AI/AIUnit.cpp`**
  - `Foundation::GetEnumNames(AIUnit::DisabledAI)` at **lines 541–546** — the
    `DisabledAINames[]` table mapping `"TARGET"/"AUTOTARGET"/"MOVE"/"ANIM"` to the
    bit values. No change needed; both new functions reuse it.
- **`C:/dev/arma_CWR/engine/Poseidon/Foundation/Enums/EnumNames.hpp`** /
  **`.cpp`**
  - `GetEnumValue<Type>(const char*)` template (hpp **lines 63–68**) →
    `GetEnumValue(const EnumName*, const char*)` (cpp **lines 92–100**), which
    returns **`-1`** via `TryGetEnumValue(...).value_or(-1)` on an unknown string.
    This detail matters (see Risks).

## How it works today

`ObjDisableAI` (GameStateExtUi.cpp:2372):

```cpp
GameStringType str = oper2;
const char* ss = str;
AIUnit::DisabledAI s = GetEnumValue<AIUnit::DisabledAI>(ss);
int dai = unit->GetAIDisabled();
if (s == INT_MIN)            // <-- guard against an unknown string
{
    s = (AIUnit::DisabledAI)0;
}
unit->SetAIDisabled(dai | s);
```

So the flow is: object → `EntityAI` (`dyn_cast`) → `CommanderUnit()` →
`AIUnit`. The string is mapped to a single bit through the shared enum-name
table, and the bit is **ORed** into `_disabledAI`. There is no operator anywhere
in `GetExtBinary` that clears bits, so the mask only grows over a unit's
lifetime. Other AI subsystems read `_disabledAI` to decide whether to move,
auto-select targets, etc.; clearing the bit is therefore sufficient to restore
behavior — no extra "re-arm" call is needed.

Note the existing code checks `s == INT_MIN`, but `GetEnumValue` actually returns
**`-1`** for an unknown name (EnumNames.cpp:89/99). So today an unrecognized
string silently sets `s = -1`, and `dai | -1` turns **every** bit on — a latent
bug worth fixing while we're here (and worth not duplicating in the mirror).

## Implementation approach

1. **Add `ObjEnableAI` in `GameStateExtUi.cpp`** directly after `ObjDisableAI`
   (after line 2399). Copy the object/cast/commander-unit resolution verbatim,
   then invert the mask write:

   ```cpp
   GameValue ObjEnableAI(const GameState* state, GameValuePar oper1, GameValuePar oper2)
   {
       Object* obj = GetObject(oper1);
       if (!obj) return NOTHING;
       EntityAI* veh = dyn_cast<EntityAI>(obj);
       if (!veh) return NOTHING;
       AIUnit* unit = veh->CommanderUnit();
       if (!unit) return NOTHING;

       GameStringType str = oper2;
       const char* ss = str;
       int s = (int)GetEnumValue<AIUnit::DisabledAI>(ss);
       if (s < 0) s = 0;                 // unknown string -> no-op (correct guard)
       int dai = unit->GetAIDisabled();
       unit->SetAIDisabled(dai & ~s);    // clear the bit(s)
       return NOTHING;
   }
   ```

   Use `s < 0` (or `== -1`) rather than the buggy `== INT_MIN`. For an unknown
   string we want a no-op: `dai & ~0 == dai`.

2. **(Optional) Add `ObjCheckAIFeature`** in the same file, returning a `Bool`
   that reports whether the feature is **enabled** (i.e. bit *not* set), which is
   the intuitive scripting polarity:

   ```cpp
   GameValue ObjCheckAIFeature(const GameState* state, GameValuePar oper1, GameValuePar oper2)
   {
       Object* obj = GetObject(oper1);
       if (!obj) return false;
       EntityAI* veh = dyn_cast<EntityAI>(obj);
       if (!veh) return false;
       AIUnit* unit = veh->CommanderUnit();
       if (!unit) return false;
       const char* ss = (GameStringType)oper2;
       int s = (int)GetEnumValue<AIUnit::DisabledAI>(ss);
       if (s < 0) return false;          // unknown feature name
       return (unit->GetAIDisabled() & s) == 0;   // true == enabled
   }
   ```

3. **Forward-declare** the new function(s) in `GameStateExt.cpp` next to the
   `ObjDisableAI` declaration at line 719:

   ```cpp
   GameValue ObjEnableAI(const GameState* state, GameValuePar oper1, GameValuePar oper2);
   GameValue ObjCheckAIFeature(const GameState* state, GameValuePar oper1, GameValuePar oper2);
   ```

4. **Register the operator(s)** in the `GetExtBinary` table immediately after the
   `disableAI` row (line 1307):

   ```cpp
   GameOperator(GameNothing, "enableAI", function, ObjEnableAI, GameObject, GameString),
   GameOperator(GameBool,    "checkAIFeature", function, ObjCheckAIFeature, GameObject, GameString),
   ```

   `GameNothing`/`GameBool` are the return-type slots; `GameObject` and
   `GameString` are the left/right operand type masks, matching `disableAI`.

5. **Build** the engine and a client/tool target so the operator table is linked:
   `cmake --build build/win-x64-clang-rwdi`.

6. **(Optional consistency fix)** Replace the `s == INT_MIN` guard in
   `ObjDisableAI` with `s < 0` so the existing command stops blasting all bits on
   a typo. Keep this as a separate, clearly-labeled change since it alters
   long-standing (buggy) behavior.

## Config / data / SQF touchpoints

No config (`config.cpp`/`config.bin`) or mission-data changes. The only new
surface is SQF:

```sqf
// freeze, then revive
_u disableAI "MOVE";        sleep 3;  _u enableAI "MOVE";
_u disableAI "AUTOTARGET";  _u enableAI "AUTOTARGET";

// optional getter (true == feature enabled)
if (_u checkAIFeature "MOVE") then { hint "unit can move" };
```

Accepted feature strings are exactly those in `DisabledAINames[]`
(AIUnit.cpp:543–545): `"TARGET"`, `"MOVE"`, `"AUTOTARGET"`, `"ANIM"`
(case-insensitive — `EqualsIgnoreAsciiCase`, EnumNames.cpp:64).

## Risks & gotchas

- **The `-1` vs `INT_MIN` trap.** `GetEnumValue` returns `-1` on an unknown
  string (EnumNames.cpp:89/99), *not* `INT_MIN`. Always guard with `s < 0`
  before using the value, or an unknown feature name would (for `enableAI`)
  clear **all** bits via `dai & ~(-1) == 0`, and (for `disableAI`, today) set all
  bits. This is the single most important correctness point in the change.
- **Polarity of `checkAIFeature`.** Decide and document whether the getter
  reports *enabled* or *disabled*. This plan returns *enabled* (bit clear) to
  match the command name; pick one and keep it consistent across docs/tests.
- **`CommanderUnit()` can be null** for objects that aren't AI-driven (empty
  vehicles, static objects). Both new functions must early-out exactly like
  `ObjDisableAI` — return `NOTHING`/`false` rather than dereferencing.
- **Serialization compatibility.** Do **not** add or renumber `DisabledAI`
  values (AIUnit.hpp:110 explicitly warns: serialized as value). We only read and
  clear existing bits, so saved games and MP state are unaffected.
- **Heavy intentional warning suppression.** The root `CMakeLists.txt` disables
  ~50 warning classes; copying the existing `dyn_cast`/C-style-cast style from
  `ObjDisableAI` is fine and idiomatic here — don't "modernize" the surrounding
  code.
- **No `ClassIsMovableZeroed`/memcpy concern** — we touch only an `int` field via
  accessors, no bitwise-movable object copies.
- **`__FILE__` asymmetry (clang-cl vs GNU clang)** is irrelevant here; no path,
  log, or assert macros are added.
- **Per-target PRIVATE PCH:** these are existing translation units already in the
  engine target, so no new PCH wiring is needed. Adding a few functions won't
  approach the `FileSize` target's 3000-line warn / 5000-line error thresholds,
  but GameStateExtUi.cpp is large — check its line count if you add a lot.

## Testing

- **Catch2 unit test** (`tests/unit/engine/Poseidon/Game/`, suite
  `PoseidonTests` or `PoseidonServerTests`): there's an existing
  `test_game_state_ext.cpp` to extend. Construct/obtain an `AIUnit` (or test
  `GetEnumValue<AIUnit::DisabledAI>` mapping directly), then assert the mask
  round-trips: after `SetAIDisabled(DAMove)`, applying the `enableAI` logic
  yields `0`, and an unknown string is a no-op. Run:
  `ctest --test-dir build/win-x64-clang-rwdi -R PoseidonTests --output-on-failure`.
  At minimum add a focused test asserting `GetEnumValue<AIUnit::DisabledAI>("MOVE") == AIUnit::DAMove`
  and `GetEnumValue<AIUnit::DisabledAI>("BOGUS") == -1` to lock in the guard.
- **Trident integration** (`tests/integration/scripting/`): add a
  `enable_ai_toggle.test.sqf` + `.toml` pair (mirror the existing pairs like
  `camcreate_any_type.test.*`). Spawn a unit, record start position, `disableAI
  "MOVE"`, issue a `move`, assert it hasn't moved, then `enableAI "MOVE"` and
  assert it does. Needs game data; run with
  `tri test -j6 --retries 2 tests/integration` after
  `cargo build --manifest-path engine/Trident/Cargo.toml`.
- **In-game smoke** with the `win-x64-clang-rwdi` build: place a unit, in the
  debug console run `this disableAI "MOVE"; this enableAI "MOVE"` and confirm the
  unit responds to move orders again; `hint str (player checkAIFeature "MOVE")`
  to eyeball the getter.

## Scope estimate

Roughly **1–2 hours** for an experienced reader of this file, including the unit
test. The change is ~25 lines of new code (one function + one table row), plus
~15 lines of test.

**Minimal first slice for a fast visible win:** implement only `ObjEnableAI`
(step 1) and register `"enableAI"` (steps 3–4), skipping `checkAIFeature` and the
`disableAI` guard fix. Build the client, then in-game run
`this disableAI "MOVE"; this enableAI "MOVE"` and watch a frozen unit start
moving — a complete, observable feature in the fewest possible edits. Add the
getter and the `-1` guard fix as follow-ups.
