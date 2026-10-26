# look-fog: volumetric fog and atmosphere (round 3, after critic round 2)

Lane of the visual-overhaul workflow (direction: modern photoreal, deep atmosphere over
distance, volumetric fog). Built and observed in the scratch copy
`D:\Arma_CWA_worktrees\look-fog`. Nothing is committed.

- **Patch**: `lane.patch` (`git diff --binary 06c7e4d`, new files added with `git add -N`
  first). Base **06c7e4d** (graphics-remaster with B8 air pass and B3 night landed). The
  scratch copy's HEAD is still 7b7e3e6; its working tree is 06c7e4d plus the lane (as in
  round 2). `git apply --cached --check` passes against 06c7e4d (temporary index).
- **Moved on since**: graphics-remaster HEAD is now **fec1075** (B6, B7 and a combat-light
  commit on top). Against fec1075 a 3-way apply conflicts in 13 files (PostStages.cpp/.hpp,
  EngineGL33.hpp, EngineGL33_Lighting.cpp/.hpp, EngineGL33_LightingShaders.cpp,
  EngineGL33_PostFX.cpp, EngineGL33_PostShaders.cpp, glsl_extract.py and four unit tests'
  count lines). Not merged.

## Decision on B8: supersede, keep it landed (unchanged)

`volFog` (on by default) runs on B8's Air stage in B8's place; B8 runs whenever `volFog=0`.
The gates turn `airBeams`, `airHaze` and `valleyMist` off while the fog's gate is on.
The comparison column "B8 air (landed)" is `remastered,volFog=0` on this tree.

## What changed in round 3 (by critic point)

