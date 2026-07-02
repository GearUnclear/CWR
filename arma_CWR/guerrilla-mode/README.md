# Guerrilla Mode (Arma: Cold War Assault / Poseidon)

The flagship game mode of this project's **total game overhaul** of Arma: Cold
War Assault — a persistent, single-player **insurgency / territory-control**
game mode built on the reworked AI/engine layer in [`../mod-plans/`](../mod-plans/README.md).
You are one Resistance (GUER) fighter
who liberates an island zone-by-zone: capture outposts, build a cell (Resources
₽ + Manpower HR), recruit and train fighters, unlock looted gear, level a named
companion, and hold ground against an escalating EAST occupier that spawns
garrisons and QRFs and runs a graduated alert state machine — all round-tripping
through `saveGame`/`loadGame`.

Master design: [`../mod-plans/13-guerrilla-mode.md`](../mod-plans/13-guerrilla-mode.md).
Phase-1 architecture spine: [`ARCHITECTURE.md`](ARCHITECTURE.md) (binding for
the Demo mission only — see its scope-correction note).

**Core requirement (drives everything after Phase 1):** factions and islands
are **swappable at new-game** — start a campaign by selecting the island, the
civilian faction, the resistance faction, and the occupying faction. The Demo
mission's fixed GUER-vs-EAST-on-Demo setup is a Phase-1 fixture, not the game;
plan 13's Phase-1.5 de-hardcoding pass moves all faction/island specifics into
data (faction descriptors + per-island zone files).

---

## What this is (and the C++ ↔ SQF split)

The mode is **overwhelmingly SQF/SQS mission scripting** — no new art, and it
runs on stock Demo data. Two tiny engine commands are the *only* C++ additions.

| Layer | Status | Where |
|-------|--------|-------|
| **Mission scripts** (bootstrap + 10 managers + helper lib) | **Written** | [`mission/Guerrilla.Demo/`](mission/Guerrilla.Demo/) |
| **Engine cmd `enableAI`** (plan 09; inverse of `disableAI`) | **Written** (needs a compile/run) | `engine/Poseidon/Game/Commands/GameStateExtUi.cpp` + registration in `GameStateExt.cpp` |
| **Engine cmd `knownTargets`** (plan 10; group→known-contacts array) | **Written** (needs a compile/run) | `engine/Poseidon/Game/Commands/GameStateExtGrp.cpp` + registration in `GameStateExt.cpp` |
| **Unit test** for the two new commands | **Written** | `tests/unit/engine/Poseidon/Game/test_game_state_ext.cpp` (`[game][gameStateExt][ai]`) |
| **Integration tests** (Gate-Zero spawn+move, capture-flip, save/reload) | **Written** | `tests/integration/scripting/guerrilla_*` |

The scripts **degrade gracefully without the two C++ commands**: alert.sqs uses
single-object `knowsAbout` polling instead of `knownTargets`, and no script hard-
depends on `enableAI` (cached units are deleted on despawn rather than AI-frozen).
So the mission runs today on an unmodified engine build; landing 09/10 is an
enhancement, not a prerequisite.

### Dialect
SQS for the bootstrap + long-lived manager tick-loops (`init.sqs` auto-loads, and
only SQS gives `~N` waits, `@cond` gates, and `#label`/`goto`). Synchronous
helpers are single-line SQF **code-strings** (`GM_fn*` in `lib.sqs`) — in this
evaluator a `{ … }` literal *is* a `GameString`, so `call`/`while`/`if-then-else`
bodies are strings (no `compile`). See ARCHITECTURE.md §0 for the ground truth.

