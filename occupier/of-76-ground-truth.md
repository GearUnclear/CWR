# Issue #76 ground truth: airfields, seaports, outposts per world

Written by the #76 ground-truth agent (agent2, build/agent2-rwdi, shadow
`tmp/remaster-shadow-of2`), 2026-10-14. The implementation agent reads this.
Every number names its source. Coordinates are engine axes `[x, z]`
(easting, northing), the same order as `position[]` in a template.

Sources:
- **CFG**: `PoseidonTools config debin <shadow>/BIN/CONFIG.BIN` (stock worlds;
  `remaster.cpp` and `config-extra.cpp` carry no CfgWorlds or ILS keys), and
  `PoseidonTools pbo show @LoBo/addons/lost.pbo config.cpp` (sinai),
  `... LoBo_Leb.pbo config.cpp` (Lebanon80). This engine's SQF has no
  `configFile` / `getArray`, so a tri probe cannot read config keys; the
  debinarized config is the source for part (a).
- **WRP**: `PoseidonTools terrain objects <world>.wrp` (stock wrps from
  `<shadow>/Worlds`; sinai.wrp and lebanon80.wrp extracted from lost.pbo and
  LoBo_Leb.pbo to the scratchpad). The 3rd column ("Y") is the stored object
  Y; for the stock OPRW worlds it is a plausible ASL height, for the 4WVR
  LoBo worlds it is NOT reliable (pier Y -5 to -18, a radar dish at 1095),
  so never read water from it there.
- **PROBE**: tri probes `arma_CWR/tmp/agent2-76-probe/probe_<world>.test.sqf`
  (boot the shipped template, `nearestObjects [p, [], 8]`, `typeOf`,
  `format "%1"` = `"<id>: <model>.p3d"`, `getPosASL`, `gmLandmass p`
  (-1 = sea), `gmGeoGrid [p, 1]` (3x3 cells of 50 m, `~` = deep water)).
  Results in the per-world PROBE blocks below.

Object class names: every landscape object the detector cares about is a
plain shape with NO config class: `typeOf` returns `""` for them (see the
PROBE blocks), so name patterns must match the **model path** (shape name),
exactly as `terrain objects` prints it. Model families seen:
- stock harbour: `data3d\molo_beton.p3d` (concrete pier), `data3d\molo_krychle.p3d`
  (quay block), `data3d\molo_drevo.p3d`, `molo_drevo_bs.p3d`, `molo_drevo_end.p3d`
  (wooden jetty), `data3d\majak.p3d` + `podesta_pod_majak.p3d` (lighthouse,
  NOT harbour evidence: the Abel ones stand on cliffs and capes),
  `znacka_leport*.p3d` / `znacka_spierre*.p3d` (town signs, not harbour).
- LoBo harbour (AGS_port): `ags_port\wrp\ags_pier0/1/1p/1h/1b/1s/1c/2/2s/2u.p3d`,
  `ags_port\prefabs\ags_pier*_0N.p3d`, `ags_port\wrp\ags_crane1.p3d`,
  `ags_port\crane1.p3d`, `ags_port\pier_a1.p3d`, `ags_port\blok100*.p3d`
  (breakwater blocks, Beirut), `ags_port\crates*.p3d`, `skc_crate.p3d`,
  `crate*.p3d`, and `o\hous\nabrezi.p3d` / `nabrezi_najezd.p3d` (quay wall /
  slipway, Tyre). `ags_port\wrp\ags_signal_red/green.p3d` are CHANNEL BUOYS
  in pairs across open water all along the gulfs: never harbour evidence.
- military: `data3d\hlaska.p3d` (watchtower), `vez.p3d` (tower: on Cain it is
  everywhere incl. hilltops, weak), `strazni_vez.p3d`, `fortress1/2.p3d`,
  `garaz*.p3d`, `fuelstation_army.p3d`, `hangar.p3d`, `budova1..5.p3d`
  (stock airfield/military buildings, also used at the Eden pier compound),
  `pletivo_wired*.p3d` (wire fence), `baracken\*_baracke0N.p3d`,
  `baracken\*_turm.p3d`, `*_start_turm.p3d` (control tower), `hangar0N.p3d`,
  `rksl-hangers\rksl_*hang*.p3d`, `rksl-hasn2\rksl_hasnato2-*.p3d` and
  `lobo_ob\air\lobo_has1d/2d.p3d` (hardened aircraft shelters),
  `lobo_ob\air\armingbarrier.p3d`, `map_milobj-pack\*`, `f3wx_radiotower.p3d`,
  `o\hous\hlidac_budka.p3d` (guard hut), `lobolebobject\lobo_runway.p3d`.

## Consequences for the implementation (read first)

1. **Lebanon80 would seed an AIRFIELD and probably a SEAPORT**: it has an
   ILS (Ramat David [8628, 2524], Israel, ~7 km south of the theatre) and a
   4-piece `nabrezi` quay at Tyre [8588, 9739..9916], inside the theatre,
   plus big ports at Haifa and Beirut outside it. The template authors no
   AIRFIELD / SEAPORT, so `seedBases` defaults ON. The owner's ruling
   ("no airfield or seaport in theatre by design") needs `playRegion[]` that
   excludes Tyre's waterfront, or `seedBases = 0`, in Guerrilla.Lebanon80.
