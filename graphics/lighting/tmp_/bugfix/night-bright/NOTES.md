# night-bright: the Remastered night is over-bright and yellow-green

Scratch worktree: `D:\Arma_CWA_worktrees\fix-night-bright` (detached at `claude/youthful-lovelace-8imbdh`,
3c53141; left in place, nothing committed). Build: `arma_CWR\build\fix-night-bright-rwdi`, game in
`arma_CWR\dist\fix-night-bright-rwdi`. Patch: `fix.patch` (7 files, +152 -23, applies cleanly to the
branch tip with `git apply --check`).

## Short version

The research tracks blamed the night rig (moon, sky fill, bounce, exposure). Measured in game, the rig
was not what lit the night: **almost all of the night light came from the street lamps**, and two bugs
made that light too strong and too wide. A third made the moon invisible.

1. **Every street lamp was white with ten times its authored ambient** (engine config parse bug,
   shared by Classic and Remastered). Its ambient term has no direction and reaches 500 m, so the six
   Saint Louis lamps washed the open country 260 m away to a flat grey, moonlit or not.
2. **The Remastered local-light sum was capped at 1.5 (display) instead of Classic's 1.0**, so
   lamp-lit surfaces got up to 2.44x the linear light Classic allows. Where the lamp pools overlap
   (the whole town square), the ground was brighter than Classic and close to noon ground. That
   brightness also escaped the night-eye greying, so the olive grass showed at full saturation: the
   yellow-green.
3. **The moon added nothing visible** (`kMoonFull = 0.0035`): switching it off changed no view.

Fixed: lamps read their authored colour and ambient, local lights clamp where Classic clamps, the
moon becomes a real key light with a moonlit sky fill, and dim lit surfaces fade to a slightly cool grey.
Result: the town is 0.84 EV darker on the ground than before and back at Classic's level with less
saturation (its lamp-lit ground still meters like a sunset ground, Risk 4), the yellow-green is gone,
the moonlit country is a readable cool grey, the moonless country is dark, dusk keeps Classic's
warmth, fires stay orange. Night vision is unchanged in the
town view; it is darker in open country near a town (explained under Risks: that is the lamp data
fix, and it also changes Classic).

## How the cause was found

Knob A/B on the branch tip build (`before_diag/`, research test `night_bright.test.sqf`), ground =
lower 28% of the frame, display RGB:

| view | Remastered | localLights=0 | moonlight=0 | saturation=1 |
|---|---|---|---|---|
| town_night (full moon 69 deg, 6 lamps, fire) | 164 159 123, sat 0.25 | 22 22 21 | 164 159 123 | 164 159 127, sat 0.23 |
| road_moonlit (country 260 m from town, moon 36 deg) | 51 51 51 | 20 20 20 | 51 50 50 | 52 51 51 |
| road_moonless (same, 23:00, no moon) | 51 50 50 | 13 13 13 | n/a | 51 51 50 |

So: local lights are ~100% of the town and ~75% of the open country at night; the moon is invisible
(`DIAG_before_town_night.png`, `DIAG_before_road_moonlit.png`); the saturation knob is a small part of
the yellow. Temporary light-list logging in `EngineGL33::UploadLitLights` (removed before the patch;
output kept in `logs/probe_light_list.txt`) then showed every street lamp as `dif (1.00 1.00 1.00)
amb (1.00 1.00 1.00)`, and on the road view six of them 220-300 m away in the terrain's light list.

## Root cause (line numbers at the branch tip 3c53141)

- `engine/Poseidon/IO/ParamFileExt.cpp:61-68`, `GetColor()`: returns `HWhite` for any array that is not
  exactly 4 entries long. The Remaster's `CfgVehicles >> StreetLamp` (BIN\CONFIG.BIN, debinarised) has
  `colorDiffuse[]={0.9,0.8,0.6}; colorAmbient[]={0.1,0.1,0.1};` (3 entries), read at
  `engine/Poseidon/World/Scene/ObjectClasses.cpp:191-192`. Every lamp became diffuse (1,1,1) and
  ambient (1,1,1). It is the only 3-entry colour among the `GetColor` keys in the base config (counted in the
  debinarised CONFIG.BIN: `color` 96x4, `ambient` 23x4, `cloudletColor` 13x4, `lightColor` 4x4,
  `smokeColor` 3x4, `colorDiffuse` and `colorAmbient` 1x3 each). @LoBo configs were not checked.
  The ambient part is added per light with no direction
  (`EngineGL33_LightingShaders.cpp:482`, classic VS the same) out to 10x `startAtten` (500 m for a lamp).
