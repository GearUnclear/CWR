# Generated occupier bases: engine and content feasibility

Read-only assessment, branch `occupier-forces`, 2026-10-15. Paths are relative to
`arma_CWR/engine/Poseidon/` unless they start with `arma_CWR/`, `tmp/` or `guerrilla-mode/`.
Design (how many, which kinds, which layouts) is the sibling agent's half; this file
answers "can the engine do it, where, with what, and how does it persist and plug in".

## 0. Verdict and the numbers behind the complaint

Feasible with no new engine subsystem. Every piece has a precedent: runtime base
seeding (`SeedBaseZones`), deterministic off-road prop placement (`TownFlags`,
`GuerrillaBase`), empty-ground scoring (`IslandScaffold` outposts, `EngineLzProbe`),
props riding the world save (`AddBuilding`), and stock statics in the base config.

Military zone counts today (roster = authored-occupier military zones):
- Malden (`Guerrilla.Abel`): 4 (Outpost, Southeast Compound, Le Port Harbour,
  Airfield) against K 14 towns: `poolStart 60 (R 32, K 14, G 56), roster 4`
  (`tmp/of-76-ground-truth.md:168-171`). `seedBases` is auto-off because the template
  authors an AIRFIELD (`Game/Guerrilla/ZoneRegistry.cpp:1457-1486`).
- Everon (`Guerrilla.Eden`): 3 authored OUTPOSTs + 1 seeded AIRFIELD + 1 seeded
  SEAPORT = 5, against 18 authored CITY zones (`guerrilla_bases_detect_eden.test.sqf` e1-e3).
- Sinai (`Guerrilla.Sinai`): 2 authored OUTPOSTs + 2 AIRFIELD + 2 SEAPORT seeded = 6,
  against 36 seeded CITY rows (`guerrilla_bases_detect_sinai.test.sqf` s1-s4).

Eden's three outposts were already generated offline by the scaffold on empty, propless
ground (`tmp/of-76-ground-truth.md:213-219`): step 1 below, done offline. Malden never was.

## 1. Placement: what the engine can compute at campaign start

Queries available to C++ (all cheap, all already used by Guerrilla code):
- Geography cell facts, one DWORD per 50 m land cell, precomputed at landscape load:
  `waterDepth`, `full`, `forestInner/Outer`, `road`, `howManyObjects`,
  `howManyHardObjects`, `gradient` 0..7 (`AI/Path/AITypes.hpp:39-55`; bands
  `World/Terrain/Geography.cpp:38-70`: 1 = 0.05, 2 = 0.10, 3 = 0.15 rise/run; object
  counts `:186-336`). Read with `GLandscape->GetGeography(x, z)`
  (`World/Terrain/Landscape.hpp:611`). A 256 x 256 map is 65,536 reads, microseconds.
  `EngineLzProbe` already turns these bits into "empty, dry, gentle"
  (`Game/Guerrilla/OccupierAir.cpp:548-593`); `gmGeoGrid` dumps them (`RoadRoute.cpp:806`).
- Heights: `SurfaceY`, `SurfaceYAboveWater(x, z, &dx, &dz)` (gradient out),
  `RoadSurfaceY` (`Landscape.hpp:707-738`). Sea level is Y 0 and `WaterDepth` is a stub,
  so height or the `waterDepth` bit is the water test (`ZoneRegistry.cpp:1600-1604`).
  `gmLandmass` is not a water test (`tmp/of-76-ground-truth.md:165-167`).
- Roads: `GRoadNet->IsOnRoad(pos, size)` (`World/Terrain/Roads.hpp:111`), and
  `RoadRoute::LinksNear` (nearest first) plus `RoadRoute::Landmass`, labelled once per
  landscape (`RoadRoute.hpp:78, 97`).
