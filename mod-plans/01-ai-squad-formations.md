# New & retuned squad formations (combat-spread, herringbone, diamond)

*Add and retune AI infantry/vehicle squad geometry by editing one hardcoded lookup table plus two small declaration sites — no models, no textures, pure data + enum plumbing.*

## Summary

Squad formation geometry in Poseidon is a single compile-time table of relative
offsets. Each squad member is told where to stand relative to its leader (or to an
earlier member) and which way to face. The whole thing lives in one array,
`formations[AI::NForms][MAX_UNITS_PER_GROUP]`, defined at
`engine/Poseidon/AI/AISubgroup.cpp:53`, and is consumed every formation update by
`AISubgroup::UpdateFormationPos()` (`engine/Poseidon/AI/AISubgroup.cpp:2112`).

This mod does two things:

1. **Retune existing formations** (column, wedge, vee, line, echelon, staggered
   column) by changing the numeric offsets in their rows. This takes effect
   immediately with no other code changes — pure data.
2. **Add brand-new formations** — *combat-spread*, *herringbone*, *diamond* — by
   inserting an enum value, appending a 12-entry row to the table, and registering
   a display name.

**Observable result:** order an AI squad (or your own subordinates via the command
menu / `setFormation`) into a new formation and watch them physically rearrange on
the ground into the new geometry and face the new directions. Retunings are visible
as tighter/looser/wider spacing and changed facing in existing formations.

## Why it's interesting

Formations are one of the most *visible* AI behaviors in the game — you watch your
squad spread out every time you give an order. Yet the entire system is ~110 lines
of plain data. That makes it an ideal first AI mod: the feedback loop is fast (edit
numbers, rebuild, see soldiers move), the blast radius is small and contained, and
the payoff is immediately legible to any player. You can ship a "better combat
spacing" tweak in an afternoon, then graduate to authoring genuinely new tactical
shapes (herringbone road-security halt, 360° diamond) that the base game never had.

## Difficulty & prerequisites

**Beginner.** Requires only:
- A working `win-x64-clang-rwdi` build (see repo `CLAUDE.md` build section).
- Comfort editing a C++ array of structs and adding one enum value in three places.
- Game data to actually see it in-game (Demo data is fine for testing squads).

**No art assets are needed.** This is 100% data + enum plumbing. No models, no
textures, no new configs required (a couple of optional string-table / radio-sentence
touchpoints are described below but are *not* required for the formation to work).

## Key files & entry points

| File | Symbol / line | Role |
|------|---------------|------|
| `engine/Poseidon/AI/AISubgroup.cpp:53` | `extern const FormInfo formations[AI::NForms][MAX_UNITS_PER_GROUP]` | The geometry table. One row per formation, 12 `FormInfo` entries per row. |
| `engine/Poseidon/AI/Path/PathPlanner.hpp:40` | `enum AI::Formation { FormColumn … FormLine, NForms }` | Enum that indexes the table. `NForms` is the row count. |
| `engine/Poseidon/AI/AIUnit.hpp:488` | `struct FormInfo { int base; Vector3 position; float angle; }` | Per-slot record: `base` = which slot to offset from (-1 = absolute/leader), `position` = (x, _, z) offset, `angle` = facing. |
| `engine/Poseidon/AI/AIUnit.hpp:503` | `extern const FormInfo formations[...]` | Forward declaration that must agree with the definition. |
| `engine/Poseidon/AI/Path/AITypes.hpp:31` | `#define MAX_UNITS_PER_GROUP 12` | Columns per row — every row MUST have exactly 12 `FormInfo`. |
| `engine/Poseidon/AI/AISubgroup.cpp:2112` | `AISubgroup::UpdateFormationPos()` | Consumer. Reads `formations[_formation][j]`, computes world-relative `_formationPos` and `_formationAngle` per unit. |
| `engine/Poseidon/AI/AICenter.cpp:159` | `GetEnumNames(AI::Formation)` | Maps enum ↔ display/string name (e.g. `"WEDGE"`). Drives the SQF name lookup and any UI. |
| `engine/Poseidon/Game/Commands/GameStateExtGrp.cpp:263` | `GrpSetFormation` (SQF `setFormation`) | Resolves a string to a `Formation` by `stricmp` against `GetEnumNames`. |
| `engine/Poseidon/Game/Commands/GameStateExtWorldWaypoint.cpp:856` | `WaypointSetFormation` (SQF `setWaypointFormation`) | Uses `GetEnumValue<AI::Formation>(str)` — also name-driven. |
| `engine/Poseidon/AI/AIRadioImpl.cpp:670` | `RadioMessageFormation::Serialize` / sentence switch (`:694`) | Network/save serialization (name-based) and the radio callout per formation. |

## How it works today