- `engine/PoseidonGL33/EngineGL33_LightingShaders.cpp:565`:
  `toLinear(min(localLights(...) * litGround.w, vec3(1.5)))`. Classic clamps the same display-space sum
  at 1 (`EngineGL33_Shaders.cpp`, `vColor = clamp(litColor, 0, 1)`); 1.5 lets a lamp-lit surface reach
  2.44 linear, above noon sunlit ground.
- `engine/Poseidon/Graphics/Remaster/LightRig.cpp:46`: `kMoonFull = 0.0035f` (moonlit ground 0.15 of
  texture brightness vs 0.08 moonless, both before the night-eye pass), and `:250` moon chroma
  (0.70, 0.80, 1.00), a strong blue.
- `EngineGL33_LightingShaders.cpp:604-608` (classic night eye, weights (0.2, 0.9, 0.4) from
  `EngineGL33_Shaders.cpp:2026`): dim pixels fade to a neutral grey, so moonlit ground had no cool cast
  at all (ground saturation 0.003).

Checked and not a cause: the exposure meter is 1 at night by construction (`LightRig.cpp:208-213`,
the meter only opens up above sin(sun) 0.08); the ground-colour bounce at night is
`0.2 * kSkyNight`, negligible; the starlight sky fill `kSkyNight` is the classic parity value and is
kept.

## The fix

| file | change |
|---|---|
| `IO/ParamFileExt.cpp` | `GetColor` accepts `{r, g, b}` (alpha 1) as well as `{r, g, b, a}`. Lamps get their authored warm colour and 0.1 ambient. |
| `PoseidonGL33/EngineGL33_LightingShaders.cpp` | Local-light clamp 1.5 -> 1.0, Classic's own clamp (still summed in display space and taken to linear, so pool size and falloff are unchanged). Night eye: lit fragments fade toward a grey of the same luminance with blue raised, `(0.985, 0.995, 1.1)`; pre-lit draws keep the classic grey. Only applies where the classic night-eye pass already applies. |
| `Graphics/Remaster/LightRig.cpp` | Moon `kMoonFull` 0.0035 -> 0.02, chroma (0.82, 0.90, 1.00) (cool, pale), dimmed by overcast like the sun (`skyThrough*0.8+0.2`). Moonlit sky fill `kMoonSky = 0.25` of the moon, added to zenith and horizons after the twilight cap (the classic night sky the cap reads stays black whatever the moon does). Ground bounce includes moonlit ground. All moon terms are zero by day (`nightEffect` 0), so day rigs are bit-identical (unit test). |
| `Graphics/Remaster/LightRig.hpp`, `README.md` | Comments, the Light rig and Local lights rows, a night calibration paragraph. |
| tests | see below |

Why this is correct rather than a retune: the lamp values are the authored config values, the local
clamp is the original renderer's clamp, and the moon numbers are calibrated against the rig's own
units (full moon at 60 deg: moonlit ground 0.29 of texture brightness after the display gamma, its
shade 0.17, moonless 0.08; noon sunlit ground is 1.0). Night vision draws through the classic programs
(README), so none of the shader or rig changes can reach it.

Variants tried and rejected (evidence kept):
- A **hue-keeping cap** (scale the whole sum back to 1, so a saturated pool keeps the lamp colour):
  warm lamp pools at night, but at dusk the lamp-lit lawn turned mustard, ground saturation 0.57 vs
  Classic 0.36 (`NEG_dusk_huekeeping_cap.png`, run `tune_v2b_huecap/`). The per-channel clamp
  (shipped) gives 0.40, Classic's look.
- A cooler night grey `(0.96, 1.005, 1.07)` (run `tune_v2/`): it lowered red against green, so
  half-greyed lamp-lit ground drifted to yellow-green (hue 62). The shipped grey raises blue only.
- `localLights` 0.75 / 0.6 / 0.45 (run `tune_v2/`): no change in the town until 0.45 (the six lamp
  discs sum to about 2x the clamp), then only darker. Not used.