- Objects: `GLandscape->GetObjects(z, x)` per cell (`Landscape.hpp:614`; the
  `nearestObjects` walk, `Game/Commands/GameStateExtUi.cpp:576-640`). The full-map
  walk in `LandscapeBaseProbe::Gather` takes about 58 ms on Abel
  (`ZoneRegistry.cpp:1718-1773`; logged "338 object(s) ... in 57.2 ms").
- Zones and region: every `ZoneRecord::pos` (authored, seeded base, seeded CITY),
  `ZoneTuning::playRegion` (`ZoneRegistry.hpp:130-157`), the CAMP.

Proposed scorer (engineering shape; the sibling sets the numbers). Per 50 m cell
inside `playRegion` and 200 m clear of the map edge, over its 3 x 3 neighbourhood
(the 150 m footprint):
- Reject: `waterDepth > 0`, `full` or `forestInner` anywhere in the 3 x 3; a centre
  cell with `road`, `howManyHardObjects > 0` or `howManyObjects > 1`; `gradient >= 2`
  anywhere (seating note in 2); `SurfaceY < 2` at the centre or a +-25 m spawn-square
  corner (`BaseDetect.hpp:182-189`); another landmass than the Camp's
  (`RoadRoute.hpp:93-96`); no road link within 400 m; closer than dZone to any zone
  (scaffold: 800 m, `IslandScaffold.hpp:120`), dTown to a town (above the 300 m city
  dedup, `ZoneRegistry.cpp:1417`) or dCamp to the CAMP (at least 1500 m).
- Score: `S = -wRelief*relief40m + wRoad*band(roadDist, 60..300 m)
  + wSpread*minDist(occupier military zones, incl. this pass's picks)
  - wTown*|dist(nearest town) - dTarget| + wJitter*Roll(seed, cellIndex)`.
  Pick greedily N times (farthest-point spread). This is the scaffold's outpost
  loop (`IslandScaffold.cpp:777-840`, candidates and `Flatness` at `:416-505`) plus
  the geography bits. That code sits in an anonymous namespace today, so its pure
  part has to move into a reusable header.
- Heading: bearing from the centre to the nearest `LinksNear` link (gate faces the
  road). Cost: under 10 ms (geography pass plus `SurfaceY`/`LinksNear` on survivors).

Load order: inside `ZoneRegistry::LoadFromParams`, AFTER `SeedCityZones`
(`ZoneRegistry.cpp:409-418`). The scorer has to see the seeded towns, and running
it earlier would let the 300 m city dedup delete a town that a base sits near. Keep
it pure like BaseDetect: a `BaseSiteProbe` seam with an engine implementation
beside `LandscapeBaseProbe` (`ZoneRegistry.hpp:436-456`) and a Catch2 fake. Gate it
on a new `CfgGuerrillaZones` key (`generateBases = N`, absent = 0) so no existing
template or test changes. `MaxZones = 64` (`ZoneRegistry.hpp:580`): Sinai already
holds about 45 rows, so N above about 15 hits the cap there.

## 2. Props

Placement API (C++ createVehicle): `NewNonAIVehicle`/`NewVehicle`, `PlaceOnSurface`,
`SetTransform`, `Init`, then `GWorld->AddBuilding` for a Static-kind type. Precedents:
- `TownFlags::CreatePole` (`Game/Guerrilla/TownFlags.cpp:197-254`), with a
  deterministic road-checked picker (`:142-191`) and lazy creation in `Simulate`
  (`:305-330`). A missing class logs one warning and the service carries on.
- `GuerrillaBase::CreateCache` / `CreateProp` (`GuerrillaBase.cpp:575-697`).
- `OccupierAir`'s pad: `NewNonAIVehicle("HeliHEmpty")`, else `HeliH`
  (`OccupierAir.cpp:1029-1032`).
- The script path: `VehCreate` (`Game/Commands/GameStateExtWorld.cpp:100-140`).
Layout props skip `AIUnit::FindFreePosition`, because it would shove a layout apart.
Pre-validate the footprint instead: off-road (`IsOnRoad` with the bounding radius),
no landscape object (`GetObjects`), dry.