`UpdateFormationPos()` walks slots `0 … MAX_UNITS_PER_GROUP-1`. For slot `j` it reads
`const FormInfo& info = formations[_formation][j]` (`AISubgroup.cpp:2156`) and computes
a position:

- If `info.base >= 0`, the slot is placed **relative to an already-computed slot**:
  `formPos[j] = formPos[info.base] + factor * info.position`
  (`AISubgroup.cpp:2179`–`2181`). `factorX/factorZ` are the *average* of the base
  unit's and this unit's `GetFormationX()/GetFormationZ()` — so spacing scales with
  vehicle/unit size.
- If `info.base < 0` (the leader, slot 0 has `FormInfo(-1, 0, 0, 0)`), the offset is
  absolute (`AISubgroup.cpp:2185`–`2187`).

The axes: `position[0]` is **lateral** (x, +right/−left), `position[2]` is
**fore/aft** (z; negative = behind the leader — see the column row where every
follower is `z = -1`). `position[1]` (height) is always 0. `info.angle` becomes
`unit->_formationAngle` (`:2192`), the unit's desired facing within the formation;
there's a special case (`:2193`–`2200`) that zeroes the angle for a lone gunner
vehicle so a single tank faces forward.

Because each follower can be based on a *previous* follower, a single row encodes a
chained shape: e.g. the column row chains `0→1→2→3…` each `z = -1` so the line grows
backward; the wedge/vee rows fan two diagonals out from the leader.

Enumeration: `enum Formation` (`PathPlanner.hpp:40`) lists 7 named values then
`NForms = 7`. `formations` therefore has exactly 7 rows today, in enum order. The
SQF command `setFormation "WEDGE"` (`GrpSetFormation`, `GameStateExtGrp.cpp:263`)
finds the enum by string-comparing against `GetEnumNames` (`AICenter.cpp:159`), then
broadcasts via `grp->SendFormation(...)`, which ends in `AISubgroup::SetFormation`
(`AISubgroup.cpp:2010`) flagging units to re-run the geometry.

## Implementation approach

### Part A — Retune existing formations (zero new symbols)

1. Open `engine/Poseidon/AI/AISubgroup.cpp` and locate the row you want
   (e.g. `// line` at `:146`, `// wedge` at `:86`).
2. Edit the numeric offsets. Examples:
   - **Wider line:** the line followers use `x = ±1`; the actual spacing comes from
     `factorX * position[0]`, so to widen *every* unit uniformly, scale the `x`
     magnitudes (e.g. `±1.5`). Leave `base` chains intact.
   - **Deeper column:** increase the `z` magnitude in the column row (currently
     `-1`) to `-1.3` for more longitudinal spacing.
3. Rebuild. No declaration changes, no enum changes — the table is `const` data
   compiled straight into the consumer.

### Part B — Add a new formation (do this once per new shape)

For each new formation (`combat-spread`, `herringbone`, `diamond`):

1. **Enum** — `engine/Poseidon/AI/Path/PathPlanner.hpp:40`. Insert the new value
   *before* `NForms` (which auto-increments the row count):
   ```cpp
   enum Formation
   {
       FormNone = -1,
       FormColumn,
       FormStaggeredColumn,
       FormWedge,
       FormEcholonLeft,
       FormEcholonRight,
       FormVee,
       FormLine,
       FormCombatSpread,   // new
       FormHerringbone,    // new
       FormDiamond,        // new
       NForms
   };
   ```
   Appending at the end keeps existing enum *values* stable (important for saves /
   waypoints that stored an index). See Risks.