### Mission layout
```
mission/Guerrilla.Demo/
  mission.sqm         one GUER player (SoldierGB) at the Camp
  description.ext     minimal metadata (respawn=0, no JIP)
  init.sqs            BOOTSTRAP: defines all globals + GM_Z_* index consts,
                      execs lib then @GM_LIB_READY, execs the 9 managers
  scripts/
    lib.sqs           7 confirmed-command helpers + GM_LIB_READY handshake
    zones.sqs         territory: markers, capture detection, ownership flip
    spawning.sqs      distance-cached garrison + QRF spawn/despawn (DAC)
    alert.sqs         GREEN/YELLOW/RED knowsAbout state machine
    escalation.sqs    War Level (slow) + per-region Heat decay (fast)
    economy.sqs       Resources + Manpower income tick (~10 min)
    loot.sqs          loot-on-kill stash + per-classname gear unlock
    recruit.sqs       addAction layer: recruit / specialist / train
    recruit_action.sqs thin addAction dispatcher (enqueues to recruit.sqs)
    companions.sqs    named-companion XP→rank/skill + permadeath
    persistence.sqs   on-load rebuild of transient live handles
```

---

## Build

Toolchain (Clang-only, Ninja, vcpkg, ccache) is described in the repo root
[`CLAUDE.md`](../CLAUDE.md). On this machine everything is already installed.

```sh
cmake --preset win-x64-clang-rwdi           # configure (Linux: linux-x64-clang-rwdi)
cmake --build build/win-x64-clang-rwdi      # build the engine + game + tools
```

Compiled apps stage into `dist/win-x64-clang-rwdi/`.

### Acquire the (free) Demo game data
Game data is **not** in this repo. Get the free **Steam CWA Demo, app id
`4819000`**, and point Trident at it (recommended gitignored layout
`packages/Demo`). On this machine the full install at
`D:\Arma_CWA\ARMA Cold War Assault` also works as `OFPR_DATA_DIR`.

---

## Run the mode

