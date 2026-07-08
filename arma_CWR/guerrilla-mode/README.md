# Guerrilla Mode (Uslu dur! / Poseidon)

The flagship game mode of this project's **total game overhaul** of Arma: Cold
War Assault — a persistent, single-player **insurgency / territory-control**
game mode. You are one resistance fighter who liberates an island
zone-by-zone: capture outposts, build a cell (Resources ₽ + Manpower HR),
recruit and train fighters, unlock looted gear, level a named companion, and
hold ground against an escalating occupier that spawns garrisons and QRFs and
runs a graduated alert state machine — all round-tripping through
`saveGame`/`loadGame`.

Master design: [`../mod-plans/13-guerrilla-mode.md`](../mod-plans/13-guerrilla-mode.md).
Architecture: [`ARCHITECTURE.md`](ARCHITECTURE.md) — §A describes the current
hybrid (native core + event-driven scripts); the old all-SQS spine sections
are kept there marked SUPERSEDED.

**Core requirement (delivered as data, Phase 1.5):** factions and islands are
**swappable at new-game**. All island facts (zones, positions, tuning) and
faction facts (unit tiers, vehicles, officer, recruit/companion/loot classes)
live in the mission's `description.ext` (`CfgGuerrillaZones` /
`CfgGuerrillaFactions`); the scripts contain **zero classnames and zero side
strings**. The new-game UI publishes `gmSelOccupier`/`gmSelResistance`; the
engine resolves them and hands scripts the side strings via the
`gmOccupierSide`/`gmResistanceSide` nulars (Demo defaults: EAST vs GUER).

---

## What this is (the C++ ↔ script split, Phase 1.5)

The simulation CORE is **native C++** (`engine/Poseidon/Game/Guerrilla/`),
activated only when a mission defines `class CfgGuerrillaZones` — no config,
no overhead, ordinary missions unaffected. The mission scripts are a thin
**event-driven policy layer**.

| Layer | What it owns | Where |
|-------|--------------|-------|
| **ZoneRegistry** (native) | zone table from config, fog-of-war reveal, marker repaint, capture flips, CITY support, heat spikes/income taps, zone events | `engine/Poseidon/Game/Guerrilla/ZoneRegistry.*` |
| **AlertMachine** (native) | per-zone GREEN/YELLOW/RED FSM (knowsAbout bands, disengage window), last-known position, undercover-break detection, alert events | `engine/Poseidon/Game/Guerrilla/AlertMachine.*` |
| **GarrisonCache** (native) | occupier garrison distance-cache (reserve ↔ live groups), officer-first spawn from faction data, survivor write-back, garrison events | `engine/Poseidon/Game/Guerrilla/GarrisonCache.*` |
| **Native persistence** | zones/alert/garrison + registered event handlers serialize; `campaignLoaded` event fires after a load | the three `Serialize` impls + `World::Serialize` |
| **Mission scripts** (policy) | capture reaction (hold garrison), QRF + garrison posture, undercover establish/react, economy, War Level, loot/unlocks, recruiting, companions, Save UX | [`mission/Guerrilla.Demo/`](mission/Guerrilla.Demo/) |

Scripts talk to the core through the `gm*` command surface (`gmZoneCount`,
`gmZone`, `gmZoneSet`, `gmZoneAlert`, `gmZoneLastKnown`, `gmGarrison*`,
`gmFactionTierClass`/`gmFactionVehicle`/`gmFactionValue`, `gmBreakUndercover`,
and the `gm*OnEvent` registrations) — see ARCHITECTURE.md §A.3.

### Dialect
SQS for the bootstrap + manager loops (`init.sqs` auto-loads; `~N`, `@cond`,
`#label`/`goto`). Synchronous helpers are `{…}` code values. The modernized
evaluator also provides `compile`, `isNil`, `setVariable`/`getVariable`,
`nearestObjects`, `distance` on position arrays, `setRank` and
`doMove`/`commandMove`. Native event handlers cannot wait, so every handler is
one line that enqueues into a `gmEvt*` global; each queue has exactly one
consuming manager (the pattern is documented in `init.sqs` and
ARCHITECTURE.md §A.4).

