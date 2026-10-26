# Look lane "light": photoreal light and camera (switch `photoLight`), critic round 2

Scratch copy `D:\Arma_CWA_worktrees\look-light`, detached at **c36ee4b** (unchanged from round 1). Nothing
committed, no branch. Patch: `lane.patch` (`git diff --binary`, new files added with `git add -N`;
`git apply --check` passes on a c36ee4b tree). The graphics-remaster tip moved to **cb3dd0d** (B7, wet
materials) during this round; the patch does not apply cleanly there (see Problems). Build
`arma_CWR/build/look-light-rwdi`, game `arma_CWR/dist/look-light-rwdi`, junction `RemasterUD-look-light`.
Round 1's deliverables and notes are in `round1/`.

## Files

- `contact_sheet.png`: Original (Classic) | Overhaul at 480 px, ten views (the eight heroes, `woods_pine2_evening`,
  `blue_hour_coast`). `contact_sheet_3col.png` adds the branch tip (`photoLight=0`) in the middle,
  `contact_sheet_r1_r2.png` puts round 1 in the middle.
- `final/<view>__{classic,off,on}.png` (1920 x 1080, one game per world, same camera, clock, weather, props
  and clouds), `final/<view>__pair.png`, `final/udcap.log` (shot log with the camera readout).
- `crops/`: the critic points at full or contrast-stretched scale (`coast_glow_gain4`, `sunrise_cypress`,
  `sinai_sky`, `night_lamp_fire`, `blue_hour_band_gain3`, `noon_hills`, `woods_canopy`; columns Classic or tip,
  round 1, round 2 as labelled), and the motion captures (`motion_*`).
- `timing.md`, `timing.csv`: GPU and CPU cost, off and on interleaved three times on seven views.
- `tools/`: `metrics.py` (the numbers below), `crop.py`, `timing_summary.py`, `motion_report.py`; `make_r2.py`
  builds `final/` and the sheets from a run.
- Capture scripts in the patch: `tests/rendering/captures/gen_look_light.py` and `gen_look_motion.py` and the
  generated `look_base_*` (the round 0 baseline), `look_r2final_*` (this round's hero shots: modes classic, off,
  on), `look_timing3_abel` (timing) and `look_motion_abel` (motion). Run with `run-captures.ps1 -Tests ...
  -GameDir dist/look-light-rwdi -DataJunction <yours>`.

## The critic points, what changed, what the shots show

Numbers from `tools/metrics.py` on `final/` (Classic in brackets, round 1 in braces; luma or RGB 0..255).

1. **CPU stall in the post pass.** The meter readback is now asynchronous for real: a ring of four pixel-pack
   buffers, a slot whose fence has not passed is never reused (that frame's meter is skipped and counted),
   fences are polled oldest first and the walk stops at the first one still in flight, every passed slot is
   freed but only the newest is copied out and metered, the readback is in the target's own format (RGBA16F
   read as half floats, converted on the CPU; it was read as GL_FLOAT), and the percentiles come from a
   1024-bin histogram (1/32 EV) instead of two sorts. The camera times itself (`| cam ... cpu=collect/queue
   skip=` in `triFeatureState`): collect 0.04 to 0.06 ms, queue 0.03 to 0.09 ms, no skipped meters in any shot.
   `drw:post` (CPU, median of three): off 0.23 to 0.44 ms, on 0.37 to 0.49 ms, so +0.05 to +0.16 ms (round 1:
   +0.5 to +2.2 ms). CPU frame and FPS are in `timing.md`; they swing with the other lanes' games on the GPU (see
   Cost).
2. **Night exposure and contrast.** `camNight` -2.5 EV more (now -3.8), the night's lift capped at -1 EV (was
   +0.7). Ground (35,34,34) [(130,127,111)] {(57,59,66)}; frame mean luma 39 [97] {52}. The lamps, fires and
   headlights take `camLocal` 2.5 x at full night, so they stand out of the darker street.
