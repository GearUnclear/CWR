# AI Diagnostics tab: one-click toggles for the engine's built-in AI debug draws

*Surface the engine's hidden steering/path/cost-map/lock-map overlays as checkboxes in the ImGui dev panel — no SQF typing, no remembering bit names.*

## Summary

The engine already has a rich AI visualizer baked into `EntityAI::DrawDiags()`: it renders
steering and turn-prediction arrows, hide-behind cover points, formation slot markers,
strategic GoTo / subgroup-command destinations, the live weapon / wanted-weapon / eye
direction arrows, attack aggressive/economical positions, the full pilot path (line + per-node
arrows), the per-cell **cost map** (road/tree/bush/water tinting) and the **lock map**
(vehicle/soldier locked cells). Every one of these is gated behind a bit in the
`Poseidon::Dev::DiagMode` bitmask, and the *only* runtime way to flip those bits today is via
three SQF operators (`diag_enable`, `diag_toggle`, `diag_drawmode`) or the single `V` cheat
hotkey that toggles a fixed `Combat + Path` preset.

This mod adds an **"AI Diag"** tab to the existing Poseidon Dev Panel
(`DebugOverlay.cpp`) with one checkbox per `DiagEnable` bit, an "all on / all off" pair, and a
combo for `DiagDrawModeState` (Normal / Roadway / Geometry / ViewGeometry / FireGeometry /
Paths). Observable result: open the panel (Ctrl+`), tick "Path" and "CostMap", and arrows +
the colored navigation grid appear around AI under the camera in real time — the same draws
that previously required typing `player diag_enable ["Path", true]` into a debug console.

## Why it's interesting

For anyone modding AI behavior, this is the single most useful debugging surface in the engine
and it's currently almost invisible. Want to see *why* a squad takes a weird route? Tick Path
and CostMap and watch the pathfinder's road-preference grid. Tuning combat? Tick Combat to see
weapon vs. eye direction and the aggressive/economical firing positions the AI is choosing.
The draws already exist and are correct — this is pure quality-of-life plumbing that turns a
power-user SQF feature into a discoverable toggle panel, sitting right next to the existing
Cheats / Console / Profile tabs.

## Difficulty & prerequisites

**Intermediate.** It is mostly a new ImGui draw function plus a small amount of header plumbing
to expose two globals — but there is one real gotcha (below) that bumps it above "trivial":
the entire diagnostics subsystem is currently compiled out.

**No art assets required.** Every overlay is drawn from shapes the engine already loads —
`GScene->ForceArrow()` (the arrow model), `GScene->Preloaded(RectangleModel)` (the grid quad),
and `ObjectLine::CreateShape()` (path segments). This mod adds zero models, textures, or
config classes; it only flips integer bits and an enum.

Prereqs: ability to build `win-x64-clang-rwdi`, run with `--dev` (the panel is gated by
`AppConfig::Instance().DevMode()`), and load a mission with AI so there is something to draw.

## Key files & entry points

- **`engine/Poseidon/Dev/Diag/DiagModes.hpp`** — declares the `DiagEnable` enum via
  `DECLARE_ENUM(DiagEnable, DE, DIAG_ENABLE_ENUM)` (10 bits: `DECostMap`, `DELockMap`,
  `DECombat`, `DEForce`, `DEAnimation`, `DEDammage`, `DECollision`, `DETransparent`, `DESound`,
  `DEPath`, plus the `NDiagEnable` count), `extern int DiagMode;` in `Poseidon::Dev`, and the
  `CHECK_DIAG(x)` macro. Whole file is wrapped in `#if _ENABLE_CHEATS`.
- **`engine/Poseidon/World/Scene/SceneDraw.cpp`**
  - line 469-477: `DIAG_DRAW_MODE_ENUM` + `DECLARE_DEFINE_ENUM(DiagDrawMode, DDM, ...)` —
    the draw-mode enum (`DDMNormal`, `DDMRoadway`, `DDMGeometry`, `DDMViewGeometry`,
    `DDMFireGeometry`, `DDMPaths`). Defined only here, **not in any header.**
  - line 484: `DiagDrawMode DiagDrawModeState = DDMNormal;` (global, file scope).
  - line 487: `int DiagMode;` (the actual definition, in `Poseidon::Dev`).
  - line 490-530: `SetDiagDrawMode` / `SetDiagEnable` / `SetDiagToggle` — the SQF op
    implementations.
  - line 534-546: `ObjUnary` / `ObjBinary` tables + `INIT_MODULE(GameStateObj, 3)` registering
    `diag_drawmode`, `diag_toggle`, `diag_enable`.
  - line 574-597: `DiagDrawModeState` consumed in `Scene::AdjustComplexity` to swap the drawn
    LOD to a geometry/roadway/paths LOD.
- **`engine/Poseidon/AI/VehicleAIDiag.cpp`** — `EntityAI::DrawDiags()` (line 48). Reads
  `CHECK_DIAG(DEPath)` (steering/turn arrows, hide-behind, formation slots, GoTo/command
  arrows, supply point, the full path), `CHECK_DIAG(DECombat)` (weapon/wanted/eye arrows,
  attack positions), `CHECK_DIAG(DECostMap)` and `CHECK_DIAG(DELockMap)` (the two grids, only
  when `this == GLOB_WORLD->CameraOn()`). `MapDiags` is a persistent rect cache reused across
  frames. Entire body is `#if _ENABLE_CHEATS`.
- **`engine/Poseidon/World/World.cpp`**
  - line 1481-1496: the draw loop calls `obj->DrawDiags()` per visible object **only when
    `if (DiagMode)` is non-zero** — so a zero mask costs nothing.
  - line 688-703: the legacy `SDL_SCANCODE_V` cheat that toggles `DiagMode` between `0` and
    `(1 << DECombat) | (1 << DEPath)`. Good reference for how the bits are set.
- **`engine/Poseidon/Dev/Debug/DebugOverlay.cpp`** — the ImGui dev panel. Tab pattern lives in
  `DrawMainWindow()` (line 1485-1565); each tab is a `BeginTabItem(...) { DrawXxxTab(); EndTabItem(); }`
  block. `DrawCheatsTab` / `DrawGameTab` (line 269 / 664) show the established checkbox +
  combo + tooltip idiom and the `Defer(...)` queue for engine-mutating actions.
- **`engine/Poseidon/Foundation/Enums/EnumNames.hpp`** — `GetEnumNames<DiagEnable>()`,
  `FindEnumName`, `GetEnumValue<T>`, and `GetEnumName(names, value)` let the tab build checkbox
  labels straight from the enum table instead of hard-coding strings.

## How it works today

`DiagMode` is a global bitmask. When any bit is set, `World.cpp`'s draw loop calls
`DrawDiags()` on every visible object; `EntityAI::DrawDiags()` then checks each relevant bit
with `CHECK_DIAG(...)` and pushes colored `ObjectColored(forceArrow, ...)` / `ObjectLineDiag`
/ rectangle objects into the scene for that frame. The cost-map and lock-map grids are limited
to the entity the **camera is on** (`this == GLOB_WORLD->CameraOn()`) and a 50 m radius, so
they don't flood the scene.

There are exactly two runtime entry points to set the bits:
1. The SQF ops registered in `SceneDraw.cpp` — `obj diag_enable ["Path", true]`,
   `obj diag_toggle "Combat"`, `obj diag_drawmode "Geometry"` (the operators ignore the left
   operand object and write the process-global `DiagMode` / `DiagDrawModeState`; `"all"` is a
   special token in `SetDiagEnable` that means every bit).
2. The `V` hotkey in `World.cpp` that flips a hard-coded `Combat + Path` preset on/off.

There is no GUI. To read or set an individual bit you must know its string name and have a
console/SQF surface available.

**Critical caveat — the whole subsystem is compiled out today.**
`engine/Poseidon/Foundation/PoseidonPCH.hpp:44` does `#define _ENABLE_CHEATS 0`, and the PCH is
force-included into every target. Because `DiagModes.hpp`, `EntityAI::DrawDiags()`,
`DiagDrawModeState`, and the SQF op registration are *all* wrapped in `#if _ENABLE_CHEATS`,
in a stock build `DiagMode` does not even exist, `DrawDiags()` is an empty body, and
`diag_enable` is not a registered operator. So step 0 of this mod is enabling the feature.

## Implementation approach

1. **Turn the feature on.** Change `engine/Poseidon/Foundation/PoseidonPCH.hpp:44` from
   `#define _ENABLE_CHEATS 0` to something that is `1` for dev/RWDI builds. Cleanest:
   ```cpp
   #ifndef _ENABLE_CHEATS
   #  if defined(NDEBUG) && !defined(POSEIDON_DEV_DIAGS)
   #    define _ENABLE_CHEATS 0
   #  else
   #    define _ENABLE_CHEATS 1
   #  endif
   #endif
   ```
   or simply flip it to `1` while developing. Note this is a **process-wide recompile** (the
   define gates code in dozens of TUs) and changes the PCH blob — see Risks. The minimal first
   slice can just hard-set it to `1`.

2. **Expose the two globals to the overlay TU.** `DiagMode` already has an `extern` in
   `DiagModes.hpp`, so `DrawAiDiagTab` can include that header and read/write
   `Poseidon::Dev::DiagMode`. `DiagDrawModeState` and the `DiagDrawMode` enum are *only* in
   `SceneDraw.cpp`. Add them to a header — extend `DiagModes.hpp` (still under `#if
   _ENABLE_CHEATS`) with:
   ```cpp
   // mirror of the SceneDraw.cpp enum, kept in sync
   #define DIAG_DRAW_MODE_ENUM(type,prefix,XX) \
       XX(type,prefix,Normal) XX(type,prefix,Roadway) XX(type,prefix,Geometry) \
       XX(type,prefix,ViewGeometry) XX(type,prefix,FireGeometry) XX(type,prefix,Paths)
   DECLARE_ENUM(DiagDrawMode, DDM, DIAG_DRAW_MODE_ENUM)
   extern DiagDrawMode DiagDrawModeState;   // defined in SceneDraw.cpp
   ```
   Then in `SceneDraw.cpp` replace the local `DECLARE_DEFINE_ENUM(DiagDrawMode, ...)` with the
   header's `DECLARE_ENUM` plus a `DEFINE_ENUM(DiagDrawMode, DDM, DIAG_DRAW_MODE_ENUM)` so the
   `GetEnumNames`/`GetEnumCount` template specializations still get a single definition.
   (Watch the namespace: `DiagDrawModeState` lives at global scope in `SceneDraw.cpp`, while
   `DiagMode` lives in `Poseidon::Dev` — keep the extern declarations matching exactly or you
   get a link error.)

3. **Write `DrawAiDiagTab()` in `DebugOverlay.cpp`** next to `DrawGameTab` (~line 664). Wrap
   the whole function body in `#if _ENABLE_CHEATS` and provide a tiny `#else` stub that prints
   `ImGui::TextDisabled("Built without _ENABLE_CHEATS")` so the tab still compiles in shipping
   builds. Body:
   - **Per-bit checkboxes** driven by the enum table so labels can't drift:
     ```cpp
     using namespace Poseidon::Dev;
     int& mode = DiagMode;
     for (int i = 0; i < NDiagEnable; ++i)
     {
         bool on = (mode & (1 << i)) != 0;
         RStringB label = FindEnumName((DiagEnable)i);   // "CostMap", "Path", ...
         if (ImGui::Checkbox(label.cstr(), &on))
             mode = on ? (mode | (1 << i)) : (mode & ~(1 << i));
     }
     ```
   - **All on / All off** buttons: `ImGui::Button("All")` → `mode = ~0;`,
     `ImGui::Button("None")` → `mode = 0;` (matches the `"all"` token semantics in
     `SetDiagEnable`).
   - **Camera hint**: `ImGui::TextDisabled("CostMap / LockMap only draw around the camera's vehicle (50 m).")`
     — accurate to the `this == GLOB_WORLD->CameraOn()` guard in `DrawDiags`.
   - **Draw-mode combo**:
     ```cpp
     int dm = (int)DiagDrawModeState;
     const char* items[NDiagDrawMode];
     for (int i = 0; i < NDiagDrawMode; ++i) items[i] = FindEnumName((DiagDrawMode)i).cstr();
     if (ImGui::Combo("Draw mode (LOD swap)", &dm, items, NDiagDrawMode))
         DiagDrawModeState = (DiagDrawMode)dm;
     ```
     with a tooltip noting it replaces the rendered LOD with the geometry/roadway/path LOD via
     `Scene::AdjustComplexity`.
   - Writing `DiagMode` / `DiagDrawModeState` is just an integer store with no engine
     reallocation, so unlike the save/reload buttons it does **not** need the `Defer(...)`
     queue — set it inline in the click handler.

4. **Register the tab** in `DrawMainWindow()` (line ~1490-1559) following the existing
   pattern:
   ```cpp
   if (ImGui::BeginTabItem("AI Diag")) { DrawAiDiagTab(); ImGui::EndTabItem(); }
   ```
   Place it after "Console" so AI debugging tools cluster together.

5. **(Optional) live read-back.** Add an `ImGui::Text("DiagMode = 0x%03X", DiagMode);` line so
   the user can confirm the mask, and so the value the `V` hotkey sets is reflected if they
   also use it.

## Config / data / SQF touchpoints

No config/CPP class changes. The SQF surface is *reused, not replaced*: `diag_enable`,
`diag_toggle`, and `diag_drawmode` keep working and will now reflect in the panel (and vice
versa), since both paths write the same `DiagMode` / `DiagDrawModeState` globals. This means
existing diagnostic scripts and the `V` hotkey stay valid; the tab is purely an additional
front-end. No stringtable entries are needed (labels come from the enum names).

## Risks & gotchas

- **`_ENABLE_CHEATS` is the whole ballgame.** Flipping `PoseidonPCH.hpp:44` recompiles a large
  fraction of the engine and re-bakes the PCH. Per the repo conventions, **PCH config is
  per-target PRIVATE** (each target in `POSEIDON_PCH_TARGETS` bakes its own compile defs into
  its PCH blob), so a clean reconfigure/rebuild of the affected targets is expected; don't be
  surprised by a long first build. If you'd rather not flip it globally, gate it to RWDI only.
- **The two globals live in different scopes.** `DiagMode` is `Poseidon::Dev::DiagMode`;
  `DiagDrawModeState` is at global namespace scope. Mismatched `extern`/definition namespaces
  produce link errors that look like "unresolved external" despite the symbol "being there".
- **One-definition for the enum tables.** `DECLARE_DEFINE_ENUM` emits the `GetEnumNames` /
  `GetEnumCount` specializations. If you move the `DiagDrawMode` declaration to the header,
  emit `DEFINE_ENUM` in exactly one TU (`SceneDraw.cpp`) or you'll get duplicate-symbol
  errors.
- **Don't "clean up" the diag draw code.** `EntityAI::DrawDiags()` and the cost-map color
  table use patterns (intentionally incomplete `switch` over `DiagDrawModeState`, raw index
  into `typeColor[NOperItemType]`, defensive compares) that fall under the repo's **large,
  intentional clang warning suppression**. Leave them be; the goal is a toggle front-end, not
  a refactor. Likewise the `MapDiags` rect cache uses the engine's movable/zeroed container
  pattern — do not touch its storage.
- **Performance:** ticking CostMap/LockMap pushes up to ~`101*101` rect objects per frame
  inside 50 m; that's by design and self-limited, but warn the user via tooltip. The cost is
  zero when `DiagMode == 0` because `World.cpp` skips the whole `DrawDiags` call.
- **File size:** `DebugOverlay.cpp` is already ~1700 lines; adding ~80 lines stays well under
  the `FileSize` target's 3000-line warn / 5000-line error thresholds, but if it ever grows
  consider splitting the AI tab into its own TU.
- **`__FILE__` asymmetry** (clang-cl absolute vs GNU prefix-mapped) is irrelevant here — this
  code logs no paths — but keep it in mind if you add any `RepoPath()`-style helpers.

## Testing

- **Build:** `cmake --preset win-x64-clang-rwdi` then
  `cmake --build build/win-x64-clang-rwdi`. Confirm it links (this is where the
  namespace/extern mistakes surface).
- **In-game (primary):** run a client with `--dev`, load any mission with AI, press
  Ctrl+` to open the panel, select **AI Diag**. Tick **Path** → orange/yellow steering and
  turn arrows plus the magenta/yellow path line appear on AI vehicles; tick **Combat** →
  red/yellow weapon and eye arrows; switch the camera onto an AI vehicle (or use the existing
  teleport/camera cheats) and tick **CostMap** / **LockMap** → the colored navigation grid and
  locked-cell quads appear within 50 m. Toggle **All / None** and confirm the draws appear and
  vanish instantly. Confirm the legacy `V` hotkey and `player diag_enable ["Path", true]` from
  the Console tab now move the same checkboxes.
- **Draw-mode combo:** pick **Geometry** / **Roadway** / **Paths** and confirm models render
  as their collision/road/path LODs (driven by `Scene::AdjustComplexity`), then **Normal**
  restores.
- **Unit (Catch2):** there's no rendering harness, but you can add a tiny test under
  `tests/unit/` (suite `PoseidonTests`, tag e.g. `[diag]`) that, with `_ENABLE_CHEATS=1`,
  drives the same bit logic the tab uses — set/clear/toggle a `DiagEnable` bit through a small
  free helper and assert `CHECK_DIAG` agrees, plus round-trip `GetEnumValue<DiagEnable>` /
  `FindEnumName`. Run with `ctest --test-dir build/win-x64-clang-rwdi -R PoseidonTests`.
- **Integration (Trident):** optional — a `tests/integration` SQF scenario can call
  `player diag_enable ["Path", true]` and assert no error / that the op resolves, validating
  that enabling `_ENABLE_CHEATS` actually registers the operator.

## Scope estimate

Roughly **half a day to a day.** The ImGui tab itself is ~1-2 hours of well-trodden code; the
real work is the `_ENABLE_CHEATS` decision + clean rebuild and the small header plumbing for
`DiagDrawModeState`/`DiagDrawMode`.

**Minimal first slice for a fast visible win:** hard-set `_ENABLE_CHEATS` to `1`, then add the
tab with *only* the 10 checkboxes + an All/None pair reading/writing `Poseidon::Dev::DiagMode`
(no draw-mode combo, no header refactor — that combo needs the extern work). That alone
reproduces and surpasses the `V` hotkey: you can independently toggle Path, Combat, CostMap and
LockMap from the panel and immediately see the arrows and grids in-mission. Add the
`DiagDrawMode` combo and the read-back line as a second pass.