- Moon at 0.6x and 1.6x of the shipped level (runs `tune_v1/`, `tune_v2/`): moonlit country ground
  31 and 43 vs 37; 1.0 kept.

## Tests

New or extended Catch2 tests (no GPU needed):
- `tests/unit/engine/Poseidon/Graphics/test_remaster_lighting.cpp`, "Remaster light rig: a full moon
  lights a cool, pale, readable night" `[Graphics][remaster]`: moonlit ground shown 0.18-0.30 (before
  the gamma pass), moon shade ratio 0.45-0.75, moon blue > green > red with red/blue 0.75-0.9, moonless
  night stays at `kSkyNight` and under half the moonlit level, overcast cuts the moon below 25%, a day
  rig with the moon up is identical to one without.
- `tests/unit/engine/Poseidon/IO/test_paramFileExt.cpp`, "GetColor reads three-entry colours with
  alpha 1" `[io][paramFileExt][remaster]`.

Before (branch-tip engine sources with the new tests, `logs/unit_tests_before.log`,
`logs/test_failbefore_*.log`): rig test FAILS 4 of 18 assertions (moonlit ground 0.0999 < 0.18; moon
red/blue 0.70 < 0.75; moonless not under half of moonlit; overcast moon 0.0035 not < 0.00088);
GetColor test FAILS 5 of 10 (lamp diffuse and ambient read as 1.0). After (`logs/unit_tests_after.log`):
both pass.

PoseidonTests on the final build: `[remaster]` 15/15, `[gpu]`, `[GL33]` 104/104, `[DebugGroup]`,
`[ShutdownOrder]`, `[UBORestoreGuard]`, `[StateOwnership]`, `[StateCacheAudit]`, `[BufferUsageAudit]`,
`[IboBindAudit]`, `[Conventions]`, `[Shaders]` (glslang compile of every remastered shader),
`[paramFileExt]`, `[io]`, `[paramfile]`, `[config]` all pass. `[graphics]` has 1 failure,
`DDSConverter: generate DDS fixture from synthetic DXT1` (`test_dds_converter.cpp:347`, width 32 vs
64); it fails the same way with the branch-tip engine sources (`logs/unit_tests_before.log`), and
DDSConverter does not use any file this patch touches. clang-format 22.1.8 `--dry-run --Werror`:
exit 0 on all six touched C++ files.

## In-game evidence

Same research test for both builds (`arma_CWR\tests\rendering\research\night-bright\night_final.test.sqf`
in the scratch worktree, git-excluded; generator `gen.py` beside it), Abel, 960x640, 4x MSAA, 1.5x
render scale, run through `run-captures.ps1` with `-DataJunction RemasterUD-fix-night-bright`.
BEFORE = `D:\Arma_CWA_worktrees\lighting\arma_CWR\dist\lighting-rwdi` (`before/`), AFTER = this fix
(`after/`). Each run folder has `shots/`, the per-view pairs, `cmp_stats.md`, `timings.md`, the tri tree.

Ground region (lower 28% of the frame), display RGB, HSV saturation, red/blue; log2 = mean log2
linear luminance:

| view | Classic (after build) | Remastered BEFORE | Remastered AFTER |
|---|---|---|---|
| town_night | 123 121 106, sat 0.14, log2 -2.39 | 164 159 123, sat 0.25, r/b 1.33, log2 -1.53 | 124 122 111, sat 0.10, r/b 1.11, log2 -2.37 |
| town_night frame luma | 96.8 | 124.2 | 96.8 |
| town_lamps (moonless) | 123 121 106 | 165 159 122, sat 0.26 | 122 120 110, sat 0.10 |
| road_moonlit | 20 20 20 (BEFORE build: 57 57 57) | 51 51 51, sat 0.01 | 37 37 40, sat 0.08, r/b 0.92 |
| road_moonless | 20 20 20 (BEFORE build: 57 57 57) | 51 50 50 | 15 15 16 |
| dusk_town (sun -1.7 deg) | 110 106 71, sat 0.36 | 155 148 93, sat 0.40 | 111 106 67, sat 0.40 |

Images: `BA_town_night.png`, `BA_town_lamps_moonless.png`, `BA_road_moonlit.png`,
`BA_road_moonless.png`, `BA_nv_town.png`, `BA_nv_road.png`, `BA_dusk_town.png`.

