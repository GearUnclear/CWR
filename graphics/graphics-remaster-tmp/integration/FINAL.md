# graphics-remaster: final check after the parallel lanes

Branch `graphics-remaster` at **fec1075** (equal to `origin/graphics-remaster`, working tree
clean), worktree `D:\Arma_CWA_worktrees\graphics-remaster`, build `build/graphics-remaster-rwdi`,
game `dist/graphics-remaster-rwdi` (PoseidonGame.exe 20:01:41, after the tip commit at 20:00:53).
Checked 2026-10-18, 20:33 to 22:30. Data junction `packages\RemasterUD-gr-final`. All evidence is
under `tmp/` (gitignored): `tmp/integration/FINAL/` and `tmp/captures/final*`.

An earlier final pass on the same build (20:00 to 20:18) was cut off before its timing run. Its
smoke, capture-set, feature-pair and all-off runs were on this exact build, so they are reused
here; I re-read their logs and images and re-ran the comparisons myself. The unit suites, the
whole timing set, a per-step cost run and a build bisect were run fresh in this pass.

**Regressions fixed: none, no commit made.** The one real regression found (the long views are
slower than A0, section 4.3) is spread over many steps, with no single defect to undo in a
minimal commit; it is the first open risk below.

## 1. Gate results (plan section 8)

