# Generated occupier bases: design half

Read-only assessment, 2026-10-15, branch `occupier-forces` at 197a7d3. The engine half (placement
queries, prop APIs, persistence mechanics) is the sibling agent's; this file covers whether bases
are the problem, how many, what kinds, how they should look and how play changes. Coordinates are
`[easting, northing]`. Times use the plan's real speeds (foot 1.5 m/s; truck 0.6 x Ural 80 km/h =
13.3 m/s; Mi17 0.8 x 240 km/h = 53 m/s) over the straight line x 1.3 as a road estimate (the trace
shows real road factors from 1.0 to 3.4, so treat the minutes as order of magnitude).

## 1. Diagnosis: Malden

**Zone set (Guerrilla.Abel, seedCities 1, seedBases resolves 0).** Camp (RESISTANCE) [7515, 5790];
Village (Houdan, CITY) [7117, 6069]; Outpost (OUTPOST, 6) [7600, 6300]; Southeast Compound (OUTPOST,
6) [9715, 3940], on its own island, air-only (3.1 landmass rule); Le Port Harbour (SEAPORT, 8)
[8410, 3690]; Airfield (AIRFIELD, 12) [8030, 9905]; 13 seeded CITY rows (garrison 4). R 32, K 14, G
56, poolStart 60.

**Sources.** Four bases, three of them ground sources, all three in the east half (x 7600 to 8410).
Stored spare when full (`CounterAttack::Spare`: stored minus floor(garrisonMax x 0.25)): Outpost 5,
Harbour 6, Airfield 9, Compound 5 (air only). A base whose garrison is materialized (player within
800 m) has no stored spare and sources nothing; the Camp is 517 m from the Outpost, so with a Camp
start the Outpost is out of the director's hands whenever the player is home.

| Town | Nearest ground base | Straight | Foot | Truck | 2nd base |
|---|---|---|---|---|---|
| Saint Louis | Airfield | 1.3 km | 19 min | 2 min | Outpost 2.7 |
| Larche | Airfield | 2.4 km | 34 min | 4 min | Outpost 2.8 |
| La Trinite | Outpost | 1.7 km | 24 min | 3 min | Airfield 2.1 |
| Dourdan | Outpost | 1.0 km | 14 min | 2 min | Airfield 3.0 |
| Houdan (Village) | Outpost | 0.5 km | 8 min | 1 min | Harbour 2.7 |
| Arudy | Outpost | 2.2 km | 32 min | 4 min | Airfield 3.9 |
| Le Port | Harbour | 0.6 km | 9 min | 1 min | Outpost 3.2 |
| Chapoi | Harbour | 2.6 km | 37 min | 4 min | Outpost 3.3 |
| Sainte Marie | Harbour | 2.9 km | 42 min | 5 min | Outpost 2.9 |
| Cancon | Harbour | 3.2 km | 46 min | 5 min | Outpost 4.2 |
| Goisse | Outpost | 4.6 km | 66 min | 8 min | Airfield 4.6 |
| La Pessagne | Outpost | 4.5 km | 65 min | 7 min | Harbour 5.9 |
| Vigny | Outpost | 4.6 km | 66 min | 7 min | Airfield 5.8 |
| La Riviere | Harbour | 4.7 km | 68 min | 8 min | Outpost 4.9 |

All 14 towns are on the main landmass (landmass 0); only the Compound is not. A helicopter from the
Airfield reaches any town in 0.4 to 2.5 min.

**No base within reasonable reach (2.5 km, about 35 min on foot):** Goisse, La Pessagne, Vigny, La
Riviere (4.5 to 4.7 km), Cancon (3.2), Sainte Marie (2.9), Chapoi (2.6). Seven of 14, the whole west
and south-west. This matches contract C5's existing fail.

**What caused what the owner saw.**

- *Forces always from the same two places.* Mostly the base count and where the bases sit. Outpost
  or Harbour is the nearest source for 12 of 14 towns, and both stand in the south-east quadrant 2.7
  km apart. The choice goes to the cheapest package, so most answers are foot squads from one of
  those two, arriving from the east. More bases spread in the west fix this directly.