What they show:
- **town_night / town_lamps**: the yellow-green flood is gone. The lamp-lit square sits at Classic's
  brightness with lower saturation (0.10 vs Classic 0.14, before 0.25); walls and the birch read pale
  grey-white instead of yellow; the fire and the smoke right above it stay orange. In the overlap of
  the six lamp pools the lamp light is near white (both channels clipped), as in Classic; the authored
  warm colour shows only where one lamp lights a surface below the clamp, and on the lamp flare.
- **road_moonlit vs road_moonless**: before, the two were the same image (grey lamp wash from the town,
  the moon contributed nothing). After, the moonlit country is a dark, readable cool grey (37 37 40) and
  the moonless country is dark (15 15 16) with the low beam pool and the town lamps on the horizon.
- **dusk_town**: the warm evening is kept; the AFTER ground matches Classic (111 106 67 vs 110 106 71);
  BEFORE was brighter and yellower.
- **Night vision** (`BA_nv_town.png`, `BA_nv_road.png`; numbers from `scripts/nb_nvdiff.py`):
  nv_town BEFORE vs AFTER, Remastered: bottom 30% of the frame mean abs diff 0.00/255, left 40%
  0.06/255; the whole-frame 2.0/255 is the animated smoke and fire (a Classic BEFORE vs Classic AFTER
  pair differs by the same 1.9/255). nv_road: **changed**, ground 0 106 0 -> 0 79 0 (bottom 30% mean abs
  8.9/255), identically in Classic and Remastered, see Risks.

