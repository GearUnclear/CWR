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

Options: `--outposts <0-32>` (default 3) and `--keep-zones`, which splices the
existing `<out>/description.ext` `class Zones { ... }` block in verbatim instead
of generating a new one, so hand edits survive a re-run. `--mod` is repeatable
and in `-mod` order.

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
ZONE CAMP      RESISTANCE Camp                       2373   7045    53.7
ZONE OUTPOST   OCCUPIER   Outpost 1                  7535   8760     8.3
ZONE OUTPOST   OCCUPIER   Outpost 2                 11439   1557    46.1
ZONE OUTPOST   OCCUPIER   Outpost 3                  9267   4432   122.6
ZONE CITY      NEUTRAL    Saint Phillippe            4674  10730    17.6
ZONE CITY      NEUTRAL    Montignac                  4935   6994   159.0
...
SUMMARY zones=22 cities=18 outposts=3 warnings=0
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
    // seedCities: see below
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
7. **A world without roads is playable but quiet.** Traffic bottoms out in
   `GRoadNet` for routes and spawn points, so it simply produces nothing. That is
   a content property of the world, not something the template can fix.