3. **Street lamp.** Its flare keeps 0.45 of its alpha at night (round 1: 0.15; by day 0, below): a halo with a
   faint star round the bulb. A street lamp's physical light is 2 x stronger under the camera
   (`kCameraLampGain`, on top of `camLocal`), and fades in from 1.5 to 4.5 m from the bulb (the head's shade), so
   it lays a warm light on the house walls, the bushes and the road beside it while its own pole is dark below
   the head (`crops/night_lamp_fire.png`; round 1's pole was lit white from top to bottom). Its classic ambient
   term keeps 0.25 under the camera. The pool on the road right under the lamp is hidden behind the bushes from
   this camera.
4. **Fire and smoke.** The rod shift now spares warm light (it fades out as (r - b) / peak goes from 0.08 to
   0.35), so the plume is warm through, with no orange particles among grey ones; additive draws take 1.35 x their
   codes at full night. Flame r-b>60 pixel count in the fire box: 3333 in the final shot [5316], 3122 to 5761
   over this round's night shots (round 1: 782; the particle timing moves it from shot to shot). The fire's
   light on the stones and the ground is warm (`camLocal`).
5. **Aerial perspective.** Under the camera the air pass's haze is 6 x B8's density (`camAerial`; a 6 km
   visibility at sea level, B8's scale height): ridges 1 to 3 km out go paler and bluer toward the horizon sky
   (`crops/noon_hills.png`; very plain in `overcast_broken` and `sunrise_ridge`). The look-fog lane's volume owns
   the haze once it lands (its notes: it replaces B8's haze); this knob is the camera's until then.
6. **Bright horizon band.** By day the clear sky within about 6 degrees of the horizon is paler (60 % toward its
   grey) and brighter (+45 %) than the table gives (`camSkyLook`). Sinai zenith (97,123,158), horizon
   (127,154,185) [(158,184,218), (173,197,216)] {horizon (119,150,184)}; noon zenith (101,130,170), horizon
   (122,150,183).
7. **Overcast.** A weather veil from the classic sky-through (`CameraWeatherVeil`: 0 at the clearest sky, 0.74
   at overcast 0.5, 1 under a deck): 60 % of the direct sun goes into an even grey sky light, the clear dome
   pales to a light grey (75 %), the clouds grey a little, the scene saturation drops by 0.3, no cool shade. The
   view reads as a grey, hazy overcast day: sky saturation 0.10 [0.25] {0.27}, MAD to Classic at 480 px 20.5
   {12}.
8. **Noon ground.** A scene brighter than a sunlit plain keeps 0.45 of its extra brightness (was 0.85), and the
   curve's toe is 1.4 (was 1.3): noon median luma 157 [173] {163}; the ground's p99 193 [194] {203}; the soil's texture shows. The field still reads pale in the thumbnail (its texture is pale: Classic's too).
9. **Golden hour.** The white balance follows the light by 8 % under 8 degrees of sun, rising to 55 % at 40 (was
   35 % at a low sun), and the rig's sun takes a summer aerosol on its own path (`kSunAerosolTau` 0.08 at 550 nm,
   Angstrom 1.3, per air mass past the zenith's), so a sun under 20 degrees is golden on what it lights. What
   the sky lights more than the sun (the world's sky light share in the frame's alpha, 0.2 to 0.6) leans cool at
   a low sun (`camShade`, 0.55 toward (0.84, 1, 1.3), full under 15 degrees, none from 45). lowsun_fp: warm
   beige sunlit gravel, blue shadows; forest_edge_golden: amber grass, cooler shade.
10. **Cloud edges.** The painted sky texture is sampled through a cubic B-spline under the camera (four
    bilinear taps): the bilinear kinks that the cloud mask turned into stair steps are gone, the clouds' edges
    are soft (`crops/sinai_sky.png`). Some blockiness from the texture's own coarse content remains visible at
    1:1 on the thin streaks.
11. **Sunset glow streaks and wedge.** The light flares keep no alpha by day (the starburst rays and the
    straight edge of the flare quad), the screen-space shafts do not run under the camera (their radial march
    drew the fan), and the air pass keeps 0.3 of its sun beams on the sky within about 20 degrees of the sun. In
    `crops/coast_glow_gain4.png` (contrast x 4) the fan, the hatch round the disc and the wedge are gone; a very
    faint diagonal grain remains in the yellow glow at x 4 (the air pass's dither), invisible at 1:1.
12. **Sunset upper sky and sea.** At a low sun the upper sky turns toward the table's zenith hue (blue) by up
    to 0.6, from 3 degrees up to full at 26, none within about 10 degrees of the sun (its golden glow stays):
    coastline frame top (148,147,155) {(133,131,133)}, a grey-blue; lowsun_fp's sky far from the sun (102,118,140).
    The sea reflects the same sky (`skyReflection` takes the look); near the horizon it mirrors the golden band.
13. **Sunrise silhouettes.** The air pass's haze light toward a low sun is capped for near surfaces at 1.5 x the
    horizon sky of the sun's side azimuth (rising to 8 x by 4 km), and the sun's beams on a surface fade in with
    distance (full at 1.5 km, no contrast boost for near surfaces): the cypresses and the bush are dark,
    complete silhouettes against the glow, no washed tops, no bright block (`crops/sunrise_cypress.png`). White
    (255,255,255) area by the sun: 3 x 9 px (the disc behind a cypress) [0] {84 x 76}; no starburst.
14. **Blue hour.** A twilight belt within about 5 degrees of the horizon, with the sun from 5 degrees up to 10
    under: brighter and warmer toward the sun's bearing (x 1 + 1.1 cos^6 of the azimuth - 0.45). Band luma across
    the frame at 1 degree up: (93,61,58) at the left edge, (152,94,80) at the sun's bearing, (121,77,69) at the
    right edge (round 1: flat). Its lower edge into the sea is still hard at the frame's sides (the far sea there
    stays dark: (42,36,40) under (88,58,56)); see Problems.
15. **Woods.** Besides the cool shade, dark tones keep 0.7 of their colour by day, rising to all of it from 0.02
    to 0.2 of scene light (`camDark`). Pine canopy (52,47,34), g/b 1.35 [1.29] {1.71}: green-dark, warm only
    where the sun reaches.
16. **Motion.** `look_motion_abel` pans lowsun_fp's camera 90 degrees across the low sun (1.5 degrees a frame)
    and tilts the woods camera from the shaded floor up through the crowns into the sky and back (1 degree a
    frame), off and on, a log line every frame and a shot every 10. `crops/motion_*_ev.png`: the exposure follows
    its target smoothly with the 1 s time constant (1.5 s opening up), no oscillation; after the moves it settles
    to within 0.01 EV. The largest per-frame steps (0.09 to 0.22 EV) all fall on the frame after a screenshot (the
    adaptation runs on wall-clock time and a screenshot frame takes long); between them the steps are at most
    0.04 EV a frame. Mid-pan frames show no ghosting at edges (`crops/motion_pan_frame50_on_off.png`). TAA runs
    after the camera's composite, on the tone-mapped display frame (`PostTemporal` reads the composite's output,
    `taaInTex`), so an exposure change needs no rescale of the history: it is the display image the history
    holds.
17. **SSR cost.** Timed on coastline_sunset (water view, ssr gate on): SSR section 0.89 ms off, 0.90 ms on
    (median of three); on blue_hour_coast 0.43 and 0.54 ms. The decode and re-encode cost at most about 0.1 ms.

## Knobs (all under `photoLight`, 18)

`camEv` 0.3, `camAdapt` 1, `camCurve` 0, `camContrast` 1.4 (was 1.3), `camShoulder` 0.95, `camWhite` 32,
`camSat` 1.15, `camWb` 0.45, `camSky` 1, `camSkyHue` 1, `camNight` -3.8 (was -1.5; range -5..1), `camVignette`
0.18, `camBloom` 1, `camHaze` 6, `camScotopic` 1, `camAdditive` 1 (range now to 2), new `camAerial` 6, new
`camLocal` 2.5. Constants (PhotoCamera.hpp, LightRig.hpp): `kBrightKeep` 0.45, `kExposureNightMaxLog2` -1,
`kWhiteBalanceShareLowSun` 0.08 / `HighSun` 0.55, `kCameraShadeCool` 0.55, `kCameraDarkSaturation` 0.7,
`kVeilSaturationDrop` 0.3, `kVeilSunLoss` 0.6, `kSunAerosolTau` 0.08, `kCameraLowSunSkyBlue` 0.6,
`kCameraFlareNight` 0.45, `kCameraAdditiveNight` 1.35, `kCameraLampGain` 2, `kCameraLampAmbient` 0.25,
`kCameraNearHazeCap` 1.5, `kCameraSkyBeamsBySun` 0.3, `kCameraBeamFullDistance` 1500, `kScotopicWarmFrom/To`
0.08 / 0.35. New LitConstants slot `CameraLook` at 80 (`Count` 81; INTEGRATION-PLAN 3.3 updated): x weather
veil, y lamp ambient kept, z lamp gain, w low-sun sky blue.

## Tried and dropped this round

- Cool shade by luminance (dark tones under 0.03 to 0.3 of scene light): it cooled lowsun_fp's sunlit gravel
  (r/b 1.06) and barely touched the pine crowns (g/b 1.78); replaced by the sky-light-share mask.
- The low-sun zenith hue everywhere above 2 degrees: the sunrise glow went lavender; it now spares the sky within
  about 10 degrees of the sun (a 20 degree exclusion left the coastline's upper sky grey).
- `nightLamp` 4 alone (diagnostic): a strong warm pool on walls and road, but the pole lit white again; the
  near-bulb fade fixes that, with a 2 x gain.
- A plain scene saturation of 0.85 fixed the pines (g/b 1.34) but muted every view; dark-tone saturation does it
  for the woods alone.
- A 3 x 3 tent on the sky texture: softer, still stepped; the B-spline is smooth.

## Cost

`timing.md` (RTX 4060 Ti, 1080p, 4x MSAA, TAA; the timing run used this round's build before the lamp gain and
the test changes, same passes). The look-fog and look-surface lanes' games were on the GPU during part of the run,
so GPU and CPU frame numbers swing more than the lane costs. GPU: the composite section (meter pass plus the
camera's composite) adds 0.03 to 0.66 ms (median; largest at noon and sunrise), the air pass is unchanged within
noise (the camera's haze and caps are a few ALU in its apply), SSR +0.01 to 0.1 ms. CPU: `drw:post` +0.05 to
+0.16 ms; the camera's own work 0.08 to 0.15 ms a frame; the physical sky integral (about 0.7 ms) runs only
when the sun has moved 0.25 degrees. CPU frame and FPS medians differ between off and on by -6 to +3 ms
depending on the view, in both directions (for example noon 29.4 off / 23.1 on, sunrise 18.8 / 21.7,
blue hour 5.5 / 7.0): the difference sits in `drw:obj` (the queue flush that waits on the GPU), not in the
camera's code. On a 3060 expect about 1.3 to 1.5 x the GPU post numbers.

## Problems

- **Rebase needed.** graphics-remaster is now at cb3dd0d (B7, wet materials). The patch conflicts there in
  FeatureGates.hpp, FeatureSwitches.cpp, LightRig.hpp, INTEGRATION-PLAN.md, EngineGL33_Lighting.hpp,
  EngineGL33_PostShaders.cpp, tests CMakeLists.txt and test_feature_switches.cpp (`git apply -3` against that
  tree). B7's surface shading and its SSR changes must then be checked under the camera (passes that read or
  write the frame's colour must `camDecode` / `camFold`).
- Blue hour: the far sea at the frame's sides stays dark under the belt, a hard line; the air pass's haze does
  not reach it (it looks like the far sea is drawn at the far plane, which the apply treats as sky). Not fixed.
- The street lamp's pool on the road right under it cannot be seen from `town_night`'s camera (bushes); its light
  on the walls and the road beside it shows.
- The flame's pixel count still varies with particle timing (3122 to 5761 this round, Classic about 5300 to
  5600).
- Sinai's thin cloud streaks keep some coarse blockiness from the sky texture itself.
- `overcast_broken` under the veil is grey and hazy; the distant hills may now be too washed for a day at
  overcast 0.5 (the veil and `camAerial` both act there).
- Unit tests: the full PoseidonTests run (3317 cases) fails only the 6 localisation cases that read game data
  through a `packages\Remaster` junction this scratch copy does not have, as in rounds 0 and 1.
