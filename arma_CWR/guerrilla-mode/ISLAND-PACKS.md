# Authoring a Guerrilla island pack

An **island pack** is a mission template that turns one world into a playable
Guerrilla Mode campaign: a zone table, a civilian population, a market, and a
player standing in the camp. This document takes a contributor from a `.wrp`
to a campaign the GUERRILLA new-game screen can launch.

The short version:

```powershell
dist\x64-win-rwdi\PoseidonTools.exe guerrilla scaffold `
  --world Eden --data-dir "D:\Arma_CWA\ARMA Cold War Assault [Classic]" `
  --out arma_CWR\guerrilla-mode\mission\Guerrilla.Eden
guerrilla-mode\install-missions.ps1
.\run-game.ps1 -Mission 'Missions\Guerrilla.Eden'
```

Then edit the four things the tool marks as placeholders (section 5) and run
the acceptance checks (section 9).

The faction half is [`FACTION-PACKS.md`](FACTION-PACKS.md). The reference
island to copy from is
[`mission/Guerrilla.Sinai/`](mission/Guerrilla.Sinai/).

---

## 1. What a world needs

| Requirement | Read by | If it is missing |
|-------------|---------|------------------|
| A `CfgWorlds >> <Class>` entry with `description` and `worldName` (the `.wrp` path), plus a `CfgWorldList` entry | the island list in the new-game screen and the editor | the world never appears anywhere. |
| A `Names` block | `ZoneRegistry::SeedCityZones`, the START TOWN cycler | no towns are auto-seeded; every CITY zone has to be hand-placed. |
| A **road net** (road-model objects in the `.wrp`) | `GRoadNet`, the native Traffic service, the scaffold | ambient traffic goes inert: routes, spawn points and the commandeer sequence all bottom out in `GRoadNet`. The campaign still plays; the roads are just empty. |
| **Enterable buildings** (a Paths LOD, at least `hqMinPos` AI positions) inside a zone | `GuerrillaBase::PickBuilding` | the headquarters cache cannot go indoors; it falls back to an off-road, dry spot on the outer rings of the zone area. A working HQ, outdoors. |
| At least a few **CITY zones** | `Market`, `TownFlags`, `economy.sqs` | no weapon or vehicle dealers (they are seeded across CITY zones only), no town flags, and the support half of the economy pays nothing. |

Nothing above is fatal. The mode degrades feature by feature and logs why. What
it cannot do without is a world entry and a `.wrp` the package can load.

