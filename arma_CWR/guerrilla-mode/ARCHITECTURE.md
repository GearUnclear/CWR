# Guerrilla Mode — Architecture Spine

This is the **canonical structure every build agent conforms to.** As of the
Phase-1.5 pass it describes a **hybrid architecture**: the simulation core is
**native C++** (`engine/Poseidon/Game/Guerrilla/`), and the mission scripts are
a thin **event-driven policy layer** on top of it. The all-SQS Phase-1 spine
this file used to bind is retired; its sections are kept below, marked
**SUPERSEDED**, because they document decisions (naming, ownership discipline,
SQS idioms) that still shape the script layer.

Master design: `mod-plans/13-guerrilla-mode.md`. The Phase-1.5 engine rewrite
landed all six issue-#1 targets (evaluator QoL, ZoneRegistry, GarrisonCache,
AlertMachine, native persistence, guerrilla game module); issue #3 moved the
island/faction data out of the scripts.

---

## A. Native systems + event-driven script layer (Phase 1.5 — CURRENT)

### A.1 Activation and data (the per-island file)

Everything activates only when the mission's `description.ext` defines
`class CfgGuerrillaZones`. No class → the engine systems are inert and the
mission is an ordinary mission.

`description.ext` is now **the per-island data file** (issue #3 item 3). It
carries:

- **`CfgGuerrillaZones`** — engine tuning (all optional; defaults:
  `tickInterval=3; zoneArea=150; revealRadius=1500; cacheRadius=800;
  supportRate=5; supportFlip=60; heatCapSpike=40; defaultIncome=25;
  captureRate=6; captureCrewCap=3; captureDecayDefended=10;
  captureDecayAbandoned=2; supportDecayOccupied=0.5; supportDecayFloor=20;
  contestOutnumberRatio=4;
  cacheInterval=5; groupSize=12; alertInterval=5; alertYellowKnows=0.5;
  alertRedKnows=1.5; alertWindowSeconds=20; alertHeatYellow=4; alertHeatRed=15;
  alertHeatBreak=25; undercoverNoticeRadius=20; undercoverBackArcCos=-0.2;
  undercoverInHandsBoost=2.0; undercoverSlungBoost=1.4;
  undercoverIdentifyAccuracy=1.5; undercoverMinVisibility=0.02;
  undercoverWarDetectScale=0.5; undercoverForgetSeconds=0;
  undercoverHeatWitness=8; undercoverBoardWitnessSeconds=10`), the `class Zones` seed (each zone may carry its own
  `captureRate` override; `captureRate=100` restores the legacy instant flip),
  and the town auto-seed (`seedCities=1; seedCitySupport=20` — CITY zones
  generated from the world's named towns, markers `gmZoneCity_<n>`).
- **`CfgGuerrillaFactions`** — one descriptor class per faction. The zone
  `owner` field accepts generic tokens `"OCCUPIER"`/`"RESISTANCE"`/`"NEUTRAL"`;
  the new-game UI publishes `gmSelOccupier`/`gmSelResistance` and the engine
  resolves them against the faction classes. Scripts get the resolved side
  strings from the nulars **`gmOccupierSide` / `gmResistanceSide`** (defaults
  `"EAST"`/`"GUER"`).

**Coordinate contract (moved from init.sqs to config):** zone `position[]` is
authored in **script/getPos order `[easting, northing, elevation]`** — *not*
the `mission.sqm` `position[]` order `[easting, elevation, northing]`. The
engine (`ZoneRegistry::LoadZones`) converts to engine axes internally and every
script-facing array (gmZone tuple index 8, `gmZoneLastKnown`) is emitted back
in getPos order. Author config positions from `getPos` output, never by
copying `mission.sqm`.

### A.2 What the engine owns (deleted script loops)

| Native system | Replaces | Behavior |
|---|---|---|
| `ZoneRegistry` | `zones.sqs` | zone table from config; fog-of-war reveal; marker recolor/label with capture/support progress text and ColorWhite while contested (only repaints markers that already exist in `markersMap` — the mission still **creates** them); military **consolidation capture** (a 0..100 capture meter: climbs per tick per attacker (crew-capped) while resistance units hold the zone area with no live occupier unit inside it — positional presence per side, NOT the `liveOccupiers` bookkeeping; frozen while contested, decays when defended/abandoned; `contestOutnumberRatio` lets overwhelming defenders re-secure past a lone straggler; flip at 100; meters freeze outside `cacheRadius`); CITY support accrual (occupier-free town only) / intimidation decay (occupier-only presence, floored at `supportDecayFloor`) / decoupled ready-threshold + flip (needs fighters in an occupier-free town; the undercover player counts for neither); capture heat spike + income tap |
| `AlertMachine` | `alert.sqs` | per-zone GREEN/YELLOW/RED FSM from garrison `knowsAbout`, disengage window, heat spikes on escalation edges, last-known player position; drains the `UndercoverSystem`'s compromise-notification queue each tick: the first-ever compromise of the campaign (from any source) fires `undercoverBroken` + `alertHeatBreak` Heat on the zone nearest the WITNESS, later witness groups add `undercoverHeatWitness` Heat quietly with no event (the old global vehicle-mount break and its `playerInVehicle` input are deleted) |
| `UndercoverSystem` | *(new 2026-07-16; replaces the global binary cover break)* | per-observer undercover perception for the player subject (SP; self-disables without a real player): each occupier group independently resolves the disguised player's perceived side at the engine's target-tracking side-resolution sites, as civilian (`TCivilian`) / suspect (`TSideUnknown`, the AI investigates) / exposed (real side), from weapon state, facing, distance, visibility and that group's own compromise memory; compromise is a serialized flag on the group's `Target` record, permanent per group by default (`undercoverForgetSeconds=0` is the decay knob); `gmUndercover` + `setCaptive` stay the script-owned baseline and are never dropped on a break |
| `GarrisonCache` | `spawning.sqs` (cache half) | occupier garrison distance-cache: integer reserve ↔ live groups across `cacheRadius` (+50 m despawn hysteresis), survivor write-back, officer-first groups (faction `officer` key), SENTRY/GUARD posture, reads script global `gmWarLevel` |
| native persistence | `persistence.sqs` | zones, alert FSM, garrison bookkeeping and **event handlers** serialize; script globals always did (GGameState); `campaignLoaded` event replaces the GM_SAVED sentinel + poll |
| `Journal` + `GuerrillaJournalPages` | *(new, no script ancestor)* | the field journal behind the map screen's Notes / Plan pages. `Game/Guerrilla/Journal` holds three plain tables (capped diary entries stamped "Day N HH:MM" from the clock + the script `gmDayCount`; keyed objectives ACTIVE/DONE/FAILED/HIDDEN; keyed status lines) fed only through `gmJournal*`; `UI/Guerrilla/GuerrillaJournalPages` is the renderer `DisplayMap::ReloadBriefingContent` calls whenever the ZoneRegistry is active: it appends to (or creates) the `Main` section (aliased `__BRIEFING`, the Notes tab) the campaign header, a live Situation block (treasury/manpower/War Level from the script globals, military zones held, towns risen, towns ready to rise, meters in progress, worst alert, hottest zone, cover state + witness count, fighters in the player's group, stash count, then the script status lines), the 10-topic field-manual index (`GM_MAN_*` pages, static player text) and the latest diary lines (full diary on `GM_LOG`); and to the `Plan` section (copied into `__PLAN` by `UpdatePlan`) the standing goal, two engine-computed objectives (hold every military zone / raise every town, with counts), the scripted objectives and the derived next steps (RED zone to break from, blown cover, towns ready, meters to finish, nearest occupier targets with garrison + distance, recruiting when HR is available, lie-low when Heat is high). Rebuilt on every map-key open (`DisplayMap::ResetHUD`, which `World::Simulate` calls on the UAMap toggle; the display itself is created once per mission by `DisplayMission::InitUI`), on a journal change while the map is open (revision compare in `OnSimulate`), on a Notes/Plan tab press and after an in-place load; serializes as `GuerrillaJournal` (save-gated on non-empty, `IsSubclass`-tolerant on load). No `briefing.html` is needed; an authored one is appended to, never replaced |
| `TownFlags` | *(new, no script ancestor)* | one physical `FlagCarrier` pole per CITY zone flying the owner's flag (in-world counterpart of the flag-icon map marker): deterministic off-road placement (`GRoadNet::IsOnRoad` hard reject, 6 m clearance) scored toward high ground + town outskirts so the flag reads from outside town; texture = faction descriptor `flag` key > per-side default (`usa`/`ussr`/`fia` from `Flags.pbo`) > generic white flag; repainted on owner flip; placement + pole refs serialize (`GuerrillaFlags` subclass), the pole object rides the world's building serializer; a package without `FlagCarrier` degrades to markers-only (logged, non-fatal) |
| `Traffic` | *(new, no script ancestor)* | ambient road traffic (2026-08-22): the road net carries civilian cars between CITY zones, occupier patrol hulls (`vehicles[0]`) between occupier-held zones and, rarely and war-level-scaled, a truck + escort supply convoy (`vehicles[1]` + `vehicles[0]`, one group, column). Pure core (one-roll rarest-first spawn decision with per-kind caps 3/1/1, route picker, farthest-in-band spawn-point selection over `GRoadNet` link centres within 700 m of the origin, the commandeer predicate, despawn/stall rules) under a world layer: group on the CIV / occupier center, hull via `NewNonAIVehicle` on a road link oriented toward the destination, crew seated through `NativeMoveIn`, route = Command::Move to the DRIVERS via `IssueCommand` (the doMove idiom; arcade waypoints / `AIGroup::Move` deliver through SendCommand and never moved a seated crew in probes) under CARELESS (civ) / SAFE (occupier) so `AIUnit::CreatePath` keeps the road net; arrival re-dispatches up to `trafficMaxLegs` while the player watches, stalls tear down only out of sight, never teleport. Commandeer: a civ car with the real player ahead in its lane (20 deg, 4 m) or a weapon-in-hands aim (15 deg) inside 25 m gets a Stop, 2.5 s later the driver is unassigned, `AllowGetIn(false)`, `DoGetOut`, sent fleeing 150 m (deleted beyond 300 m / 60 s); the hull moves to the released table and is deleted only when the player is far and nobody ever boarded. Every civ driver carries the `driverKilled` killed-EH expression (init.sqs routes it into `GM_fnCivKilledEH`, so road murders land in `gmCivKilled`). Live rows (hull/escort/group/driver refs + zone names), released hulls, fleeing drivers and handlers serialize as `GuerrillaTraffic`; zone indices re-resolve by name. Missing `civVehicles[]` (plan-15 probe) leaves civ traffic inert; fewer than two occupier vehicle rungs skip convoys. Accepted gap: patrol traffic feeds no AlertMachine input |
| `GuerrillaBase` | *(new 2026-08-22, issues #16 M1+M4 / #28; no script ancestor)* | the player's headquarters: one per campaign, elected either by the new-game START TOWN cycler (`gmSelStartTown`; the first `Simulate` tick resolves the name through `FindZoneIndex`, establishes and relocates the real player beside the garage - one shot, `autoTried` serialized) or by `gmHqEstablish [pos]` from the action menu (scripts/market.sqs), in ANY zone whose area contains the position. Siting: the best enterable building inside the zone (Paths LOD, `NPos() >= hqMinPos`, standing; most AI positions, then nearest the centre - `PickHqBuilding`) gets the cache set straight onto its first interior AI position (no `PlaceOnSurface`: the Static branch ignores floors) and a garage spot on 20/35/50 m rings beside it; no such building (a CAMP, a hamlet) = edge-of-town fallback on the 0.6/0.75/0.45/0.9 x zoneArea rings (`PickSpot`: off-road via `GRoadNet::IsOnRoad`, dry, then an AI free-position nudge re-checked), cache 4 m beside the garage. The cache is a `WeaponHolder` flagged keep-when-empty and registered with the StashRegistry; re-establishing MOVES it (contents travel, `MoveNetAware`), bumps `moveCount` (the script debits `hqMoveCost`) and releases garage rows left beyond 1.5 x `garageRadius` of the new ring. Garage: `GarageLock(veh, lock)` on any Transport inside `garageRadius` not carrying the real player - `SetLock(LSLocked)` + `SetAllowDammage(false)` (knob `garageInvulnerable`), both re-asserted every 2 s tick and on the load pass since neither is serialized for us; rows prune dead/deleted hulls and release the ones that left the ring; the lock cue is the hull's own horn muzzle (a magazine-less muzzle with a `drySound`, Car/Motorcycle only) played as two short `OpenAndPlayOnce` bursts, silent on horn-less hulls / headless. A null cache is self-healed at the serialized spot. Serializes as `GuerrillaBase` (election scalars, `SerializeRef` building/cache/rows; gated on the registry being active; a block-less save reloads its config on the first tick). Tuning keys in `CfgGuerrillaZones`: `hqMinPos=4`, `garageRadius=100`, `garageInvulnerable=1` |
| `Market` | *(new 2026-08-22, issue #27; no script ancestor)* | the dealer market behind `class CfgGuerrillaMarket` (description.ext): tuning `dealerShare=0.34`, `dealerRespawnSeconds=600`, `hqMoveCost=500`, `dealerClass` (else the CIV descriptor's `civClass1`, else `Civilian`, each rung package-probed), optional `weaponDealers[]`/`vehicleDealers[]` (zone names that override the draw), `class Weapons { class X { weapon=; magazine=; mags=; price=; }; }` (weapon + magazines, weapon alone, or a magazine-only bundle; OFP magazines are CfgWeapons classes, a CfgMagazines bank is accepted too) and `class Vehicles { class Y { vehicle=; price=; }; }`; rows are probed against the package at load and dropped non-fatally, display names come from the package config (localized). On the first tick (`_assigned`, serialized, so a load never re-draws) each kind is planned over the CITY zones: `DealerQuota = max(1, round(n * share))` towns from a seeded, salted, per-kind-independent order (`ShuffleOrder`, pure; `_seed` drawn once and serialized); each row gets a deterministic off-road dry spot on the 0.3/0.4/0.5 x zoneArea rings (weapon dealers on the cardinal bearings, vehicle dealers on the diagonals, the vehicle dealer's LOT = the next acceptable sample >= 15 m away) and a CIV NPC (`CreateUnit` into a CIV group from the shared `EnsureSideCenter`/`CreateSideGroup` helpers, `DAMove|DATarget|DAAutoTarget`), respawned after `dealerRespawnSeconds` when dead. The purchase itself is script policy (market.sqs). Serializes as `GuerrillaMarket` (rows by zone name + NPC refs, groups, seed, assigned; gated on the class being configured) |

**Heat direction contract is now engine-enforced:** there is no direct `heat`
setter — `gmZoneSet` exposes only `"heatRaise"` (+=, clamped 100) and
`"heatDecay"` (−=, clamped 0). The old OWNS-table raise/decay split survives
as API shape.

**Markers on load (verified):** dynamically created markers live in
`markersMap`, which `World::Serialize` saves (`AIGlobalSerialize`,
`AICenterImpl.cpp`). They **survive a load intact**; the old persistence.sqs
marker rebuild was defensive and is gone. `init.sqs` creates each zone's
marker exactly once per campaign.

**Undercover perception rules (2026-07-16).** On foot: a weapon **in hands**
multiplies the observer's side accuracy by `undercoverInHandsBoost` (2.0,
war-level scaled: `1 + undercoverWarDetectScale * warDetect`, reading the
script global `gmWarLevel` directly) toward the existing 1.5 identification
threshold, so the player reads suspect until identified, exposed after (a
beat at range, instant up close). A **slung rifle** is exposed within
`undercoverNoticeRadius` (20 m) and suspect/exposed only when the observer
sees the target's back (`cosFacing < undercoverBackArcCos`, boost x1.4);
front-on beyond 20 m the rifle stays hidden behind the torso. **Unarmed**
reads civilian unless THIS group is already compromised. Suspicion
(`TSideUnknown`) fades with side accuracy (~4 min); compromise does not.
In vehicles: a **civilian vehicle** is anonymous, the vehicular equivalent
of walking unarmed; a **stolen occupier military vehicle** reads friendly at
range (the theft IS the disguise), suspect at observation accuracy >= 1.35
(the vanilla stolen-vehicle idiom, kept active), and exposed within
`undercoverNoticeRadius` (the driver is recognized), which also marks the
vehicle's own `Target` record compromised. A group whose person-record is
already compromised and that watched the boarding (last seen within
`undercoverBoardWitnessSeconds`, 10 s) or sees the vehicle close marks the
VEHICLE compromised: the getaway car is hot for its witnesses while every
other group still sees a civilian car. Cover re-establishment is emergent,
not a command: stow the weapon, break contact, or eliminate the witness
groups; group knowledge never propagates between groups, so unseen
garrisons keep reading a civilian. Everything serializes with the campaign:
per-group flags ride the existing `Target` records, system state (campaign
first-compromise latch + pending notifications) nests as an `"Undercover"`
subclass in the ZoneRegistry save block.

### A.3 Script command surface

Zone registry: `gmZoneCount` (nular→scalar), `gmZone <i>` (→ 10-tuple
`[name, type, owner, garrison, support, income, heat, marker, [x,y,z],
capture]`, the `GM_Z_*` indices 0..9), `gmZoneIndex "<name>"` (→ index or −1),
`gmZoneSet [i, "field", v]` (fields `owner|garrison|support|income|
liveOccupiers|capture|heatRaise|heatDecay`; `capture` clamps 0..100; an
`owner` write resets the capture meter — ownership discontinuity),
`gmZoneOnEvent ["<event>", handler]`
(`captured`/`supportThreshold`/`revealed`/`captureStarted`/`contested`/
`captureLost` with `_this=[zoneIndex, zoneName, ownerString]`;
`campaignLoaded` with `_this=[saveVersion]`). `supportThreshold` is
decoupled from the flip: it announces a town crossed `supportFlip` and is
READY; `captured` fires on the actual flip.

Factions: `gmFactionTierClass [side, warLevel]`, `gmFactionVehicle [side,
warLevel]`, `gmFactionValue [side, "key"]` (any plain key), `gmFactionSquad
[side, warLevel, count]` (→ array of classnames from the plan-15 role
template: leader slot first, MG/AT/medic/sniper slots interleaved with
riflemen, roles the tier does not field come back as riflemen; `[]` when the
faction has no tiers), `gmClassExists "<class>"` (→ bool, the CfgVehicles
probe the descriptor-resolution pass uses), and the side nulars
`gmOccupierSide`/`gmResistanceSide`.

Alert: `gmZoneAlert <i>` (0/1/2), `gmZoneLastKnown <i>` (`[x,y,z]` or `[]`),
`gmAlertOnEvent ["alertChanged"|"undercoverBroken", h]`
(`_this=[zoneIndex, zoneName, oldState, newState]` / `_this=[reason]`).
`undercoverBroken` fires ONCE per campaign, on the first compromise from
any source.

Undercover: `gmUndercoverStatus` (nular scalar: 0 clean / 1 suspected, an
occupier group holds a `TSideUnknown` record of the player / 2 compromised),
`gmUndercoverWitnesses` (nular scalar: count of occupier groups holding a
compromised record), `gmUndercoverForget` (clears every compromised record
plus the campaign latch; test aid + future disguise-swap hook), and
`gmBreakUndercover "<reason>"`, whose semantics changed with the
per-observer rework: it marks every occupier group that currently knows the
player as compromised; with zero witnesses it latches silently (no event,
no Heat). The global `gmUndercover` is **script-owned**; the engine reads it
each tick and the undercover layer is armed only while it is `true` (and
the player is captive). The old `gmUndercoverDetect` global is gone: the
engine reads `gmWarLevel` directly for war-level detection scaling.

Garrison: `gmGarrisonSpawned <i>`, `gmGarrisonLive <i>`, `gmGarrisonGroups <i>`
(array of live groups), `gmGarrisonOnEvent ["garrisonSpawned"|
"garrisonDespawned", h]` (`_this=[zoneIndex, zoneName, count]`),
`gmGarrisonForceDespawn <i>`.

Stashes: `gmStashRegister <obj>` (flags a supply object, typically a
WeaponHolder, keep-when-empty so emptying it no longer self-deletes it, and
tracks it; idempotent, no-op on non-supply objects), `gmStashUnregister <obj>`
(reverts to stock behavior: the holder self-deletes on the next removal that
leaves it empty), `gmStashCount` (nular → scalar), `gmStash <i>` (→
`[object, [x,y,z]]`, `[]` out of range). Registered stashes serialize
(`GuerrillaStashes`) and, unlike the other gm* systems, work outside
Guerrilla missions too.

Journal (the map-screen Notes/Plan pages): `gmJournalLog "<text>"` (diary
entry, stamped "Day N HH:MM" from the clock + `gmDayCount`; the diary keeps
the newest 200), `gmJournalObjective [id, text, "ACTIVE"|"DONE"|"FAILED"|
"HIDDEN"]` (upsert by id; `""` text keeps the existing text, so a state
flip is `["firstZone", "", "DONE"]`), `gmJournalStatus [key, text]` (a
"key: text" line in the Notes Situation block; `""` removes), and the
queries `gmJournalCount` (nular), `gmJournalEntry <i>` (→ `[stamp, text]`,
0 = oldest, `[]` out of range), `gmJournalObjectiveState "<id>"` (→ state
name or `""`), `gmJournalStatusText "<key>"` (→ text or `""`), plus
`gmIslandName` (nular → the world's CfgWorlds `description`, e.g. "Malden";
the only sanctioned island-name source for script text). The engine never
writes the diary itself: the managers do (see A.6); the engine-derived
Situation / next-steps content is computed at render time and needs no
script. All three tables serialize as `GuerrillaJournal`.

Traffic: `gmTrafficCount "civ"|"patrol"|"convoy"|"all"`, `gmTrafficVehicles`
(nular -> live hulls), `gmTrafficInfo <veh>` (-> `[kind, originIdx, destIdx,
state]`, `[]` when untracked), `gmTrafficOnEvent ["spawned"|"despawned"|
"commandeered"|"arrived"|"driverKilled", h]` (`spawned` `_this=[veh, kind,
originIdx, destIdx]`, `despawned` `[kind, reason]`, `commandeered` `[veh,
driver]`, `arrived` `[veh, kind, destIdx]`; `driverKilled` is the killed-EH
EXPRESSION attached to every civ driver, `_this=[victim, killer]`),
`gmTrafficRelease <veh>` (registry half of a commandeer), `gmTrafficForceSpawn
[kind, zoneIdx]` (-> OBJECT; test aid, bypasses chance/caps, not the road
placement). Roads (issue #33, first half): `gmRoadNearest <pos>` (-> road
point within 20 m or `[]`), `gmRoadPath [from, to]` (-> positions; the
engine's short-hop A*, empty on failure), `gmRoadsNear [pos, radius]` and its
binary alias `<pos> nearestRoads <radius>` (-> unlocked road-segment centre
positions). All positions in getPos order with height above ground.

Headquarters (GuerrillaBase): `gmHqCanEstablish [x,y,z]` (-> bool, a zone's
area contains the position), `gmHqEstablish [x,y,z]` (-> bool; establish, or
MOVE when one stands - the cache travels, `gmHqMoveCount` bumps, far garage
rows release), the nulars `gmHqEstablished`, `gmHqZone` (name or `""`),
`gmHqPos` / `gmHqGaragePos` / `gmHqCachePos` (`[x,y,z]`, `[]` while none),
`gmHqBuilding` / `gmHqCache` (objects, objNull while none / gone),
`gmHqMoveCount`, `gmHqIndoors`, and `gmHqValue "hqMinPos"|"garageRadius"|
"garageInvulnerable"`. Garage: `gmGarageLock [veh, bool]` (-> bool; lock
refuses a non-Transport, a hull outside the ring or one carrying the real
player; lock = beep-beep), `gmGarageCount`, `gmGarageVehicle <i>`,
`gmGarageHas <veh>`, `gmGarageInRange [x,y,z]`.

Market: `gmMarketActive` (nular; CfgGuerrillaMarket present + zones active),
`gmMarketValue "dealerShare"|"dealerRespawnSeconds"|"hqMoveCost"`,
`gmDealerCount`, `gmDealer <i>` (-> `[zoneName, "WEAPON"|"VEHICLE", [x,y,z],
npc, [lotx,loty,lotz]]`, `[]` out of range; npc objNull while dead /
unspawned), `gmDealerNearest ["WEAPON"|"VEHICLE", [x,y,z]]` (-> index or -1),
`gmDealerStock "WEAPON"` (-> `[[class, displayName, price, magazine, mags],
...]`; class `""` = magazine-only bundle, magazine `""` = weapon alone) and
`gmDealerStock "VEHICLE"` (-> `[[class, displayName, price], ...]`). The
purchase is the script's: market.sqs debits `gmResources` and does the
`addWeaponCargo` / `createVehicle` / `gmGarageLock`.

Modernized evaluator (usable everywhere now): `compile`, `isNil`,
`setVariable`/`getVariable` (objects), `nearestObjects`, `distance` on
position arrays, `setRank`/`rank`, `private`, `doMove`/`commandMove`,
`weaponCargo`/`magazineCargo` (→ classnames / `[classname, ammo]` pairs) and
`removeWeaponCargo`/`removeMagazineCargo` (one instance by classname).

### A.4 The event-queue pattern (binding for the script layer)

Native event handlers execute **synchronously inside the engine tick** — they
cannot `~` wait. Every handler registered by `init.sqs` is therefore ONE line
that appends its `_this` tuple to a `gmEvt*` queue global:

```
gmZoneOnEvent ["captured", {gmEvtCaptured = gmEvtCaptured + [_this]}]
```

Exactly **one manager consumes each queue** inside its own `~` loop, draining
it with an atomic swap on a single SQS line (one evaluated unit — the sim
cannot interleave a handler between the copy and the reset):

```
GM_cBatch = gmEvtCaptured + []; gmEvtCaptured = []
```

Queues are plain globals (serialize free) and handlers are serialized by the
engine, so the whole event plumbing survives save/load with no re-arm.

Queue → consumer map: `gmEvtCaptured`/`gmEvtSupport`/`gmEvtRevealed`/
`gmEvtCapStart`/`gmEvtContested`/`gmEvtCapLost` → `capture.sqs`;
`gmEvtAlert` → `qrf.sqs`; `gmEvtUcBroken` → `undercover.sqs`;
`gmEvtLoaded` → `campaign.sqs`; `gmEvtGarSpawned`/`gmEvtGarDespawned` →
`loot.sqs`.

### A.5 Faction keys (the naming scheme)

`gmFactionValue` reads any plain (non-array, non-class) key off a faction
class. The scripts use, on the **occupier**: `officer` (+ the structured
`tiers[]`/`tierThresholds[]`/`vehicles[]`/`vehicleThreshold` via the dedicated
commands). On the **resistance**: `holdClass`, `holdCount`, `recruitFighter`,
`recruitSpecialist`, `companionClass`, `baseWeapon`, `baseMagazine`, and the
loot-role loadouts `loot<Role>Weapon` / `loot<Role>Mag` for
`Role ∈ {Rifleman, Medic, MG, AT, Sniper}`. Numeric keys (e.g. `holdCount`)
are fetched with the lib helper `GM_fnFactionNum` (`gmFactionValue` +
`compile`).

**Plan-15 extensions (both sides, all optional):** per-tier role arrays
`tiersMG[]` / `tiersAT[]` / `tiersMedic[]` / `tiersSniper[]` parallel to
`tiers[]` (`""` or a short/missing array = the tier fields no such role; the
squad composer substitutes the tier rifleman), the full escalation ladder
`vehicleThresholds[]` (mirrors `tierThresholds[]`; supersedes the legacy
2-step `vehicleThreshold` scalar), and `fallbackClass` (probed first by the
resolution pass's side-fallback chain).

**Descriptor resolution pass (plan 15, engine path only):** at campaign
load, after side resolution, every classname-valued key is probed against
the loaded data package (`Pars >> CfgVehicles` / `CfgWeapons`) and
unresolvable entries are substituted per a fixed table — tiers fall back to
the nearest resolved tier then the per-side fallback list (`WEST SoldierWB`,
`EAST SoldierEB`, `GUER SoldierGB → SoldierGFakeE → SoldierEB → SoldierWB`,
`CIV Civilian → SoldierWCaptive`); roles blank to the tier rifleman;
vehicle rungs drop and the ladder compacts onto its authored war-level
gates; weapon/mag keys fall back to `baseWeapon` / their weapon sibling; a
fully-unresolvable CIV roster forces `civClassCount=0` (the managers'
no-CIV-layer path). Every substitution is RptF-logged. This is the
"missing SoldierG* posture" for the Demo [Remaster] package (#13): a
descriptor authored for one package degrades on another instead of
boot-failing or silently spawning nothing.

**Character outfit family (issue #25, all optional, resistance-side):** the
warrior/civilian outfit axis picked on the new-game screen (`gmSelOutfit`,
cycler idc 153; `WARRIOR` is defined as "the authored class" so publishing it
equals publishing nothing). Per-role scalars `playerClassWarrior` /
`playerClassCiv` (the engine substitutes the player's `mission.sqm` class at
`InitVehicles` when the selection is civilian — `World/WorldInit.cpp` →
`Game/Guerrilla/OutfitSelect.cpp`, raw-config lookup because the registry
loads later; probe failure keeps the authored class), and
`recruitFighterCiv` / `recruitSpecialistCiv` / `companionClassCiv` /
`holdClassCiv` (the managers pick them over their base keys when
`GM_OUTFIT_CIV`, which `init.sqs` folds once from `gmSelOutfit`; hold squads
under the civilian outfit bypass `gmFactionSquad` and spawn the
`holdClassCiv` monoculture — no `tiersCiv[]` role arrays exist). The array
`civTier[]` is the civilian-outfit ladder for AI garrison/guard/militia
rungs (served by `gmFactionCivTier`, same `tierThresholds[]` gates, clamped
to its own length; issue #16's guard surface). Scalar Civ keys ride the
plan-15 unit-key resolution (missing class → warrior fallback); `civTier[]`
has its OWN ladder `SoldierGFakeC → SoldierGFakeC2 → Civilian → tiers[0]`
(never the CIV-side `SoldierWCaptive` list). Disguised fighters never enter
the ambience layer (`GM_CIV_GROUPS`) — binding refusal documented in
`civilians.sqs`. Presence policy: civ-outfit units COUNT toward zone
presence like warriors (decided in issue #25 M3.4; rationale at
`ZoneRegistry.cpp CountSidePresence`).

**Player-body BODY browser (the class-driven answer to issue #25's open
vocabulary question, player-only):** the new-game screen's BODY cycler (idc
155) enumerates every creatable Man-derived `CfgVehicles` class of the
loaded package — no descriptor key involved — grouped `WEST/EAST/GUER/CIV`
(`GuerrillaListPlayerBodies`: `scope > 0`, deduped by displayName+model,
capped per side, config scan order; classes whose shape the package lacks
are skipped). A pick publishes `gmSelPlayerClass` (exact classname) and
beats the `gmSelOutfit` token for the PLAYER BODY ONLY at the same
`OutfitSelect` seam; squad matching (recruits/companions/hold) stays on the
outfit family and its `*Civ` keys. The browser's `(match outfit)` default
publishes nothing, keeping the untouched screen byte-identical to before. A
pick that fails the package probe keeps the AUTHORED class (never the token
body, never a substitute). Accepted emergent gameplay, not a bug: the body
keeps its CONFIG side for distant identification (vanilla side-resolve
ladder, stolen-uniform band at 1.35× the id range, real identity at 1.5×),
so an occupier-roster body reads as occupier at range while the instance
side stays the mission side — any body fights as GUER. The undercover layer
is untouched.

**Island/faction-agnostic rule (definition of done for issue #3):** the
`scripts/` directory contains **zero classnames and zero side-string
literals** — sides come from `gmOccupierSide`/`gmResistanceSide` (converted to
side *values* by the generic `GM_fnSideFromString`, which matches by
formatting the engine side nulars), classes come from `gmFaction*`. Engine
enum words (`"SAFE"`, `"MOVE"`, `"SENTRY"`, `"Flag"`, `"RoadSegment"`,
`"CITY"`, rank names) are engine facts, not island data, and stay in scripts.

### A.6 Script layer file map

```
mission/Guerrilla.Demo/
  description.ext      island data: CfgGuerrillaZones (+seedCities) + CfgGuerrillaFactions
  mission.sqm          one resistance player at the Camp (unchanged)
  init.sqs             THIN bootstrap: script-state seed, zone markers,
                       native handler registration, exec lib + managers
  scripts/
    lib.sqs            helpers: GM_fnRandPosNear/SpawnGroup/SideFromString/
                       CountOwnedBy/ZoneOfType/FactionNum/BumpGear + GM_LIB_READY
    capture.sqs        capture-event reactions: hold garrison (holdClass x
                       holdCount) on "captured", hints for the whole arc
                       (started/contested/lost/town-ready/liberated), throttled
                       in-field titleText progress ticker (GM_Z_CAPTURE)
    qrf.sqs            alert POLICY: garrison posture on alertChanged edges,
                       YELLOW investigate moves to gmZoneLastKnown, RED QRF convoy
                       (officer + tier squad + faction vehicle, road-snap,
                       teleport-on-stall fallback, stand-down cleanup)
    undercover.sqs     cover establish (gmUndercover=true + setCaptive, kept
                       for the whole campaign; never dropped on a break) +
                       "fired" EH -> gmBreakUndercover + ADVISORY
                       undercoverBroken hint (gmUndercoverWitnesses)
    campaign.sqs       Save addAction (self-dispatching) + campaignLoaded
                       reconciliation (companion handles, GM_PLAYER_GROUPS, action);
                       journal: "campaign begins" (gmIslandName) / saved /
                       restored diary lines
    (journal writes)   every manager below also feeds the native Journal
                       (gmJournalLog / gmJournalObjective / gmJournalStatus;
                       A.3): capture = ready/lost/liberated lines + firstZone/
                       firstTown objectives; qrf = QRF launch; undercover =
                       cover blown; companions = promotion/death + the
                       "Companions" status line (GM_fnCompStatus); loot =
                       unlock + "Unlocked gear" status line (GM_fnLootStatus)
                       + firstUnlock objective; escalation = War Level edges
                       (gmEsc_prevWL); recruit = recruit/train + firstRecruit
                       objective. No script renders anything: the map pages
                       are native (UI/Guerrilla/GuerrillaJournalPages).
    economy.sqs        income tick (unchanged formulas; reads gmZone tuples)
    escalation.sqs     War Level ladder (unchanged) + Heat decay via gmZoneSet,
                       gated on native GREEN
    loot.sqs           loot-on-kill + unlocks (unchanged logic; bodies from
                       gmGarrisonGroups; classnames from faction keys)
    recruit.sqs        Camp menu (unchanged flow; anchor = first CAMP-type zone;
                       classes from faction keys)
    recruit_action.sqs thin addAction dispatcher (unchanged)
    market.sqs         HQ / cache / garage / dealer action menus over the native
                       GuerrillaBase + Market facts (issues #16/#27/#28): the
                       Establish/Move HQ action (debits hqMoveCost on a move),
                       Stash <weapon> at the cache (removeWeapon + cargo;
                       retrieval = the holder's own TAKE actions), Lock/Unlock
                       the nearest vehicle inside the garage ring, the BUY menu
                       beside a live dealer (<=8 rows + a here/HQ delivery
                       toggle: WeaponHolder at your feet or the HQ cache; the
                       dealer's lot or the HQ garage locked); one-time map
                       markers (dealers, HQ flag); hqEstablish objective +
                       diary lines. The SECOND "-" writer of gmResources.
    market_action.sqs  thin addAction dispatcher (gmMktReq*), recruit_action twin
    companions.sqs     XP/rank/permadeath (unchanged model; companionClass from
                       faction key; live setRank on promotion now)
```

The reference slices `Qrf.<World>` (lib + qrf.sqs), `Undercover.<World>` (lib +
undercover.sqs) and `Market.<World>` (lib + market.sqs + market_action.sqs)
carry byte-identical SUBSETS of this core under their own bootstrap.

Deleted: `zones.sqs`, `spawning.sqs`, `alert.sqs`, `persistence.sqs` (their
loops are native; see A.2).

### A.7 What still binds from the Phase-1 spine

- **Naming:** `GM_*` data / `gm*` scalars / `GM_fn*` helpers / `GM_tmp*`
  scratch, plus `gmEvt*` for the native-event queues.
- **Single-writer ownership** per global (§4 below, updated by the file map
  above: each `gmEvt*` queue has one consumer; `gmResources`/`gmManpower` keep
  the +/− direction split; heat direction is engine-enforced).
- **SQS dialect rules:** one statement per line, `~N`, `@cond`,
  `#label`/`goto`, `? cond : stmt`; `call` bodies are strings and may not
  wait. Managers still gate on `@GM_LIB_READY` (kept over `isNil` polling for
  simplicity — one flag, one gate).

---

## 0. Dialect decision: **SQS**, with SQF code-strings as synchronous helpers

> **PARTLY SUPERSEDED (Phase 1.5).** The dialect choice (SQS managers,
> code-string helpers) still stands. The "confirmed-absent" list below does
> **not**: `compile`, `isNil`, `setVariable`/`getVariable`, `nearestObjects`,
> `rank`/`setRank`, `private`, `doMove`/`commandMove` and array `distance`
> all exist in the modernized evaluator and the scripts use them freely.

**Managers and bootstrap are SQS (`.sqs`).** Evidence:

- `init.sqs` is auto-loaded by `RunInitScript` (`engine/Poseidon/UI/DisplayUI.cpp:155-162`); `init.sqf`, if present, is loaded *after* but runs **unscheduled** — it has no `~` scheduler, so it cannot host the timed tick-loops the meta-game needs.
- SQS gives the three primitives the loops require: `~N` waits, `@cond` wait-until, and `goto "label"` + `#label` (`ScriptGoto`, `GameStateExt.cpp`).
- `[args] exec "file.sqs"` spawns a script asynchronously.

**Helpers are SQF code-strings.** In this evaluator a `{ … }` literal **is a
`GameString`**, and the body operands of `call` / `while` / `do` / `then` /
`else` / `forEach` are strings (`engine/Evaluator/express.cpp`). So
`[args] call GM_fnX` runs the string with `_this` bound and returns the last
expression's value — synchronous, no `~` inside.

---

## 1. Mission directory layout

> **SUPERSEDED (Phase 1.5).** See §A.6 for the current layout. The world
> still comes from the mission folder's final suffix (`.Demo`).

---

## 2. Naming convention (binding)

| Prefix    | Meaning                                              | Persisted? | Examples |
|-----------|------------------------------------------------------|-----------|----------|
| `GM_*`    | constants / tunables / structured data (UPPER words) | yes (data) | `GM_GEAR_THRESHOLD`, `GM_COMP_NAMES` |
| `gm*`     | mutable scalar faction state                         | yes        | `gmResources`, `gmManpower`, `gmWarLevel` |
| `gmEvt*`  | native-event queues (handler appends, one consumer)  | yes        | `gmEvtCaptured`, `gmEvtAlert` |
| `GM_fn*`  | code-string helpers (defined in `lib.sqs`)           | no (redefined each load) | `GM_fnSpawnGroup`, `GM_fnSideFromString` |
| `GM_tmp*` | scratch globals used inside helpers                  | no (never read) | `GM_tmpG`, `GM_tmpI` |
| `GM_*OBJ`/`GM_PLAYER_GROUPS`/`GM_LOOT_TAGGED` | live handles | reconciled on load | `GM_COMP_OBJ` |

**Persistence rule, updated:** script globals — including object/group
references — serialize with GGameState. Live-handle globals are still
*verified* after a load (`campaign.sqs`: a link that came back null/dead is
rebuilt from the persisted rows; a valid link is kept). Zone/alert/garrison
state is native and needs nothing.

---

## 3. Global-variable schema

> **SUPERSEDED for zones/alert/cache (Phase 1.5).** `GM_ZONES`,
> `GM_ALERT_STATE`, `GM_ALERT_TIMER`, `GM_CACHE_GROUPS`, `GM_CACHE_ZONEIDX`
> and `GM_CACHE_RADIUS` no longer exist — zone state lives in the native
> `ZoneRegistry` (read via `gmZone`, written via `gmZoneSet`), alert state in
> the `AlertMachine` (`gmZoneAlert`), garrison bookkeeping in the
> `GarrisonCache` (`gmGarrison*`). The `GM_Z_*` index constants (0..8) are
> kept in `init.sqs` purely to make `gmZone` tuple reads legible.
>
> Still live and script-owned, as seeded by `init.sqs`: the faction scalars
> (`gmResources`, `gmManpower`, `gmWarLevel`, `gmHeatDecay`, `gmUndercover`),
> the gear arrays (`GM_GEAR_*`, sole writer `GM_fnBumpGear`), the companion
> roster (`GM_COMP_*`), `GM_PLAYER_GROUPS`, and the `gmEvt*` queues.

---

## 4. Manager API contract (read / write / owns)

> **SUPERSEDED table (Phase 1.5)** — replaced by:

| Script            | OWNS (exclusive write)                                   | Reads (native surface + globals) |
|-------------------|----------------------------------------------------------|----------------------------------|
| `lib.sqs`         | `GM_fn*`, `GM_LIB_READY`, `GM_tmp*`                       | — |
| `capture.sqs`     | consumes `gmEvtCaptured`/`gmEvtSupport`/`gmEvtRevealed`/`gmEvtCapStart`/`gmEvtContested`/`gmEvtCapLost`; `GM_HOLD_*`, `GM_TT_*` | `gmZone`, faction hold keys |
| `qrf.sqs`         | consumes `gmEvtAlert`; `GM_QRF_*` (QRF group/vehicle transient) | `gmZoneAlert`, `gmZoneLastKnown`, `gmGarrison*`, `gmWarLevel`, occupier faction keys |
| `undercover.sqs`  | `gmUndercover`, consumes `gmEvtUcBroken`, player fired-EH | `gmUndercoverWitnesses` (advisory hint only) |
| `campaign.sqs`    | consumes `gmEvtLoaded`; `GM_pSaveAct`; reconciles `GM_COMP_OBJ` nulls, `GM_PLAYER_GROUPS` reseed | companion rows |
| `economy.sqs`     | `gmResources`/`gmManpower` (+ only), `GM_ECON_*`          | `gmZone` tuples, `gmResistanceSide` |
| `escalation.sqs`  | `gmWarLevel`, heat **decay** (`gmZoneSet heatDecay`) | `gmZone` owners, `gmZoneAlert` |
| `loot.sqs`        | `GM_GEAR_*` (via `GM_fnBumpGear`), consumes `gmEvtGar*`   | `gmGarrisonSpawned/Groups`, resistance loot keys |
| `recruit.sqs`     | `GM_PLAYER_GROUPS`, `gmAct*`/`gmReq*` (consumer), `gmResources`/`gmManpower` (−) | `gmZone` (CAMP anchor), faction recruit keys |
| `market.sqs`      | `gmMkt*` / `GM_MKT_*` (menus, request queue consumer, markers), `gmResources` (−: purchases + HQ moves; the second debit writer next to recruit.sqs), the HQ election / garage lock / cache stash CALLS (`gmHqEstablish`, `gmGarageLock`, cargo moves), `hqEstablish` objective | `gmHq*` / `gmGarage*` (GuerrillaBase), `gmMarket*` / `gmDealer*` (Market), the player's weapons / magazines |
| `companions.sqs`  | `GM_COMP_XP/RANK/SKILL/ALIVE/OBJ`, `GM_fnComp*`           | `GM_COMP_NAMES/LOADOUT`, companionClass key |

Heat raising is native-only now (capture, alert edges, undercover
compromise on the witness's nearest zone); `escalation.sqs` holds the sole
scripted decay verb.

CITY `support` has exactly two sanctioned writers: the scripts'
`GM_fnSupportAdd` (civilian-kill deltas + delayed resentment, clamps 0–100,
lands regardless of presence) and the **native channel** in
`ZoneRegistry::EvaluateTick` (presence-gated accrual while a town is
occupier-free; intimidation decay while occupier-only, floored at
`supportDecayFloor` — the floor applies to the native decay ONLY, so a
script-authored value below it is never raised).

---

## 5. Bootstrap contract (`init.sqs`)

> **SUPERSEDED (Phase 1.5).** The bootstrap is THIN now: single-run guard
> (`isNil "GM_BOOTED"`), script-state seed (§3 note above), one marker per
> zone (`createMarker [name, pos]`, size-2 form; the engine only repaints
> existing markers), the eight one-line native handler registrations, then
> lib + managers. No `GM_ZONES`, no trigger objects, no marker rebuild logic.

### SQS idioms used (all confirmed, unchanged)
- one statement per line; `~N` to wait N seconds; `@cond` to wait-until.
- `#label` + `goto "label"` for tick loops; `? cond : stmt` conditionals.
- `[args] exec "file.sqs"` to spawn; `[args] call GM_fn` for synchronous helpers.

---

## 6. `lib.sqs` helper surface

> **UPDATED (Phase 1.5).** Pruned: `GM_fnDist2D` (binary `distance` works on
> position arrays), `GM_fnZoneIndex` (native `gmZoneIndex`), `GM_fnPickTier`
> (native `gmFactionTierClass`).

| Helper                | Signature → result                  | Notes |
|-----------------------|-------------------------------------|-------|
| `GM_fnRandPosNear`    | `[center,radius] → [x,y,0]`         | square jitter, getPos order |
| `GM_fnSpawnGroup`     | `[sideVALUE,class,count,pos] → group` | callers split above the 12-cap |
| `GM_fnSpawnSquad`     | `[sideVALUE,classes[],pos] → group` | one unit per array entry (from `gmFactionSquad`) |
| `GM_fnSideFromString` | `sideString → side value`           | matches by formatting the side nulars; no side literals |
| `GM_fnCountOwnedBy`   | `ownerString → scalar`              | scans `gmZone` tuples |
| `GM_fnZoneOfType`     | `typeString → first index (−1)`     | e.g. the CAMP anchor |
| `GM_fnFactionNum`     | `[side,key,default] → scalar`       | `gmFactionValue` + `compile` |
| `GM_fnBumpGear`       | `[class,amount] → newCount`         | sole `GM_GEAR_*` writer |

**`createMarker` reminder:** size-2 `createMarker [name, pos]`.

---

## 7. `mission.sqm` (minimal valid)

Unchanged: `version=11`, one resistance player unit at the Camp
`[6519.04, 149.66, 6473.68]` (sqm order: easting, **elevation**, northing —
see the coordinate contract in §A.1). `.sqm` parsing tolerates no `//`
comments, so notes live here.

---

## 8. What the build agents inherit (do / don't)

- **DO** read zone/alert/garrison state ONLY through the `gm*` native
  commands; write heat only through `gmZoneSet heatRaise/heatDecay` (and only
  in the direction your script owns).
- **DO** keep every classname and side string out of `scripts/` — faction
  keys + `gmOccupierSide`/`gmResistanceSide` are the only sources; grep before
  you ship.
- **DO** follow the event-queue pattern (§A.4): handlers enqueue one line,
  managers drain-swap atomically and react in their own loop.
- **DON'T** re-register native handlers on load, rebuild markers on load, or
  reintroduce a `GM_SAVED`-style sentinel — persistence is native
  (`campaignLoaded` is the load signal).
- **DON'T** put `~` waits inside a `call`'d helper or an event handler.