**Unpacked (dev / testing — no packing needed).** The engine and the Trident
harness both load a mission straight from its folder. Copy
`guerrilla-mode/mission/Guerrilla.Demo` into your user `Missions/` directory (or
point the game's `--test-mission` at it) and launch it.

**Packaging to a `.pbo` (distribution only).** A CWA mission is a directory; the
`.pbo` is just that directory archived. This tree ships **`TcPbo`** (build target
`TcPbo` → module **`pbo`**), the repo's Total Commander **WCX plugin** for PBO
archives — it exposes the read/extract side (`OpenArchive`/`ReadHeader`/
`ProcessFile`), so you use it *inside Total Commander* to open/verify
`Guerrilla.Demo.pbo`. To **create** the pbo for release, pack the
`Guerrilla.Demo` folder with the WCX plugin (or any standard OFP/CWA `MakePbo`).
For everything in this repo — the Trident tests included — the **unpacked folder
is what runs**, so packing is optional.

---

## Run the tests

### C++ unit test (the two new engine commands)
```sh
ctest --test-dir build/win-x64-clang-rwdi -R PoseidonTests --output-on-failure
# the new cases are tagged [game][gameStateExt][ai] in test_game_state_ext.cpp
```

### Integration tests (Trident, needs Demo data)
Build the runner, copy the env template, then run by path or tag:
```sh
cargo build --manifest-path engine/Trident/Cargo.toml       # -> engine/Trident/target/debug/tri
cp .trident.env.example .trident.env                        # set OFPR_GAME_DIR + OFPR_DATA_DIR
#   OFPR_GAME_DIR = dist/win-x64-clang-rwdi   (has the binaries)
#   OFPR_DATA_DIR = packages/Demo             (the Demo data)

# all guerrilla integration tests:
tri test -j6 --retries 2 tests/integration/scripting/guerrilla_spawn_domove.test.sqf
tri test -j6 --retries 2 tests/integration/scripting/guerrilla_capture_flip.test.sqf
tri test -j6 --retries 2 tests/integration/scripting/guerrilla_save_reload.seq

# or by tag (all three carry "guerrilla"; two carry "gate-zero"):
tri test -j6 --retries 2 --tags guerrilla tests/integration
```

| Test | Kind | Proves |
|------|------|--------|
| `guerrilla_spawn_domove` | `.test.sqf` | Gate-Zero: runtime-spawn 8 AI in two groups + move a QRF; framerate-agnostic (unit counts + time-bounded displacement); no crash. |
| `guerrilla_capture_flip` | `.test.sqf` | Clearing the Outpost garrison + friendly present flips `GM_Z_OWNER`→`"GUER"` and recolors the marker red→green (drives the real `zones.sqs`). |
| `guerrilla_save_reload` | `.seq` (2 phases, shared user dir) | Sentinel globals (₽/HR/WarLevel, a zone flip, companion XP, a gear unlock) round-trip a `triSaveGame`→fresh-boot→`triLoadGame`. |

`doMove` note: it is **declared but not registered** in this engine, so the
"spawn_domove" test (like the mode) drives movement with group `move` +
`addWaypoint`/`setWaypointType`. The name keeps the plan-13 vocabulary.

---

## Gate-Zero manual checklist (from plan 13)

Do this **once, on the real build with Demo data, before trusting the mode**
(the automated tests cover 2–4 headlessly; step 1 and the framerate/pathing
*eyeball* still want a human):

1. **Stock mission renders + simulates AI at framerate** on this C++20 port.
   (Load any shipped Demo mission; confirm it draws and AI thinks.)
2. **Spawn ~6–8 AI in two groups + drive a QRF**, watching **framerate and
   pathing** on a real map. → automated by `guerrilla_spawn_domove`, but watch it
   render once to judge pathing quality and frame cost within the 12/group budget.
3. **Stuff globals, `saveGame`, reload, diff.** → automated by
   `guerrilla_save_reload`.
4. **Re-confirm the live command tables** in `GameStateExt*.cpp` (dynamic
   markers/triggers/groups). → done via source grep + the unit test.

If the port can't run a populated mission at framerate, everything else is
blocked — that is why Gate-Zero is first.

---

## What is verified vs. what still needs an in-game run

**Verified by reading engine source (this pass):**
- Every script command used is a real registered command / core operator
  (grepped in `GameStateExt*.cpp` and `Evaluator/express.cpp`). No invented
  commands; unconfirmed data-dependent classnames are tagged `// VERIFY:`.
- Cross-file wiring: every `[] exec` target exists; the whole global schema is
  internally consistent (one OWNS-writer per global); marker names match.
- **Two reconciliation fixes applied this pass** (see below).
- Save round-trips SQF globals: `World::Serialize` writes
  `ar.Serialize("GameState", GGameState, …)` (WorldImpl.cpp) — the whole game
  state, including every global variable.

**Verified only once you build + run (the automated tests do this):**
- The engine actually spawns/moves AI at framerate on Demo data (Gate-Zero #1–2).
- The capture flip and the save/reload round-trip execute end-to-end.
- The two new C++ commands (`enableAI`, `knownTargets`) compile and link, and the
  unit test passes (`ctest -R PoseidonTests`).

**The money moment — still needs a human at the controls:** the full Phase-1
*acceptance* play-through (the "visible win"): boot `Guerrilla.Demo`, walk to the
Outpost, fight/clear the ~8-man garrison for real (not the test's scripted
clear), watch the marker flip green, see income tick, recruit a fighter at the
Camp, loot toward the AK unlock, and take a companion through a promotion — then
`saveGame`, reload, and confirm the campaign resumes. That end-to-end feel is not
something a headless assertion can certify.

**Known Phase-1 limitations (documented, not bugs):**
- The `// VERIFY:` classnames (`SoldierGB`/`SoldierEB`/weapon classes/marker
  colors) are stock OFP/CWA names; confirm against the actual Demo `CfgVehicles`/
  `CfgWeapons`/`CfgMarkers` during the acceptance run and swap in place if the
  Demo dataset names them differently (never flip the GUER↔EAST side model).
- Saving while an enemy garrison is *spawned* reverts that garrison to its cached
  reserve integer on load; save at the Camp, away from live garrisons.
- Undercover cover is *established* (escalation.sqs) but the break condition
  (alert.sqs setting `gmUndercover=false`) is a Phase-2 hook, not yet wired.
