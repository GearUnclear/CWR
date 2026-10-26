# Look director: the combined photoreal look (light + fog + surface)

Scratch copy `D:\Arma_CWA_worktrees\look-director`, detached at **fec1075** (the graphics-remaster
tip; B3, B4, B6, B7 and B8 had all landed, nothing newer). Nothing committed, no branch. The three
lane patches were applied with `git apply --3way` (surface clean; light 44 conflict hunks in 18 files;
fog 33 hunks in 17 files) and merged by hand. `combined.patch` is `git diff --cached --binary fec1075`
after `git add -A` (91 files, +14,968 / -188); `git apply --cached --check` passes on a fresh index of
fec1075. Build `arma_CWR/build/look-director-rwdi`, game `arma_CWR/dist/look-director-rwdi`, junction
`RemasterUD-look-director`.

## Look first

`SHEET.png`: 27 Original | Overhaul pairs at 480 px a panel. The first 15 are the full
`run-captures.ps1` set (abel_lighting, eden_lighting, sinai_lobo_lighting: 960x640 at render scale
1.5); then 11 lane views at 1920x1080 (woods_pine2_evening, blue_hour_coast, dawn_valley, pine_rays,
town_fog_night, town_night_rain, jeep_close_noon, town_noon, forest_morning, rain_day, noon_vista)
and a rifle sight in morning mist. Full-size shots: `final\<view>__classic.png` / `__overhaul.png`
(1080p copies of the shared hero views are `<view>__1080__*.png`).

What the owner should look at first, in this order: town_night and town_night_rain (dark, lamp-lit,
wet reflections), sunrise_ridge and coastline_sunset (a physical sun glow instead of the pink wash),
overcast_heavy and noon_vista (depth of air over distance), pine_rays (sun beams through the wood),
lowsun_fp (long blue shadows, warm light), then the weak ones listed under Known problems.

## What the look is

Modern photoreal, one camera over everything:
- **Light and camera (look-light, `photoLight`)**: the world is drawn as scene light and a metered
  camera in the composite turns it into the picture: auto exposure with eye adaptation, a filmic
  hue-keeping curve, white balance that follows the light partly, a physical sun and sky from the sky
  table, a cool sky-lit shade at a low sun, a dark night with lamps, fires and headlights 2.5 x, and
  a weather veil that makes overcast days grey and soft.
