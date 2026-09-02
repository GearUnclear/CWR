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
Modding contracts: [`FACTION-PACKS.md`](FACTION-PACKS.md) (ship a roster) and
[`ISLAND-PACKS.md`](ISLAND-PACKS.md) (ship an island).

**Core requirement (delivered as data, Phase 1.5):** factions and islands are
**swappable at new-game**. Island facts (zones, positions, tuning, the civilian
population) live in the mission's `description.ext` (`CfgGuerrillaZones` /
`CfgGuerrillaFactions`); faction facts (unit tiers, vehicles, officer,
recruit/companion/loot classes) live in the global **faction library** below;
the scripts contain **zero classnames and zero side strings**. The new-game UI
publishes `gmSelOccupier`/`gmSelResistance`; the engine resolves them and hands
scripts the side strings via the `gmOccupierSide`/`gmResistanceSide` nulars
(Demo defaults: EAST vs GUER).

### Faction library

`CfgGuerrillaFactions` is **not** authored per island. The engine builds the
table as the UNION of a global block and the island template's own, the island
winning on a class-name collision (`Game/Guerrilla/FactionSources.*`), so a
roster ships once and shows up on every island:

- **Vanilla rosters (WEST / EAST / GUER)**:
  [`config/guerrilla-factions.hpp`](config/guerrilla-factions.hpp). The
  installer copies it to `<GameDir>\bin\guerrilla-factions.hpp` and makes sure
  `<GameDir>\bin\config-extra.cpp` `#include`s it; the engine merges that file
  into the global config last. An existing `config-extra.cpp` is appended to,
  never overwritten.
- **@LoBo rosters (IDF, EgyptFrontier, EgyptArmy, Syria, Jordan, Hizballah,
  PLO, PLO_East)**: `tests/fixtures/mods-lobo/@lobofixup/bin/config.cpp`, a
  mod folder's `bin\config.cpp`, mounted after `@LoBo`.
- **`class CIV` is island-owned and never global.** It names the population
  models and ambient-traffic hulls of *that island's* data set, so each
  template's `description.ext` keeps exactly that one faction class. A unit
  test pins it.
- **An island override** is a global class redeclared in a template's own
  block. It replaces the whole class (no per-key merge), needs a comment
  saying what differs, and has to be listed in that same test.
  `Guerrilla.Demo`'s `GUER` is the only one today.
- **A mod ships its own** by putting a `class CfgGuerrillaFactions` in an
  addon pbo's config, in its `<mod>\bin\config.cpp`, or in another
  `bin\config-extra.cpp` include - all three land in `Pars` and join the
  table. No template edit and no engine change are involved.

**Authoring one: [`FACTION-PACKS.md`](FACTION-PACKS.md).** It carries the full
key contract (arrays, plain values, which config bank each is probed against),
the `side` / `sideTwin` semantics, the fallback ladder per key family, the
outfit keys, the menu-gating rules, a minimal worked `config.cpp` that is a
committed test fixture, and the three validating tools with their exact
invocations and output grammar:
`PoseidonTools mod doctor`, `guerrilla lint` and `guerrilla probe`. The
annotated reference roster stays `config/guerrilla-factions.hpp`.

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
| **Journal** (native) | the map screen's Notes / Plan pages: field manual, live Situation block, running diary, objectives + next steps; fed by the scripts through `gmJournal*`, serialized as `GuerrillaJournal` | `engine/Poseidon/Game/Guerrilla/Journal.*` + `UI/Guerrilla/GuerrillaJournalPages.*` |
| **Traffic** (native) | ambient road traffic: civilian cars town-to-town, occupier patrol vehicles between occupier zones, occasional supply convoys; player-distance band spawn/despawn, commandeer sequence (stop, driver bails + flees, hull released), civ-driver killed-EH feeding the civilian kill ledger; `gmTraffic*` + the road queries `gmRoadNearest` / `gmRoadPath` / `gmRoadsNear` (`nearestRoads` alias); serialized as `GuerrillaTraffic` | `engine/Poseidon/Game/Guerrilla/Traffic.*` + `TrafficCommands.cpp` |
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

The script core exists ONCE, in `core/`. A mission template is data plus a
two-line bootstrap; no template carries a `scripts/` directory.