Seating (`World/Simulation/Simul.cpp:1252-1311`): a static's origin goes on
`RoadSurfaceYAboveWater` plus the bounding centre. `StaticSeatOffsetY`
(`World/Entities/Vehicles/Vehicle.hpp:260-264`) rescues a mesh that sits below its
origin. Statics always stand upright: the hint test `(GetOrHints() & ClipLandKeep |
ClipLandOn)` is always true by precedence, and the slope branch only assigns a
shadowing local (`Simul.cpp:1267-1276`). `trans.Direction()` is kept, so heading
works, but on a 10 % slope a 10 m wall floats or buries 1 m end to end. Hence the
hard `gradient < 2` gate.

Layout template: a config class installed next to `guerrilla-hq.hpp` (moddable,
LoBo-overridable), e.g. `class CfgGuerrillaBaseLayouts { class Checkpoint { items[] =
{"GMBaseTower", 12, -8, 90, ...}; }; };`. Each row is `{class, dx, dz, dirDeg}` in
base-local metres, +z towards the road; world position = centre + Rot(heading)*(dx, dz),
direction = heading + dirDeg. Read from `Pars` like the faction table; an unresolved
class is skipped with one LOG_WARN (plan-15 rule). Faction-themed rows come through
descriptor keys resolved by the `ClassProbe` pass, as `flag` does for `TownFlags`.

Inventory: stock CfgVehicles from the decompiled dump `tmp/hq-model-candidates/config.cpp`
(no game folder read). The Remaster payload equals Classic 1.99 (memory note
`full-retail-remaster-install`) and @LoBo mounts on top, so all of these exist on
both target sets. All are Static-kind unless marked:
- tents: `Camp`, `CampEmpty`, `CampEast`, `CampEastC`, `ACamp`, `MASH` (`:21862-21973`);
- sandbag positions: `Fortress1`, `Fortress2` (`:21886-21907`), `Fence` (pytle_BSL),
  `FenceWood` (`:21974-21996`);
- wire: `Wire`, `WireFence` (`:21997-22012`);
- wrecks, which are Camp subclasses and so static: `JeepWreck1-3`, `M113Wreck`,
  `UralWreck` (`:22233-22292`);
- sheds and buildings: `Shed`, `ShedSmall`, and the scope-1 classes `Barracks`,
  `Hangar*`, `Housegaraz_s_tankem`, `Houserepair_center`, `Houseammostore2`
  (`:22293-22364, 22523-22656`);
- markings and fuel: `FlagCarrier`, `Danger` (`:21759-21769`), `HeliH`, `HeliHEmpty`
  (`:22748-22777`), `FuelStationNew2` (fuelstation_army, `:22841-22861`).

Caveats: `ReammoBox*` crates come PREFILLED (`guerrilla-hq.hpp:1-2`); barrels and
pallets are physics-simulated `Thing`s (`:22013-22227`); there is no stock
watchtower, camo-net or hedgehog class. Precedent fix: `guerrilla-hq.hpp:29-41` wraps
`stulsuplikmapa.p3d` (a stock `Thing`) as `GMHqDesk : Camp`. `GMBase*` wrappers can do
the same over models Malden already places (so they ship): `hlaska`, `strazni_vez`,
`jezek kov`, `plechbud`, `pletivo_wired`, `fuelstation_army`, `Barels`
(`tmp/of-76-ground-truth.md:115-148`); verify paths with `PoseidonTools terrain objects`.

Save/load: `AddBuilding` puts a static in `World::_buildings` (`World/World.hpp:234, 476`),
and the save writes that list whole (`World/WorldImpl.cpp:1683`). HQ props, the cache
and flagpoles are saved as world objects, not re-created: the services hold `LLink`
refs (`GuerrillaBase.cpp:1564-1567`, `TownFlags.hpp:76-80`) and re-create only when a
link comes back null (`TownFlags.cpp:317-330`, `GuerrillaBase.cpp:727`). Copy this.