### Mission layout
```
mission/Guerrilla.Demo/
  mission.sqm         one resistance player (SoldierGB) at the Camp
  description.ext     THE per-island data file: CfgGuerrillaZones (tuning +
                      zone seed + seedCities=1) + CfgGuerrillaFactions
  init.sqs            THIN bootstrap: script-state seed, zone markers,
                      native event-handler registration, exec managers
  scripts/
    lib.sqs           helpers + GM_LIB_READY handshake
    capture.sqs       "captured" event -> hold garrison + notify
    qrf.sqs           alert policy: posture edges, YELLOW investigate, RED QRF
    undercover.sqs    cover establish + fired-EH + break reaction
    campaign.sqs      Save action + campaignLoaded reconciliation
    economy.sqs       ₽/HR income tick
    escalation.sqs    War Level ladder + Heat decay (GREEN-gated)
    loot.sqs          loot-on-kill stash + gear unlocks
    recruit.sqs       Camp action menu (+ recruit_action.sqs dispatcher)
    companions.sqs    companion XP -> rank/skill + permadeath
```

**Coordinate order note:** zone `position[]` in `description.ext` is authored
in **getPos order `[easting, northing, elevation]`** — the engine documents
and converts this (`ZoneRegistry::LoadZones`). Do **not** copy positions from
`mission.sqm`, whose `position[]` middle element is the *elevation*.

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
`D:\Arma_CWA\ARMA Cold War Assault [Classic]` also works as `OFPR_DATA_DIR`.

---

## Run the mode