| Gate item | Result | Evidence |
|---|---|---|
| 1 Build | the tip's incremental build, exit 0 | `FINAL/build.log` |
| 3 Unit, full suites from `D:\` (re-run 20:35) | PoseidonTests 3,341 cases: 3,331 passed, 6 failed, 4 skipped (7 of 4,056,661 assertions). The 6 are the known environment set: 5 `[ui]` (STRINGTABLE_MAP legend keys, STRINGTABLE_EQUIPMENT M16 keys, three `equipment.utf8.html` cases) and `MP version rejection message follows Czech language selection`. PoseidonCoreTests 827 cases, 823 passed, 4 skipped. PoseidonFoundationTests 475 cases, all passed | `FINAL/unit-*-r2.log` |
| 4 Smoke, three islands | `passed 3/3` (smoke_abel, smoke_eden, smoke_sinai_lobo). KHR_debug ids 131154, 131169, 131218 only (the A0 known set); no shader compile or link error, no ERROR line. The capture-set and feature-pair games show the same three ids only | `tmp/captures/final-smoke` |
| 5 Off equals baseline, whole branch | `baseline` (all 26 switches off) on the final build against A1's `remastered` at the 16 hero views: 11 views within 0 to 2 levels with 0 pixels over 2 levels. `airhaze_valley_0515`, `shade_lowsun_fp`, `ssr_bay_offshore`, `night_road_flat_drive_moonless` and `clouds_coast_oc_up` differ in a few hundred to 9,043 pixels, the same kind and size as A1's own boot-to-boot repeat (A1 runs 012542 against 015544: 531, 1,009, 6,724, 13 and 0). The fire, smoke and rain views (`combat_ffh`, `lighttiles_town_night`, `materials_town_night_rain`) differ by 197k to 556k pixels against A1 repeats of 81k to 349k; the difference image of the rain view is the smoke plume and the rain streaks only | `FINAL/baseline-vs-A1.md`, `tmp/captures/final-baseline` |
| 6 Capture set, everything on | 3 islands, 30 shots, 15 pairs. `firefight_1..3` and `sinai_lowsun_fp` report UNSETTLED:300 (as at A0) | `tmp/captures/final` |
| 6 Feature pairs, off and on | 16 hero views, 32 shots | `tmp/captures/final-features` |
| 7 Timing | section 4 | `FINAL/pool-*.md`, `FINAL/table-vs-a0.md`, `FINAL/bisect.md` |
| 8 Look check | section 5 | |

## 2. Per-feature status

All 26 switches report `landed=26/26` in every shot's feature log, and every step's switch was
shot off and on at its hero view in one game (`final-features`). Cost is "on minus that step's
keys off", GPU frame median of 2 interleaved rounds on a quiet machine, everything else on
(`final-cost-timing`, section 4.2); "n/a" where the view gates the feature off.

| Step | Switches | Gate at noon_fields | Hero pair (final-features) | Cost at valley 05:15 | Cost at sunrise_ridge |
|---|---|---|---|---|---|
| A4 perf | drawGrouping, hillCull | on | sunrise_ridge: draws 1,912 against 12,910, image unchanged (0 pixels over 2 levels off/baseline) | saves 13.6 ms | saves 17.3 ms |
| A5 temporal | taa | on | forest_edge_golden: stills only, motion not judged here | +3.1 ms (CPU +2.5) | +3.0 ms |
| A6 light list | lightTiles | off (no local light) | town_night: lamps and fire lit; night darker overall (B3) | n/a | n/a |
| A7 atmosphere | atmosphere | on | coastline_sunset: aureole, gold band by the sun, anti-sun band warm grey | +2.4 ms (CPU +2.3) | within noise |
| B1 shadows | treeShadows, softShadows | on | woods_pine2_evening: frame 2% brighter than all-off, not darker | +0.6 ms | within noise |
| B2 shade | lowSunRig, crownCanopy | on | lowsun_fp: little visible change against its off | +0.7 ms | within noise |
| B3 night | nightLights, moonShadows, stars | off (day) | road_flat_drive_moonless: stars, wide low-beam pool, hard top edge; beam white, not warm | n/a | n/a |
| B4 combat | combatLights, combatShadows | off | ffh: long radial shadows from the fire, men lit | n/a | n/a |
| B5 water | seaWaves | on (per sea fragment) | coastline_sunset: wave rows, broken sun path | +1.3 ms | within noise |
| B6 ssr | ssr | off (dry, no water) | bay_offshore: boat mirrored with ripples; hill mirror olive-khaki | +0.8 ms | n/a (gated) |
| B7 materials | wetWeather, lampStreaks | off (dry) | town_night_rain: lamp streaks, puddles; no white sheets seen | n/a | n/a |
| B8 air | airBeams, airHaze, valleyMist | haze on, beams and mist off | pinewood_morn: beams through crowns; valley 05:15: pink valley mist | +1.0 ms | within noise (beams off read 1.1 ms slower: the legacy shafts come back) |
| B9 hfrt | canopyShadows | off (sun above 25 deg) | hill_1730: long tree streaks on the slope | +1.2 ms | within noise |
| B10 clouds | litClouds, skyCloudShadows | off (clear sky) | coast_oc_up: peach-lit undersides | n/a (clear) | n/a |
| B11 texsharp | terrainDetail, sharpen | terrain detail on, sharpen off (TAA) | forest_edge: grain on the slope | +0.5 ms | within noise |

15 of 26 gates read off at noon_fields and 14 at sea_noon (the sea turns ssr on), so the night,
wet, combat, beam, mist, canopy, cloud, light-list and sharpen work is not paid at noon.

## 3. Timing method

Every run: 1920x1080 windowed, render scale 1.0, 4x MSAA, `triGpuTimes` over the 90 frames before
each shot, RTX 4060 Ti, GPU clock and foreign games logged every 2 to 3 s by `gpu_sampler.ps1`.
Other sessions (look-surface, look-fog, look-director) started games through the whole evening,
so a fully quiet run was not possible: 105 of 196 and 117 of 199 sampler rows of the first two
Abel attempts saw a foreign game. Instead `FINAL/pool_quiet.py` keeps only the shots with **no
foreign game in the 10 s before the shot** and pools them over the attempts (3 Abel attempts,
one each of Eden and Sinai). Each timed run also started only after 45 to 60 s with no tri slot
held and no PoseidonGame running. Clean shots per view: 3 to 6 (column "clean").

## 4. Timing

### 4.1 Everything on against A0 (plan 5 A0)

GPU frame in ms, median / minimum over the clean shots; CPU frame median.

| view | A0 frame | final frame | change (min) | final world | final post | A0 CPU | final CPU | A0 draws | final draws | clean | final MHz (P) |
|---|---|---|---|---|---|---|---|---|---|---|---|
| sunrise_ridge | 17.41 / 17.34 | 10.16 / 9.82 | -7.52 | 8.20 | 1.65 | 26.57 | 15.86 | 12,910 | 1,912 | 5 of 9 | 2,625 (P0) |
| forest_edge_golden | 6.18 / 5.58 | 6.34 / 6.26 | +0.68 | 4.95 | 1.05 | 8.71 | 10.04 | 2,141 | 866 | 5 of 9 | 2,640 (P0) |
| town_night | 6.53 / 5.42 | 10.02 / 9.52 | +4.10 | 8.75 | 0.97 | 8.15 | 12.97 | 1,228 | 1,211 | 5 of 9 | 2,790 (P0) |
| coastline_sunset | 3.81 / 1.88 | 5.97 / 5.86 | +3.98 | 2.97 | 2.67 | 4.10 | 5.48 | 178 | 177 | 6 of 9 | 1,410 (P0/P3) |
| woods_pine2_evening | 6.47 / 6.45 | 9.64 / 9.54 | +3.09 | 7.79 | 1.61 | 9.25 | 12.53 | 2,840 | 1,421 | 6 of 9 | 2,775 (P0) |
| lowsun_fp | 5.59 / 4.60 | 7.71 / 7.59 | +2.99 | 5.99 | 1.46 | 8.30 | 10.73 | 1,891 | 1,212 | 6 of 9 | 2,782 (P0) |
| road_flat_drive_moonless | 5.27 / 4.52 | 8.35 / 8.28 | +3.76 | 6.88 | 1.32 | 7.69 | 11.06 | 1,806 | 1,411 | 5 of 9 | 2,790 (P0) |
| ffh (static stand-in) | 6.06 / 5.66 | 6.39 / 6.24 | +0.58 | 5.13 | 1.01 | 7.73 | 7.76 | 796 | 770 | 6 of 9 | 2,700 (P0) |
| bay_offshore | 7.73 / 7.72 | 9.59 / 9.49 | +1.77 | 7.86 | 1.50 | 12.44 | 13.62 | 4,557 | 1,794 | 6 of 9 | 2,786 (P0) |
| town_night_rain | 5.23 / 5.13 | 11.08 / 10.48 | +5.35 | 9.30 | 1.47 | 7.10 | 13.20 | 809 | 937 | 5 of 9 | 2,775 (P0/P3) |
| pinewood_morn | 5.84 / 5.46 | 6.27 / 6.21 | +0.75 | 4.18 | 1.69 | 8.58 | 8.62 | 2,844 | 381 | 5 of 9 | 2,092 (P0) |
| valley 05:15 | 23.30 / 22.47 | 31.63 / 26.55 | +4.08 | 29.21 | 2.36 | 31.02 | 34.21 | 11,404 | 5,242 | 5 of 9 | 2,775 (P0) |
| hill_1730 | 1.99 / 1.58 | 4.08 / 3.13 | +1.55 | 2.69 | 1.19 | 3.51 | 3.37 | 1,663 | 692 | 4 of 9 | 1,376 (P0/P3) |
| coast_oc_up | 3.87 / 1.73 | 6.02 / 5.92 | +4.19 | 2.30 | 2.63 | 3.61 | 5.84 | 186 | 186 | 3 of 9 | 1,560 (P0/P3) |
| sea_noon | 4.56 / 1.20 | 5.00 / 3.71 | +2.51 | 2.97 | 1.79 | 5.41 | 5.25 | 180 | 179 | 4 of 9 | 1,504 (P0/P3) |
| noon_fields | 9.79 / 9.06 | 8.10 / 8.01 | -1.05 | 7.05 | 0.82 | 14.79 | 13.46 | 6,399 | 1,566 | 4 of 9 | 2,782 (P0) |
| overcast_broken (Eden) | 11.85 / 11.77 | 8.92 / 8.91 | -2.86 | 7.52 | 1.13 | 18.51 | 15.70 | 8,066 | 2,214 | 3 of 3 | 2,790 (P0) |
| broken_high (Eden) | 24.31 / 24.12 | 29.20 / 29.09 | +4.97 | 27.18 | 1.78 | 27.35 | 31.95 | 10,784 | 5,479 | 3 of 3 | 2,790 (P0) |
| sinai_lowsun_fp | 5.06 / 4.24 | 6.35 / 5.82 | +1.58 | 4.25 | 1.77 | 7.46 | 9.59 | 764 | 497 | 3 of 3 | 1,822 (P0/P3) |
| sinai_desert_noon | 9.70 / 9.31 | 6.96 / 6.94 | -2.37 | 5.35 | 1.31 | 14.42 | 11.90 | 2,672 | 1,266 | 3 of 3 | 2,790 (P0) |

Comparable at P0 on both sides (plan item 7): sunrise_ridge, woods_pine2_evening, valley 05:15,
noon_fields, overcast_broken, broken_high, bay_offshore. Sky and sea views (coastline_sunset,
coast_oc_up, sea_noon, hill_1730) ran at P0/P3 here and P3/P5 at A0, so their milliseconds are
not comparable and overstate the cost.

What it says:

- Draw grouping pays off where draws were the limit and nothing else is heavy: sunrise_ridge
  17.4 to 10.2 ms, noon_fields, Eden overcast_broken and Sinai noon all faster than A0 with
  every feature on.
- Medium views carry the features' cost: +3 to +5.4 ms at the night town, the wet town, the
  moonless road, the woods and lowsun_fp. All stay under 11.1 ms on the 4060 Ti; with MENU's 1.5
  to 2x for a 3060, town_night_rain (10.5 to 11.1) lands at 16 to 22 ms, around or over the
  16.7 ms budget.
- **The two long views are slower than A0 with everything on**: valley 05:15 26.6 to 31.6 ms
  (A0 22.5 to 23.3), Eden broken_high 29.1 to 29.2 (A0 24.1 to 24.3), with half A0's draws. Both
  are CPU-bound (CPU frame 32 to 34 ms, about 30 fps on this machine; A0 was 27 to 31). Section
  4.3 finds where it comes from.
- The CPU frame rose on most views with features (town_night 8.2 to 13.0 ms, town_night_rain
  7.1 to 13.2, moonless road 7.7 to 11.1). 60 fps needs the CPU frame under 16.7 ms: everything
  but the two long views passes on this machine.

### 4.2 Per-step cost at the long views

`FINAL/cost/cost_abel.test.sqf` (generator `gen_cost.py`): valley 05:15 and sunrise_ridge, all on,
each step's keys off in turn, and `baseline`, 2 interleaved rounds, one game, all 48 shots clean
(`FINAL/pool-cost.md`). GPU frame median, CPU frame median, ms:

| mode | valley GPU | valley CPU | valley draws | sunrise GPU | sunrise CPU | sunrise draws |
|---|---|---|---|---|---|---|
| everything on | 26.96 | 34.52 | 5,242 | 9.98 | 15.35 | 1,912 |
| A4 off | 40.55 | 41.85 | 11,406 | 27.24 | 28.70 | 12,910 |
| A5 off | 23.82 | 32.00 | 5,242 | 6.96 | 15.58 | 1,912 |
| A7 off | 24.55 | 32.22 | 5,242 | 10.14 | 15.77 | 1,912 |
| B1 off | 26.38 | 33.98 | 5,242 | 10.36 | 15.79 | 1,912 |
| B2 off | 26.23 | 33.83 | 5,242 | 10.22 | 15.80 | 1,912 |
| B5 off | 25.62 | 33.30 | 5,242 | 10.14 | 15.82 | 1,912 |
| B6 off | 26.12 | 34.01 | 5,242 | 10.14 | 15.75 | 1,912 |
| B8 off | 25.96 | 33.97 | 5,242 | 11.06 | 15.96 | 1,912 |
| B9 off | 25.77 | 33.26 | 5,242 | 10.30 | 15.88 | 1,912 |
| B11 off | 26.51 | 34.00 | 5,242 | 10.18 | 15.54 | 1,912 |
| baseline (all off) | 34.20 | 36.66 | 11,406 | 21.27 | 28.14 | 12,910 |

At the valley the pass1 section (the world's opaque draws) is 18.7 to 19.8 ms of the 27 and its
GPU span follows the CPU's draw submission (`drw:obj` 15.8 ms of CPU against 5.5 ms on the A4
lane build). No one step dominates: each of A5, A7, B5, B9, B8, B6, B2, B1 and B11 adds 0.5 to
3.1 ms of GPU span and 0.5 to 2.5 ms of CPU there. At sunrise_ridge only TAA's cost (3.0 ms)
stands out from the about 0.3 ms round-to-round spread.

### 4.3 Bisect of the valley over the lane builds

`FINAL/bisect.ps1`, valley 05:15 on and baseline, 2 rounds per build, run back to back within 22
minutes (`FINAL/bisect.md`). Clean shots only except where marked:

| build | on: GPU frame | on: CPU frame / drw:obj | baseline: GPU frame | baseline: CPU frame / drw:obj |
|---|---|---|---|---|
| A3 tip (`dist/graphics-remaster-rwdi-A3`) | 22.39, 26.05 (A3 has no switches: "on" is the A3 look) | 30.2, 32.2 / 7.3 to 8.3 | 22.40, 22.67 | 31.9, 32.8 / 5.7 to 6.9 |
| A4 lane (`gr-A4-rwdi`) | 15.91 (and 15.45 not clean) | 23.5 to 24.8 / 5.3 to 5.8 | 22.41 (not clean), 29.91 | 30.2 to 37.9 / 5.5 to 8.0 |
| B1 lane (A4 + A5 + B1) | 18.50, 20.13 | 26.5 to 28.1 / 8.0 to 9.6 | 23.53, 32.10 | 31.6 to 33.9 / 8.9 to 10.7 |
| B2 lane (+ A7, B11, B1, B2) | 20.63, 21.24 | 28.7 to 29.3 / 10.4 to 10.9 | 24.60, 24.73 | 32.5 to 33.1 / 9.8 to 10.1 |
| tip minus B4 (`gr-B4-before`, cb3dd0d) | 26.08, 26.68 | 34.0 to 36.8 / 11.1 to 14.7 | 32.50, 43.53 | 34.8 to 38.4 / 10.9 to 11.6 |
| final (fec1075) | 26.48, 27.28 | 34.0 to 34.9 / 14.7 to 15.4 | 34.13, 34.17 | 36.7 / 12.8 to 13.6 |

So A4 took the valley from about 22.5 to 15.7 ms, and the later steps added it back step by
step: +3 to +4 ms with A5 and B1, +1 with A7, B11 and B2, about +5.5 with A6, B5, B10, B9, B8,
B3, B6 and B7 together, B4 about nothing. The CPU time per object draw grows with them
(`drw:obj` 5.5 to 15 ms for the same 5,242 draws), and so does the **all-off path**: baseline
costs 34 ms at the valley against A3's 22.5 with the same 11,406 draws and the same pixels, so
"off" equals the old look in pixels but not in cost. No single change to undo, hence no minimal
fix in this pass.

## 5. Look check

Opened: the capture-set contact sheet and the noon_fields, overcast_broken, lowsun_fp and
town_night pairs at full size; the feature-pair contact sheet and the night road, hill_1730,
coastline_sunset atmosphere, town_night_rain, bay_offshore, pinewood_morn, valley 05:15, ffh and
coast_oc_up pairs at full size, plus crops and measurements in this pass.

Against the rejected looks:

- **Blue haze or a wash over the frame**: none seen. noon_fields' sky is a deeper blue (the
  owner's atmoSat 1.15) and the far hills take a light aerial blue, but the ground band keeps
  its colour (Classic 157 / R-B +12, final 157 / +7). Two warm "walls" are there: at hill_1730 a
  flat tan band from the horizon to about 40% of the frame height, with a hard top edge, which is
  the view-distance fog wall already present (pink) in A1's image, recoloured by A7's fog colour;
  and at sunrise the far massif under pink mist (B8's known risk).
- **Lambert-only flat shading**: not seen; hulls keep shaded sides at lowsun_fp.
- **Mustard or khaki ground**: the fire-lit grass round the town_night fire reads olive-yellow
  (mean 99, 86, 56: hue 42 deg, saturation 0.28) where Classic's ground is grey (113, 112, 100).
  It is firelight on the grass texture, the same item B4's commit lists ("the grass texture's own
  sand-olive colour"); for the owner to judge. B6's hill mirror at bay_offshore is olive-khaki.
- **Crushed shade / only darker**: on against all-off, whole-frame luminance 0.96 to 1.76 on 14
  of 16 hero views. Only the two night towns are darker (0.60 and 0.63, ground 115 to 72): the
  B3 dark night with warm lamps and fire, R/B 1.05 to 1.21. Eden overcast_broken's ground is 24%
  darker than Classic (27% than A0) under a sky-matched cloud shadow (B10's listed risk: the still lands
  under a cloud).
- **Flattened crowns**: at pinewood_morn the crowns nearest the sun take a pale grey-brown veil
  from the beams (B8's listed risk, close to this item).
- **Evenings less warm than Classic on hulls**: lowsun_fp R/B 0.99 both off and on; sunrise_ridge
  as a whole loses warmth (R/B 1.35 all-off to 1.25 on), as B8's commit warns for dawn.
- **Grading, vignette, grain, lens dirt**: none.

Against the plan 8 hero list: A4 same image, 1,912 draws (plan: about 1,700); A7 aureole and gold
band seen, the anti-sun band warm grey (hue 17 to 20 deg, saturation 0.09 to 0.11), not olive;
B3 dark starry night and a hard-topped pool, but the low beam is white, not warm (B3 chose this;
plan must-see not met); B4 long radial shadows and lit men, the fire's smoke reads white where
the off image's is orange-lit; B5 wave rows and a broken sun path; B6 boat mirrored with ripples,
no seams seen in the still; B7 lamp streaks and puddles, no white sheets or triangle edges seen;
B8 beams through crowns and pink valley mist, no horizon glow band at noon_fields; B9 long tree
streaks; B10 peach-lit undersides; B11 grain on the slope. Motion items (A5 ghosting, B5
popping, B11 shimmer) cannot be judged from stills.

## 6. Open risks

1. **Long views are over budget and slower than A0** (4.1 to 4.3): valley 05:15 and Eden from 250
   m at 27 to 31 ms GPU span and 32 to 34 ms CPU on a 4060 Ti, against 22.5 to 24.3 at A0 and
   15.7 on the A4 lane build. Cause: per-draw CPU cost added by most steps, including on the
   all-off path. Follow-up: profile the object draw loop (`drw:obj`) at the valley on the final
   build against the A4 lane build.
2. **"Off" is not free**: the all-off path costs 11.6 ms more than A3 at the valley with identical
   pixels; the video's "off" is honest in pixels, not in cost.
3. **With TAA on, captures do not repeat between boots**: two boots of the final build differ in
   23k to 488k pixels over 2 levels at the same fixed clock (sea and foliage views most), where
   A1's "on" repeated to 0 pixels on most views. The TAA jitter index runs from boot
   (`_taa.sequence`), so off and on must be shot in the same game, and later gate-5 checks need
   off/on in one game or TAA off.
4. **RTX 3060 not measured**; with the 1.5 to 2x rule the wet and night towns (10.5 to 11.1 ms)
   reach the 16.7 ms budget.
5. Look items for the owner: white (not warm) low beams; olive-yellow fire-lit ground; khaki hill
   mirror in the bay; tan fog wall with a hard top at hill_1730; veiled crowns by the sun at
   pinewood_morn; dawn less warm than the all-off look.
6. UNSETTLED views (`firefight_1..3`, `sinai_lowsun_fp`, `combat_ffh`, `lighttiles_town_night`
   off) were shot after 300 frames without settling; their pairs carry draw-count mismatches of
   5% or more (firefights, the wet town).
7. Not judged here: motion behaviour of every feature (the video is the first real test), crew
   and optics views beyond the stills (both look unchanged against Classic).

## 7. Files

- Report inputs: `tmp/integration/FINAL/` (`unit-*-r2.log`, `pool-abel.md`, `pool-eden-sinai.md`,
  `pool-cost.md`, `table-vs-a0.md`, `bisect.md`, `baseline-vs-A1.md`, the scripts
  `pool_quiet.py`, `table_vs_a0.py`, `bisect_summary.py`, `final-seq.ps1`, `bisect.ps1`,
  `ab-builds.ps1`, `cost/gen_cost*.py`). `FINAL/FINAL.md` is the cut-off earlier draft.
- Captures: `tmp/captures/final` (set), `final-features` (pairs), `final-baseline` (all off),
  `final-smoke`, `final-{abel-a1,abel-a2,abel-a3,eden-a1,sinai-a1}-timing`, `final-cost-timing`,
  `final-bisect-*-timing`, `final-ab-*-timing` (contended, not used in the tables).