- *Quiet stretches.* Partly base count, partly cadence. A quiet flip is not urgent, so the cheapest
  package (foot, 4 bodies) wins. In the west that means a 65-minute walk, and the player has moved
  on before it arrives. Spare is thin too: the network holds about 20 ground bodies when full, two
  operations take 8 of them ("reserve now 2" in the trace), and regrowth is 1 body per 600 s per
  base, so three ground bases rebuild 18 bodies an hour between them. But tokens set the overall
  rate (STRONG 1 per 30 min, at most 2 in hand, `forcesOpsMax` 2), and more bases do not change
  that. Bases change where answers come from and how soon they arrive, not how many there are.
- *"No operation on Airfield: no occupier base can spare the minimum", repeated through the run 3 trace.* This is
  the **ladder and the minimum rule**, not the count. A base retake needs `min(max(4, garrisonMax),
  12)` bodies from ONE source, so 12 for the Airfield. Once the Airfield falls, the best spare left
  is the Harbour's 6. The largest base on an island can never be retaken by an ordinary operation,
  only by a wave strike. The same holds one rung down: retaking the Harbour (8) needs a source with
  garrisonMax 10 or more. Generated outposts of 6 do not change this.
- *The Compound air-only.* This is placement (an island). Its 6 bodies count in R and poolStart but
  can only leave by helicopter. The owner has parked this until the vehicle system work.
- *Insertions rare while bases are stocked.* This is price, not count. For a 4-body town retake:
  foot 4, truck 8, helicopter 16. An urgent target may spend up to 1.5 x the cheapest, which is 6,
  so the helicopter loses both ways. It only appears through the re-plan step, when every ground
  source is watched or used up. **More bases make insertions rarer still.** If the owner wants
  helicopters in the air, the lever is package choice (for example, prefer air when the foot ETA is
  over N minutes), not bases.

**Verdict.** "Not enough bases" is real for geography and for the bearing monotony on Malden and
Everon. The two other headline symptoms (the Airfield refusal, rare air) come from the single-source
minimum and from prices. Generation should ship with two director-side companions, listed in 6.

## 2. Target density

Per-square-km rules break on these maps: Sinai is mostly empty desert, Lebanon80 is a theatre cut
out of a 25 km map, and the sea makes up a large, island-specific share. The occupier model cares
about towns and travel time, so key the rule to K:

1. **Coverage (primary).** Every K town has a ground-capable occupier base on its own landmass
   within 2.5 km straight line. That is about 35 min on foot by road and about 4 min by truck, so a
   non-urgent foot answer lands inside a typical play hour.
2. **Redundancy.** At least 3 ground sources per landmass that carries towns. One may be live next
   to the player and one may be drained by a recent operation; the third keeps the director able to
   answer.
3. **Floor and ceiling.** Total bases at least ceil(K / 3) and at most ceil(K / 2), and never more
   than 10. The cap protects the 40-group budget (each held base keeps a live 3-man hold squad) and
   Traffic's patrol-pair count.

Generation adds bases only until rules 1 and 2 hold (rule 3 caps it). It places the fewest bases
that close the gaps, preferring a detected OUTPOST candidate (real props already standing) over
empty ground.