Cost: frame time is small (`_buildings` run `SimulateOnly(..., SimulateVisibleNear)`,
`WorldImpl.cpp:697-703`; static simulation is near-empty; shapes are shared; 30
statics are fewer objects than a village). Save size: each entity writes type, shape,
transform, id, speed, angular momentum and EntityAI state (`Simul.cpp:1417-1457`), a
few hundred bytes, so 0.1 to 0.3 MB for 8 bases x 30 (unmeasured; log it in the seq).

## 3. Integration: every registry field a generated zone needs

Build the `ZoneRecord` as `SeedBaseZones` does (`ZoneRegistry.cpp:1863-1885`):
- `name`: unique and stable, the save key (`ApplyPendingLoad`, `:3789-3800`); named
  after the nearest town as BaseDetect does (`BaseDetect.hpp:193-195`).
- `type = "OUTPOST"`: any type except CAMP/CITY is military (`CampaignEnd.cpp:95-102`),
  so the zone joins the win roster and R. `"AIRFIELD"` would also make it the air
  origin (`CounterAttackAir.cpp:260-277, 703`; `CounterAttack.cpp:1324`) and the first
  wave hub (`OccupierForces.cpp:2445`); keep that type for real airfields.
- `ownerConfig = "OCCUPIER"`, with `owner` from `ResolveOwnerToken`:
  `IsAuthoredOccupier` reads `ownerConfig` (`CampaignEnd.cpp:104-115`).
- `garrison` and `garrisonMax` (whole bodies): `IsGarrisonable` needs a non-CITY zone,
  owner = the occupier side, and a tier class (`ZoneRegistry.cpp:2144-2155`). R sums
  the roster's `garrisonMax` (`OccupierForces.hpp:22-36`), so each base raises `poolStart`.
- `income = defaultIncome`; `captureRateOverride`, `support` and `heat` all 0.
- `marker` (unique, e.g. `gmZoneGen_<n>`; `gmcore` `init.sqs:249-256` makes a Flag
  marker for every zone) and `pos = (x, SurfaceY, z)`.
- `seeded = true` is safe, since "seeded town" readers also test CITY
  (`ZoneRegistry.hpp:195-201`). A transient `generated` flag plus `gmGeneratedBases`
  keeps these zones apart from `gmSeededBases` (`:624-630`).

What then works with no further code:
- Pool: OccupierForces takes its roster on the first active tick
  (`OccupierForces.cpp:1081-1095`), after `InitMission` has generated the zones. K counts
  towns only, so it does not change.
- Director source: a base can source while `CounterAttack::Spare` > 0
  (`CounterAttack.cpp:653-662`: stored + live minus `garrisonMax * sourceKeep`). The
  pool buys hulls and crews, which are born on the route, not stored at a base
  (`ForcePackages.hpp:38-86`). There are no per-zone `forces*` keys; every `forces*`
  key is a faction or tuning key.
- airOrigin: only an occupier-held AIRFIELD qualifies, and aircraft are born airborne
  (`OccupierAir.hpp:10-16`). A pad prop is set dressing.
- GarrisonCache: bodies spawn at a hashed `SpawnPoint` in a +-50 m square
  (`GarrisonCache.cpp:453-459, 704`) through `CreateUnit`, which calls
  `FindFreePosition(soldier)` (`GameStateExtWorld.cpp:155-185`), so they are nudged out
  of props. The hidden 3 x 3 release grid (`GarrisonCache.cpp:461-480`) needs no change.
- Waves: the hub is `forcesWaveHub`, else an AIRFIELD, else a SEAPORT, else the
  strongest base (`OccupierForces.hpp:73-76`). Reinforce columns and the final stand
  treat generated bases like any roster base.