- **Air (look-fog, `volFog`)**: a froxel volume replaces the flat distance fog and supersedes B8's
  air pass: clear-day haze with aerial perspective (a summer day's ~8 km visibility), weather fog,
  rain air, valley mist on misty mornings, sun beams in wet air, lamp halos and night glow in fog.
- **Surfaces (look-surface, `surfaces`)**: horizon-based occlusion (GTAO) with temporal accumulation,
  canopy sky occlusion, a deeper clear-day shade, specular, texture relief and small cavities, and
  occlusion inside vehicle interiors.

## Merge decisions (overlaps)

- **LitConstants slots**: the surface lane keeps 79 to 88 (`Surface`, `SurfaceCanopy`, `Surface2`,
  `SurfaceSh[6]`, `Surface3`); the camera moves to **89** (`Camera`) and **90** (`CameraLook`),
  `Count` 91 (static_asserts, GLSL block, layout test and INTEGRATION-PLAN 3.3 updated).
- **Feature enum / registry order**: ... `Surfaces`, `PhotoLight`, `VolFog` (29 landed of 29).
- **Post shader order**: ... SSR, `CombatMask`, `CombatMaskBlur`, `Gtao`, `GtaoAccum`, `Meter`, then
  the six fog programs (35 post shaders).
- **Composite**: one shader: the camera head and main (light) with the surface lane's depth-aware
  occlusion upsample, multi-bounce and contact shadows feeding `keep`, and B4's combat scatter added
  to the camera path as light at the exposure.
- **Exposure vs fog brightness**: the fog was written for display space. Under the camera it now
  works in the frame's scene light: the apply, the horizon ring and the night glow `camDecode` the
  frame, the model's air light and the near field (sun beams, lamps) take the frame's pre-exposure
  (`litCamera.w`), the apply writes `camFold` with no shoulder of its own (the camera's curve comes
  after). The meter meters the frame after the fog (`PostSceneTex`).
- **B8 vs volumetric fog**: the fog runs on the Air stage in B8's place (B8 runs when `volFog=0`).
  `camAerial` (the light lane's 6 x on B8's haze) is therefore inert in the default look; the fog's
  own clear haze was raised to give the same depth (see knobs).
- **SSR + fog**: the fog, the meter, the bloom and the composite read `f.frameColor` (the frame with
  the reflections), so reflections are fogged and metered too.
- **AO vs ambient**: GTAO keeps a floor of 0.35 (a Jeep's underbody at a low sun went to 0); the
  clear-day shade (`surfShade`) raised from 0.45 to 0.65 and the canopy occlusion from 1.5 to 1.0,
  together with the camera's toe (`camContrast` 1.2), so vehicles and soldiers stay readable.

## Fixes made here (from the lanes' critic points)

- Optics: the fog no longer switches off under a gunner's optic overlay; the apply leaves the
  overlay's black mask alone (`optics_mist_sight` shows the same mist through the sight; no pop
  between frame 1 and frame 30 of raising it).
- Fog gap closing: 4.5 px window (was 9 px): no square notches on fogged silhouettes.
- Fog in rain: the dark plate beyond the land is gone (sky pixels keep the dome's fill in weather
  fog and rain; the horizon ring takes the mean of its samples under a grey sky). Debug log removed.
- Dawn/sun lobe gain 0.45 -> 0.3 (the peach wash).
- Cavity: the land texture's metre-scale term is capped at a few percent (the wet-stain blotches),
  `kSurfBaseCavity` 1.5 -> 0.8; relief and cavity fade out with the sun (no texture reads at night).
- Camera: dark-tone desaturation 0.7 -> 0.88 (grey forest floors), weather-veil saturation drop
  0.3 -> 0.15, lamp flare at night 0.45 -> 0.25 (star rays), meter sky weight 0.6 -> 0.4,
  bright-scene keep 0.45 -> 0.15, new dusk exposure (-1.25 EV from sun +4 to -4 degrees, handing over
  to the night's `camNight`): sunset firefights and the blue hour read as evening.
- Blue hour: the low-sun zenith hue ramps over 0 to 30 degrees, the twilight belt falls off over ~10
  degrees and the horizon's afterglow is carried up ~12 degrees: no stripe edge.
- Combat flash 1.0 -> 0.6 (a shell at dusk lit the plain white to the horizon).
- Interior occlusion radius 0.6 -> 1.0 m.

## Knobs and defaults (A1 registry; `triSetLighting "remastered,<key>=<v>"`)

Switches (all default 1): `photoLight`, `volFog`, `surfaces` (plus every landed A/B feature).

| knob | default | note |
|---|---|---|
| camEv | **0.6** (lane 0.3) | exposure compensation, EV |
| camAdapt | 1 | eye adaptation, s |
| camCurve | 0 | hue-keeping filmic |
| camContrast | **1.2** (lane 1.4) | toe |
| camShoulder | 0.95 | |
| camWhite | 32 | |
| camSat | 1.15 | |
| camWb | 0.45 | white balance follow |
| camSky | 1 | clear sky brightness |
| camSkyHue | 1 | physical sky hue |
| camNight | -3.8 | EV at full night |
| camVignette | 0.18 | |
| camBloom | 1 | |
| camHaze | 6 | sky table haze |
| camScotopic | 1 | night rod shift |
| camAdditive | 1 | |
| camAerial | 6 | B8 haze gain; inert while volFog runs |
| camLocal | 2.5 | lamps/fires/headlights at night |
| fogDensity | 1 | all media |
| fogQuality | 1 | 12 px cells, 64 slices |
| fogMist | 1 | valley mist |
| fogSun | 1 | beams and forward lobe |
| fogLights | 1 | lamp light in fog, night glow |
| surfAo | 3 | GTAO power |
| surfAoRadius | 4 | m |
| surfCanopyAo | **1.0** (lane 1.5) | |
| surfSkyLight | 0 | |
| surfSkyTint | 0.25 | |
| surfSkyLevel | 0 | |
| surfSpec | 1 | |
| surfContact | 1 | contact shadows |
| surfRelief | 1.5 | fades out with the sun |
| surfShade | **0.65** (lane 0.45) | clear-day shade fill |
| surfCavity | 2 | fades out with the sun |
| combatFlash | **0.6** (B4 1.0) | |

Constants changed here: `kHazeClear` 4e-4 -> **6e-4**, `kHazeNearKeep` 0.12 -> **0.35**,
`kHazeNearFrom` 500 -> **200** m, `kHazeNearTo` 3500 -> **2500** m, `kFogLobeGain` 0.45 -> **0.3**
(VolumetricFog.hpp + the GLSL mirror); `kCameraDarkSaturation` **0.88**, `kVeilSaturationDrop`
**0.15**, `kCameraFlareNight` **0.25**, `kMeterSkyWeight` **0.4**, `kBrightKeep` **0.15**, new
`kCameraDuskStartDeg` 4, `kCameraDuskFullDeg` -4, `kCameraDuskEv` -1.25 (PhotoCamera.hpp);
`kSurfBaseCavity` **0.8**, `kGtaoInteriorRadius` **1.0**.

Gates that save cost: the fog's near field runs only with the sun in view of wet air or lights at
night, the night glow only at night in wet air; surfaces off at night without lamps; relief and
cavity zero at night; GTAO 2 slices under TAA.

## Cost (`cost.md`)

RTX 4060 Ti, 1920x1080, render scale 1.0, 4x MSAA, TAA. Tip = `remastered,photoLight=0,surfaces=0,
volFog=0` (graphics-remaster at fec1075), Overhaul = `remastered`; two interleaved rounds per view in
one game, mean (`tmp\captures\timing` in the scratch copy). Whether other sessions' games shared the
GPU during this run was not logged; at the start of the session one of the B-lane workflow's games
was running.

| view | tip -> overhaul GPU ms | delta |
|---|---|---|
| noon_fields | 8.95 -> 10.68 | +1.73 |
| lowsun_fp | 8.38 -> 10.06 | +1.69 |
| forest_edge_golden | 6.73 -> 8.39 | +1.66 |
| sunrise_ridge | 12.12 -> 11.97 | -0.16 |
| coastline_sunset | 5.93 -> 6.27 | +0.34 |
| woods_pine2_evening | 11.58 -> 12.96 | +1.38 |
| blue_hour_coast | 5.24 -> 6.03 | +0.79 |
| dawn_valley | 30.81 -> 33.14 | +2.32 |
| pine_rays | 6.21 -> 7.28 | +1.06 |
| town_night | 11.34 -> 12.88 | +1.54 |
| town_fog_night | 12.75 -> 13.83 | +1.08 |
| town_night_rain | 12.32 -> 13.43 | +1.11 |
| jeep_close_noon | 6.83 -> 8.47 | +1.64 |
| town_noon | 6.90 -> 8.66 | +1.76 |
| forest_morning | 6.63 -> 7.59 | +0.96 |
| overcast_broken | 9.85 -> 10.99 | +1.14 |
| overcast_heavy | 9.10 -> 10.85 | +1.75 |
| rain_day | 6.67 -> 7.71 | +1.05 |
| noon_vista | 26.98 -> 28.82 | +1.84 |
| sinai_lowsun_fp | 6.36 -> 7.06 | +0.70 |
| sinai_desert_noon | 7.58 -> 8.93 | +1.35 |

The look costs about +0.3 to +2.3 ms (mostly +1 to +1.8): world pass +0.2 to +1.3 (surface terms),
ao 0.1 -> 0.3, air (fog) 0.4 to 1.0, composite +0.1. On a 3060 expect roughly 1.3 to 1.5 x. Most
views stay under 16.6 ms on the 4060 Ti; dawn_valley (33 ms) and noon_vista (29 ms) are over budget
already at the tip (their world pass is 27 to 31 ms with or without the look: long view distance and
dense trees), so 60 fps at 1080p is not met there whatever the look does.

## Thumbnail difference (480 px, Original vs Overhaul)

Mean abs difference / share of pixels changing by more than 16 levels: town_night 50 / 80%,
town_night_rain 52 / 81%, town_fog_night 47 / 76%, firefight_2 42 / 81%, forest_edge_golden 40 / 83%,
blue_hour 37 / 80%, noon_vista 36 / 80%, sunrise_ridge 31 to 37 / 85 to 95%, overcast_heavy 33 / 52%,
lowsun_fp 30 / 67%, sinai_desert_noon 30 / 65%, town_noon 30 / 75%, coastline 26 / 47%, dawn_valley
22 / 47%, noon_fields 22 / 42%, woods 22 / 59%, rain_day 21 / 46%, pine_rays 21 / 56%, overcast_broken
18 / 44%, sinai_lowsun 18 / 34 to 42%, jeep_close_noon 16 / 35%, tank_gunner_interior 11 / 17%,
tank_gunner_optics 11 / 28%, optics_mist_sight 3.5 / 0%.

## Tests

Unit gate tags (`[remaster],[FeatureSwitches],[FeatureGates],[PostStages],[fog],[SurfaceLight],
[AirMedium],[ssr],[wet],[CombatLights],[Shaders],[StateCacheAudit],[PhotoCamera]`): 301 cases,
953,802 assertions, all pass (glslang compiles every post program). Full suite: 3381 cases, the 6
known localisation cases fail (no `packages\Remaster` junction in the scratch copy). Updated
expectations: slot numbers, feature/knob counts, post shader count 35, the fog's optics gate (now on),
the fog haze bounds, combatFlash's default. clang-format applied to the changed lines.

Capture tools added: `gen_look_director.py` (the director's views, any modes), `look_final_*`
(Original | Overhaul), `look_timing_*` (tip | overhaul, interleaved), `look_director_optics`
(hand-written, from the fog lane's optics test), `make_look_sheet.py` (the contact sheet).

## Known problems

1. **Not obviously different**: tank_gunner_interior (pre-lit interior; the camera and the occlusion
   change it little, 17% of pixels) and the rifle sight in mist (the volumetric mist there matches
   the classic fog, 0%). tank_gunner_optics changes the sky and colour only (28%).
2. **Noon is the weakest daytime look**: noon_fields and jeep_close_noon still have a pale near-white
   ground (the texture); what differs is the deeper sky, the haze on the far hills and the darker cast
   shadows. Lowering exposure further at noon (`kBrightKeep` 0.45 -> 0.15) changed little.
3. **Dusk firefights**: the ground still reads lighter than in the Original (luma 78 to 86 vs 46),
   and muzzle flashes and fires make large yellow glows on the ground.
4. **Rain far field**: the far hills in rain_day go to a dark grey silhouette rather than a pale
   fade: the fog's air light in rain is darker than the sky.
5. **Sunrise and dawn ground** stays a dark grey-green; the far mountains keep their shape but the
   valley mist did not form on dawn_valley's date (the misty morning is a seeded weather event).
6. The fog lane's own open items stay: no forward fog term for smoke particles; town_fog_night's
   houses lose only part of their contrast; headlight in-scatter in fog lights the air in front of
   the Jeep body.
7. The surface lane's open items stay: low-sun texture grain under TAA (about 1.7 x the tip's frame
   change), town facades gain little, two carriers of the surface class.
8. Cost: +1 to +2 ms in most views, over the 16.6 ms budget only where the tip already is; not
   measured on a 3060, and the timing run did not log foreign games on the GPU.
9. The final captures were shot before the last clang-format pass and the combatFlash unit-test edit
   (no behavioural change; the rebuilt game and tests pass).
