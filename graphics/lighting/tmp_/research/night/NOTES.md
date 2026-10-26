# Night track (TRACKID = night): moonlit night, lamp light, headlights

Status: DONE (resumed run). Verdict: **promising**. The prototype turns a CWA night from "dim grey
dusk" into a real night: dark moonless country with a starfield, a physical low beam that lights
the road ahead with a warm hot spot and a sharp cut-off, a cool desaturated moon key that casts
shadows, warm lamp light with inverse-square falloff, fire-lit smoke. Night vision is unchanged.
Two sub-ideas failed and are reported as failures (volumetric lamp haze, lamp ground bounce), and
headlights on Abel's steep test hill are still weak (explained below).

Best images (Classic | Remastered current | Night prototype, GPU frame ms on each panel):

| file | source run | what it shows |
|---|---|---|
| `BEST_road_flat_drive_moonless.png` | run10 (v9) | **the headline**: moonless night, driver's eye on a flat road. Classic and Remastered show a bright grey landscape; the prototype shows darkness, stars, and a warm low beam with a hot spot 20-35 m ahead and spill onto the verges |
| `BEST_road_flat_moonless.png` | run10 (v9) | same night, third person behind the jeep |
| `BEST_road_flat_head.png` | run10 (v9) | full moon at 36 deg: moonlit field and trees, stars, headlights |
| `BEST_town_night.png` | run10 (v9) | full moon at 69 deg, six street lamps, campfire |
| `BEST_town_lamps.png` | run10 (v9) | moonless, street lamps only |
| `BEST_town_moonlow.png` | run8 (v8) | full moon at 24 deg in the east |
| `BEST_road_head.png` | run8 (v8) | full moon on a 14 deg uphill road: the hard case (see "Hill road") |
| `BEST_moon_sky.png` | run8 (v8) | looking at the moon: moonlit crowns, star splats (Classic trees are black on black) |
| `BEST_nv_town.png` | run8 (v8) | night vision: unchanged (pixel diff below) |

v8 and v9 differ only in one shader line (v9 drops the classic per-light "ambient" for headlights
and caps it for lamps). `make_best.py` rebuilds these strips from the run folders.

## Why CWA nights look the way they do (code, plus what the logs showed)

- `LightSun::Recalculate` sets the sun colour to black at night and the ambient to
  `BackgroundColor * 0.05`. Classic has no directional moonlight; shipped Remastered adds a weak
  shadowless moon term (`kMoonFull = 0.0035`).
- Local lights use a flat disc: full light out to `startAtten` (street lamp 50 m; headlight
  reflector 110 m inside a 12 deg cone), then start^2/d^2 out to 10x start. The six Saint Louis
  lamps flat-light the whole square, and lamps hundreds of metres away still leak light into open
  country: the moonless Classic road view (run9/run10) is a bright grey landscape at 23:00. Shipped
  Remastered keeps that model on purpose, so it inherits the bright, yellow-green town.