**Unpacked (dev / testing — no packing needed).** The engine and the Trident
harness both load a mission straight from its folder. Copy
`guerrilla-mode/mission/Guerrilla.Demo` into your user `Missions/` directory
(or point the game's `--test-mission` at it) and launch it.

**Packaging to a `.pbo` (distribution only).** A CWA mission is a directory;
the `.pbo` is just that directory archived — pack with any standard OFP/CWA
`MakePbo` (the in-repo `TcPbo` WCX plugin covers the read/verify side inside
Total Commander). For everything in this repo the **unpacked folder is what
runs**, so packing is optional.

### Install the new-game templates

The GUERRILLA button on the main menu launches the template mission
`missions\Guerrilla.<island>` resolved **relative to the game data dir**
(either `Missions\Guerrilla.<island>.pbo` or the unpacked
`Missions\Guerrilla.<island>\mission.sqm` — `DisplayMain::OnChildDestroyed`,
`IDD_GUERRILLA_NEW_GAME` case in `UI/OptionsUIApp.cpp`). Install every
`mission/Guerrilla.*` template there with:

```powershell
guerrilla-mode\install-missions.ps1                 # default D:\Arma_CWA\ARMA Cold War Assault [Classic]
guerrilla-mode\install-missions.ps1 -GameDir <dir>  # any other install
```

Idempotent — re-run it after editing a template.

### Start a Sinai campaign (@LoBo)

[`mission/Guerrilla.Sinai/`](mission/Guerrilla.Sinai/) is the second island
pack and the **reference for authoring new ones**: same `init.sqs` +
`scripts/` byte-for-byte (a parity test enforces it), all island/faction
facts in its `description.ext`. It deliberately flips the sides —
**occupier = IDF (WEST), resistance = Egyptian Frontier Corps (EAST)** — to
prove nothing in the core assumes who resists (`defaultOccupier="IDF"` /
`defaultResistance="EgyptFrontier"` cover direct launches with no UI
selection).

1. Install the templates (above).
2. Enable the **@LoBo** mod (Sinai world + IDF/Egypt factions;
   `D:\Arma_CWA\@LoBo` on this machine) — plus the repo fixture
   `tests/fixtures/mods-lobo/@lobofixup`, which patches @LoBo's malformed
   `tracerColor` floats (script errors under `--autotest`), preloads the
   addons the template spawns from, and provides the menu-level
   `CfgGuerrillaFactions` the faction cyclers list.
3. Main menu → **GUERRILLA** → pick *Southern Sinai* in the island list,
   cycle OCCUPIER/RESISTANCE, OK. You spawn as a lone Frontier Corps
   rifleman at the Camp, with an IDF checkpoint 500 m east.

Authoring a new island pack = copy `Guerrilla.Sinai`, keep `scripts/` +
`init.sqs` untouched, rewrite `description.ext` (zones + factions +
`defaultOccupier`/`defaultResistance`) and `mission.sqm` (player class/side/
position + the **transitive** `addOns[]` set — a class is only buildable when
its addon *and the addons of every weapon/magazine it references* are listed;
a miss aborts the mission boot with "StartAutoTest could not boot").

Integration coverage: `tests/integration/scripting/guerrilla_sinai_swap.test.*`
(direct template launch on Sinai: flipped sides, town auto-seed, native
garrison spawn, marker colors) and
`tests/integration/ui/guerrilla_new_game_e2e.test.*` (the full menu flow
above, asserting the `gmSel*` publication and side resolution in-mission).
Both need the full CWA install + @LoBo (tags `full_cwa`, `lobo`) and the
templates installed.

---

## Tests

The native systems carry unit coverage over their pure cores
(`ZoneRegistry::EvaluateTick`, `AlertMachine::EvaluateAlert`,
`GarrisonCache::Decide`/`PlanGroups`):

```sh
ctest --test-dir build/win-x64-clang-rwdi -R PoseidonTests --output-on-failure
```

The Trident integration suite is fully migrated to the `gm*` command surface
(the retired `GM_ZONES`/`zones.sqs` spine survives only in comments). Demo-data
tests (`guerrilla_capture_flip`, `guerrilla_spawn_domove`,
`guerrilla_save_reload.seq`) run headless with the free Steam Demo; the
`guerrilla_native_*` twins need the full CWA install (`full_cwa`), and
`guerrilla_sinai_swap` + `ui/guerrilla_new_game_e2e` additionally need @LoBo
(`lobo`), the generated fixture PBOs, and installed templates. See
[`STATUS.md`](STATUS.md) for the full table.

---

## What is verified vs. what still needs an in-game run

**Verified by reading engine source (this pass):**
- Every native command the scripts use is registered
  (`ZoneRegistryCommands.cpp`, `GarrisonCacheCommands.cpp`; evaluator
  additions in `express.cpp`/`GameStateExt.cpp`).
- Event payloads and dispatch (`_this` binding) match the handlers
  `init.sqs` registers; handlers and queues both serialize, so the event
  plumbing survives save/load with no re-arm.
- **Created markers survive a load** (markersMap is saved by
  `World::Serialize` → `AIGlobalSerialize`) — the old on-load marker rebuild
  is correctly gone.
- `scripts/` is island/faction-agnostic: grep for classnames / side-string
  literals returns only comments.

**Still needs a human at the controls (the money moment):** boot
`Guerrilla.Demo`, walk to the Outpost, clear the ~8-man garrison, watch the
marker flip green and a hold garrison appear, see income tick, recruit at the
Camp, loot toward the AK unlock, trip a YELLOW→RED alert and meet the QRF,
take Petra through a promotion — then Save, reload, and confirm the campaign
resumes (zones/alert/garrisons native, companion rebuilt, one Save action).

**Known limitations (documented, not bugs):**
- The Demo faction classnames in `description.ext` are stock OFP/CWA names;
  confirm against the actual Demo `CfgVehicles`/`CfgWeapons` during the
  acceptance run and fix them **in the config** (scripts need no change).
- `seedCities=1` makes town income and the War-Level denominator scale with
  the island's town count — retune `GM_ECON_*` / the ladder there if the Demo
  world over- or under-pays.
- Undercover re-establishment after a break (disguise swap) remains a
  Phase-2 hook, as in Phase 1.