- Capture: `capture`, `occCapture`, `RevertBase` and `lostCount` depend only on type and
  owner (`ZoneRegistry.cpp:2252-2308`).
- Win rule: `CampaignEnd::TakeRoster` includes every authored-occupier military zone
  (`CampaignEnd.cpp:117-139`), so generated bases are win conditions. Optional bases
  would need an exclusion flag.
- Traffic: every zone is a candidate (`Traffic.cpp:1490-1510`), and patrols need a road
  within 700 m (`Traffic.hpp:803`). The 400 m road gate in section 1 covers that.

## 4. Persistence and determinism

BaseDetect seeds are re-derived on every load, never stored. `LoadFromConfig` reruns
on the second load pass (`ZoneRegistry.cpp:3936-3945`), and a saved row whose zone
is not re-produced is dropped (`:3793-3797`). That is safe for BaseDetect, whose
only input is the map's objects. It is not safe for generation: a later build that
tunes the scorer would move bases under a running campaign. Their props, saved at
the old spots, and their saved rows would then be orphaned.

Store the generated zones instead. In `ZoneRegistry::Serialize` (`:3843`), add
`genVersion` and a `Generated[]` array (name, type, pos, garrisonMax, heading,
layout id, layout version). Read it on pass 1, the way `_pendingOccupierSide` is
read (`:3889-3902`). The generation step in `LoadFromParams` then has three cases:
- New mission (`InitMission`): draw the seed once (the `GRandGen` draw of
  `LegendRegistry.cpp:518`, or a pinned `generateBasesSeed` key), generate, keep rows.
- Save with `Generated[]`: re-inject the stored rows unchanged, no scoring.
- Old save (no `genVersion`): generate nothing. Its saved OccupierForces roster/R and
  CampaignEnd roster (`CampaignEnd.hpp:196-197`) do not know the new bases, which
  would be garrisoned but counted by neither the pool nor the win rule.

Centre, heading and layout id are enough to rebuild props. The props still ride the
world save, so a reload rebuilds only those whose link is null. The props service
needs its own optional subclass block in `WorldImpl.cpp`, after `GuerrillaZones`
(the pattern at `:2079-2124`).

## 5. Risks

- Pathing into prop clusters: `OperField::CreateField` rasterizes every grid object
  with mass >= 5 and geometry (`Game/OperMapDoor.cpp:576-640`), so props are
  obstacles, but cached oper fields are invalidated only on destruction
  (`World/Entities/Weapons/Dammage.cpp:483-503`): a prop created after its field was
  cached is invisible to pathing and `FindFreePosition`. Create props behind the
  loading screen, before any garrison spawns, and call
  `GLandscape->OperationalCache()->RemoveField(x, z)` on the 3 x 3 cells around each
  prop (the `Dammage.cpp:501` idiom).
- Roads: statics do not lock road links, but vehicles drive into whatever stands on
  them. (#87: stopped hulls and standing soldiers do lock links, and `SearchPath`
  skips locked links.) Keep every prop at least 6 m off any link (the
  `TownFlags::RoadClearance` idiom), and put no barrier across the access road.
- Yard: a joined route starts at the base itself (`RoadRoute.hpp:27-29`). Keep about
  25 m prop-free at the centre for births, the hold posture and capture presence.
  Props mostly on the perimeter keep garrison nudges short.
- Helicopter LZs: `EngineLzProbe` reads load-time geography bits
  (`OccupierAir.cpp:583-586`), which runtime props never update. The 300..500 m LZ
  ring around a target near a generated base can therefore pick a cell full of
  props. Either exclude generated footprints in the probe or write `howManyObjects`
  into those cells.
- Slope: statics stand upright (section 2); gate on gradient < 0.10. Missing classes
  skip with one warning; a layout must still read as a base without themed rows.
- Budget: more garrisons in the bubble; check `budgetLiveBodies 130` /
  `budgetSideGroups 40` (`ZoneRegistry.hpp:53-54`) on `remaster_ud_offscreen_budget_abel`.