2. **Table row** — `engine/Poseidon/AI/AISubgroup.cpp:53`. Append one 12-entry row
   per new enum value, **in the same order as the enum**, after the `// line` row
   (close the existing last brace, add a comma). Each row must have exactly
   `MAX_UNITS_PER_GROUP` (12) `FormInfo(base, x, z, angle)` entries. Starter rows
   (tune to taste — these are illustrative, not authoritative):

   ```cpp
   {
       // combat_spread  (wide line, even units echeloned slightly back for depth)
       FormInfo(-1, 0, 0, 0),
       FormInfo(0,  1.5, -0.3, 0),
       FormInfo(0, -1.5, -0.3, 0),
       FormInfo(1,  1.5, -0.3, 0),
       FormInfo(2, -1.5, -0.3, 0),
       FormInfo(3,  1.5, -0.3, 0),
       FormInfo(4, -1.5, -0.3, 0),
       FormInfo(5,  1.5, -0.3, 0),
       FormInfo(6, -1.5, -0.3, 0),
       FormInfo(7,  1.5, -0.3, 0),
       FormInfo(8, -1.5, -0.3, 0),
       FormInfo(9,  1.5, -0.3, 0),
   },
   {
       // herringbone  (column halt; odd units face right, even face left)
       FormInfo(-1, 0, 0, 0),
       FormInfo(0,  0.5, -1,  0.5 * H_PI),   // face right
       FormInfo(1, -0.5, -1, -0.5 * H_PI),   // face left
       FormInfo(2,  0.5, -1,  0.5 * H_PI),
       FormInfo(3, -0.5, -1, -0.5 * H_PI),
       FormInfo(4,  0.5, -1,  0.5 * H_PI),
       FormInfo(5, -0.5, -1, -0.5 * H_PI),
       FormInfo(6,  0.5, -1,  0.5 * H_PI),
       FormInfo(7, -0.5, -1, -0.5 * H_PI),
       FormInfo(8,  0.5, -1,  0.5 * H_PI),
       FormInfo(9, -0.5, -1, -0.5 * H_PI),
       FormInfo(10, 0.5, -1,  0.5 * H_PI),
   },
   {
       // diamond  (point/flanks/rear, then fill — 360 security)
       FormInfo(-1, 0, 0, 0),          // 0 leader / point
       FormInfo(0,  1.2, -1.2,  0.25 * H_PI),   // 1 right flank
       FormInfo(0, -1.2, -1.2, -0.25 * H_PI),   // 2 left flank
       FormInfo(1, -1.2, -1.2,  H_PI),          // 3 rear point (behind right flank)
       FormInfo(1,  1,   -0.5,  0.5 * H_PI),    // 4 right of right flank
       FormInfo(2, -1,   -0.5, -0.5 * H_PI),    // 5 left of left flank
       FormInfo(3,  1,   -1,    H_PI),          // 6 rear right
       FormInfo(3, -1,   -1,    H_PI),          // 7 rear left
       FormInfo(4,  1,   -0.5,  0.5 * H_PI),    // 8
       FormInfo(5, -1,   -0.5, -0.5 * H_PI),    // 9
       FormInfo(6,  0,   -1,    H_PI),          // 10
       FormInfo(7,  0,   -1,    H_PI),          // 11
   },
   ```
   Key authoring rule discovered in `UpdateFormationPos`: `info.base` for a slot
   **must reference a lower slot index than the slot itself**, because positions are
   built cumulatively (`formPos[base]` must already be computed when slot `j` is
   processed — `AISubgroup.cpp:2179`). Slot 0 must stay `FormInfo(-1, 0, 0, 0)`.

3. **Display names** — `engine/Poseidon/AI/AICenter.cpp:159`, function
   `GetEnumNames(AI::Formation)`. Add an `EnumName` entry per new value, *before* the
   terminating `EnumName()`:
   ```cpp
   EnumName(AI::FormCombatSpread, "COMBAT SPREAD"),
   EnumName(AI::FormHerringbone,  "HERRINGBONE"),
   EnumName(AI::FormDiamond,      "DIAMOND"),
   ```
   This single table powers both the SQF string lookup (`GrpSetFormation`,
   `GameStateExtGrp.cpp:280`) and `GetEnumValue<AI::Formation>` used by
   `setWaypointFormation` (`GameStateExtWorldWaypoint.cpp:864`). No further command
   registration is needed — `setFormation` is already wired
   (`GameStateExt.cpp:1301`).

4. Rebuild. The new formations are now selectable from SQF and any UI that
   enumerates `GetEnumNames`.

### Part C — Optional polish

- **Radio callout:** `RadioMessageFormation` (`AIRadioImpl.cpp:694`) `switch`es on
  formation to pick a spoken sentence id (`SentFormWedge`, etc.); the `default`
  falls through to `SentFormLine` (`:714`). New formations will *speak the line
  callout* until you add `case AI::FormDiamond: sentence = "SentFormDiamond"; break;`
  and a matching string-table/sound entry. Purely cosmetic.

## Config / data / SQF touchpoints

- **SQF — runtime control already works** once the enum name is registered:
  ```sqf
  group player setFormation "DIAMOND";
  group player setFormation "COMBAT SPREAD";
  ```
  Names are matched case-insensitively (`stricmp`, `GameStateExtGrp.cpp:280`), so
  `"diamond"` works too.
- **SQF — waypoints:** `[grp, idx] setWaypointFormation "HERRINGBONE";`
  (`WaypointSetFormation`, `GameStateExtWorldWaypoint.cpp:856`).
- **Mission editor / waypoints in saves:** `ArcadeWaypointInfo` serializes the
  formation by enum (`ArcadeTemplate.cpp:1106`, `SerializeEnum("formation", …)`),
  which is name-based via the same `GetEnumNames` table — so editor dropdowns and
  saved missions pick up new names automatically once `GetEnumNames` is updated.
- **No `config.cpp`/CfgVehicles changes required.** Per-unit spacing scaling reuses
  the existing `GetFormationX()/GetFormationZ()` already defined on entities.

## Risks & gotchas