**The smallest pack that satisfies all of it** is the repo's own synthetic
island, `tests/fixtures/mods-island/@udisland` (written from scratch by
`generate_udisland.py`, issue #56 task 7): one pbo holding a 64 x 64-cell
`.wrp`, a 10 m house box tagged `class=house`, a road slab, and a `config.cpp`
with `CfgPatches` (`worlds[]`), `CfgWorldList`, a `CfgWorlds` entry with three
`Names` towns, and a `House`-derived class per house model so the runtime turns
the placed shapes into `Building` objects (the settlement probe counts
Buildings, not shapes). It scaffolds on the config-only
`tests/fixtures/packages/mini` package in CI and the resulting
`tests/integration/missions/Guerrilla.UdIsland` boots on the Classic package
(`guerrilla_udisland_boot`). Read it as the minimum a world addon must carry.

**Names blocks vary wildly.** OFP-era entries carry only a name and a 2D
`position[]` with no `type`, so type-less entries are accepted as towns.
Arma-style entries carry `type`, and only `NameCity`, `NameCityCapital` and
`NameVillage` count. Sea and terrain labels are dropped by the classifier, not
by you: see section 6.

## 2. Naming

A template folder is always **`Guerrilla.<CfgWorlds class>`**, never the display
name. `Guerrilla.Abel`, not `Guerrilla.Malden`. The island list *shows* the
display name but *acts on* the class name, and the launch path builds
`missions\Guerrilla.<class>` from it. A display-named folder lists fine in the
repo and is then never found. A unit test pins the rule; see
[`WORLD-NAMES.md`](WORLD-NAMES.md) for the class-vs-display convention and for
how to read a package's world list out of its shipped configs.

## 3. Run the scaffold

`PoseidonTools guerrilla scaffold` reads `CfgWorlds >> <class>`, opens the
world's `.wrp`, samples real ground heights, classifies the placed objects into
roads and buildings, and writes a whole template.

```powershell
# a base-game world
dist\x64-win-rwdi\PoseidonTools.exe guerrilla scaffold `
  --world Eden `
  --data-dir "D:\Arma_CWA\ARMA Cold War Assault [Classic]" `
  --out arma_CWR\guerrilla-mode\mission\Guerrilla.Eden

# a world that lives inside a mod pbo
dist\x64-win-rwdi\PoseidonTools.exe guerrilla scaffold `
  --world sinai `
  --data-dir "D:\Arma_CWA\ARMA Cold War Assault [Classic]" `
  --mod D:\Arma_CWA\@LoBo `
  --out C:\temp\Guerrilla.Sinai
```

(`Guerrilla.Sinai` already exists in this repo as the reference island, so that
second run writes somewhere scratch. Point `--out` at
`guerrilla-mode\mission\Guerrilla.<class>` when you are scaffolding a world that
has no template yet.)

Options: `--outposts <0-32>` (default 3, the OUTPOST total: detected outposts
first, see section 10), `--play-region minX minZ maxX maxZ` (writes
`playRegion[]` and keeps the towns, camp and outposts inside it) and
`--keep-zones`, which splices the existing `<out>/description.ext`
`class Zones { ... }` block in verbatim instead of generating a new one, so hand
edits survive a re-run. `--mod` is repeatable and in `-mod` order.

The scaffold also runs the base detector and writes the AIRFIELD, SEAPORT and
OUTPOST zones the world models, each with its evidence (section 10).

**The output directory holds exactly three files**, which is the whole of a
template:

```
Guerrilla.Eden/
  description.ext    zones, tuning, the CIV block, the default* placeholders
  mission.sqm        one player unit + the minimal addOns[]
  init.sqs           two lines, into the shared core
```

There is no `scripts/` directory. Do not add one: the managers come from
`<GameDir>\gmcore` (section 8).

### What it prints

```
WORLD Eden "Everon" worlds\eden.wrp (OPRW v2, 256x256 at 50 m, 56740 objects)
CLASSIFIED roads=4670 buildings=387
PLAYER SoldierGB side=GUER at 2378,7049
ADDONS (none: base-game world and player)
CIV classes=10 vehicles=9
BASE seed AIRFIELD Everon Airfield 4841 11323 conf=0.90
  evidence: ...
BASE seed SEAPORT Chotain Harbour 7358 6803 conf=0.70
BASE offline OUTPOST Saint Phillippe Post 5122 10475 conf=0.90
BASE offline OUTPOST Levie Post 7524 4182 conf=0.90
ZONE CAMP      RESISTANCE Camp                       2373   7045    53.7
ZONE AIRFIELD  OCCUPIER   Everon Airfield            4841  11323    ...
...
SUMMARY zones=24 cities=18 outposts=3 (detected 2) bases=4 warnings=0
WROTE ...\Guerrilla.Eden (description.ext, mission.sqm, init.sqs)
```

Every refusal is a `WARN` line naming the entry and the reason, which is the
fastest way to see what a world's `Names` block actually contains. Sinai, whose
block is a mix of towns and theatre labels:

```
WARN town 'Eilat' skipped: sampled height 0.0 m, it is in the water
WARN town 'Coloured Canyon' skipped: 0 building(s) within 300 m, a town needs 3
WARN town 'Mt.Sinai' skipped: 1 building(s) within 300 m, a town needs 3
```

### A generated zone

```cpp
class Camp
{
    // 133 m to the nearest road, 1501 m to the nearest town, 1.3 m of relief across 40 m
    name = "Camp";
    type = "CAMP";
    owner = "RESISTANCE";
    garrison = 0;
    income = 0;
    support = 100;
    marker = "gmZoneMarker_0";
    position[] = {2373, 7045, 53.7};
};
```

The comment above each zone is its provenance: for a town, the `Names` entry it
came from and the building count that qualified it; for a placed zone, the
distances that satisfied the placement rules.

### What the scaffold knows, and what it does not

Placement rules (`Game/Guerrilla/IslandScaffold.hpp` carries the numbers): the
CAMP goes on dry, flat, off-road ground at the **smallest** town distance that
still clears 1500 m, which puts it on the rim of the settled area rather than
on the far edge of the map; OUTPOSTs go 150 to 400 m off a road, within 2500 m
of a town, at least 800 m apart. Towns use the engine's own numbers (300 m
dedup, 3 house-sized buildings within 300 m) so the scaffolded CITY set and the
runtime classifier agree.

Two real limits:

1. **Object classification is name-based.** A headless `.wrp` read gives model
   paths, not shapes, so `ModelIsRoad` / `ModelIsBuilding` match the OFP and
   Resistance model vocabulary (`data3d\`, `o\road\`, `o\hous\`) plus generic
   directory tokens. A mod that names its houses something unrelated in a
   directory that says nothing will be missed, and the `CLASSIFIED` counts are
   how you catch that.
2. **It knows terrain, not architecture.** Water, slope and road distance are
   real measurements. Fences, compounds, walls and minefields are invisible to
   it, so a spot can be legal and still be walled in. Walk the CAMP and the
   OUTPOSTs in the editor before shipping.

Elevations are never guessed: heights come from the `.wrp` heightmap through
the same triangle interpolation `Landscape::SurfaceY` uses, so a scaffolded
elevation is the number the running engine reports for that spot.

## 4. The zone schema

`class CfgGuerrillaZones` in `description.ext` is the island's data file.
`ZoneRegistry::LoadZones` reads the tuning keys off the class itself and the
zones out of `class Zones`.

### Per-zone keys

| Key | Default | Meaning |
|-----|---------|---------|
| `name` | the class name | Display name, and the **savegame key**. Must be unique. |
| `type` | `"OUTPOST"` | `CAMP`, `OUTPOST`, `CITY`, `AIRFIELD`, `SEAPORT`. See below. |
| `owner` | `"NEUTRAL"` | `"OCCUPIER"` / `"RESISTANCE"` (generic tokens, resolved per campaign) or a literal side string. The raw value is kept and re-resolved, so the token survives a faction swap. |
| `garrison` | 0 | Reserve garrison strength the GarrisonCache spawns from. |
| `garrisonMax` | `garrison` | Ceiling for off-screen reserve recovery (issue #71 D2: a distant, calm post regains 1 body per `garrisonRecoverInterval`). |
| `support` | 0 | Starting population support (CITY zones). |
| `income` | 0 | Resources per tick when resistance-held. |
| `heat` | 0 | Starting heat. |
| `marker` | none | Map marker name the registry repaints. Convention: `gmZoneMarker_<n>`. |
| `captureRate` | 0 | Per-zone capture pacing override; 0 means use the tuning value. |
| `position[]` | none | **getPos order: `{easting, northing, elevation}`.** |

**Coordinate order is load-bearing.** `position[]` here is `{easting, northing,
elevation}`. `mission.sqm`'s `position[]` is `{easting, ELEVATION, northing}`.
Do not copy positions between the two files without swapping.

**Only `CITY` has engine meaning.** Market dealers are seeded across CITY zones,
`TownFlags` flies flags on them, Traffic treats them as its town nodes, and the
GarrisonCache treats them differently from installations. The script layer adds
`CAMP` (the recruiter anchor: `recruit.sqs` finds the *first* zone of type
`CAMP`) and pays installation income for `OUTPOST`, `AIRFIELD` and `SEAPORT`.
Any other string is a generic non-city zone: it captures and garrisons
normally, but nothing keys on the label.

### Campaign-level keys

```cpp
class CfgGuerrillaZones
{
    defaultOccupier = "EAST";       // used when no new-game selection was published
    defaultResistance = "GUER";     // (single-mission browser, --test-mission)
    playerSide = "GUER";            // the side mission.sqm welds the player to
    seedCitySupport = 20;
    // seedCities: see section 6
    // seedBases, playRegion[]: see section 10
    class Zones { /* ... */ };
};
```

- **`defaultOccupier` / `defaultResistance`** name classes in the **merged**
  faction table, so a new island usually declares no war roster at all: it names
  rosters that already exist globally or in a mounted pack. Side precedence is
  `gmSel*` (the new-game UI) > these keys > the built-in EAST/GUER.
- **`playerSide`** must be the side the `mission.sqm` player is on. The engine
  counts resistance presence out of that side's center, so the resistance cycler
  picks a **roster**, not a side, and the roster is pinned onto `playerSide`.
  Leaving `playerSide` unset disables that whole rebase pass and reverts to the
  older behaviour where the menu simply blocks a same-side pair.
- **`islandPhrase`** (optional) is how the generated campaign history names the
  world as a place, preposition first: `"in the Sinai"`, `"in Lebanon"`. Absent,
  the history reads "on <world name>", which is right for an island; author it
  for a mainland or regional world. The phrase without its preposition doubles
  as the noun ("the people of the Sinai").
- Everything else in `ZoneTuning` (`tickInterval`, `zoneArea`, `revealRadius`,
  `cacheRadius`, `supportRate`, `supportFlip`, `heatCapSpike`, `defaultIncome`,
  `holdCount`, the `capture*` / `support*` / `contest*` rates, plus the alert,
  undercover, headquarters and `traffic*` keys) is optional and documented in
  `Guerrilla.Abel/description.ext`, which lists every stock default in a comment
  rather than restating them as keys.

## 5. The four things to edit after scaffolding

The generated `description.ext` says this in its own header:

1. **`defaultOccupier` / `defaultResistance` are placeholders.** Set them to
   faction class names your data set actually ships, and set `playerSide` to
   match the `mission.sqm` player.
2. **Walk the CAMP and the OUTPOSTs in the editor.** See the limits in section
   3.
3. **Rename the OUTPOSTs to real places**, and tune `garrison` / `income`.
   Check the network against the coverage targets C1 to C7 in
   [OFFSCREEN-CONTRACT.md](OFFSCREEN-CONTRACT.md) section 5: every airfield,
   harbour and garrisoned compound the map models should be a zone of the
   matching type, with a rising garrison ladder and an occupier pair within
   3 km of the Camp. The scaffold writes the airfields, harbours and
   military compounds it detects (section 10); check them, and add what the
   detector cannot see by hand.
4. **Add a `CfgGuerrillaMarket` block** if this island should have dealers.

## 6. `seedCities` is tri-state

The town auto-seed is controlled by one optional key, and *absent* is a real
third state, not a synonym for either value:

| Value | Mode | Behaviour |
|-------|------|-----------|
| key absent | **Auto** (recommended) | Every town-typed or type-less `Names` entry is put through the settlement classifier: **dry land** (`SurfaceY` above sea level) with **at least 3 house-sized buildings** (bounding sphere > 4 m) **within 300 m**. Refusals log at INFO with the reason. |
| `seedCities = 1` | All | The legacy override: seed every town-typed / type-less entry with no classification. |
| `seedCities = 0` | Off | Seed nothing. Hand-place every CITY zone. |

Auto is what lets a theatre map work unattended. On Lebanon80's `Names` block
it refuses all thirteen labels (the two seas, the four country names, the
ranges, the two bridges, UNDOF) and accepts the nine places that really have
houses modelled (Beirut, Haifa, Damascus, the airports, Sabra, Shatila, ...).
Those are towns; whether they belong to YOUR campaign is a play-area decision
no classifier can make, which is exactly what the `= 0` override is for: the
shipped `Guerrilla.Lebanon80` keeps it and hand-places its three towns in play,
while `tests/integration/missions/guerrilla_names.Lebanon80` (the same file
without the key) pins the classifier's verdict with an exact zone count
(`guerrilla_names_classify`). Read the INFO lines on first boot and decide.

Seeded zones are deduped by name and at 300 m against every authored zone, get
`type = "CITY"`, `owner = "NEUTRAL"`, `support = seedCitySupport` and markers
named `gmZoneCity_<n>`.

One asymmetry worth knowing: at **menu** time no landscape is loaded, so the
classifier cannot run. The START TOWN cycler under Auto therefore lists every
town-typed or type-less entry, and the mission may seed fewer. A pick that
becomes no zone falls back to the camp.

## 7. The CIV block

`class CfgGuerrillaFactions` in an island template carries **exactly one class,
`CIV`**, and never a war roster. See
[`FACTION-PACKS.md`](FACTION-PACKS.md#8-class-civ-is-island-owned) for why the
rule exists. The scaffold writes a probed stub:

```cpp
class CfgGuerrillaFactions
{
    class CIV
    {
        side = "CIV";              // exactly "CIV", not "CIVILIAN"
        civClassCount = 3;         // numbered keys, not an array
        civClass1 = "Civilian";
        civClass2 = "Civilian2";
        civClass3 = "Civilian3";
        civVehicles[] = {"Skoda", "SkodaBlue", "Rapid", "Trabant", "Bus"};
    };
};
```

The scaffold only knows the stock `Civilian*` and car classnames, because
nothing in a config marks a class as "civilian population" or "traffic hull".
If your data set ships its own civilians, replace them by hand. Unresolvable
entries are dropped at load and logged; if no `civClass<N>` resolves,
`civClassCount` is forced to 0 and the civilian layer soft-disables.

An island may also **override** a global faction class by redeclaring it in this
block. That is a whole-class replacement, wants a comment saying what differs,
and has to be listed in the unit test that pins the CIV-only rule.

## 8. `mission.sqm` and `init.sqs`

**`mission.sqm` needs only the world and the player's own addon in `addOns[]`.**
The engine's addon-dependency walk now follows `weapons[]` and `magazines[]`
(`Game/Guerrilla/AddonActivation.hpp`), activating the transitive closure at
launch, and `ZoneRegistry::ActivateFactionAddons` does the same for everything
the faction descriptors name, after unit creation. Both grants are
runtime-only: nothing is written back into the template's manifest. So Sinai's
manifest is two entries:

```cpp
addOns[]={"sinai","LoBoEgypt"};
addOnsAuto[]={"sinai","LoBoEgypt"};
```

A base-game world with a base-game player needs neither, and the scaffold prints
`ADDONS (none: base-game world and player)`.

**`init.sqs` is two lines and is not yours to edit:**

```
; Guerrilla Mode bootstrap: the shared script core lives at <GameDir>\gmcore, ...
[] exec "\gmcore\init.sqs"
```

The leading backslash is load-bearing: `OpenScript` strips it and resolves the
rest against the game data root, so the mission reaches `<GameDir>\gmcore`
instead of its own folder. The script core exists exactly once, in
`guerrilla-mode/core`, and is installed by `install-missions.ps1`. A template
that carries its own `scripts/` directory is a bug. See
[`ARCHITECTURE.md`](ARCHITECTURE.md) A.6.

## 9. Install, and the acceptance checks

```powershell
guerrilla-mode\install-missions.ps1                 # default Classic install
guerrilla-mode\install-missions.ps1 -GameDir <dir>
```

One run installs three things: the global faction library to
`<GameDir>\bin\guerrilla-factions.hpp` (included from `bin\config-extra.cpp`),
the shared script core to `<GameDir>\gmcore`, and every template to
`<GameDir>\Missions\`. It is idempotent; re-run it after editing a template, the
core or the library.

**The installer finds your world by itself.** It looks for `<World>.wrp` as a
loose file under `<GameDir>\Worlds`, then in the **entry tables** of every
`*.pbo` under `AddOns\`, `Dta\` and each mod folder's `addons\` (only the pbo
header is read, so a full scan costs about a second). A world inside a mod pbo
therefore needs no flag and no table edit anywhere. `-IncludeWorld` survives as
a manual override. A template whose world is nowhere is skipped, and a stale
install of it is removed, because a mission on a missing world silently keeps
the current island and drowns the player at sea.

Then, in order:

**1. It boots and the zone table is right.**

```powershell
.\run-game.ps1 -Mission 'Missions\Guerrilla.Eden'
```

Read `logs\game-<ts>.log` afterwards rather than watching the window. What you
are looking for: no `civVehicles[] <dropped>` surprises, no
`ZoneRegistry: faction ... not in the loaded data package` lines, and
`Names entry '<x>' not seeded` lines only where you expect them.

**2. The descriptors resolve.**

```powershell
dist\x64-win-rwdi\PoseidonTools.exe guerrilla lint `
  --data-dir "D:\Arma_CWA\ARMA Cold War Assault [Classic]" `
  --island arma_CWR\guerrilla-mode\mission\Guerrilla.Eden
```

This lints the union your island actually sees, global table plus your CIV block
and any override. Expect `ISSUE CIV civ-exempt authors no tiers[]` (CIV is
exempt by design) and `unresolvable=0`. Exit code 1 means a faction your island
offers cannot field its `tiers[0]`.

**3. An integration test asserts the seed.** Copy the pattern of
`tests/integration/scripting/guerrilla_lebanon80_boot.test.{toml,sqf}`: boot the
repo-root-relative template path, wait for `GM_LIB_READY`, assert the resolved
sides, assert the **exact** `gmZoneCount` (an exact count is what catches a
spurious auto-seed, where a `>=` would not), assert a named zone's owner, wait
for the garrison to spawn near the player, and check a marker colour. The
`.toml` names the template in `mission =`, tags the lane, and passes any
`--mod` the world needs through `extra_args`.

**4. The menu offers it.** Launch to the main menu, open GUERRILLA, and confirm
the island appears under its **display** name, the START TOWN cycler lists your
towns, and both faction cyclers offer a selectable pair. A faction greyed with
`(not in loaded data)` means its addons are not mounted; that is section 9 of
the faction doc, not an island problem.

---

## 10. Detected bases: AIRFIELD, SEAPORT and OUTPOST (issue #76)

An island's occupation network is its airfields, harbours and garrisoned
compounds. The engine finds the ones a world models on its own
(`Game/Guerrilla/BaseDetect`), so a new island gets AIRFIELD and SEAPORT zones
without a hand survey. The same detector runs in two places:

- **At mission load** (`ZoneRegistry`, gated by `seedBases`): it reads the
  loaded landscape and adds every AIRFIELD and SEAPORT candidate with status
  `seed` as an occupier zone. **OUTPOSTs are never seeded at runtime**: the
  garrison ladder is the author's.
- **Offline** (`guerrilla scaffold` and `guerrilla lint --bases`): it reads the
  `.wrp` and prints every candidate, OUTPOSTs included, with its evidence.

Authored zones always win. A candidate an authored zone already models is
`covered` and never seeded.

### What is detected, and how

All numbers live in `BaseDetectOptions` (`BaseDetect.hpp`); this table is the
one place the docs restate them.

| Type | Evidence | Rule | Confidence |
|------|----------|------|------------|
| AIRFIELD (ILS) | `CfgWorlds >> <world>`: `ilsPosition[]`, `ilsDirection[]`, `ilsTaxiIn[]`, `ilsTaxiOff[]`, plus every subclass of an optional `class SecondaryAirports` with the same keys | One per entry. Rejected at the map origin, outside the map, or with the runway (touchdown, far end, midpoint) in the water. A second entry within 1500 m of an airfield already found is merged into it. | 0.9 with taxi paths (two or more taxi points), 0.7 without, 0.3 when the world only **inherits** `ilsPosition[]` from a base class |
| AIRFIELD (objects) | hangars, aircraft shelters (weight 1) and control towers (0.5), single-link clustered at 300 m | at least 3 aviation objects, 2 of them hangars or shelters, and more than 1500 m from an airfield already found (or outside its footprint); a weaker strip within 1500 m of a stronger one folds into it and widens its footprint. This is how a map with more airfields than its one config runway gets them (Sinai's Wadi strip and Egyptian Airbase) | 0.5 + 0.05 per object, at most 0.85 |
| SEAPORT | harbour-named objects, single-link clustered at 150 m: a concrete pier, quay block, crane, dock or harbour structure weighs 1; a wooden jetty, quay wall (`nabrezi`), mooring or boat 0.5; port crates 0.25. Inside a port set directory (`ags_port` and the like) everything weighs 1 except buoys and signals (0) and crates (0.25). A building whose footprint touches water within 150 m of the cluster adds 0.25 (at most 0.5 in all, never enough on its own) | at least 2 objects, a score of at least 1.5, and water within 30 m of at least one member | 0.3 + 0.2 x score, at most 0.95; **at most 0.4 without one weight-1 structure** (two jetties are a fishing landing, not a harbour) |
| OUTPOST | military-named objects (fortress, bunker, watchtower `hlaska` / `strazni_vez`, barracks, military garage `garaz_s_tankem` / `garaz_bez_tanku`, hangar, `repair_center`, `fuelstation_army`), single-link clustered at 120 m | at least 4 members (3 gave about 70 border-tower "posts" on @LoBo's Sinai), and not inside an AIRFIELD or SEAPORT candidate's footprint | 0.4 + 0.1 per member, at most 0.9 |

The zone centre is placed on **dry ground the garrison can spawn on**: at least
2 m above sea level, with the 50 m spawn square at least 0.5 m dry at every
corner. An airfield centres on its apron (the first `ilsTaxiIn[]` point) when
that is dry, else the runway footprint's midpoint; a cluster's centre is walked
inland up to 300 m until it fits. No dry ground, no candidate (the reason is a
`BASENOTE`).

Model vocabulary is a **name guess**, like `ModelIsBuilding`: the ground truth
is the Remaster's `abel.wrp` and `eden.wrp` plus @LoBo's Sinai and Lebanon80
(the token lists and the reasoning are at the top of `BaseDetect.cpp`). Road
signs, fences and walls never count. A mod pier named something unrelated in a
directory that says nothing is invisible: run the scaffold and read the BASE
lines before trusting it.

### Status

Each candidate gets exactly one status, in this order:

| Status | When | Seeded at runtime? |
|--------|------|--------------------|
| `outside-region` | the centre lies outside the theatre (`playRegion[]`, below) | no |
| `covered` | within 300 m (or the candidate's own footprint radius, whichever is larger) of an authored AIRFIELD, SEAPORT or OUTPOST, or within 150 m (`zoneArea`) of any other authored zone. `coveredBy` names that zone | no |
| `low-confidence` | below 0.5 | no |
| `seed` | an AIRFIELD or SEAPORT that passed everything | yes, when `seedBases` is on |
| `offline` | an OUTPOST that passed everything | never: the tool reports it, the author decides |

### Naming

- An AIRFIELD takes the name of a `Names` label that reads like an airfield
  (`airbase`, `airfield`, `airport`, `airstrip`, `air base`, `air field`,
  `letiste`, `aerodrom`, `runway`, `flugplatz`) inside its footprint or within
  300 m of its rim ("Egyptian Airbase"). That keeps the label out of the CITY
  seed, whose name dedup then drops it; the scaffold drops it from the
  generated CITY set the same way (a `WARN town '...' skipped: the label names
  a detected AIRFIELD` line).
- Otherwise an ILS airfield is "`<World>` Airfield" ("Malden Airfield", the
  display name unless it is an unresolved `$STR` key), and an object airfield
  "`<nearest town>` Airfield".
- A harbour is "`<nearest town within 4 km>` Harbour", an outpost
  "`<nearest town>` Post"; with no town in reach, the world's name. "Town"
  here means a `Names` entry that passes the settlement rule (dry, at least 3
  buildings within 300 m) and does not read like an airfield: a bare label
  (an islet in the sea, a border, an airbase) names nothing, so the Sharm port
  is not "Tiran Is Harbour". Stray spaces in a label are collapsed.
- Two object airfields within 1500 m of each other are one airbase: the
  weaker folds into the stronger, which re-centres on the middle of the
  clusters and widens its footprint over them (Sinai's Egyptian Airbase,
  hangar rows about 1 km apart behind one fence).
- Every name is made unique against the authored zones and the other
  candidates with a numeric suffix ("Le Port Post 2"). The name is the zone's
  savegame key, as for any zone.

### The theatre: `playRegion[]`

```cpp
class CfgGuerrillaZones
{
    playRegion[] = {minX, minZ, maxX, maxZ}; // engine metres: x easting, z northing
    ...
};
```

What the box bounds depends on whether the template authors it:

| `playRegion[]` | Base seed (AIRFIELD / SEAPORT) | CITY seed and START TOWN list |
|---|---|---|
| authored (a valid box) | inside the box only | inside the box only |
| absent, or not a valid box | **the default box: the authored zones' bounding box, padded by 5 km on every side** | unbounded, every Names town as before issue #76 |
| absent, and no authored zone has a position | whole map | unbounded |

A base candidate outside the box reports `outside-region`. The default box is
for bases only: it is what keeps a theatre map such as Lebanon80 from seeding
Beirut's harbour 9 km past its last authored town, while the town seed keeps
the whole map it always had. Author the key when you want the towns bounded
too, or when the default is wrong for your bases. `guerrilla scaffold
--play-region minX minZ maxX maxZ` writes it and also keeps the generated
towns, camp and outposts inside it. The runtime's base detection INFO line
ends with which applies: `playRegion authored [minX, minZ, maxX, maxZ]`,
`default box for bases only [...]` or `no playRegion (whole map)`.

### `seedBases`: absent means "only if the template authors none"

| Value | Behaviour |
|-------|-----------|
| key absent | **1 when the template authors no AIRFIELD and no SEAPORT zone, else 0.** An island that authors its network keeps exactly what it authored; a bare island gets its bases for free. |
| `seedBases = 1` | Seed every `seed` candidate even next to authored bases (authored zones still cover the candidates they model). |
| `seedBases = 0` | Seed nothing. The opt-out for a theatre whose bases are scenery (`Guerrilla.Lebanon80`: Ramat David is the only ILS entry, 7 km south of the theatre, and the Tyre quay wall is not a harbour). |

Seeded bases are occupier-owned, carry the type's garrison (AIRFIELD 12,
SEAPORT 8; detected OUTPOSTs in the scaffold get 6, Abel's authored ladder)
and pay installation income like an OUTPOST, since the plan 13 AIRFIELD and
SEAPORT yields are not implemented. They are saved by name like any zone.
`gmSeededBases` lists what the seed added and `gmDetectedBases` every AIRFIELD
and SEAPORT candidate with its status (`OCCUPIER-FORCES.md` section 16).

### Checking a world: `scaffold` and `lint --bases`

The scaffold runs the detector against the zones it generates (the CAMP and
the CITYs, or the kept block with `--keep-zones`) and writes:

- every `seed` AIRFIELD and SEAPORT as a real occupier zone class, inserted
  after the CAMP;
- `offline` OUTPOSTs, strongest first, as real OUTPOST classes up to the
  `--outposts N` budget. **N is the OUTPOST total**: detected outposts first,
  heuristically placed ones fill the rest and keep 800 m clear of every
  detected base; detected outposts over the budget become notes;
- the evidence as `// evidence:` comments above each zone class;
- every other candidate (`covered`, `outside-region`, `low-confidence`, over
  budget) as a commented-out `// NOT EMITTED:` note at the end of the Zones
  block;
- a `seedBases` comment in the header. Since the scaffolded block usually
  authors its AIRFIELD and SEAPORT zones, the runtime seed resolves to 0 for
  it; when the world has none, the comment says the seed is on.

It prints one line per candidate, plus the evidence:

```
BASE seed AIRFIELD Malden Airfield 7991 9990 conf=0.90
  evidence: ILS CfgWorlds >> Abel: ilsPosition [7922, 9590], ilsTaxiIn 6 point(s), ilsTaxiOff 6 point(s)
  evidence: runway [7922, 9590]..[7983, 10458], 870 m; centre on the apron (first ilsTaxiIn point)
BASE seed SEAPORT Le Port Harbour 8446 3736 conf=0.70
  evidence: data3d\molo_beton.p3d at [8476, 3754]
  evidence: data3d\molo_krychle.p3d at [8472, 3774]
BASE offline OUTPOST Le Port Post 9725 4004 conf=0.90
```

`lint --bases` judges an existing template against its world (the world is
the folder suffix, or `--world`):

```powershell
dist\x64-win-rwdi\PoseidonTools.exe guerrilla lint --bases `
  --data-dir D:\Arma_CWA\tmp\remaster-shadow-67 `
  --island arma_CWR\guerrilla-mode\mission\Guerrilla.Abel
```

```
BASES Guerrilla.Abel seedBases=0 (default: the template authors 1 AIRFIELD / 1 SEAPORT) authored=6 candidates=5 playRegion={2117, -1310, 14715, 14905} (default: authored zones + 5 km)
BASE Guerrilla.Abel COVERED AIRFIELD "Malden Airfield" at 7991 9990 conf=0.90 by "Airfield" (AIRFIELD, 94 m)
BASE Guerrilla.Abel MISSING OUTPOST "Saint Louis Post" at 7490 10569 conf=0.80: offline candidate, never seeded (the outpost ladder is the author's)
```

`COVERED` names the authored zone that models the candidate and how far away
it is; `MISSING` is a candidate the template does not author, with whether the
runtime will seed it under the template's resolved `seedBases` (an OUTPOST
never is); `SKIPPED` is outside the theatre or below 0.5. The base audit is
advisory: it never fails the run (an unreadable world exits 2).