| Island | K | Bases now | Gaps at 2.5 km | Floor | Proposed | Added |
|---|---|---|---|---|---|---|
| Malden | 14 | 4 (3 ground + Compound) | 7 towns, west and south-west | 5 | 6 | 2 generated: West (Vigny, La Pessagne, Goisse, about [3500, 7400]) and South-west (Cancon, La Riviere, Chapoi, Sainte Marie, about [4800, 3300]); afterwards the worst town is Larche at 2.4 km |
| Everon | 18 | 5 (Outposts 1 to 3, prop-less; seeded Airfield 12, Harbour 8) | 6: Lamentin 6.1, Le Moule 5.0, Morton 3.6, Gravette 3.4, Regina 2.9, Figari 2.5 | 6 | 8 | promote Levie Post (detected compound [7524, 4182]) and generate West (about [1900, 5000]), Central (about [4300, 6900]) and South (about [5600, 4300]); Outpost 1 [7535, 8760] then covers no town alone (drop it or retype it as a checkpoint, template call); worst town 2.3 km |
| Sinai | 26 (plan 5.1; the contract's poolStart 112 implies 13, so measure it) | 9 inside the default base box (2 authored + 7 seeded, R 86) | none inside the box; Abu Ageila, Ras Sudr, Bir El Dab, Ruins, Ismalia (2.9 to 6.8 km) are all north of it | 9 | 9 | 0 until the Sinai `playRegion[]` (#75) decides whether the north is in the theatre; at most 2 if it is. Sinai's problem is the opposite one: too many seeded bases, some of them odd (Aqaba is Jordan) |
| Lebanon80 | 3 | 2 (Litani 6, Marjayoun 8) | Tyre 3.5 km | 3 | 3 to 6 | none generated: author the #76 list (Tyre compound, two border posts, Mt Dov) and set `generateBases = 0`, like `seedBases = 0` |

**The pool.** `poolStart = max(24, R + 2K)`, so more bases raise poolStart, waves (half of
poolStart), refill (`(poolStart + R + G) / 4`) and spare together. Malden with two outposts of 6: R
44, poolStart 72, waves 36, refill 43 an hour, against 60, 30 and 37 now. That is an occupier about
20 % stronger just for having more buildings.

Recommended default: **conservation**. Generated garrisons are bodies moved out of the pool into
bases, not new bodies: `poolStart = max(24, R_authored+seeded + 2K) - R_generated`, with wave size
and refill computed from the unchanged theatre total.

- Malden: pool + reserves stays 92, so poolStart 48; waves stay 30; refill stays 37 an hour.
- Everon: pool + reserves stays 112, so poolStart about 56.

The occupier then holds more infantry spare and slightly less money for hulls. That is more presence
at the same strength, which matches the complaint. Strengthening instead (no subtraction) should be
a difficulty knob, not the default. Bands are shares of poolStart, so they need no change.

## 3. Kinds and layouts

Two sizing facts drive the catalogue:
- Spare is `garrisonMax - floor(garrisonMax / 4)`. A garrison of 4 spares 3, which cannot source the
  4-body minimum. Garrisons of 5, 6, 8, 12 and 16 spare 4, 5, 6, 9 and 12.
- A base retake needs one source sparing the target's garrisonMax (capped at 12). Only a 16-garrison
  base can retake an airfield.

| Kind | Role in the forces model | Footprint | Garrison | Props |
|---|---|---|---|---|
| Checkpoint | Tripwire: alarm generator, Traffic roadblock node; sources nothing at 4, one town retake at 5 | 30 x 20 m at the road shoulder | 4 to 5 | 10 to 14 |
| Outpost | Town-retake source; the workhorse of step 1 | 60 to 80 m square | 6 to 8 | 16 to 24 |
| FOB | Regional hub: base-retake source (spare 12), wave hub fallback (the "strongest base") | 150 to 200 m | 12 to 16 | 26 to 30 |
| Depot | Stored hulls: motorized packages from here; a vehicle prize | 80 x 60 m at a junction | 6 | 14 to 20 |
| Helipad annex | Air origin (insertions start here when no AIRFIELD is held) | 120 x 80 m, LZ-grade ground | 6 | 10 to 16 |
| Coastal post | Coastal tripwire, source on its landmass, future boat origin | 40 x 40 m on a headland | 4 to 6 | 10 to 14 |
| Radar / AA site | Comms node: optional intel or alarm modifier (owner call, step 3 at the earliest) | 60 m ring on a hilltop | 4 to 5 | 10 to 14 |

**Composition rules (what makes it read as placed).** Pick one base heading: the gate faces the
access road, and the axis runs perpendicular to the road. Fences, towers and huts sit on that grid
at exact corners; only crates, tents and sandbags take up to 5 degrees of jitter. Use one symmetric
core with one purposeful asymmetry (the vehicle bay on the road side). Towers and bunkers face the
road and the nearest town (the threat side). Buildings stay upright, sandbags follow the ground.
Keep 4 m lanes between props so the garrison AI can path (the sibling checks that `GarrisonCache`
spawn points miss props). Pick a footprint free of trees rather than clearing one. Nothing stands
within 15 m of a road edge except a checkpoint's hut and raised barrier on the shoulder, and nothing
ever stands across the carriageway (stopped objects lock RoadLinks, the #87 trap). Stock models
observed in the #76 ground truth: `hlaska`, `strazni_vez`, `fortress1/2`, `garaz_bez_tanku`,
`garaz_s_tankem`, `fuelstation_army`, `repair_center`, `budova1..5`, `plechbud`, `pletivo_wired`
with its `_branal/_branap` gate pieces, `plot_provizorni`, `jezek kov`, `barbedwire`, `zavora`,
`podprapornik`. From `guerrilla-hq.hpp`: `bedna_ammo`, `hromada_beden`, `stulsuplikmapa`,
`CampEmpty`. LoBo adds `brown_wachhaus`, `brown_turm`, `brown_baracke0N`, `radioantenna_l/s`,
`f3wx_radiotower`, `chd_barrier`, `Pristresek`. Choose a set per faction style (stock grey for
EAST/WEST, LoBo brown for IDF and the Arab armies).

- **Checkpoint.** Model: a Czechoslovak Border Guard crossing (a striped `zavora` barrier, a wooden
  `strazni_vez`, a guard hut) and the Soviet road KPP. Hut 8 to 12 m off the edge facing the road;
  barrier raised on the shoulder; a sandbag horseshoe MG nest 15 m up-road with fire along it; 4 to
  6 hedgehogs staggered on the far shoulder; the tower 20 to 30 m back on the high side; tent, crate
  and flagpole behind the hut. One parked rung-0 car (UAZ, Jeep) on the shoulder, stealable.
- **Outpost.** Model: the Soviet *zastava*, the platoon road post of Afghanistan (on a rise over the
  road, a dug-in BMP as a corner pillbox, a wire ring, a mortar pit in the middle), and the IDF
  Sinai *mutzav* (a square perimeter, corner towers, a tank ramp, an antenna mast). Square perimeter
  on a rise 100 to 250 m off the road, joined to it by open ground. Gate on the road side: barrier,
  hut, sandbags. Wire on two sides (8 to 12 segments) with the open sides covered by bunkers. Towers
  on two opposite corners. In the middle: 2 or 3 tents or huts, a command tent with the map desk, a
  flagpole. A vehicle bay (a garage or a sandbag revetment) with one APC or truck. An ammo crate by
  the command post becomes a stash when captured (`StashRegistry` keep-when-empty, as the HQ cache
  works).
- **FOB.** Model: a Soviet battalion garrison or an IDF brigade forward base. Main gate to the road
  plus a rear gate; wire all round with 4 towers. A motor pool (2 or 3 garages, 2 or 3 parked
  hulls), a fuel point (`fuelstation_army` and a fuel truck), `repair_center`, a barracks row, an HQ
  building with a radio mast, a `HeliH` at the rear, and a sandbagged ammo dump (`hromada_beden`).
  Optional seat for an enemy Legend boss (the dossier Change 3 precedent).
- **Depot.** A fenced yard by a road junction with 3 or 4 hulls in rows facing the gate, ready to
  roll; crate stacks, tin sheds, a fuel point, one gate tower and a guard tent.
- **Helipad annex.** Ground that passes the LZ probe's own test (dry, slope 0.25 or less over 22 m,
  object-free). Two `HeliH` pads 40 m apart, a parked fuel truck, a crew tent, sandbag rings on two
  corners, one tower.
- **Coastal post.** Model: a Soviet coastal observation post, or the IDF posts on the Gulf coast
  (Ras Nasrani). A tower facing the sea, 2 bunkers covering the beach, a sandbag line, a hut and a
  flagpole.
- **Radar / AA site.** Model: a Soviet P-15 radar post with ZU-23-2 pits. A radio mast, 2 sandbagged
  AA pits (with the faction's AA hull where its ladder has one, such as LoBo's `Ural zsu23`), a
  generator shed, a command tent, wire and one tower.

## 4. Placement rules (player's side)

- **Towns.** 500 m to 2.5 km from the centre of every town it serves, and at least 500 m from any
  settlement. A base inside a town's 250 m birth bound is a source whose births fail (the declined
  L13 `ArriveLive` trap), and its garrison would muddle the town census.
