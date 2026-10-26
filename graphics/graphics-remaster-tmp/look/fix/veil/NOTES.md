# Veil fixer: clear noon veil and the sun-glow smear

Scratch copy `D:\Arma_CWA_worktrees\look-fix-veil` (fec1075 + `fix\base.patch`, base tree
07d0ae0). Build `build/look-fix-veil-rwdi`, game `dist/look-fix-veil-rwdi`, junction
`RemasterUD-fix-veil`. Nothing committed. The patch is `fix.patch` (10 files, +169 / -9; it
applies on a fresh index of the base tree, checked with `git apply --cached --check`).

## Images

- `veil_noon_vista.png`: (a) before | after, with a 2x crop of the left woods.
- `smear_noon_vista_left.png`: (b) the pale strip down the left edge of noon_vista, 2x crop.
- `smear_coastline_sunset.png`: (b) the flat glow column at sunset, 1.6x crop round the sun.
- `check_golden_dawn_sunset.png`: coastline_sunset, sunrise_ridge, forest_edge_golden,
  dawn_valley before | after.
- `check_noon_views.png`: noon_vista, noon_fields, town_noon, sinai_desert_noon,
  jeep_close_noon before | after.
- `check_other_weather.png`: overcast_broken, rain_day, blue_hour_coast, pine_rays,
  forest_morning, lowsun_fp, sinai_lowsun_fp before | after (the tile order slips at the
  end: the lowsun_fp glob also caught the Sinai shot).
- Shots: `shots_1\` (the fixed build, nine views), `shots_2\` (the other weather), probes in
  `shots_p1\` to `shots_p3\`, probe shaders in `work\ov2\`, `work\ov3\`.

"Before" is `director\final\<view>__1080__overhaul.png` (`__overhaul.png` where no 1080 copy).

## (a) Milky veil over the near woods at clear noon

Cause: the clear haze near the eye. The fog's air takes the light of the horizon sky in its
azimuth (the ring), and that sky is 10 to 15 times as bright as a dark conifer wood. The
near thinning kept 0.35 of the haze at the eye and reached the whole at 2.5 km, so the first
300 to 600 m had an optical depth of 0.05 to 0.1. A few percent of the horizon's light is
then most of a dark wood's light: the woods under noon_vista went from (47,52,43) in the
classic to (103,114,126).

Probe evidence:
- `debug=8` (the fog's in-scatter alone, `shots_p1`): the in-scatter on the left wood is
  (88,97,108) where the final pixel is (104,115,126), so about 70% of that wood's light was air.
- `fogSun=0` and `bloom=0` change nothing (no forward lobe, no bloom at a 72 degree sun).
- A live override of the injection's near thinning (keep 0.06, whole at 3 km;
  `shots_p2` nearD, `shots_p3` fog) clears the woods and keeps the far ridges blue.

Change: `VolumetricFog.hpp` `kHazeNearKeep` and `kHazeNearTo`. Optical depth of the clear
haze on a level ray 300 m up: 500 m 0.08 -> 0.015, 1 km 0.18 -> 0.05, 2 km 0.51 -> 0.27,
3 km 0.96 -> 0.67, 5 km 1.86 -> 1.57. The grey deck, rain and fog keep their own near air
(`kHazeNearKeepGrey`, `kHazeNearToGrey` unchanged), dust keeps `kDustNearKeep`.

Result (noon_vista, 1080p): left wood (103,114,126) -> (72,82,89), near right wood
(61,72,76) -> (43,53,47) (classic 41,49,38), far ridge stays pale blue (146,171,196) ->
(151,176,200). The camera meters the darker woods: the sky is about 8 levels brighter.

## (b) The smear: two causes

### Noon_vista, the pale strip down the left edge

Cause: the fog's air colour along the screen's edges. Each fog column takes the ring's
measured sky from nine taps over 22 degrees of azimuth, and mixed toward the model's sky
colour by the taps' mean confidence. Past the screen's edge the ring has never seen the sky
(confidence 0), so the edge columns read as only 60% sure and took 40% of the model's
colour, which is paler than the measured sky (it is the horizon band 1 degree up). The air
over the hill and the horizon brightened over the left 300 px (green 180 -> 189).

Probe: a live injection that takes the ring wherever any tap is measured (`shots_p2` ring)
flattens the left edge (green 177 to 179 across the first 330 px, was 189 down to 180).

Change: `EngineGL33_PostFogShaders.cpp`, the injection: the ring's weight is now the
freshest tap's confidence (0.1 less per tap from the centre), not the taps' mean. Azimuths
out of view still fade to the model as their measurements age (`kFogRingDecay`).

### Coastline_sunset (and every low sun): the flat glow column

Cause: the sky dome's Mie phase. The sky table's single-scattered Mie is shaded per pixel
with one broad Cornette-Shanks lobe (g 0.8), which is almost flat over the first 5 degrees
(1 degree from the sun only twice as bright as 10). The low sky toward the sun was one wide
flat wash, and the camera's sky roll-off pressed it into a column from the sun down to the
horizon with no gathering at the disc.

Probe evidence (`shots_p1`, `shots_p2`): `volFog=0`, `fogSun=0` and `bloom=0` all keep the
column, so it is not the fog's lobe nor bloom. A live dome without the Mie term (`domeR`)
removes it (and every sun glow). Without the sky roll-off (`noroll`) the column is still
there, brighter at the bottom. A two-lobe phase (`shots_p3` newA) turns it into a glow
round the disc with the warm horizon under it.

Change: the haze's phase is a broad lobe plus the aureole's narrow forward peak:
`0.7 CS(0.75) + 0.3 HG(0.97)`. About 9 times the old phase at 1 degree, 2 at 3, 0.7 at 10,
0.9 at 30, the same from 60 on; it still integrates to 1 over the sphere. Changed together
in `Atmosphere.hpp` / `.cpp` (`AtmosphereMiePhase`, which the sky light integrals of the
camera also read), the dome's `skyMiePhase` (`EngineGL33_LightingShadersSky.cpp`) and the
row-mean pass's `miePhase` (`EngineGL33_PostSkyTableShaders.cpp`), so the GPU table and
its CPU mirror still agree.

Result: coastline_sunset: the glow gathers round the sun and fades out radially; the sky
at 300 px beside the sun 203,173,132 -> 191,166,130, the horizon under the sun unchanged
(luma 209 -> 210). Sunrise_ridge and dawn_valley keep their warm glow, a little more
compact round the disc; forest_edge_golden (sun behind) is unchanged.

## Constants touched

- `VolumetricFog.hpp` `kHazeNearKeep`: 0.35 -> 0.06
- `VolumetricFog.hpp` `kHazeNearTo`: 2500 -> 3000 m
- `Atmosphere.hpp` `atmo::kMieG`: 0.8 -> 0.75 (now the broad lobe only)
- new `atmo::kMieAureoleG` 0.97, `atmo::kMieAureoleShare` 0.3 (GLSL mirrors
  `kSkyAureoleG`, `kSkyAureoleShare`, `kAureoleG`, `kAureoleShare`)
- injection: ring weight `clamp(ring.a)` -> `clamp(ringSure)` (freshest tap)

New C++ mirror `FogHazeNearKeep(m, dist)` (VolumetricFog.hpp/.cpp) for the tests.

## Tests

`PoseidonTests.exe "[remaster],[FeatureSwitches],[FeatureGates],[PostStages],[fog],
[SurfaceLight],[AirMedium],[ssr],[wet],[CombatLights],[Shaders],[StateCacheAudit],
[PhotoCamera],[atmosphere]"` run from D:\: all pass, 304 cases, 953,843 assertions
(glslang compiles every changed program). New cases:
- `Volumetric fog: clear noon air is nearly clear for the first kilometre, the far ridges
  keep theirs` (optical depth at 500 m, 1 km, 3 km, 5 km; grey deck unchanged).