| Critic point | What was done | Observed (final run `runs\20261018-203720_final5`) |
|---|---|---|
| Orange rectangle over the far sea in dawn_valley (blocking) | Cause found by elimination: not the shadow, canopy, terrain or cloud visibility and not the near field (six override runs, `view\r3dbg.png`). It was the apply's highlight expansion: the clipped sun path on the far sea (display 1.0) was expanded up to 8.6x linear and punched through the mist with hard edges. Now the inverse shoulder stops at a peak of 0.97 and is weighted by the fog's transmittance; what the cap removes is given back in proportion to T, so an unfogged pixel is unchanged. | Gone: `crop_dawn_valley_box_r2_vs_r3.png`. dawn_valley, coastline_sunset and sunrise_ridge re-shot with the sun just above the horizon. |
| town_fog_night: flat sepia sky (blocking) | The sepia came from the near-field lamp light (an override run with the near field off took the upper-left sky from (29,26,22) to (4,4,6)). Lamps now: phase 0.55 HG(0.85) + 0.4 HG(0.6) + 0.05 isotropic (tight aureole, little light every way), reach 40 m with the window starting at 0.1 R^2, colour 60 % desaturated (the lamp head reads white). Headlights keep the milder phase. Fire 15 at 25 m. | Sky upper left (25,25,26) std 3.0, neutral grey (B8 (15,15,19)); upper right near the lamp (59,55,52). `crop_town_fog_night_r2_vs_r3.png`. |
| Lamp head a flat white disc, small halo | The aureole above, plus a new **night glow**: the frame's light (highlights expanded back, so a lamp head counts as several times white) blurred at 1/4 and 1/16 resolution, laid into the fog in front of the nearer of the pixel and the light's own luminance-weighted depth, gain 1.6 x night x fogLights. Walls lit by lamps tint the fog in front of them; dark sky stays dark. | Lamp has a soft halo and glow; Jeep body readable, headlights two bright points with a lit road patch. |
| Houses 40 to 100 m untouched by the fog at night | Weather fog at night is denser at the floor by (1 + 1.5 x night) for setFog (kNightFogBoost), and the glow gives it a lamp-tinted air light. | Middle house block (x 420..780, y 220..560) lum std 22.7 (B8) to 17.2: about a quarter of its contrast lost, short of the third asked; right house block (x 1150..1500, y 370..560) 32.7 to 25.8. |
| Stars through the fog | `FogStarKeep`: the wet air's optical depth straight up from the eye (weather fog layer, mist, humid air); the star alpha in `DrawPoints` is scaled by it (0 above tau 0.3). | Log `stars=0.00` in town_fog_night; 1 on a clear night (unit test). |
| Clear-noon milky near field (noon_vista) | Clear haze near thinning now keeps 12 % at the eye and reaches the whole only at 3.5 km (was 25 % to 700 m); haze 4.0e-4 far; Rayleigh 1.25 x physical (was 2.5). Under a grey deck, rain or fog the thinning ends at 800 m with 30 % at the eye (so overcast_heavy keeps its fog). | 1 km forest (x 0..400, y 730..900) (63,70,70), B8 (60,68,70), round 2 (94,106,115). Mid grass (x 700..1100, y 620..720) (83,93,84), B8 (80,91,81), round 2 (105,119,124); its lum std 12.3 (B8 13.3). Far ridges fade blue: `crop_noon_vista_r2_vs_r3.png`. |
| Mist veils the hilltop camera (dawn_valley) | The humid ground air is cut above the fog floor + mist layer + 20 m (fade over 30 m); the mist already falls off above its layer. | Bottom-left 700x380 std 23.9 (B8 24.8, round 2 17.7), p5 46 (B8 43); frame min 25 (B8 19). The hilltop near field is crisp, the valley holds the mist. |
| Trunk billboard reads as a pale picket fence (overcast_heavy); forest side faces as stripes | Gap closing in the apply: per pixel, the nearest depth on each side across and down within 9 px; if both sides of a direction are nearer than 0.8 x the pixel's depth, the pixel is fogged at the further of the two (a hole in nearer geometry takes that geometry's fog). Silhouette edges have nearer geometry on one side only and keep their own depth. | Trunk row (x 1450..1900, y 500..570) (80,83,83) std 4.6, darker than the canopy band above it (116,119,119). `crop_overcast_heavy_forest_r2_vs_r3.png`. |
| Lower sky band below the horizon line flat | Two parts: (1) the forward fog edge (the classic fog kept for the world's end) starts at 0.93 of the fog end, not 0.82, so far terrain keeps its relief until the volume fades it; (2) sky and the far horizon polygon under the horizon are a virtual far surface where the ray meets sea level, darker than the dome fill and in value-noise patches (1.8 km and 520 m), fading into the fill with the haze between the far end and that point. | noon_vista band (x 200..400): rows 450 to 520 go (161,183,205) to (154,176,199) (B8 flat (159,176,190) from 430 to 510). A gentle gradient, small at thumbnail size: the far sea under 5 km of haze stays close to flat. |
| Nothing visible in coastline_sunset, sinai_desert_noon, forest_edge_golden | (1) The sun's forward lobe in the volume: HG(0.6)'s excess over isotropic times the air's luminance in the sun's tint, gain 0.45, on the haze (thinned only near the eye: 10 % at 30 m to whole at 800 m) and the dust, shadowed by cascades, canopy, terrain and clouds. (2) In the sky past the far end: the same lobe over 40 km of clear air, faded above the horizon with exp(-sin(el)/0.05). (3) Dust for arid maps: aridity from the mean red of the terrain's ground tint (log: Abel 0, Eden 0, Sinai 1.00), 1.2e-3 /m at the ground, scale height 250 m over the ground, warm tint, 30 % near the eye to whole at 1.5 km; the sky band takes 20 % of it. | coastline_sunset: a warm glow band over the horizon toward the sun and on the sea under it, obvious at 480 px. sinai_lowsun_fp: warm haze on the left hills toward the sun. sinai_desert_noon and forest_edge_golden: still within 0.005 mean of B8. The desert view looks up a dune slope 1 km away (the sky rows over the dune line are 8 degrees up, from a debug override), so neither the dust band nor the volume dust has distance to show; forest_edge_golden has no long view. |
| Forward fog term for smoke and particles | **Not built.** Particles are pre-lit blended draws without depth; the apply fogs the pixel by the opaque depth behind them. A correct term needs the particle shader to read last frame's integrated volume and half-res depth and pre-compensate its colour (p' = (p - S(p to bg)) / T(p to bg)); that adds a 3D sampler and a depth sampler to the world programs' prelude and two texture units the A3 unit table has no room for. | The smoke in town_fog_night sits in a neutral fog now, so the mismatch is less visible, but it is not fixed. |
| HUD/optics capture | `tests\foglane_optics.test.sqf`, `optics_hud.png` (player view with the fog, then the weapon sight 1, 2 and 30 frames after the switch; then the same with volFog=0). | The player view is fogged normally. Under the weapon-sight overlay the fog's gate is off ("optics overlay"), so the volumetric fog disappears entirely while aiming: frames 1, 2 and 30 show only the classic edge; B8 behaves the same. No one-frame artefact was visible at frame 1. A real gap: aiming through a sight in mist shows clear air. |
| Re-shoot the fast pan with the reworked ring | `runs\20261018-204131_motion3`, `motion_frames.png` (frames 1, 8, 16, 24 of the pan; 1, 10, 20, 30 of the Jeep). | No trails behind the Jeep's headlights, no haze lag in the pan, no vertical banding in those frames. Not checked frame by frame. |