- **Player start, Camp and HQ.** At least 1.6 km away, computed after the START TOWN pick, so a
  generated base is never live while the player is home (a live base has no stored spare) and is
  never a free first-minute ambush. (The playtest's Cancon start would push the south-west base
  toward La Riviere and Chapoi.)
- **Spacing.** At least 1.6 km between bases (twice `cacheRadius`), so a raid materializes one
  garrison, not two.
- **Road.** Centre 60 to 250 m from a road link that joins the island network (contract C8), on the
  base's own landmass. Never on a road. At least one in three generated bases sits 400 m or more
  from the trunk road, on a track, so arrivals do not always come up the same highway.
- **Bearing.** For each town, its two nearest sources should differ in bearing by at least 60
  degrees where the island allows it.
- **Exclusions.** Never within 300 m of the Camp, the HQ or any authored or seeded zone; never in
  water; never in another landmass's coverage role.
- **Visibility.** Step 1 uses the existing rule: a zone reveals within `revealRadius` (1500 m) like
  authored ones. Later, the player's discovery could mirror the director's D9 intel: an informant in
  a town with support at 50 or more points to the nearest unknown post with a rough marker (300 m
  jitter), sharpened on approach. Showing every generated base from minute one would empty the map
  of discovery. Owner call (6).

## 5. Player experience

