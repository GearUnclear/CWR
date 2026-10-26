# Atmosphere track: a physically based sky under CWA's own sky textures

Track `atmosphere`, research prototype on a detached scratch worktree
(`D:\Arma_CWA_worktrees\gfx-atmosphere`, built to `dist/gfx-atmosphere-rwdi`).
Nothing committed. The C++ change is `prototype.patch` (10 files, about 930
lines); the final GLSL is in `glsl_final/`.

## The one-line result

A Hillaire-style clear-sky atmosphere (Rayleigh, Mie, ozone, multiple
scattering) integrated on the CPU into a small sky-view table now drives the
sky dome under the original sky texture's clouds, the fog colour of every
lit and pre-lit world draw (aerial perspective, including the far "horizont"
polygon), and the sky that water reflects. Sunsets get a golden horizon band
and a real sun aureole, dusk gets a proper orange twilight band under a
violet sky, noon gets a clean graded blue, and night is untouched. It is the
largest visible change of the sky in this branch, at no measurable GPU cost.
The sky-light part (feeding the rig's ambient from the atmosphere) turned the
ground blue and is rejected.

Best images: `BEST_<view>.png` (Classic | Remastered (current) | Remastered +
atmosphere, physical hue, sky saturation 1.3). `ALT4_<view>.png` adds the
taste-safe "half classic hue" variant as a fourth panel.

## What draws the sky, and what a shader can see (findings)

- The sky is `Landscape::_skyObject` (`data3d\obloha.p3d`, 53 points, 77
  faces), drawn first in `Landscape::Draw` through the software T&L path:
  `TLVertexTable::DoSkyLighting` writes a per-vertex colour
  `SunSkyColor * cos^6 + SkyColor` times the eye accommodation, and the GL33
  backend draws the pre-transformed vertices with `VSScreen`. The fragment
  shader only sees screen position, colour and UV; it has no view direction.
- The sky textures are 256x256 P8: `jasno.pac` (clear) is a pure vertical
  blue gradient with no clouds at all, `oblacno.pac` is blue with cirrus,
  `zatazeno.pac` is grey overcast. Overcast 0 uses 86% jasno + 14% oblacno
  (`Weather::SetSky` interpolates), overcast 0.5 is oblacno, 0.66+ zatazeno.
  `skytex/sky_textures_sheet.png` shows them.
- The band of flat fog colour between the sea or land horizon and the sky is
  not the dome: it is `Landscape::DrawHorizont`, a polygon at 0.99 of the far
  plane reaching about 3.4 degrees up, drawn pre-lit and fully fogged.
- The dome is the first draw of the frame and happens before the world's first
  `BeginPass`, so on this branch it always drew through the CLASSIC programs
  (`InitDraw` resets `_worldEffectsOn`). Run 1 showed the sky shader never ran
  for that reason (`run1/`).

## How the prototype works

C++ (pure math, `Graphics/Remaster/Atmosphere.{hpp,cpp}`):

- Earth atmosphere with Bruneton/Hillaire constants: Rayleigh (5.8, 13.6,
  33.1)e-6/m, 8 km; Mie 4.0e-6 scattering, 4.4e-6 extinction, 1.2 km,
  Cornette-Shanks g 0.8; ozone tent at 25 km; ground albedo 0.3.
- Transmittance table 256x64 (Bruneton parametrisation) and Hillaire's
  multiple-scattering table Psi_ms 32x32 (64 sphere directions, 20 steps),
  both built once (about 100 ms, again only if the haze knob changes).
- Sky-view table, three regions of 64x64 each in one RGBA32F texture on
  unit 6: Rayleigh + multiple scattering with the phase applied; Mie single
  scattering without its phase (applied per pixel, so the aureole is sharp);
  and the azimuthal mean of the full radiance per elevation row. u = sqrt(azimuth
  from sun / pi), v = Hillaire's horizon-dense elevation mapping. Rebuilt when
  the sun moves 0.02 degrees: 8 to 12 ms on the CPU (logged as `ATMO lut`).
- `offline/` has a standalone build of this code and `OFFLINE_sky_panoramas.png`,
  360 degree panoramas from noon to -4 degrees: blue noon sky with a white
  horizon, golden aureole at sunset, pink-violet twilight. The model is right.

Engine plumbing:

- `DoSkyLighting` marks dome vertices with `t1 = 4096` (the clipper
  interpolates t1, no classic shader reads it).
- `vsScreenRemaster` rebuilds the camera-relative point on the view ray from
  NDC and three camera-basis slots (`right * tan(fovx/2)`, `up * tan(fovy/2)`,
  `forward`) times w, so perspective interpolation gives an exact per-pixel
  view direction. Dome vertices get `vLitMode 2`; every other pre-lit world
  draw gets `vLitMode -1` (keeps its colour, takes the atmosphere fog).
- `EngineGL33::BeginMesh` opens the remastered programs and uploads the frame
  constants for the dome when it comes before the world pass (the run 1 bug).
- New `LitConstants` slots 58 to 64, sampler `skyLut` on unit 6, knobs
  through `triSetLighting key=value`: `atmosphere`, `atmoFog`, `atmoLight`,
  `atmoSun`, `atmoHaze`, `atmoExposure`, `atmoClouds`, `atmoHue`, `atmoSat`,
  `atmoDebug` (1 cloud mask, 2 raw atmosphere, 3 view direction).

Shading:

- Clear sky: each row keeps the classic brightness of that row (the row's
  clearest texel times the classic dome light, no classic cos^6 glow), and the
  atmosphere supplies how a direction differs from the mean of its elevation
  (sun glow, aureole, darker anti-sun side). `atmoHue` 1 takes the
  atmosphere's own hue at that brightness, 0 keeps the classic hue and only
  relights it per channel, 0.5 is between.
- Clouds come from the original texture: a texel brighter than the clearest
  texel of its row (12 taps along U, a 0.015 dead zone against the P8 palette
  steps) is cloud, and the overcast texture's share is cloud everywhere.
  Clouds are lit by the sun as seen from 2.5 km (transmittance table) with a
  Henyey-Greenstein forward lobe, plus the sky; their luminance matches the
  classic dome light, so cloud brightness stays classic.
- Aerial perspective: fog colour is the same clear-sky function at about one
  degree above the horizon in the fragment's azimuth, referenced to the
  classic fog colour; under an overcast deck it falls back to the classic fog.
  Fog amount is unchanged. Water reflects the atmosphere at the reflected
  elevation (the water track owns the rest of the water shader).
- Night: the whole thing fades out between -3 and -9 degrees sun elevation.
  `town_night` Remastered vs Remastered + atmosphere differs by at most 3
  levels in 8 bits on 0.0% of pixels.

## Variants and what the captures showed

All on real Remaster data, 960x640, 150% scale, 4x MSAA, same camera, date,
weather and sim clock per view. Runs `run1/` to `run5/` each hold the raw
captures, `*__compare*.png` / `*__best3.png` panels and `timings.md`.

| Run | Variant | Verdict |
|---|---|---|
| 1 | fog + sky light only (dome silently stayed classic) | found the pre-world-pass bug |
| 2 | one global exposure, sky + fog + sky light | sunsets good; overcast_broken sky washed pale and flat (`REJECTED_run2_overcast_broken_globalK.png`); pink fog band left at the horizon |
| 3 | per-row classic brightness: physical hue / classic hue relit / hazy | overcast fixed (deep blue kept); physical sunrise muted grey-lavender; relit gives a crimson sun glow |
| 4 | + sky saturation 1.3, hue 0.5, haze 1.6, + sky light | saturation fixes the muted look; sky light turns the ground blue (below) |
| 5 | final: physical sat 1.3 (`atmoPS`) and hue 0.5 sat 1.15 (`atmoM`), perf alternation | BEST images |

View by view (run 5, `BEST_*.png`):

- `coastline_sunset`: the strongest result. Classic's pink-cyan wash and the
  flat pink horizon band become a golden horizon glow under a blue sky with
  lit cirrus, and the horizon polygon now matches the sky behind it.
- `firefight_dusk` (sun -1.7): classic's uniform salmon becomes a real civil
  twilight, an orange band on the horizon under a violet-blue sky.
- `lowsun_fp`: a warm aureole around the low sun and lit cirrus over a graded
  blue sky; the props are lit as in Remastered (run 4, `measure`-style crops:
  M113 hull and near ground within 0.01 of Remastered in every channel).
- `sunrise_ridge`: golden glow at the sun, blue-lavender sky above; reads as
  real dawn but cooler than Classic's pink. `atmoM` keeps a mauve-pink sky
  with a coral glow at the sun (`ALT4_sunrise_ridge.png`).
- `noon_fields`: a clean graded blue sky with a white horizon. At saturation
  1.3 it is a strong blue; 1.15 might suit better.
- `forest_edge_golden` (sun behind the camera): the lavender sky becomes blue.
- `overcast_broken`: the texture's deep blue and clouds are kept, clouds a bit
  whiter, blue a bit more saturated. Small change, no regression.

Rejected: `atmoLight` (rig sky light from the atmosphere irradiance). On
`lowsun_fp` it moved the near ground from r/b 1.16 to 0.99 (blue cast,
`VARIANTS_run4_lowsun_fp.png`, last panel) and at noon it made the shadows
visibly blue (`run2/noon_fields__compare.png`, third panel). It
contradicts the owner's "no blue wash" and "warm evenings". `atmoSun`
(physical sun chroma) was built but not captured.

## Cost

- GPU: below measurement noise. The machine ran 3 other agents' games at
  the same time, so single readings swing by 10+ ms. Alternating Remastered
  and Remastered + atmosphere five times on the same view (run 5):
  `coastline_sunset` frame median 3.53 vs 2.98 ms (min 3.03 vs 1.94);
  `sunrise_ridge` median 39.44 vs 40.47 ms (min 27.76 vs 33.13). No consistent
  difference. By construction it adds 3 bilinear taps of a 192x64 RGBA32F
  table and about 40 ALU ops (asin, acos, pow) per lit or pre-lit world
  fragment, and 15 taps per sky pixel; expected well under 0.3 ms at
  1440x960. Classic numbers are untouched.
- CPU: 8 to 12 ms per sky-view rebuild when the sun moves 0.02 degrees (in
  play, about every 5 s of game time at normal time speed), plus about
  110 ms once per haze value. This is a hitch in a production build; see below.

## Verdict

Promising. The sky is the part of the frame Classic shades worst (a texture
times one vertex colour), and a physical sky under the original clouds gives
exactly the "RTX on" jump in the sky and horizon at low sun and dusk without
new art. It does not change the near field. It does not by itself fix the
known ground and hull problems, and its sky-light extension made them worse,
so it stays a sky / aerial-perspective feature.

## What a production version needs

- Build the sky-view table on the GPU (one fragment pass into a 192x64
  target) or on a worker thread with double buffering; the transmittance and
  multiple-scattering tables once at load.
- A real dome direction instead of the t1 marker: the sky object could carry
  its own flag through the draw descriptor; the NDC reconstruction needs the
  pillarbox viewport offset when the world rect is cropped.
- Open the remastered programs for the pre-world draws properly (the dome
  `BeginMesh` hook is a hack); make sure portrait, preview and night-vision
  paths still take Classic.
- Move the cloud mask and the classic per-row reference to a one-time CPU
  analysis of the current sky texture (row minimum and cloud alpha baked into
  a small texture whenever `Weather::SetSky` changes), instead of 12 taps per
  pixel.
- Decide hue and saturation with the owner (physical vs half classic, 1.15 vs
  1.3); tie haze to fog density and overcast; add the moon and a twilight
  fade that is continuous with Classic's night sky.
- The 3D cloudlets (`DrawClouds`) still use Classic's lighting; they should
  take the same sun-from-the-deck light as the dome clouds.
- Unit test: `test_remaster_lighting.cpp` checks `LitConstants` against the
  GLSL block and would need the new slots; the atmosphere math can get a CPU
  reference test from `Atmosphere::Sample`.

## Risks against the owner's taste list

- Blue: at sunrise the physical sky is blue-lavender where Classic is pink,
  and the far ridge takes a slightly bluer aerial perspective. The ground is
  not tinted (measured), but the frame reads cooler. `atmoHue 0.5` keeps the
  pink mood; aerial perspective could also keep classic chroma.
- Saturation 1.3 is close to a grade on the sky. It is limited to the dome and
  the fog colour, never the near field.
- No darkening: each sky row keeps Classic's brightness by construction.
- Classic is untouched in this prototype except that `DoSkyLighting` now
  writes t1 for the dome (no classic shader reads it; a production version
  should gate it on the lighting mode to be bit-identical in buffers too).

## Files

- `prototype.patch`: the C++ change against the lighting branch tip 3c53141.
- `glsl_final/`: the remastered GLSL of the prototype (for overrides).
- `glsl_v1/`, `glsl_dump/`: earlier dumps (v1 prototype, the branch's GLSL).
- `BEST_<view>.png`, `ALT4_<view>.png`, `VARIANTS_run4_*.png`,
  `REJECTED_run2_overcast_broken_globalK.png`, `DEBUG_cloudmask_run2.png`,
  `OFFLINE_sky_panoramas.png`.
- `timings_run4.md`, `timings_run5.md`; `runN/timings.md`.
- `make_compare.py` (N-way panels with GPU ms), `gen_tests.py` (generates the
  capture tests), `measure.py` (region colour statistics), `tests/` (the
  generated SQF/toml capture tests, `atmo_lib.sqf`, the copied
  `capture_lib.sqf`), `skytex/` (extracted sky textures and sky shape),
  `offline/` (standalone atmosphere build and panoramas).