## Pipeline now

1. `psFogRing` (256x1, unchanged).
2. `psFogInject` (froxels 160x90x64 at 12 px): haze, Rayleigh, dust, humid ground air
   (cut above the mist's reach), weather fog, valley mist; sky-matched light (ring), sun
   share shadowed, plus the sun's forward lobe on haze and dust.
3. `psFogIntegrate` (unchanged).
4. `psFogNear` (half res): sun beams in wet air; lamps, headlights and fires in closed form.
5. `psFogGlow` (night with wet air only): 4x4 down to 1/4, Gaussian across and down, down to
   1/16, Gaussian across and down, combine (55 % quarter, 45 % sixteenth). Seven small draws.
6. `psFogApply` (full res): gap closing, volume lookup, sky rules (horizon fade, wet share,
   weather beyond the far plane, virtual far surface under the horizon, lobe and dust past
   the far end), near field, night glow, capped expansion with identity residual, shoulder,
   dither. The apply now pastes the LitConstants block (the sun's direction and colour).

## Switch and knobs (A1 registry, unchanged keys)

`volFog` master; `fogDensity`, `fogQuality` (0: 16 px / 48 slices / 12 steps, 1: 12 / 64
/ 20, 2: 8 / 96 / 32), `fogMist`, `fogSun` (beams and the forward lobe), `fogLights`
(lamp light in the air and the night glow). No new key this round.

## Results

`contact_sheet.png` (Original (Classic) | Overhaul, 480 px panels), `contact_sheet_3way.png`
(classic | B8 | volFog), `hero\<view>.png` (960 px panels), the four `crop_*_r2_vs_r3.png`,
`optics_hud.png`, `motion_frames.png`. Numbers: `metrics_final.txt` (metrics.py on the final
run). Looked at at thumbnail size against the Original: obvious in sunrise_ridge,
coastline_sunset, overcast_heavy, town_night, noon_vista, dawn_valley, pine_rays,
town_fog_night, sinai_lowsun_fp; visible in overcast_broken; small in forest_edge_golden,
noon_fields and sinai_desert_noon (most of what differs there is the other lanes' lighting,
not the fog). Against B8 the lane's own change is 0.004 to 0.005 mean in forest_edge_golden,
overcast_broken, noon_fields and sinai_desert_noon.

## Cost (RTX 4060 Ti, 1080p, scale 1.0, 4x MSAA, TAA on)

`runs\20261018-204317_timing3`, `timing_r3.md`, GPU clock log `timing3_gpu.csv` (62 rows,
clock 1935 to 2790 MHz, median 2775). Not quiet: a game of another session ran during 11 of
the 62 sampled rows. Not clock-locked (locking GPU clocks is a system setting change this
lane may not make). Air section from `triGpuTimes`, three interleaved rounds at quality 1:

| view | air ms (3 rounds) | q0 | q2 |
|---|---|---|---|
| sunrise_ridge | 0.93, 0.72, 0.48 | 0.40 | 1.01 |
| dawn_valley | 0.64, 0.67, 0.66 | 0.47 | 1.36 |
| pine_rays (beams) | 1.11, 1.10, 1.09 | 0.78 | 1.79 |
| town_fog_night (lamps + night glow) | 0.73, 0.78, 0.73 | 0.64 | 1.21 |
| noon_fields | 0.48, 0.43, 0.41 | 0.31 | 0.82 |

Worst at quality 1: 1.11 ms (pine_rays). town_fog_night read 0.62 ms in round 2 (without
the glow; another machine state). A 3060 is not measured; estimated 1.5 to 2x on these
fill-bound passes, so about 0.6 to 2.2 ms at quality 1.

## Tests

- Build 0 errors. clang-format clean on VolumetricFog.*, EngineGL33_PostFog*.cpp and
  test_volumetric_fog.cpp. (Python edits had written CRLF into 11 files; converted back to
  LF; roadmap.yaml kept as its CRLF checkout.)
- Gate tags `[fog],[PostStages],[FeatureSwitches],[FeatureGates],[Shaders],[AirMedium],
  [remaster],[StateCacheAudit]`: 223 cases, 721,643 assertions, all pass (`unit-gate.log`),
  including the glslang compile of every post program and the GL state-cache audit. Full
  suite: 3303 cases, 6 failed, the known localisation cases (`unit-full.log`).
- New test case "round 3 medium rules" (clear vs grey near thinning, dust and rain, dust
  over the ground, night fog boost, ground air on a hilltop, star keep, aridity); the shader
  constants test checks the new near, ground-air and dust constants and the glow program;
  post shader count 25.
- In game: no shader compile or link error in any run; `smoke_abel` passed in the final run
  (4/4).
- A log line `GL33 fog: eye (x,y,z) ground floor layer` every 600 frames was added while
  chasing the dust; it is still in the patch.

## Tried and dropped this round

| What | Why dropped (observed) |
|---|---|
| Sky lobe over the full haze column (no elevation fade) | a large pink disc round the sun in coastline_sunset, sunrise_ridge, dawn_valley and sinai_lowsun_fp (`view\r3b.png`) |
| Volume lobe gain 1.0 with HG(0.7) | dawn_valley's right side washed to (248,213,189), spit and sea lost |
| Lobe on the near-thinned haze only (gain 0.3) | coastline_sunset unchanged from B8: the sea is within 2 km and the thinned haze gave about 0.04 of light there |
| Dust by absolute height | no dust on Sinai (the plateau is 130 m up; the 250 m scale height took most of it away) |
| Dust 3.5e-4 and 7e-4 /m | no visible change in sinai_desert_noon |
| Under-horizon surface only for depth-1 sky | noon_vista's band is the far sea geometry inside the classic edge, not sky; moved the edge out as well |
| Lamp phase with 15 % isotropic, window from 0.4 R^2 | the sky behind the street brown-grey far from the lamp |

## Known problems and risks

- Patch conflicts with fec1075 (13 files), see top.
- No forward fog term for smoke and particles (see table).
- Under a weapon-sight or optics overlay the volumetric fog is gated off and the scene shows
  clear air with the classic edge (`optics_hud.png`).
- sinai_desert_noon and forest_edge_golden still look like B8 at thumbnail size.
- town_fog_night: houses at 80 to 100 m lose about a quarter of their contrast, short of the
  third asked; the Jeep's headlight points are brighter than in round 2 (Jeep box lum std 57
  vs B8 45).
- dawn_valley's sun side is a bright warm haze (right block mean (244,213,190), std 15.9 vs
  B8 35); it reads as morning glare, a critic may find it too strong.
- The gap closing can lift fog off the far landscape seen through a gap of up to about 18 px
  between two near objects (two near trees close together).
- Not measured on a quiet machine or on an RTX 3060.

## Files here

`lane.patch`, `NOTES.md`, `contact_sheet.png`, `contact_sheet_3way.png`, `hero\`,
`crop_*.png`, `optics_hud.png`, `motion_frames.png`, `metrics.py`, `metrics_final.txt`,
`meas.py`, `cropgrid.py`, `gen_fog.py`, `gen_motion.py`, `make_sheet.py`, `timing.py`,
`timing_r3.md`, `timing3_gpu.csv`, `run.ps1`, `runvar.sh`, `build.ps1`, `unit-gate.log`,
`unit-full.log`, `tests\` (incl. `foglane_optics.test.sqf`), `runs\` (junction to E:).