- **Arrivals.** Western Malden answers come from about 1.1 km (17 min on foot) instead of 4.6 km (66
  min), so a quiet flip is answered while the player is still in the area. Forces come from five
  bearings, not two. The overall rate is unchanged (tokens), so this fills the map with plausible
  answers without doubling them.
- **Alarms.** A raided base pulls an answer from a neighbour 1.6 to 3 km away, so raids become
  timed: hit, loot, leave before the truck comes.
- **Targets.** More places to raid, each with a stash crate, a vehicle bay and a flag to take.
  Checkpoints become the early-game hit-and-run target that Malden lacks in the west.
- **Hulls.** Checkpoint cars, outpost APCs and depot trucks can be stolen. This touches #99: free
  vehicles undercut the dealer. Proposal: parked hulls are locked while the base is occupier-held
  and only become usable once the base is captured, and a stolen hull is booked as a hull loss
  (`CAOpHullLost` semantics) so the pool pays it.
- **Traffic.** More base pairs within 3 km means more patrol routes (C3), which is welcome, but
  watch the frame budget.
- **The win.** `CampaignEnd` needs every roster zone held at once. With 6 roster bases instead of 4,
  two of them far west, and any occupier unit able to undo a base in about 18 s (D19), the win gets
  materially longer. Each held base also keeps an always-live 3-man hold squad. Proposal:
  checkpoints, coastal posts, helipads and radar sites are *minor posts* (`roster = 0`). They
  capture, source and count for alarms, but are left out of the win, control M and R. Outposts,
  FOBs, depots, airfields and seaports count. On Malden that keeps the win at 6 counted zones at
  most, or 4 if both generated bases are minor. Hold squads as a cached reserve (step 2 of the
  abstract-director assessment) becomes more urgent with more bases.