- Campaign length grows with every roster base (design call). Determinism: one stored
  seed draw, otherwise no engine RNG (`LegendSeed.hpp:3-10` rule).

Tests that would pin it (patterns: `guerrilla_bases_detect_eden_save.seq`, the
`HeliHEmpty` dry probe in `remaster_ud_abel_occupation_network.test.sqf:68-75`): on
Abel with a pinned seed, N OUTPOSTs, each dry over its spawn square, a road within
400 m, on the Camp's landmass, dZone from every zone, occupier-held with a garrison,
in the `gmCampaignEnd` roster and R; a save/reload seq with identical names,
positions and prop counts, no duplicates, a deleted prop re-created once; props with
no road within 6 m (`nearestRoads 6` empty), a full garrison whose bodies can all
`doMove`; a forced counter-attack from a generated base reaching its town; Catch2
with a fake probe for the rejects, spread, old-save path, stored rows beating
re-scoring, and the LZ exclusion.

## 6. Recommendation: incremental path

Step 0 (optional, about 1 day): seed BaseDetect's OUTPOST candidates behind a
`seedOutposts` key (today dropped, `ZoneRegistry.cpp:1829-1835`, `BaseDetect.hpp:12-14`).
They come with real map props; on Malden the garage, fuel and tower cluster at
[7474, 10538] and fortress/hlaska clusters at [10477..10526, 4018] and
[9540..9594, 3038..3083] (`tmp/of-76-ground-truth.md:119-148`). Few qualify and they
bunch in the SE, so not the whole answer; it reverses a #76 decision (owner call).

Step 1 (about 3 to 4 engineer-days): generated OUTPOST zones without props. A pure
`BaseGenerate` core (scorer plus spread, lifted from IslandScaffold) with
`BaseSiteProbe` and an engine probe; the `generateBases` and `generateBasesSeed`
keys, the hook after `SeedCityZones`, stored `Generated[]` plus `genVersion` with the
old-save gate, and `gmGeneratedBases`; on in `Guerrilla.Abel` only. Tests: Catch2
(`test_zone_registry.cpp` or a new `test_base_generate.cpp`), one Abel tri test and a
save seq. The new `.cpp` must be listed in `Showcase.Abel/human/coverage.json` or
`test_human_suite.py` goes red. Result: fixes the base count, R, the win roster and
the traffic endpoints.

Step 2 (about 3 to 4 days): one layout of 8 to 12 statics. `GMBase*` wrapper classes
in an installed hpp plus the layouts config; a props service in the TownFlags shape
(`LLink` rows, re-created on null) with WorldImpl, WorldInit and World.cpp hooks;
footprint validation, oper-cache invalidation and the LZ exclusion; a tri props seq.

Step 3 (about 4 to 6 days): layouts per base kind and per faction: descriptor keys
resolved by plan 15, LoBo variants, optionally a locked parked hull as loot (the
garage lock idiom, `GuerrillaBase::AssertRow`, `GuerrillaBase.cpp:881`), and the
scaffold/lint tools printing generated sites for review.

Tests that change when a template turns generation on (the opt-in key keeps every
other test untouched): `ui/remaster/remaster_ud_abel_occupation_network.test.sqf:65`
(`triAssertEq [onMil, 4]`); `remaster_ud_guerrilla_abel_win` (roster counted from the
table: passes, runs longer); `remaster_ud_offscreen_budget_abel`; the forces tests that
boot `Guerrilla.Abel` (poolStart and R shift); and if Eden or Sinai opt in,
`guerrilla_bases_detect_eden` e3 (exactly three OUTPOSTs),
`guerrilla_bases_detect_eden_save.seq` and `guerrilla_bases_detect_sinai` s4 (R). For
scale, 29 integration tomls boot `Guerrilla.Abel`, 10 Sinai, 4 Eden, 2 Lebanon80;
most assert no counts.
