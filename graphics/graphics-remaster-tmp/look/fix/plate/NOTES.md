# Plate fixer: grey weather plate, tree halos, rain visibility

Scratch copy `D:\Arma_CWA_worktrees\look-fix-plate` (fec1075 + `LOOK/fix/base.patch`, base tree
`07d0ae092969bc26cff732fbdbae2c5c1397db20`). Build `build/look-fix-plate-rwdi`, game
`dist/look-fix-plate-rwdi`, junction `RemasterUD-fix-plate`. Nothing committed, no branch.

Patch: `fix.patch` (9 files, +246 / -33, only my delta over the base tree, no probe tests).
`git apply --cached --check fix.patch` passes on an index read from the base tree.

## Look first

- `sbs_a_overcast_heavy_plate.png`, `sbs_a_rain_day_plate.png`: the plate and its seam, before | after
  at 1080p, with a 2x crop of the gap.
- `sbs_b_rain_day_treeline.png`: the dark rims at the foot of the fogged trees, and the hill about
  300 m out, before | after, 2x crop.
- `sbs_thumbnails.png`: the same views at 480 px (the owner's thumbnail test), plus the 960 px set.
- Checks: `sbs_optics_mist_sight.png`, `sbs_optics_mist_hud.png` (setFog mist: the same warm plate was
  there too and is gone), `sbs_check_overcast_broken.png` (unchanged), `sbs_check_night_dawn.png`
  (dawn_valley, town_fog_night, town_night_rain: no regression; smoke and fire differ by timing).

Before = `LOOK/director/final/<view>__1080__overhaul.png` (and `__overhaul.png` for the 960 px set and
the optics). After = `shots_5/tri/...` (final build), `shots_4/overcast_heavy__remastered.png` (960 px),
`shots_6/tri/...` (night and dawn checks).

## (a) The plate between the hills

Cause, proved with probes:
- The gap is filled by the classic horizon polygon (`Landscape::DrawHorizont`: from under the horizon up
  to 0.06 of the far distance over the eye, 3.4 degrees) and the dome's band under it, drawn in the
  classic fog colour. Probe 1 (`probe1/`, tint by class in `psFogApply`) shows the whole band is sky
  depth. With `volFog=0` the band is the same warm grey, (193, 194, 188), under a cool, brighter deck,
  (219, 222, 229). The top of the polygon's fade, 30 px of warm under cool, is the hard seam.
- The fog's air colour comes from the horizon ring. Under a grey sky the ring sampled just above the
  land, so it measured that warm band, and every far ridge converged to it: land and band one flat
  colour, the painted plate.
- The air itself was too thick on a dry grey day: the grey haze gain 3.5 left 2 % of a ridge at 2 km.
  `fogDensity=0.5` (probe 2) made the far ridges fade one behind the other.
- Probe 2 (`probe2/`: ring sampled above 3.8 degrees, the band below the polygon's top replaced by the
  air) removed the plate and the seam in both views.

Change:
- Ring (`kPostFogRingMainGLSL`): under a grey sky the samples start at max(first sky + 0.3 degrees,
  3.8 degrees), above the polygon, step 0.6 degrees. The air takes the deck's colour over the horizon.
- Apply (`kPostFogApplyMainGLSL`): under a grey sky (new `fogSkyWeather.z` = `FogGreySky`) or in rain or
  fog, sky and far-polygon pixels below the polygon's top are the air's colour, and the deck above sinks
  into it over about 5 degrees. It was rain and fog only, 90 %, from the horizon line.
- Medium: the grey haze gain 3.5 -> 1.2 (`kHazeGreyGain`).
- `FogGreySky` replaces the inline grey formula of the ring (same values).

Result: no seam (column from deck to gap is flat to 1 level), the gap is the deck's air, the ridges at
1 to 3 km fade into it gradually in overcast_heavy. In rain_day everything past about 1.3 km is air
(heavy rain), continuous with the sky, no colour step.

## (b) Dark scalloped rims at the foot of fogged trees

Cause, proved with probes (it was not the fog's reduced-resolution upsample):
- Probe 2 turned off the apply's gap closing (`fogGapZ`) on the right fifth: rims unchanged.
- Probe 3 (`probe3/`) made the volume lookup bilateral (cell centre depth): rims unchanged.
- Probe 4: `surfAo=0` removes the rims and the pale skirts entirely; `surfCanopyAo=0` does not.
- So the GTAO and contact shadows of the composite darken the whole fogged pixel. The world's surface
  shares only hold the classic fog's edge, not the volumetric fog, so the occlusion acted on the air's
  light in front of the trees. Probe 5 (debug view of the composite) showed the trees at T = 0.8, so a
  T veil alone is not enough: the air in front is several times the trees' own light.

Change: the fog apply writes a second target (`FogState::veilTex`, R8, attachment 1 of the fog's
FBO): the surface's share of the pixel's light after the fog, lum(C T) / lum(C T + S). The composite
scales the surface target's sky and sun shares by it before the occlusion and the contact shadow. That is
exactly the occlusion applied to the surface before the fog. The composite reads it on unit 12
(`kFogVeilUnit` = the free light-data unit the fog floor also uses) and unbinds it after. No new pass.

Result: the rims and the pale skirts are gone at 1080p and at thumbnail size. Near views (town_noon,
forest_morning, pine_rays) keep their occlusion: the veil is about 1 near the eye.

## (c) Rain visibility, and the AI

The AI link is intact: `Target.cpp` 636 to 646 still multiplies by `FogSightTransmittance`, which reads
the medium `PostFog` hands to `SetFogSight` (line 324). I did not touch the link, the ground air or the
mist, which are what the AI path reads. The AI's weather part is the classic tactical fog.

Rain was too strong: rain 0.8 left a hill 300 m away about 40 % of its contrast and nothing past a
kilometre, while the AI's classic tactical sight there is full to 392 m and ends at 980 m. I tuned the
rain fog, not the link: rain's weather fog takes `kRainFogShare` 0.5 of the extinction that gives the
classic visibility (setFog keeps all of it), with the lower grey haze above.

Visibility now (rendered, a level line 10 m over the ground outside a valley mist layer, from the
shader's medium; transmittance of the surface's contrast):

| view | 300 m | 1 km | 2 km | 5 % contrast |
|---|---|---|---|---|
| rain_day (rain 0.8, VD 3500), before | 0.41 | 0.02 | 0 | about 840 m |
| rain_day, after | 0.61 | 0.12 | 0.01 | about 1350 m |
| overcast_heavy (dry), before | 0.79 | 0.19 | 0.02 | about 1530 m |
| overcast_heavy, after | 0.88 | 0.42 | 0.12 | about 2650 m |

AI in rain_day: tactical range 980 m (full to 392 m) times the fog sight factor (ground air, about 0.96
over 300 m on this line). The player now sees the 300 m hill clearly and a faint ridge to 1.3 km, a
little past the AI's tactical end.

## Constants (name: old -> new)

- `kHazeGreyGain` (VolumetricFog.hpp, was the inline 3.5 in `ComputeFogMedium`): 3.5 -> 1.2.
- `kRainFogShare` (new): rain's weather fog 1.0 -> 0.5 of the classic extinction.
- `kFogRingGreyFromDeg` (new, GLSL `kRingGreyFromDeg`): grey-sky ring start, first sky + 0.3 degrees ->
  max(first sky + 0.3, 3.8) degrees; grey-sky ring step 0.4 -> 0.6 degrees.
- `kFogHorizonFillTop` (new, GLSL `kApplyFillTop`) 0.06: the deck sink starts there; was 0.9 x
  exp(-sin(el) / 0.09) x wet, now exp(-max(sin(el) - 0.06, 0) / 0.09) x max(wet, grey).
- New: `FogGreySky`, `kFogVeilUnit`, `FogState::veilTex`, `PostFogVeilTex`, composite `fogVeilTex` and
  `surfParams2.y`.

## Tests

- New in `test_volumetric_fog.cpp`: the grey day's medium and ridge fade, rain's share and its 300 m /
  980 m / 2 km contrast, setFog unchanged; the ring and apply constants against the C++ mirrors; the
  veil output, its use before the occlusion, and its texture unit.
- Gate tags `[remaster],[FeatureSwitches],[FeatureGates],[PostStages],[fog],[SurfaceLight],[AirMedium],
  [ssr],[wet],[CombatLights],[Shaders],[StateCacheAudit],[PhotoCamera],[StateOwnership]`: 308 cases,
  953,891 assertions, all pass.
- Full suite: 3384 cases, only the 6 known localisation cases fail (no `packages\Remaster` in the
  scratch copy), as in round 2.
- clang-format applied to the changed lines.

## Still open

- In heavy rain the gap beyond about 1.3 km is plain air: correct for rain 0.8, but no ridge layering
  there. Light rain keeps more.
- The classic horizon polygon is still drawn in the classic fog colour; the fog post replaces it under
  a grey sky and in rain or fog only. Under a clear sky the old path is unchanged (not a problem in the
  views I shot).
- Under a grey sky the ring now reads the deck at 4 to 7 degrees. A dark cloud band sitting exactly
  there would darken the far air in that azimuth (not seen in these views).
- Cost of the veil (one R8 full-resolution write in the apply, one fetch in the composite) not timed.
- Rain at night (town_night_rain) is a little clearer, as intended; not compared against the AI there.
- Probe tests (`look_probe*`, `look_fix*`) are in the scratch copy's captures folder, not in the patch.