```
core/                 THE shared script core (installed to <GameDir>\gmcore)
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
    market.sqs        HQ / cache / garage / dealer action menus + the purchase
                      debit (+ market_action.sqs dispatcher); the native
                      GuerrillaBase + Market own the facts (CfgGuerrillaMarket
                      in description.ext = the dealer stock)
    companions.sqs    companion XP -> rank/skill + permadeath
    civilians.sqs     town population cache + kill queue + panic FSM
    shakedown.sqs     occupier street theatre + resentment ticker

config/               THE global faction library (installed to <GameDir>\bin)
  guerrilla-factions.hpp  CfgGuerrillaFactions: the vanilla WEST/EAST/GUER
  config-extra.cpp        the seed bin\config-extra.cpp that #includes it

mission/Guerrilla.Demo/
  mission.sqm         one resistance player (SoldierGB) at the Camp
  description.ext     THE per-island data file: CfgGuerrillaZones (tuning +
                      zone seed + seedCities=1), CfgGuerrillaFactions (CIV +
                      any island override) and CfgGuerrillaMarket
  init.sqs            TWO LINES: a comment + [] exec "\gmcore\init.sqs"
```

The leading backslash is load-bearing: `OpenScript` strips it and resolves the
rest against the game data root (pbo banks first, then a loose file), so the
mission reaches `<GameDir>\gmcore` instead of its own folder. Core scripts
name their siblings the same way, `"\gmcore\scripts\<x>.sqs"`, including the
`addAction` dispatch paths. Mission-local scripts keep relative paths. See
ARCHITECTURE.md A.6.

Reference slices next to the templates: `Qrf.Abel` (alert -> QRF),
`Undercover.Abel` (disguise) and `Market.Abel` (HQ / cache / garage /
dealers, a 50000 R treasury and teleport debug actions) - each runs its own
bootstrap that execs the ONE core policy script it demonstrates, and each has
a direct main-menu button once installed.

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
`packages/Demo`). On this machine two packages already exist: the full 1.99
install `D:\Arma_CWA\ARMA Cold War Assault [Classic]` (the default
`OFPR_DATA_DIR`) and the official remaster Demo at
`D:\Arma_CWA\Arma Cold War Assault Demo [Remaster]`; pass the latter per run with
`tri test --data-dir "…Demo [Remaster]"`. The Demo package is a content subset —
see the Tests section for why the Demo-world Guerrilla tests don't run on it yet.

---

## Run the mode