GPU cost: none measurable. The shader change is one constant and one multiply in the night-eye path;
the frame times in `before/timings.md` and `after/timings.md` swing by +-2 ms between runs on the
shared GPU (other agents' games were running), in both directions.

## Risks and owner decisions

1. **Night vision near street lamps is darker** (nv_road ground 106 -> 79 of 255 green, still bright
   and readable; nv_town unchanged). Not caused by the night rig or the shaders (night vision draws with
   the classic programs, and Classic NV changes by exactly the same amount): it is the lamp data fix.
   Before, the bogus 1.0 lamp ambient flooded the country near a town to the NV maximum. If night vision
   must stay bit-identical everywhere, drop the `ParamFileExt.cpp` hunk and its test: the rest of the
   patch still fixes the town brightness, the yellow-green and the moon, but the grey lamp wash around
   every town (up to 500 m) stays in all modes, and lamps stay white.
2. **Classic changes too** (same data fix): Classic town views are unchanged (the lamps clip there as
   before), but Classic country near towns loses the lamp wash (road views 57 -> 20) and lamp flares
   are warm. The street lamps' CPU-lit smoke and particle lighting also uses the corrected values.
3. **Lamp warmth**: overlapping lamp pools are near white, as in Classic. Warm overlaps need either the
   hue-keeping cap (mustard at dusk, rejected) or a lamp falloff redesign.
4. **The lamp-lit town is still as bright as Classic's.** Same script on the Remastered shots of
   `tmp/captures/20261017-final` (the branch tip's day views), mean log2 linear luminance:

   | | whole frame | ground (lower 28%) |
   |---|---|---|
   | town_night BEFORE / AFTER | -3.19 / -3.99 | -1.53 / -2.37 |
   | coastline_sunset | -2.14 | -2.39 |
   | lowsun_fp | -2.35 | -2.91 |
   | sinai_lowsun_fp | -3.15 | -4.04 |
   | forest_edge_golden | -3.19 | -3.22 |
   | sunrise_ridge | -4.14 | -6.45 |
   | noon_fields | -1.43 | -1.39 |

   The whole night frame now meters darker than four of the five low-sun views, but the lamp-lit
   ground of the square still meters like the sunset coast's ground (-2.37 vs -2.39), down from near
   noon (-1.53). That remainder is Classic's lamp model: 50 m flat discs that overlap across the whole
   square and clip at the same level as noon sun. Real lamp pools need the night track's falloff and
   intensity rework (out of scope here: it changes the authored pool size and was judged "mostly just
   darker" by that track's critic).
5. **The moon casts no shadows**: it lights everything facing it, under roofs and crowns too, as the
   sky fill already does. Five to six times stronger than before, so interiors at night get some
   moonlight. Moon shadows are part of the night track's bigger change.
6. Local lights at night, including fires, headlights and any muzzle-flash light, peak at 1.0 instead of
   1.5 (Classic's limit). Fires stay orange; a combat-light rework that wants HDR flashes should note it.
7. The cool night grey applies to every dim lit surface at night (moonless too); it is mild
   (red/blue 0.92 on the ground, saturation 0.08). Dusk is unaffected (the night-eye pass is near off
   while the classic sun colour is still red).

## Files

Deliverables: `fix.patch`, `NOTES.md`, `BA_*.png` (before/after), `NEG_dusk_huekeeping_cap.png`,
`DIAG_before_*.png`. `scripts/`: the research test generator and the final test it wrote, the
statistics, strip and night-vision diff scripts. Run folders: `before/`, `after/` (final evidence), `before_diag/` (knob
decomposition on the tip build), `probe/` (tip + temporary light-list logging), `tune_v1/`, `tune_v2/`,
`tune_v2b_huecap/`, `tune_v3/` (iterations). `logs/`: build logs, capture console logs, unit test logs,
`probe_light_list.txt`.

Directory listing of this folder (`ls -la`, plus `logs/` and `scripts/`, taken just before this listing was pasted in, so NOTES.md shows its earlier size):

```
total 9832
drwxr-xr-x 1 Dane 197121       0 Oct 17 21:57 .
drwxr-xr-x 1 Dane 197121       0 Oct 17 21:41 ..
-rw-r--r-- 1 Dane 197121 1634441 Oct 17 21:55 BA_dusk_town.png
-rw-r--r-- 1 Dane 197121  347542 Oct 17 21:55 BA_nv_road.png
-rw-r--r-- 1 Dane 197121  358323 Oct 17 21:55 BA_nv_town.png
-rw-r--r-- 1 Dane 197121  780067 Oct 17 21:55 BA_road_moonless.png
-rw-r--r-- 1 Dane 197121  818892 Oct 17 21:55 BA_road_moonlit.png
-rw-r--r-- 1 Dane 197121 1560457 Oct 17 21:55 BA_town_lamps_moonless.png
-rw-r--r-- 1 Dane 197121 1557516 Oct 17 21:55 BA_town_night.png
-rw-r--r-- 1 Dane 197121  427698 Oct 17 21:55 DIAG_before_road_moonlit.png
-rw-r--r-- 1 Dane 197121  842239 Oct 17 21:55 DIAG_before_town_night.png
-rw-r--r-- 1 Dane 197121 1621960 Oct 17 21:55 NEG_dusk_huekeeping_cap.png
-rw-r--r-- 1 Dane 197121   16379 Oct 17 21:57 NOTES.md
drwxr-xr-x 1 Dane 197121       0 Oct 17 21:54 after
drwxr-xr-x 1 Dane 197121       0 Oct 17 21:47 before
drwxr-xr-x 1 Dane 197121       0 Oct 17 21:28 before_diag
-rw-r--r-- 1 Dane 197121   21685 Oct 17 21:57 fix.patch
drwxr-xr-x 1 Dane 197121       0 Oct 17 21:53 logs
drwxr-xr-x 1 Dane 197121       0 Oct 17 21:30 probe
drwxr-xr-x 1 Dane 197121       0 Oct 17 21:57 scripts
drwxr-xr-x 1 Dane 197121       0 Oct 17 21:38 tune_v1
drwxr-xr-x 1 Dane 197121       0 Oct 17 21:43 tune_v2
drwxr-xr-x 1 Dane 197121       0 Oct 17 21:47 tune_v2b_huecap
drwxr-xr-x 1 Dane 197121       0 Oct 17 21:49 tune_v3

logs/:
after.console.log
before.console.log
before_diag.console.log
build0.log
build_final.log
build_final2.log
build_final3.log
build_probe.log
build_tests_failbefore.log
build_tests_final.log
build_tests_tipsrc.log
build_tests_v1.log
build_v1.log
build_v2.log
build_v3.log
configure.log
probe.console.log
probe_light_list.txt
test_failbefore_getcolor.log
test_failbefore_rig.log
tune_v1.console.log
tune_v2.console.log
tune_v3.console.log
unit_tests_after.log
unit_tests_before.log

scripts/:
gen_night_bright_test.py
nb_analyze.py
nb_nvdiff.py
nb_strip.py
night_final.test.sqf
night_final.test.toml
```