### Opting out

- the whole seed: `seedBases = 0;`
- one place: author a zone there (any type within 150 m, a military type
  within 300 m covers it), or draw `playRegion[]` so it falls outside;
- one model family: none. Detection is not configurable per template; if a
  model is misread, fix the vocabulary in `BaseDetect.cpp` and say so in the
  commit.

## Pitfalls

1. **Folder suffix must be the `CfgWorlds` class**, not the display name. A
   display-named folder passes every test in the repo and is never found by the
   launch path.
2. **`position[]` order differs between `description.ext` and `mission.sqm`.**
   Getting it wrong usually puts the zone in the sea, which reads as "the camp
   is underwater" rather than as a coordinate bug.
3. **A zone `name` is the savegame key.** Renaming a zone in a shipped template
   orphans that zone's row in existing saves.
4. **Do not add a `scripts/` directory**, and do not edit the two-line
   `init.sqs`.
5. **Re-running the scaffold overwrites `description.ext`.** Pass
   `--keep-zones` once you have hand-edited zones.
6. **The scaffold cannot pick your factions.** `defaultOccupier` and
   `defaultResistance` ship as `EAST`/`GUER` placeholders and are wrong for any
   modded island.
7. **Detected bases are a name guess.** Read the scaffold's `BASE` lines and
   their evidence before shipping, and run `guerrilla lint --bases` after hand
   edits: a template that authors no AIRFIELD or SEAPORT gets the runtime seed
   (section 10), which may not be the network you meant.
8. **A world without roads is playable but quiet.** Traffic bottoms out in
   `GRoadNet` for routes and spawn points, so it simply produces nothing. That is
   a content property of the world, not something the template can fix.