**Unpacked (dev / testing — no packing needed).** The engine and the Trident
harness both load a mission straight from its folder. Copy
`guerrilla-mode/mission/Guerrilla.Demo` into your user `Missions/` directory
(or point the game's `--test-mission` at it) and launch it. It needs the
shared core beside it: `guerrilla-mode/core` copied to `<GameDir>\gmcore`, or
just run the installer below, which does both.

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
`mission/*` template there, plus the global faction library at `<GameDir>\bin`
and the shared script core at `<GameDir>\gmcore` (both go first: a template
without the core boots into a mission with no managers, and without the
library the cyclers offer only what that island declares itself):

```powershell
guerrilla-mode\install-missions.ps1                 # default D:\Arma_CWA\ARMA Cold War Assault [Classic]
guerrilla-mode\install-missions.ps1 -GameDir <dir>  # any other install
```

Idempotent: re-run it after editing a template, the core **or the faction
library**.

**A template installs when its world is there, and the installer finds the
world itself.** It looks for `<World>.wrp` in `<GameDir>\Worlds`, then in the
entry tables of every `*.pbo` under `AddOns\`, `Dta\` and each mod folder's
`addons\` (mod folders = every `@*` beside the game dir and inside it, or
whatever `-ModDir` names). Only the pbo *header* is read, never the body, so
scanning a full install costs a second. That is why worlds shipped inside a
mod pbo - @LoBo's Sinai and Lebanon80 - no longer need `-IncludeWorld`; the
flag survives as a manual override. Each installed template prints where its
world was found; a template whose world is nowhere is skipped, and a stale
install of it is removed (a mission on a missing world silently keeps the
current island and drowns the player at sea).

### Start a Sinai campaign (@LoBo)

[`mission/Guerrilla.Sinai/`](mission/Guerrilla.Sinai/) is the second island
pack and the **reference for authoring new ones**: the same two-line
`init.sqs` into the one shared core, all island facts in its
`description.ext`, its war rosters in the @LoBo faction library. It
deliberately flips the sides:
**occupier = IDF (WEST), resistance = Egyptian Frontier Corps (EAST)** — to
prove nothing in the core assumes who resists (`defaultOccupier="IDF"` /
`defaultResistance="EgyptFrontier"` cover direct launches with no UI
selection).

1. Install the templates (above).
2. Enable the **@LoBo** mod (Sinai world + IDF/Egypt classes;
   `D:\Arma_CWA\@LoBo` on this machine) — plus the repo fixture
   `tests/fixtures/mods-lobo/@lobofixup`, mounted **after** it. That fixture
   patches @LoBo's malformed `tracerColor` floats (script errors under
   `--autotest`), preloads the addons the template spawns from, and carries
   the whole @LoBo `CfgGuerrillaFactions` roster. Without it the Sinai and
   Lebanon80 templates have no war factions at all: their blocks moved out of
   `description.ext` into that config (issue #54 A4).
3. Main menu → **GUERRILLA** → pick *Southern Sinai* in the island list,
   cycle OCCUPIER/RESISTANCE, OK. You spawn as a lone Frontier Corps
   rifleman at the Camp, with an IDF checkpoint 500 m east.

**Authoring a new island pack = run the scaffold**, then edit what it marks as
placeholders. [`ISLAND-PACKS.md`](ISLAND-PACKS.md) is the full contract; the
short form is:

```powershell
dist\x64-win-rwdi\PoseidonTools.exe guerrilla scaffold `
  --world <CfgWorlds class> --data-dir <package> [--mod <folder>] `
  --out guerrilla-mode\mission\Guerrilla.<class>
```

It reads the world's `CfgWorlds` entry and `.wrp`, seeds CITY zones from the
`Names` entries that pass the engine's own settlement test, places a CAMP and N
OUTPOSTs on flat dry off-road ground, samples every elevation from the
heightmap, and writes the three files a template consists of: `description.ext`,
`mission.sqm` and the two-line `init.sqs`. Re-run with `--keep-zones` to keep
hand-edited zones. Afterwards: point `defaultOccupier` / `defaultResistance` at
real faction classes, replace the `class CIV` stub if your data set ships its
own civilians, walk the placed zones in the editor, and add a
`CfgGuerrillaMarket` block if the island should have dealers. `mission.sqm`
needs only its world and the player's own addon in `addOns[]` (the engine walks
the rest at launch), and the `init.sqs` bootstrap is never edited.

Integration coverage: `tests/integration/scripting/guerrilla_sinai_swap.test.*`
(direct template launch on Sinai: flipped sides, town auto-seed, native
garrison spawn, marker colors) and
`tests/integration/ui/guerrilla_new_game_e2e.test.*` (the full menu flow
above, asserting the `gmSel*` publication and side resolution in-mission).
Both need the full CWA install + @LoBo (tags `full_cwa`, `lobo`) and the
templates installed.

### Start a Lebanon campaign (@LoBo)

[`mission/Guerrilla.Lebanon80/`](mission/Guerrilla.Lebanon80/) is the third
island pack: South Lebanon on the @LoBo *Lebanon (80's)* world
(`Lebanon80`, inside `@LoBo\addons\LoBo_Leb.pbo`), **occupier = IDF (WEST),
resistance = Hizballah (EAST)**. You spawn as a lone Hizballah rifleman at a
hill camp north of the Litani, with an IDF checkpoint 500 m east and the
Marjayoun barracks beyond it. Unlike Sinai it sets `seedCities=0`: the
Lebanon80 `Names` block is a theater map (seas, countries, mountain ranges),
not a town list, so the towns in play (Tyre, Saida, Ghajar) are explicit
hand-placed zones instead. Setup is identical to Sinai (install templates,
mount @LoBo + @lobofixup); the world's `.wrp` also lives inside a mod pbo,
which the installer's world gate now detects on its own.
Integration coverage:
`tests/integration/scripting/guerrilla_lebanon80_boot.test.*` (direct
template boot: side resolution, exact 6-zone seed, native garrison spawn,
marker colors).

---

## Tests

The native systems carry unit coverage over their pure cores
(`ZoneRegistry::EvaluateTick`, `AlertMachine::EvaluateAlert`,
`GarrisonCache::Decide`/`PlanGroups`):

```sh
ctest --test-dir build/win-x64-clang-rwdi -R PoseidonTests --output-on-failure
```

The Trident integration suite is fully migrated to the `gm*` command surface
(the retired `GM_ZONES`/`zones.sqs` spine survives only in comments). The
`guerrilla_native_*` twins run against the full 1.99 install (`full_cwa`), and
`guerrilla_sinai_swap` + `ui/guerrilla_new_game_e2e` additionally need @LoBo
(`lobo`), the generated fixture PBOs, and installed templates.

The three Demo-world tests (`guerrilla_capture_flip`, `guerrilla_spawn_domove`,
`guerrilla_save_reload.seq`) bind to the `demo` world and their player +
resistance units are GUER classes (`SoldierGB`, …). The local remaster Demo
package (`Arma Cold War Assault Demo [Remaster]`) supplies the world
(`demo\demo.wrp`, byte-identical to `abel.wrp`) but **not** the GUER roster —
its `CONFIG.BIN` has no `SoldierG*` except `SoldierGFakeE` (issue #13) — so a
`tri --data-dir "…Demo [Remaster]"` run boot-fails on `SoldierGB` (strict-mode
fatal within ~60 ms; verified 2026-07-08). They become runnable once a
Demo-package faction descriptor remaps the resistance onto Demo's EAST/WEST
classes, or the missions' Gate-Zero class substitution lands. See
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
- `core/scripts/` is island/faction-agnostic: grep for classnames /
  side-string literals returns only comments.

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