- `Volumetric fog: the air's colour along the screen's edges is the ring's, not the model's`.
- `Sky table: the haze's phase has the sun's aureole and conserves the light`
  (normalised, 1 degree over 10 degrees, broad lobe within 0.65 to 1.25 of the old).
- The sky table shader test checks the aureole constants in the dome and the row-mean pass.
clang-format applied to the changed lines.

## AI sight and fog

Unchanged and intact: `Target.cpp` lines 636 to 645 still multiply AI visibility by
`FogSightTransmittance` while `FogSightActive`; `SetFogSight` / `FogSightTransmittance` /
`FogSightExtinctionAt` in `VolumetricFog.cpp` are untouched. The clear haze was never in AI
sight (only the mist and the humid ground air are), so this change does not alter it.

## Still open

- The sun's core under the photoreal camera stops at the sky roll-off's cap (about 0.85 of
  white, `PhotoCamera.hpp` `kSkyRolloffCap`): the glow round the disc is a soft plateau of
  luma 207 out to about 2 degrees, then falls off. A real low sun burns white there. That is
  the camera's constant, not touched here.
- The near-field clear haze is an eye-centred thinning, not physics: a far observer of the
  same wood sees it with the full haze. At 3 km and beyond both agree.
- dawn_valley's foreground is darker (bottom-half luma 94 -> 80) now the near hill is not
  veiled; still readable, worth a look in play.
- The ring change also affects twilight and grey days at the screen's edges; checked on
  blue_hour_coast, overcast_broken and rain_day (no visible change), not on night views
  (the ring is not used at night).