2. **Sinai's only ILS is the Israeli Airbase** (NE, [12176, 11476]); the
   Wadi airstrip [8130, 3774] and the Egyptian Airbase (Names [3443, 1479],
   hangars [3573, 1583]) have none, so an ILS-only AIRFIELD rule cannot find
   the two the owner named. They need authoring (template zones or a
   SecondaryAirports-style overlay) or an object rule (hangar + control tower
   + HAS clusters, see the model list). Without a playRegion the ILS rule
   seeds the Israeli Airbase, and the harbour rule seeds Suez, Taba/Eilat,
   Aqaba (Jordan), Sharm and El Tor (five SEAPORTs).
3. **"Egyptian Airbase" and "Israeli Airbase" are seeded CITY rows today**
   (also "Aqaba Internation Airport", the fire bases and border labels: C7).
4. **Distance to an ILS anchor is the wrong yardstick**: Abel's authored
   Airfield is 333 m from the anchor but 93 m from the first taxiIn point and
   108 m from the runway segment. Eden's taxiIn point is 406 m from its anchor.
5. **Water**: piers stand in `~` (deep water) cells with origins at or below
   sea level; `gmLandmass` still says 0 at Le Port's pier, so it is not a
   water test. The engine's `nearestObjects` is 3D: the LoBo piers sit 5 to
   18 m below the surface origin-wise, so gather objects by 2D distance.
6. **Name patterns must use the model path**: stock harbour/army shapes have
   no class (`typeOf` ""), a few stock buildings get `House<model>` classes,
   LoBo hangars and towers have real classes (`Hangar_Hall`, `Hangar01`,
   `Brown_Start_Turm`, `RKSL_HASN2_D`, `ViewTower`, `AGS_crane1wrp`,
   `MAP_HangarRes`, `Fortress2`), piers (`ags_pier*`, `nabrezi`, `blok100`)
   have none.
7. **Eden's pier is a harbour by the Le Port rule** (the same molo_beton +
   molo_krychle pair, both in deep water); the tests expect a seeded SEAPORT.
8. Cain has no ILS (origin) and no Names; two molo pairs. Noe is not shipped.
9. **False harbour token (seen in the first runtime run, 16:27)**: the
   strong token `jerab` (crane) matches `data3d\str jerabina.p3d`, a ROWAN
   TREE: 63 on Eden, 3 on Cain. Eden's log shows 21 "harbour-named objects
   ... stand away from the water" clusters, all rowans; the water test saves
   it today, but a rowan by the shore would score 1 per tree. Match `jerab`
   only as a whole word / not followed by `ina`, or drop it (no stock or LoBo
   crane uses it; the cranes are `ags_crane1`, `crane1`).

## Abel (Malden)

### (a) ILS (CFG, CfgWorlds >> Abel)
| key | value |
|---|---|
| ilsPosition[] | {7922, 9590, 0} |
| ilsDirection[] | {0, 0.08, -1} |
| ilsTaxiIn[] | {"7922+69","9590+400", "7922+69","9590+45-18", "7922+61","9590+37-18", "7922+8","9590+37-18", 7922,"9590+45-18", 7922,"9590+100-18"} = [7991,9990] [7991,9617] [7983,9609] [7930,9609] [7922,9617] [7922,9672] |
| ilsTaxiOff[] | {7922,9590, 7922,"9590+878-18", "7922+8","9590+886-18", "7922+61","9590+886-18", "7922+69","9590+878-18", "7922+69","9590+400"} = [7922,9590] [7922,10450] [7930,10458] [7983,10458] [7991,10450] [7991,9990] |
| SecondaryAirports | none |