- The night-eye pass (`rgbEyeCoef`) pulls dim pixels toward a brighter grey (the grey wash).
- From the `NIGHT:` log lines (my logging, runs 5-10): the Abel `lampazel` lamps carry diffuse
  (1, 1, 1), not a warm colour (the earlier note's (0.9, 0.8, 0.6) was wrong for these lamps); they
  hang 8-12 m above the ground; the jeep reflectors are start 110, colour (0.9, 0.8, 0.8); the jeep
  model's beam axis points 4.7 deg UP in world space while the jeep sits on a 14 deg slope.
- Not a bug, though it looked like one for two runs: terrain does receive local lights in the
  remastered shader. Debug views (`nightDebug=2/3`, `run6/cmp_*_dbg.png`) showed correct
  camera-relative positions, material kd and irradiance; the pools were weak because the lamps are high.

## The prototype

Scratch worktree `D:\Arma_CWA_worktrees\gfx-night` (detached at the lighting tip 3c53141), build
`dist/gfx-night-rwdi`, C++ diff in `prototype.patch`, fragment shader override `ovr_v9`
(`psRemasterCommon.glsl`; `ovr_v4`-`ovr_v8` are the earlier steps). Everything is behind
`triSetLighting night=1` (default 0 = shipped behaviour) plus knobs; the recommended setting is

`night=1, nightP1=0.3, nightP2=0.85, nightMoonSat=0.5, nightSpot=1.5, nightCamLights=1, nightLampWarm=0.7` (haze 0, bounce 0)

| piece | where | knob |
|---|---|---|
| Moon as key light: the sun slots carry the moon once the sun is 3 deg under, so the shadow map (moon shadows), terrain horizon, glossy and cloud paths follow the moon. Radiance 0.05 x phase x height (about 14x the shipped moon fill); chroma (0.80, 0.88, 1.0) mixed half way to white | `EngineGL33_Lighting.cpp`, `SceneDraw.cpp`, `SceneShadowPass.cpp` | `nightMoon`, `nightMoonSat` |
| Moonlit sky fill tied to the moon (a quarter of the key), half desaturated | same | `nightSkyFrac` |
| Physical local lights: lamps I = 4 x start x P1 with 1/d^2 from the bulb (no flat disc); headlights get a low-beam pattern (wide horizontally, sharp cut-off 0.9 deg above the axis, hot band 1.7 deg below) of peak 4000 x P1 x spot | shader `localLightsNight` | `nightP1`, `nightSpot` |
| Self-levelling headlight aim: the beam is aimed so its hot band lands on the ground 25 m ahead (`Landscape::SurfaceY`) | `UploadLitLights` | on with night=1 |
| Rough diffuse for local lights (Oren-Nayar sigma 0.5, Fujii form, normalised to 1 at normal incidence): gravel and asphalt scatter back toward a light near the eye, so a road under a grazing low beam reads lit | shader | none |
| Warm lamps: lamp colour blended toward ~2700 K (luminance kept), headlights half | `UploadLitLights` | `nightLampWarm` |
| Camera-centric light list: one list of the 8 lights strongest at the camera for every draw, so terrain, roads and objects agree | `UploadLitLights` | `nightCamLights` |
| Purkinje shift: below ~0.06 linear the colour fades toward a rod-weighted grey with a slight cool cast; lamp pools stay coloured. The classic night-eye wash is skipped for lit fragments | shader `nightPurkinje` | `nightP2` (strength) |
| Smoke and particles (CPU-lit decals) lit by the same inverse-square lights plus moon and sky, summed in linear light | `Lights.cpp`, `Object.cpp` | follows `nightP1` |
| Stars as soft round splats with a faint per-star colour temperature, instead of 2x2 dots the render-scale resolve smeared away | `EngineGL33_DrawShared.cpp` | night=1 |
| Volumetric haze: equiangular single scattering of each local light (half res, post pass, shaft target); lamp halos with density falling off within ~1.5 m of the bulb | `EngineGL33_PostFX.cpp` | `nightHaze` (failed, keep 0) |
| Ground bounce of lamps: the pool under a lamp as an upward Lambertian emitter tinted by the horizon map's ground colour | shader `nightGroundBounce` | `nightBounce` (failed, keep 0) |
| Debug views | shader | `nightDebug` 1 count/local/terrain, 2 dist/kd/forced, 3 point irradiance, 4 beam pattern |

Bug fixed on resume: the earlier agent's run3 crashed (0xC0000005 in `EngineGL33::EndWorldScene`)
right after a view change, because the haze kept raw `Light*` pointers from the draws and
dereferenced them after the fire object was deleted. The frame list now stores copies of the light
descriptions and is emptied whenever the haze reads it. Runs 4-10 had no crash.

## Runs on disk

| run | contents | outcome |
|---|---|---|
| run1-run3 | earlier agent: v1-v3 (moon key, physical lamps, stars, haze) | run3 crashed after town_night (bug above); its `tri/` has the 5 town_night shots |
| run4 | v4: P1 0.08, Purkinje, bounce | first real night look; headlights and lamp pools invisible |
| run5 | v5: camera list, debug 1 | pools still too weak; found a knob-reset leak in my generator (fixed) |
| run6 | v6: P1 0.25, warm lamps, ground-slope aim, debug 2/3 | terrain lighting verified; verges lit, road not |
| run7 / run7dbg | v7: rough diffuse, spot 1.5, tighter haze; debug 4 | beam aimed above the 14 deg road |
| run8 | v8: self-levelling aim, P2 0.85; 9 views incl. NV, moon sky, bounce A/B | final look on the town and hill views |
| probe_roads | road centres and ground heights from `gmRoadsNear` | found a flat straight road west of the town (0.4 m over 121 m) |
| run9 | flat road: v8, headlights x2, haze, debug 4 | headline images |
| run_timing | classic, remastered, v8, v8h alternated 2-3x per view | cost estimate |
| run10 | v9 shader on the 5 key views | final BEST images for those views |

Each run folder has `shots/`, `cmp_*.png` grids, `timings.md` / `timings.json` and the raw `tri/`
tree; its `game_stdout.log` shows the override loading and the `NIGHT:` lines. The research test
files are copied under `tests\` (`gen_night_test.py` generates them).

## What the real captures show (honest)

- **Moonless country road (run9/run10)**: the strongest result. Classic and shipped Remastered
  light the whole landscape grey at a moonless 23:00. The prototype is dark with a starfield; the
  low beam lays a warm pool on the road with a hot spot 20-35 m ahead, spill on the verges and a hard
  top edge, and the town's lamps glow on the horizon 260 m away.
- **Moonlit views**: moonlit ground is a cool, low-saturation grey (road_head ground hue 220 deg,
  r/b 0.92), tree crowns are silver-edged, the jeep casts a moon shadow, stars are visible.
  Classic draws moonlit trees black against a black sky (moon_sky).
- **Town**: dark moonlit square, warm lamp-lit facades, fire-lit smoke instead of white smoke. The
  ground pools are present but soft, because the lamps are 8-12 m high: a correct night, less
  dramatic than the road.
- **Ground colour** (run8, lower 28 percent of the frame): Classic town ground hue 52 deg, sat 0.14;
  shipped Remastered sat 0.25 (the mustard); prototype hue 46-48 deg, sat 0.11-0.17 at about half of
  Classic's brightness. No new mustard; the lamp-lit lawn keeps Classic's olive-beige.
- **Night vision**: mean abs pixel difference shipped Remastered vs prototype 1.7/255, Classic vs
  shipped Remastered 2.0/255 (the difference is the animated fire and smoke). NV is unchanged.
- The jeep's rear reads pale grey in the moonless flat-road views; Classic and shipped Remastered
  show the same, so it is not introduced by the prototype (cause not traced).

## Failures and dead ends (with evidence)

- **Volumetric lamp haze: failed.** At useful strengths it turns the sky around the town into a
  brown, lamp-coloured wash (`run6/cmp_town_lamps_haze.png`, `run7/cmp_town_lamps_haze.png`), the
  look the owner rejects. Localising the density to the bulb (`exp(-d/4)`, then `exp(-d/1.5)`)
  shrinks it but leaves a soft brown disc in the sky (run8 v8h). For headlights it reads as a light
  mist over the near road (`run9/cmp_road_flat_drive_moonless_haze.png`): not ugly, not worth
  0.1-0.24 ms. Keep `nightHaze=0`.
- **Lamp ground bounce: no visible effect.** A/B in run8 (`cmp_town_lamps_bounce.png`): mean abs
  difference 0.21/255, 1.35 percent of pixels change by more than 4 levels. A 0.2-albedo lawn under
  a 10 m lamp bounces very little. Drop it.
- **Hill road (road_head, road_drive)**: Abel's test road climbs at 14 deg and is concave. A sharp
  low-beam cut-off is very sensitive to aim: aimed along the local slope the beam passes over the
  road (road dark, verges lit, run6/run7); self-levelling to the ground 25 m ahead helps but the
  near road still sits near the cut-off (run8). On a flat road (run9/run10) the same beam is right.
  Production wants a softer cut-off or an aim that follows the body plus a down-tilt, tested on hills.
- The classic per-light "ambient" added inverse-square light with no direction; v9 drops it for
  headlights and caps it for lamps.

## GPU cost

960x640, render scale 1.5, 4x MSAA, timestamp queries. The machine was shared with up to three other
agents' games, so single numbers swing by up to 40 percent. Alternating run
(`run_timing/timing_summary.md`, medians of 3):

| view | shipped Remastered | prototype v8 | v8 + haze |
|---|---|---|---|
| town_lamps | 3.90 ms | 4.70 ms | 4.68 ms (haze 0.10) |
| town_night | 20.37 ms | 22.32 ms | 21.75 ms (haze 0.12) |
| road_flat_drive_moonless | 13.89 ms | 17.29 ms | 20.01 ms (haze 0.24) |

Estimate: +0.8 to +3 ms GPU, from the per-fragment light loop now running up to 8 lights with an
`atan`-based beam pattern and rough diffuse on every lit fragment (the camera list hands every draw
all 8 lights). Moon shadow pass 0.03 ms. Haze 0.10-0.24 ms.

## What a production version needs

1. A real light list: clustered or tiled culling (lights in an SSBO/UBO, per-tile indices) instead
   of 8 uniforms shared by every draw; removes the cost on far geometry and the 8-light cap.
2. Lamp and headlight shadows (spot/cube shadow maps for the 2-4 lights nearest the camera); local
   light still leaks through walls, as in Classic.
3. Beam aim from the vehicle body with a softer cut-off, tested on hills; per-vehicle light colour
   and intensity from config instead of one `nightSpot`.
4. A night exposure and brightness policy agreed with the owner (the prototype keeps exposure 1 at
   night and sets brightness through the moon key and lamp intensities).
5. Classic bit-identity: every behaviour change is gated on `night >= 1`; the always-on edits are
   unused `w` lanes of four LitConstants slots and the light-upload refactor (same output when the
   night variant is off). Keep the gate and add a unit test.
6. Fold the shader into `EngineGL33_LightingShaders.cpp`; delete the haze and bounce code and the
   debug views.

## Risks against the owner's taste list

- "Only darker": the frame is darker, but it gains structure (stars, beam, moon shadows, warm vs
  cool). Moonless country is very dark away from the beam; that must be accepted for playability
  (moonlit views stay readable at about half Classic's ground brightness).
- "Blue haze / wash": avoided in the recommended setting (haze off; moon chroma half way to white;
  slight Purkinje cast). The haze variant does produce a wash and is rejected.
- "Mustard/khaki ground": lamp-lit lawns keep Classic's olive-beige (sat 0.11-0.17), less saturated
  than shipped Remastered (0.25); lower `nightLampWarm` if it reads khaki.
- Warm lamps are a choice: Abel's lamps are configured white; real 1985 mercury lamps were cooler.
- Night vision and Classic are unchanged.