- **Row count MUST equal enum count and order MUST match.** `formations` is a
  fixed `[AI::NForms][MAX_UNITS_PER_GROUP]` 2D array. If you add an enum value but
  forget a row (or vice versa), the array's row count silently shifts and
  `formations[_formation]` reads the wrong row or runs off the end. Add enum +
  row + name **together**, and append at the end.
- **Every row needs exactly 12 entries.** `UpdateFormationPos` iterates the full
  `MAX_UNITS_PER_GROUP`. A short row is a compile-time aggregate that zero-fills,
  putting stragglers on top of the leader. Count them.
- **`base` must reference a strictly earlier slot.** Forward/self references read an
  uninitialized `formPos` (it's `Init()`-ed to zero just-in-time at `:2159`), so a
  slot based on a later slot collapses to the origin. Validate your chain.
- **Save/MP enum stability:** `RadioMessageFormation::Serialize`
  (`AIRadioImpl.cpp:675`) and waypoint serialization are **name-based**
  (`SerializeEnum`), so appending values is forward/backward compatible for *names*.
  But anything that stored a raw integer index will shift if you *insert* in the
  middle — always append before `NForms`, never reorder existing values.
- **Repo convention — intentional warning suppression:** `CMakeLists.txt` globally
  silences ~50 warning classes. The existing formation `switch` in `AIRadioImpl.cpp`
  is deliberately non-exhaustive (relies on `default`); do **not** "fix" it into an
  exhaustive switch unless you intend to handle every new case. Likewise leave the
  defensive/tautological patterns elsewhere alone.
- **`ClassIsMovableZeroed` / memcpy pattern:** `FormInfo` is a trivial POD
  (`int`, `Vector3`, `float`) and the table is `const` — you will not trip the
  bitwise-move conventions here, but don't add non-trivial members (e.g. an
  `RString` label) to `FormInfo` without understanding that pattern.
- **PCH / build registration:** PCH is per-target PRIVATE; editing these existing
  `.cpp/.hpp` files needs no CMake changes (no new translation units, no new
  subdirectory to register).
- **`__FILE__` asymmetry** (clang-cl absolute vs GNU-driver remapped) is irrelevant
  here unless you add asserts that print paths.
- **FileSize target:** `AISubgroup.cpp` is already large; the `FileSize` target
  warns >3000 and errors >5000 lines. Adding three 12-line rows is negligible, but
  if you do a big retuning pass, keep an eye on it.

## Testing

- **Unit (Catch2, `tests/unit/`):** the geometry table is pure data, ideal for a
  cheap regression test in the AI suite (`PoseidonTests` / a `[ai]`-tagged case).
  Suggested assertions without spinning up a world:
  - `AI::NForms` equals the number of rows you authored (guard against the
    enum/row mismatch gotcha — e.g. compute `sizeof(formations)/sizeof(formations[0])`
    and compare to `AI::NForms`).
  - Slot 0 of every row is `FormInfo(-1, 0, 0, 0)` (leader anchor invariant).
  - Every row's `info.base < j` for `j > 0` (the earlier-slot invariant).
  - `GetEnumNames(AI::Formation)` has a valid entry for each new value and the
    `GetEnumValue<AI::Formation>("DIAMOND")` round-trips.
  Run: `ctest --test-dir build/win-x64-clang-rwdi -R PoseidonTests --output-on-failure`.
- **Integration (Trident, `tests/integration/`):** write an SQF scenario that spawns
  a full squad, issues `group leader setFormation "DIAMOND"`, waits, then asserts on
  relative unit positions (`getPos`/`getDir`) to confirm the shape and facings.
  Needs game data (`OFPR_GAME_DIR`/`OFPR_DATA_DIR`); see `CLAUDE.md`. Run via
  `tri test -j6 tests/integration`.
- **In-game (fastest visual check):** build `win-x64-clang-rwdi`, launch with squad
  AI, open the command menu (or run `group player setFormation "HERRINGBONE"` from a
  debug/init line) and watch the soldiers rearrange. Toggle between old and new
  formations to confirm distinct geometry and facing.

## Scope estimate

- **Retuning only (Part A):** ~30 minutes including a rebuild and an in-game look.
- **One new formation end-to-end (Parts B):** ~1–2 hours including iteration on the
  offset numbers (most of the time is eyeballing spacing in-game).
- **All three new formations + optional radio callouts + a unit test:** ~half a day.

**Suggested minimal first slice for a fast visible win:** do *just* the `diamond`
formation. Add the single enum value (`FormDiamond` before `NForms`), append its one
12-entry row, add the `EnumName(AI::FormDiamond, "DIAMOND")` line, rebuild, then
`group player setFormation "DIAMOND"` in-game. That exercises the entire add-a-
formation path (enum + table + name + SQF) in the smallest possible change and gives
you an unmistakable on-screen result to validate the workflow before you author the
trickier herringbone/combat-spread geometry.