Runway: the segment x 7922, z 9590..10450 (taxiOff's first two points),
midpoint [7922, 10020]; parallel taxiway at x 7991. First taxiIn point
[7991, 9990] is 93 m from the authored Airfield [8030, 9905]. The ILS anchor
itself is **333 m** from the authored Airfield, so a 300 m "covered" test
against the bare anchor would miss it; against the taxiIn point or the
runway segment (108 m) it is covered.

Objects near the runway (WRP, 700 m): `pletivo_wired.p3d` x456 (perimeter
wire, nearest [7799, 9593]), `budova1.p3d` x6 (hangar row, nearest
[8088, 9922]), `budova3.p3d` [8074, 9984], `budova5.p3d` [7980, 9306],
`plot_provizorni.p3d` x2 [7797, 9835]. No `hangar.p3d` here (the only one on
Abel is at [659, 12081]). Abel's other army cluster: `garaz.p3d`
[7474, 10538], `fuelstation_army.p3d` x2 [7508, 10561..10580], `hlaska.p3d`
[7438, 10532] (900 m NW of the runway).

### (b) harbour objects
Le Port quay (WRP, 400 m of [8410, 3690]): `molo_beton.p3d` [8476, 3754]
Y 2.5, `molo_krychle.p3d` [8472, 3774] Y 2.5. Nothing else harbour-like.
Le Port town (400 m of [8225, 3161]): only the town signs and `majak.p3d`
[8269, 2860] Y 29.4 (lighthouse on the headland). Whole island harbour list:
`molo_beton` 1, `molo_krychle` 1 (both at the Le Port quay), wooden
`molo_drevo` [1459, 4696], `molo_drevo_end` [1438, 4694] (west coast jetty,
score 1 + 0.5?), `molo_drevo_bs` [3544, 3240], [7491, 3602], [8099, 5687],
`molo_drevo_end` [8080, 5677] (single jetties). Lighthouses (`majak`) at
[1106, 471], [5142, 1211], [6744, 11512], [8269, 2860], [8287, 6038],
[9692, 5895], [11024, 2222].
So the Le Port quay is a 2-object cluster (score 2 with the weights in
BaseDetect.hpp); the authored "Le Port Harbour" [8410, 3690] is 92 m from
the pier: covered.

### (c) military cluster at the compound
WRP, 400 m of [9690, 3990]: `garaz_bez_tanku.p3d` x3 [9691..9727, 4002],
`garaz_s_tankem.p3d` [9745, 4002], `garaz.p3d` x2 [9657, 3914..3934],
`hlaska.p3d` x4 [9639, 4006] [9642, 3867] [9792, 3866] [9796, 4007],
`strazni_vez.p3d` [9771, 4006], `plechbud.p3d` x3, `pletivo_wired.p3d` x100,
gates `pletivo_wired_branal/branap.p3d` x3 each, `jezek kov.p3d` x39
(hedgehogs). Further: firing range (`strel_post*.p3d`, `plot_provizorni`,
`barbedwire`) at [9539..9544, 4152..4167]; `fortress1.p3d` [10010, 4029].
Other Abel clusters: `fortress1/2` + `hlaska` at [10477..10526, 4018..4025],
[9540..9594, 3038..3083]; `hlaska` x3 at [11563..11690, 4408..4478].

### PROBE (engine, Guerrilla.Abel)
Probe run 2026-10-14 16:20 (dist/agent2-rwdi at HEAD 8909838, before any
#76 code): `site | object (format %1) | typeOf | getPosASL | gmLandmass | gmGeoGrid centre cell`.
| site | object | typeOf | ASL pos | lm | geo |
|---|---|---|---|---|---|
| molo_beton [8476, 3754] | `8155: molo_beton` | "" | [8476.3, 3754.1, -0.56] | 0 | `~` |
| molo_krychle [8472, 3774] | `8157: molo_krychle` | "" | [8472.0, 3774.2, 0.67] | 0 | `~` |
| molo_drevo_bs [7491, 3602] | `15152: molo_drevo_bs` | "" | z 1.00 | 0 | `R` |
| majak [8269, 2860] | `15144: majak` | `Housemajak` | z 28.4 | 0 | `R` |
| budova1 [8088, 9922] | `35900: budova1` | `Housebudova1` | z 34.2 | 0 | `R` |
| pletivo_wired [7799, 9593] | `86144: pletivo_wired` | "" | z 33.7 | 0 | `6` |
| garaz [9657, 3914] | `664: garaz` | `Housegaraz` | z 41.4 | 2 | `0` |
| garaz_s_tankem [9745, 4002] | `669: garaz_s_tankem` | `Housegaraz_s_tankem` | z 42.0 | 2 | `0` |
| ILS anchor, runway mid, zone centres | (no object within 8 m) | | | 0 (compound 2) | `0`/`R` |
So: the Le Port pier and quay block stand IN deep water (`~` cell, origin at
sea level), the wooden jetty does not register as deep water. The compound
is on landmass 2 (its own island, contract C8), the rest of Abel on 0.
**`gmLandmass` reports 0 at the pier although the cell is deep water**: it is
not a water test; use the geography cell (`waterDepth >= 2`) or the height.
Zones at boot: Camp, Village, Outpost (EAST 6), Southeast Compound (OUTPOST,
EAST 6), Le Port Harbour (SEAPORT, EAST 8), Airfield (AIRFIELD, EAST 12), 13
seeded CITY (garrison 4). Log: `OccupierForces: active, poolStart 60 (R 32,
K 14, G 56), roster 4 zone(s)`; `gmOccupierForces` select 7 (K) = 14.

## Eden (Everon)

### (a) ILS (CFG, CfgWorlds >> Eden)
| key | value |
|---|---|
| ilsPosition[] | {4772, 10923} (2 values) |
| ilsDirection[] | {0, 0.08, -1} |
| ilsTaxiIn[] | {"4772+69","10923+400", "4772+69","10923+95", "4772+61","10923+87", "4772+8","10923+87", 4772,"10923+95", 4772,"10923+150"} = [4841,11323] [4841,11018] [4833,11010] [4780,11010] [4772,11018] [4772,11073] |
| ilsTaxiOff[] | {4772,10923, 4772,"10923+878", "4772+8","10923+886", "4772+61","10923+886", "4772+69","10923+878", "4772+69","10923+400"} = [4772,10923] [4772,11801] [4780,11809] [4833,11809] [4841,11801] [4841,11323] |
| SecondaryAirports | none |

Runway: x 4772, z 10923..11801, midpoint [4772, 11362]; first taxiIn point
[4841, 11323]. **Conflict to watch**: the explicit CITY "Saint Phillippe"
[4674, 10730] is 216 m from the ILS anchor (616 m from the taxiIn point,
637 m from the runway midpoint). A base centred on the anchor is outside
coverAnyDist (150 m) of the town but inside any 300 m town dedup; centring
on the taxiIn point (as BaseDetect.hpp says) clears both. Note the taxiIn
point is 406 m from the anchor, so an assert "within 300 m of [4772, 10923]"
is the wrong ground truth: the tests measure distance to the RUNWAY SEGMENT
(anchor to the taxiOff end), not to the anchor.

Objects (WRP): `znacka_letiste.p3d` (airport sign) [5007, 11048]; airfield
buildings west of the runway: `budova5.p3d` x2 [4477, 11203],
`budova1.p3d` x3 [4457, 11233], `podprapornik.p3d` (flagpole)
[4497, 11201], `pletivo.p3d` x107 fence from [4495, 11200]. A walled army
base SE of the town: `hlaska.p3d` x2 [5101, 10499] [5154, 10446], `zed2.p3d`
x33, `budova2/3/4.p3d` [5122..5158, 10469..10517], `vrata.p3d` gates
[5111, 10484]. No `hangar.p3d` on Eden.

### (b) the pier near [7392, 6835]
WRP, 400 m: `molo_beton.p3d` [7392, 6835] Y 1.5, `molo_krychle.p3d`
[7381, 6828] Y 1.5, then a walled compound 100 m inland: `vez.p3d` x2
[7299, 6798] [7299, 6755], `zed2.p3d` x40 from [7299, 6795], `budova1.p3d`
x3 from [7280, 6785], `budova2.p3d` [7281, 6766]. **Verdict: the same
2-object concrete pier + quay block composition as Abel's authored Le Port
Harbour**, so a rule that accepts Le Port must accept this one: SEAPORT. (It
also has a small walled military post beside it.) The only other harbour
object on Eden is a lone wooden jetty `molo_drevo_bs.p3d` [10759, 8509]
(score 0.5, not a harbour). Lighthouses: [6358, 9637], [10312, 320].

### (c) the three generated outposts
WRP, 400 m, non-vegetation objects: Outpost 1 [7535, 8760]: only olive trees;
Outpost 2 [11439, 1557]: a chapel (`kaple.p3d`) at 340 m; Outpost 3
[9267, 4432]: nothing at all. None stands on a military object cluster.
Eden's real military sites: the airfield buildings and the walled base
above, the pier compound, and `hlaska.p3d` [7540, 4225] (lone tower).
Authored roster today: 3 OUTPOSTs x garrison 6 = R 18.

### PROBE (engine, Guerrilla.Eden)
Probe run 2026-10-14 (same build, no #76 code):
| site | object | typeOf | ASL pos | lm | geo |
|---|---|---|---|---|---|
| molo_beton [7392, 6835] | `28128: molo_beton` | "" | [7392.0, 6834.8, -2.51] | 0 | `~` |
| molo_krychle [7381, 6828] | `28129: molo_krychle` | "" | [7381.4, 6827.7, 1.54] | 0 | `~` |
| budova1 (pier compound) [7280, 6785] | `28069: budova1` | `Housebudova1` | z 32.4 | 0 | `0` |
| znacka_letiste [5007, 11048] | `6430: znacka_letiste` | "" | z 34.6 | 0 | `R` |
| budova5 (airfield) [4477, 11203] | `43: budova5` | `Housebudova5` | z 19.3 | 0 | `R` |
| budova1 (airfield) [4457, 11233] | `42: budova1` | `Housebudova1` | z 19.0 | 0 | `R` |
| budova2 (army base) [5122, 10517] | `5638: budova2` | `Housebudova2` | z 32.5 | 0 | `0` |
| ILS anchor, runway mid, Outposts 1..3 | (none within 8 m) | | | 0 | `0`, `0`, `2`, `4`, `5` |
The Eden pier stands in deep water exactly like Le Port's. Zones at boot:
Camp, Outpost 1/2/3 (OUTPOST, EAST 6 each), 18 CITY (Saint Phillippe ...
Saint Pierre, garrison 4). Log: `poolStart 54 (R 18, K 18, G 72), roster 3
zone(s)`.

## Sinai (@LoBo lost.pbo, world class `sinai`)

### (a) ILS (CFG, lost.pbo config.cpp, class sinai : intro)
| key | value |
|---|---|
| ilsPosition[] | {12176, 11476, 0} |
| ilsDirection[] | {0, 0.08, -1} |
| ilsTaxiIn[] | {} |
| ilsTaxiOff[] | {} |
| SecondaryAirports | none |

**The only ILS on Sinai is the Israeli Airbase in the north-east** (Names
"Israeli Airbase" [11799, 11607]; RKSL hangars `rksl_tmphanger_des.p3d` x7
from [11928, 11245], `grey_baracke*`, `grey_start_turm.p3d` [11581, 11545],
`fuelstation_army.p3d` x2 [11659, 11759]). Neither the Wadi airstrip nor the
Egyptian Airbase has an ILS entry: **an ILS-only AIRFIELD rule cannot find
either**. With empty taxi arrays the runway footprint is unknown (no
taxiOff), so only the anchor is available.

Wadi airstrip (the fenced compound x 7687..8215, z 3167..4190 named in the
Guerrilla.Sinai header; WRP cluster centroid [8130, 3774]):
`data3d\hangar.p3d` x2 [8079, 3767], `baracken\brown_start_turm.p3d`
(control tower) [8045, 3732], `rksl-hasn2\rksl_hasnato2-d.p3d` (HAS)
[8067, 3598], `brown_baracke01/02/03` [8048..8081, 3891],
`radioantenna_l.p3d` [8033, 3892], gate post `brown_wachhaus`/`brown_turm`/
`fortress2`/sandbags at [8207..8218, 3747..3768], fence
`lobobloc\plot_provizorni_25.p3d` x120. Authored "Wadi Checkpoint" OUTPOST
[8300, 3500] is ~330 m from the hangars.

Egyptian Airbase (Names "Egyptian Airbase" [3443, 1479], type-less, so the
template's seedCities seeds it as a CITY today): WRP cluster spans
x 2399..3611, z 812..2368: `baracken\hangar01.p3d` x5 (nearest [3573, 1583]),
`hangar03.p3d` x6, `brown_turm.p3d` x15, perimeter `pletivo_wired_25.p3d`
x108 and `ofp_zaun09.p3d`, gates `ofp_zauntorl/r1` [3610, 1258..1264],
`rksl_medhang_des.p3d` x3 [2810, 1328], `rksl_tmphanger_des.p3d`
[2662, 1504], barracks/repair/fuel at [2799..2839, 1479..1694], a guard post
`brown_wachhaus` + `zavora` (barrier) at [3606..3611, 1955..1970].

Other airfield-like clusters (WRP, link 200 m): [10740, 8636] (Aqaba side:
`hangar01` x2, `hangar02`, `hangar03` x2, RKSL), [11796, 12347]
(`hangar01` x4, `hangar_2` x2), [6907, 9127], [2387, 12112], [4145, 12161]
(each with a `*_start_turm`), [9864, 2680] (`grey_turm` x6,
`grey_start_turm`).

### (b) harbours (WRP clusters of pier/crane/crate models, link 150 m)
| cluster | centroid | members | nearest Names |
|---|---|---|---|
| Suez | [1402, 10429] | 83: ags_pier* x~50, ags_crane1 x6, crates | As-Suways [1891, 11100] |
| **Sharm** | [10729, 3488] (bbox x 10463..11013, z 3244..3627) | 58: ags_pier0 x19, ags_pier1/1p/1h/1b/1s/2/2s/2u, prefabs, ags_crane1 x3 [10587, 3284] [10587, 3382] [10946, 3417], crates | Sharm el Sheikh [9284, 3763] is **1.47 km** W |
| Taba/Eilat | [10287, 10075] | 39: ags_pier* , ags_crane1 x4 | Taba [10028, 9913] |
| Aqaba | [10654, 9527] | 15: ags_pier*, ags_crane1 x2 | Aqaba [11264, 9273] (Jordan) |
| **El Tor** | [5923, 3202] (bbox x 5834..5984, z 3144..3257) | 10: ags_crane1 x2 [5908, 3158] [5897, 3257], ags_pier1 / 1h / 1p / 2 / 2s x2 / 2u x2 | El Tor [6827, 2808] is 985 m E |
| crates only | [10928, 10492] | 10 crates, no pier | Eilot |
Sharm port has a shore strip behind it: `hangar_2.p3d` [10708, 3614],
`ags_inds\whouse1/2.p3d`, `grey_baracke*` x6 [10626..10682, 3601..3621],
`garaz.p3d` x2 [10574, 3608], oil tanks [10425, 3568].
`ags_signal_red/green` pairs (buoys) run down both gulfs: not harbours.

### (c) military clusters
El Tor garrison (WRP cluster [6098, 3318], bbox x 5818..6351,
z 3042..3717, 550..700 m NW of the El Tor Names point): `hlaska.p3d` x3
[6333, 3055], `fortress1.p3d` x3 [6351, 3042], `lobobloc\zaun5.p3d` x18,
`brown_wachhaus` [6335, 3113], `blech_torl/r` gate [6340, 3099..3106],
`brown_baracke01/03` x13, `dach2` x4, `rksl_medhangl_des.p3d` [6305, 3180],
`fuelstation_army` x2 [6286, 3231], `radioantenna_l` [6117, 3097]. It
reaches the harbour cluster at its west edge (x 5818).
A large base 600 m E of the Camp: cluster [8964, 2767], bbox x 8754..9204,
z 2450..3213: `brown_baracke01` x12, `brown_turm` x9, `brown_baracke03` x8,
`f3wx_radiotower`, `radioantenna_l/s`, `rksl_medhangl_des`, fuel, repair.
Border lines of `hlaska.p3d`: x44 along z 9553..9904 ([11633, 9775]) and
x48 along z 7594..7725 ([11503, 7610]).

### PROBE (engine, Guerrilla.Sinai)
Probe run 2026-10-14 (same build, no #76 code). LoBo models DO have config
classes (typeOf non-empty) unlike the stock shapes:
| site | object | typeOf | ASL pos | lm | geo |
|---|---|---|---|---|---|
| IsrAB rksl_tmphanger_des [11928, 11245] | `28318: rksl_tmphanger_des` | `RKSL_TMPHANG_DES` | z 33.9 | 0 | `0` |
| Wadi hangar [8079, 3767] | `5631: hangar` | `Hangar_Hall` | z 99.4 | 0 | `R` |
| Wadi brown_start_turm [8045, 3732] | `5522: brown_start_turm` | `Brown_Start_Turm` | z 98.3 | 0 | `0` |
| Wadi HAS [8067, 3598] | `5168: rksl_hasnato2-d` | `RKSL_HASN2_D` | z 100.3 | 0 | `#` |
| EgAB hangar01 [3573, 1583] | `1752: hangar01` | `Hangar01` | z 43.1 | 0 | `0` |
| EgAB brown_turm [3606, 1441] | `1552: brown_turm` | `Brown_Turm` | z 39.3 | 0 | `0` |
| El Tor ags_crane1 [5908, 3158] | `4227: ags_crane1` | `AGS_crane1wrp` | z 9.4 | 0 | `~` |
| El Tor ags_pier2 [5934, 3244] | `4403: ags_pier2` | "" | z -5.0 | 0 | `~` |
| El Tor ags_pier1p [5984, 3194] | `4228: ags_pier1p` | "" | z -5.0 | (probe 2) | `~` |
| El Tor garrison hlaska [6333, 3055] | `4017: hlaska` | `ViewTower` | z 18.7 | 0 | `#` |
| El Tor garrison fortress1 [6351, 3042] | `3943: fortress1` | "" | z 15.2 | 0 | `3` |
| Sharm ags_crane1 [10587, 3382] | `4701: ags_crane1` | `AGS_crane1wrp` | z 2.8 | **-1** | `~` |
| Sharm ags_pier0 [10713, 3494] | `4933: ags_pier0` | "" | z -12.2 | 0 | `~` |
| Sharm ags_pier1p [10813, 3444] | `4811: ags_pier1p` | "" | z -11.7 | -1 | `~` |
| Sharm hangar_2 [10708, 3614] | `5363: hangar_2` | `MAP_HangarRes` | z 11.7 | 0 | `0` |
| Sharm el Sheikh Names [9284, 3763] | (none) | | | 0 | `~` |
**The Sharm el Sheikh Names point is itself in deep water** (`~`), so the
seeded "Sharm el Sheikh" CITY sits in the sea today (a C6 defect, and the
probe-out list below confirms the settlement probe rejects it).
**3D trap**: `nearestObjects [[x, z, 0], [], 8]` missed the piers (origin
12 m below the surface): the engine's object search measures 3D distance, so
a detector that gathers candidates by radius must use 2D (x, z) distance.
Zones at boot: Camp (EAST), Wadi Checkpoint (OUTPOST, WEST 6), Ras Nasrani
Outpost (OUTPOST, WEST 8), El Tor (CITY), 36 seeded CITY rows incl.
"Israeli Airbase" [11799, 11607] and **"Egyptian Airbase" [3443, 1479]
(CITY, NEUTRAL, garrison 4)**, "Aqaba Internation Airport", "Israeli Fire
Base", "Egyptian Fire Base", the border labels, "Tiran Is", "Mt.Sinai".
Log: `poolStart 66 (R 14, K 26, G 104), roster 2 zone(s), 11 town(s) not
counted (11 probed out)`; excluded: Eilat, Coloured Canyon, Egypt-Israel
Border, Taba, Ras Gharib, Dahab, Tiran Is, Mt.Sinai, Eilot, Sharm el
Sheikh, As-Suways.

## Lebanon80 (@LoBo LoBo_Leb.pbo, world class `Lebanon80`)

### (a) ILS (CFG, LoBo_Leb.pbo config.cpp, class Lebanon80 : intro)
| key | value |
|---|---|
| ilsPosition[] | {8628, 2524, 0} |
| ilsDirection[] | {0, 0.08, -1} |
| ilsTaxiIn[] | {} |
| ilsTaxiOff[] | {} |
| SecondaryAirports | none |

**Lebanon80 HAS an ILS**: Ramat David Airbase (Names "Ramat David Airbase"
[8117, 2889], Israel), objects: `rksl_hasnato2-w.p3d` x2 [8449, 2426..2476],
`lobo_ob\air\lobo_has1d.p3d` x13 [9073..9410, 2489..2662],
`lobo_has2d.p3d` x2, `armingbarrier.p3d` x4, RKSL hangars at
[8003, 2875..3065], [8830..8850, 2325], [9074..9326, 2280..2358], fuel
tanks [8181..8186, 2664..2682]. It lies ~7 km south of the template's
theatre (zones span x 11500..14071, z 9800..13000; Tyre CITY [9300, 9800]).
Beirut International (Names [11669, 15532]; `baracken\hangar01.p3d` x4 at
[11725..11874, 16080]) has no ILS. Two `lobolebobject\lobo_runway.p3d` at
[17647, 16191] and [21145, 20375] (far NE, Syria) have no ILS either.
**Consequence: with no playRegion[] and seedBases on, the ILS rule seeds
Ramat David as an occupier AIRFIELD on Lebanon80**, contradicting the owner's
"no airfield or seaport in theatre by design". The template needs
`playRegion[]` (or `seedBases = 0`).

### (b) harbours (WRP, link 150 m)
| cluster | centroid | members | note |
|---|---|---|---|
| Haifa A | [6120, 5415] | 148: ags_pier0 x70, ags_crane1 x14, ... | Israel, outside theatre |
| Haifa B | [6996, 5716] | 76: ags_pier0 x32, ags_crane1 x6, ... | Israel, outside theatre |
| Beirut | [12913, 18076] | 50: `ags_port\blok100.p3d` x17, `blok100_k.p3d` x20, `crane1.p3d` [12871, 18001], `pier_a1.p3d` [13019, 17964], crates | north, outside theatre |
| **Tyre** | [8588, 9828] | 4: `o\hous\nabrezi.p3d` [8588, 9739] [8588, 9857], `nabrezi_najezd.p3d` [8588, 9798] [8588, 9916] | 470 m W of the Tyre Names point, ~710 m W of the Tyre CITY zone [9300, 9800] |
If `nabrezi*` counts as a quay at weight 1, **Tyre scores 4 and would be
seeded as a SEAPORT inside the theatre**. Owner ruling: no seaport in
theatre. Either exclude nabrezi (it is a quay wall, also used on inland
rivers in stock data) or bound the template with playRegion and keep Tyre
out, or set seedBases = 0 on Lebanon80.

### (c) military clusters
Tyre compound (WRP, 200 m S of the Tyre Names point): `fortress1.p3d` x2
[9027, 9681] [9043, 9637], `fortress2.p3d` x3 [9039, 9657] [9056, 9690]
[9094, 9657], `f3wx_radiotower.p3d` [9066, 9664], `radioantenna_s.p3d` x2
[9049, 9671], `pytle.p3d` x13 / `Pytle_BSL.p3d` x3 (sandbags),
`barbedwire.p3d` x8, `Pristresek*.p3d` (shelters) [9049..9078, 9675..9686],
`chd_barrier1/5.p3d` x22. Centre about [9055, 9670].
Mt Dov (Names [15416, 10684]): nothing military within 600 m (conifers
only). The nearest post is at Shaaba Farms: `fortress1.p3d` [14348, 10599],
`fortress2.p3d` x2 [14353, 10605] [14382, 10605], `dhangar_radio.p3d`
[14155, 10768], `dhangar_psacistul.p3d` [14156, 10761], `lobo_belltower`
[14135, 10812] (~1.1 km W of the Mt Dov point).
Border posts: `o\hous\hlidac_budka.p3d` x6 at [8481..8543, 8421..8522] with
`rksl_tmphanger.p3d` [8385, 8456] (coastal border crossing, x 8.4 km,
z 8.4 km); Haifa harbour guard huts x5 [5863..6913, 5119..5393].
`f3wx_radiotower.p3d` pairs at [11812..11842, 8160..8173] and
[16353..16599, 10848..11050] (Hermon heights).

### PROBE (engine, Guerrilla.Lebanon80)
Probe run 2026-10-14 (same build, no #76 code):
| site | object | typeOf | ASL pos | lm | geo |
|---|---|---|---|---|---|
| Ramat David HAS [8449, 2476] | `3689: rksl_hasnato2-w` | `RKSL_HASN2_W` | z 45.3 | 0 | `#` |
| Tyre nabrezi [8588, 9739] | `34677: nabrezi` | "" | [8588.4, 9738.7, -18.1] | 0 | `R` |
| Tyre nabrezi_najezd [8588, 9798] | `35146: nabrezi_najezd` | "" | z -18.4 | | |
| Tyre nabrezi [8588, 9857] | `36032: nabrezi` | "" | z -18.1 | | |
| Tyre fortress2 [9056, 9690] | `34308: fortress2` | `Fortress2` | z 9.0 | 0 | `R` |
| Tyre fortress1 [9027, 9681] | `34265: fortress1` | "" | z 9.5 | 0 | `R` |
| Haifa ags_pier0 [6126, 5433] / [6126, 5383] | `10634` / `10505: ags_pier0` | not probed (the `<ptr>#` prefix in format means a config-class entity) | z -15 | -1 | `~` |
| Beirut crane1 [12871, 18001] | `78088: crane1` | not probed (class entity) | z 19.2 | 0 | `#` |
| Beirut blok100 [12726, 18073] | `78232: blok100` | "" | z 0.7 | 0 | `~` |
| Beirut pier_a1 [13019, 17964] | `77900: pier_a1` | "" | z 4.95 | | |
| Mt Dov Names [15416, 10684] | (none) | | | 0 | `3` |
| Shaaba fortress2 [14353, 10605] | `40905: fortress2` | `Fortress2` | z 253.0 | 0 | `R` |
| Shaaba dhangar_radio [14155, 10768] | `41758: dhangar_radio` | "" | z 270.2 | 0 | `R` |
Tyre's quay: the nabrezi pieces stand on the waterline with a quay road
(`kos25`, `kos10 25`, `sil6konec` at x 8600..8630) and town walls behind
(`helfenburk_budova2`, `lobo_arch*`, `lobo_wall_0N`). By objects it IS a
small harbour; by the owner's ruling it must not be seeded.
Zones at boot: Camp (EAST), Litani Checkpoint (OUTPOST, WEST 6), Marjayoun
Barracks (OUTPOST, WEST 8), Tyre / Saida / Ghajar (CITY). Log: `poolStart 24
(R 14, K 3, G 12), roster 2 zone(s)` (24 = forcesPoolMin).

## Cain (Kolgujev) and Noe

Cain (CFG): `ilsPosition[] = {0, 0, 0}`, taxi arrays empty, **Names class
empty** (no towns). Origin ILS = rejected. Harbour objects (WRP):
`molo_beton.p3d` + `molo_krychle.p3d` at [9597, 8588] / [9609, 8604]
(2 objects) and `molo_beton` x2 + `molo_krychle` at [11875, 10575]
[11907, 10551] [11889, 10561] (3 objects): two seaports by the Le Port rule.
Military: `vez.p3d` x38 (towers all over, many on hilltops above 300 m),
`garaz_bez_tanku.p3d` x14 (rows at [7388..7442, 4715], [7423..7477, 4914..4917]
with `vez` x6 around [7319..7602, 4700..4925]: a fenced base), `hlaska.p3d` x5
in a line [2407..2723, 6250..7597].
Intro: ILS {0, 0, 0}. Noe (Resistance's island): **not shipped** on the
Remaster shadow (`Worlds\` holds abel, cain, eden, intro; CfgWorldList is
Eden, Abel, Cain, Demo, Intro). No Guerrilla template exists for Cain.

## Runtime check of the implementation (agent2 build at 16:26, working tree)

`guerrilla_bases_detect_{abel,eden,lebanon80}` and the Eden save `.seq`
pass. Logged seeds:
- Abel: `Malden Airfield` [7991, 9990] covered by Airfield; `Le Port
  Harbour` [8446, 3736] covered; R 32, poolStart 60 (unchanged).
- Eden: `Everon Airfield` [4841, 11323] (garrison 12, conf 0.90), `Chotain
  Harbour` [7358, 6803] (garrison 8, conf 0.70); R 38, poolStart 74 (was 54).
- Lebanon80 (`seedBases = 0`): 7 candidates, nothing seeded; `Tyre Harbour`
  [8617, 9856] reported low-confidence.
- Sinai: 7 AIRFIELDs + 5 SEAPORTs seeded, R 138, **poolStart 182 (was 66)**:
  Israeli Airbase [12176, 11906], Egyptian Airbase Airfield [2503, 1538],
  Sharm el Sheikh Airfield [8068, 3725] (= the Wadi airstrip), Eilat Airfield
  [10980, 10425], Egyptian Airbase [3573, 1652], Egyptian Fire Base Airfield
  [7006, 9194], Aqaba Internation Airport [10894, 8176]; As-Suways Harbour
  [1338, 10385], El Tor Harbour [5997, 3233], Taba Harbour [10322, 10113],
  Israel-Jordan Border Harbour [10696, 9548] (= Aqaba port), Tiran Is
  Harbour [10727, 3550] (= the Sharm port).

Defects against this ground truth:
1. **The Egyptian Airbase is seeded TWICE** ([2503, 1538] and [3573, 1652],
   1.07 km apart). It is one fenced base (WRP cluster n=97 over
   x 2399..3611, z 812..2368, `ofp_zaun09` fence x31 joining the west RKSL
   hangar group and the east `hangar01` row). `guerrilla_bases_detect_sinai`
   s2 is red on this: "no second AIRFIELD within 1300 m of [3005, 1590]".
   Merge airfield clusters that share a perimeter, or link at ~1.2 km.
2. Naming: the Sharm port is named after "Tiran Is" (the island, 1.15 km)
   and the Wadi airstrip after Sharm el Sheikh; the Aqaba port after a
   border label. Prefer Names entries that pass the settlement probe or are
   not labels; the owner calls them the Sharm harbour and the Wadi airstrip.
3. Sinai's pool almost triples (12 bases, R 14 -> 138). Owner asked for the
   Wadi airstrip, the Egyptian Airbase, Sharm and El Tor (4 bases); the
   other 8 (Israeli Airbase, Eilat, Egyptian Fire Base, Aqaba airport,
   Suez, Taba, Aqaba port, plus the duplicate) are beyond the ask and need
   an owner decision or a playRegion.

Re-run on the 16:45 working tree (agent2 rebuild): Sinai now seeds 8 (5
AIRFIELD, 3 SEAPORT; R 98, poolStart 124): Aqaba Internation Airport,
Egyptian Airbase Airfield [2503, 1538], Egyptian Airbase [3573, 1652],
Egyptian Fire Base Airfield, Sharm el Sheikh Airfield [8068, 3725]; El Tor
Harbour, Israel-Jordan Border Harbour, Tiran Is Harbour. The Egyptian
Airbase duplicate (defect 1) is still there; s2 stays red on it.