- **Names.** Reuse the `BaseDetect` naming chain over `WorldNames` settlements: "<nearest
  settlement> <noun>", for example "Vigny Outpost" or "La Riviere Checkpoint", unique by numeric
  suffix as for "Le Port Post 2". Fallback for open country: "Hill <elevation> <noun>", the Cold War
  convention ("Hill 225 Post"). The noun comes from the kind, optionally flavoured by the faction
  descriptor ("Strongpoint" for EAST, "Outpost" for the IDF). The name is the savegame key, so it is
  generated once and stored, never recomputed.
- **Journal and map.** Revealed generated bases use the ordinary zone marker, with a kind icon
  later. The dossier gains a "known enemy positions" list (name, kind, garrison as last seen by the
  player, never the true count), which fits #98. A FOB may name its commander through the enemy
  Legend boss precedent.
- **Saves.** Generate once at new game, after the START TOWN pick, from the campaign seed. Store
  each base as a zone row with `generated`, kind, heading and layout seed. Rebuild props on load
  from (kind, position, heading, seed) and save only a destroyed-props mask and the parked-hull
  state (the sibling decides how). **Old saves get nothing**: no rows means no generation. This
  avoids repeating the #76 lesson (old Eden and Sinai saves gained seeded bases outside the ledger)
  and follows D20 (old saves keep the core behaviour). A later template edit cannot shift a running
  campaign's bases because they are saved.

## 6. Recommendation and incremental path

Generate bases, but as coverage repair, not as decoration. Ship them with the two director fixes, or
the owner will still see the Airfield refusal and no helicopters.

- **Step 0, director-side companions (small, independent).**
  - (a) Base-retake minimum: let two sources combine up to the minimum, or cap the minimum at the
    largest available spare when the target is a base. Either one ends "no occupier base can spare
    the minimum" for the biggest rung.
  - (b) Owner's choice: a foot package whose ETA exceeds about 30 min counts as too slow, so the
    director takes truck or air. This puts trucks and helicopters on screen without new bases.
- **Step 1, zones only (no props).** A `generateBases` tri-state like `seedBases` (absent: generate
  when the coverage rule fails; 0 off; 1 on). It places OUTPOST zones (garrison 6) where coverage
  fails, preferring a detected OUTPOST candidate (real props; this reverses the #76 rule that
  OUTPOSTs are never seeded at runtime, an owner call), then empty ground by the section 4 rules.
  Includes the conservation pool rule, proximity reveal, names and save rows. This fixes the count
  and the sourcing. Eden's Outposts 1 to 3 show its limit already: a garrison of 6 in an olive grove
  reads as scattered, which is why step 2 follows closely.
- **Step 2, one layout.** The Outpost layout (16 to 24 props) with its stash crate and one parked
  hull, plus the destroyed-props save.
- **Step 3, the catalogue.** Checkpoint, FOB (which also removes the need for step 0a on islands
  that get one), depot with stored hulls, helipad annex as an air origin (`HeldAirfield` extended to
  helipads), coastal post; the `roster = 0` minor-post flag. Radar and AA last, only if the owner
  wants the comms mechanic.

**Decide with the owner first.**
1. The density rule: 2.5 km coverage, 3 ground sources, ceil(K / 3) floor.
2. Whether authored templates keep authoring with generation on top for gaps only (recommended;
   Lebanon80 opts out), or generation replaces hand ladders.
3. Whether generated bases show on the map from the start, by proximity (recommended for step 1) or
   by informant rumour (later).
4. Conservation (spread the same strength) or extra strength.
5. Whether minor posts count toward the win.
6. Whether detected OUTPOSTs may be seeded at runtime when they fill a gap.

**Issue to file** on GearUnclear/CWR: "Guerrilla: generated occupier bases close town-coverage gaps
(zones, then layouts)". Body: sections 1, 2 and 6 here plus the sibling's engine plan; link #76
(network), #97 (the Compound and outpost-ladder content items), #98 (journal), #99 (stealable hulls
vs the dealer) and #75 (the Sinai playRegion). File step 0a/0b as a separate director issue, since
it does not depend on generation.
