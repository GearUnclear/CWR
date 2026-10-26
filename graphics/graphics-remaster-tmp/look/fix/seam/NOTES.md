# Seam fix: the wet ground no longer stops at a straight line

Scratch copy `D:\Arma_CWA_worktrees\look-fix-seam` (fec1075 + `LOOK/fix/base.patch`, base tree
`07d0ae0`). Build `build/look-fix-seam-rwdi`, game `dist/look-fix-seam-rwdi`, junction
`RemasterUD-fix-seam`. Nothing committed, no branch. Patch: `fix.patch` (my delta over the base tree,
11 files, +374 / -48, no probe tests).

## Look first

- `sbs_town_night_rain.png`: the owner's view. Before (look director final) | after, full frame on top,
  2x crop of the tree base below. The hard line from the tree base is gone; the sheen, the lamp
  streaks and the puddles run on across it.
- `sbs_seam_b_second_angle.png`: the same square from heading 20 (the same seam seen from the other
  side), before from the look director build with the same test.
- `sbs_rain_day.png`: Eden rain_day. It had no seam (one grass texture); it stays the same.
- `probe_classes.png`: the probe that proved the cause (see below), before | after.

## Cause

The landscape draws every terrain square with its own texture (CWA's corner-blend textures,
`abel\p4p4ttp4`-style). B7's wet shader (`wetSurface`) classed the land per pixel from that square's
albedo: `wetVegetation(albedo)` (grass share) and `wetGroundPorosity(albedo)`. Both are steep in the
colour (green excess times 10, green/blue ratio between 1.1 and 1.3). Two neighbouring squares whose
textures differ only a little in hue (here grey gravel patches on one, a uniform olive on the other)
got very different classes, and everything wet follows the class: the film (sheen, sky and lamp
reflection), the darkening, the porosity, and the puddle threshold (`wetPuddle`'s `veg`). So the wet
look stopped exactly at the square's edge, a straight world-axis line (it runs to the vanishing point
15 degrees right of the view axis, as a north-south grid line should). B6's trace did the same with the
frame's colour (`grass` from `c0`), so its reflection weights and puddles stepped there too.

Probe evidence (`shots_probe1`, `shots_probe2`, `probe_classes.png`):
- `ssr=0`: the seam stays. `wetWeather=0`: the seam is gone. So it is the wet forward pass, not B6.
- SSR tag view (`debug=9`): the whole square is terrain (green), so not a road decal edge.
- A shader override (`work/glsl_probe1`) wrote the wet classes out through the debug views:
  `debug=7` showed (road flag, veg, porosity), `debug=2` (puddle, damp, film), `debug=1` (the per-draw
  class and kind). veg jumped from mottled 0/1 to a flat 1 at the line, porosity and film with it, the
  puddles ended at it. The per-draw class and kind were the same on both sides. The albedo itself
  (`debug=7` in the stock build) shows only a faint texture edge there.
- Strip means 8 px either side of the line: albedo (0.472, 0.461, 0.426) | (0.451, 0.442, 0.400), a
  2 to 3 level step; veg 0.68 | 0.82, film 0.34 | 0.30 (in the probe's tone mapped units).

Not the cause: the triangle or cell edges of the height map (the puddle field already reads the height
map smoothed), the SSR tile or mask (the reflection-only view `debug=10` has no line), road decals,
per-vertex lamp or fire light.

## The change

The land is now classed from a continuous world-space field, the land's own colour, the same in the
forward pass and the trace:

- `TerrainHorizonMap::groundColor` (new): the per-vertex ground colours the horizon map builder already
  received (the landscape textures' average colours, blended over the texture cells around each
  vertex by `ExpandGroundColors`; on Malden the 50 m texture cells over the 12.5 m terrain grid), kept
  in display space instead of only as the bounce tint. `SampleGroundColor` is its CPU mirror.
- Uploaded as `_groundFieldTex` (RGBA16, a = 1) on unit 12, `kGroundFieldUnit` (the unused
  `kLightDataUnit` slot; the fog's floor still borrows it in its own pass, `kFogFloorUnit` =
  `kGroundFieldUnit`, and the world rebinds the field in `BindLightingTextures`). A transparent
  stand-in (a = 0) until a map is built: then the shader falls back to the albedo as before.
- Shaders: `groundFieldAt(xz)` in the Puddles piece reads it with the smoothstep interpolation the
  puddle heights use, so it has no edge and no crease. `wetVegetation` and `wetGroundPorosity` moved
  into that piece, which the forward prelude and the SSR trace each paste once.
  - `wetSurface`: the land's veg and porosity come from `groundFieldAt(posAbs.xz)`; the albedo still
    darkens per pixel, so the texture's own detail stays. The land never pools as a road by its
    texture's name any more (`cls == 7` dropped for the land; the trace never treated land as road).
  - SSR trace: the terrain's `grass` is `wetVegetation(field)`, the frame-colour tests only without a
    field. The trace's puddles are the forward pass's again, pixel for pixel.
  - `wetPuddle`: the grass threshold now mixes by `veg * veg`. The field's veg is an average over mixed
    ground; with a linear mix a mixed square held fewer puddles than its bare and grassy parts did
    apart (the threshold is convex in coverage). This keeps the puddles the owner likes.

AI sight and fog: untouched. `Target.cpp` around line 636 still multiplies `vis` by
`FogSightTransmittance` while `FogSightActive()`, and `VolumetricFog.cpp` 425 to 460 is unchanged
(`git diff` against the base tree is empty for both files).

## Constants and names touched

- `wetPuddle` grass mix: `veg` -> `veg * veg`.
- Land puddle road flag: `(g_wetDraw == 0 || cls == 7)` -> `g_wetDraw == 0`.
- `kLightDataUnit` (12, unused) -> `kGroundFieldUnit` (12); `kFogFloorUnit` = `kGroundFieldUnit` (still 12).
- New: `TerrainHorizonMap::groundColor`, `SampleGroundColor`, `EngineGL33::_groundFieldTex`,
  `_dummyGroundTex`, `GroundFieldTexture()`, sampler `groundField` (row in `kLitSamplers`).
- No tuning constant of the look changed.

## Results

Numbers from the probe overrides (`shots_probe2` before, `shots_probe4` after), lower half of the
frame (y 600 to 1080):

| view | puddle share | damp share | film mean |
|---|---|---|---|
| town_night_rain before | 0.115 | 0.222 | 0.349 |
| town_night_rain after | 0.134 | 0.228 | 0.371 |
| seam_b before | 0.039 | 0.063 | 0.315 |
| seam_b after | 0.034 | 0.061 | 0.355 |

With a linear grass mix the puddle share fell to 0.108 and 0.033; `veg * veg` brings it back.

Views shot (final build, `shots_4`; before with the look director build, `shots_before2`, same test):
town_night_rain, seam_b (heading 20), seam_c (x 7150 z 8930 heading 300), seam_d (x 7160 z 8905
heading 10), seam_day (the square at 11:00 in rain), rain_day (Eden). No straight seam in any of them
after the fix, also under auto contrast (`work/f1_zac.png`, `work/f1_cd_z.png`, `work/f2_day_z.png`,
`work/ba_d_z.png`). rain_day is unchanged within the shot's noise.

What looks different: in mixed ground (the town square) the wet response is now even instead of
mottled by the texture's grass and gravel patches. The square reads as one wet surface with long lamp
and window streaks; the puddles sit in other places (the noise field without the per-pixel grass mask)
at about the same share.

Unit tests (`build/look-fix-seam-rwdi/tests/unit/engine/Poseidon/PoseidonTests.exe`, run from D:):
- Gate tags of the look director plus `[TerrainHorizon]`: 303 cases, 956,831 assertions, all pass
  (glslang compiles every program).
- Full suite: 3383 cases, the 6 known localisation cases fail (no `packages\Remaster` junction in the
  scratch copy), as before.
- New in `test_wet_weather.cpp`: "Wet weather land: the classes come from a field that blends across a
  texture edge" (two textures whose per-pixel classes differ by more than 0.5: the field's class
  changes by less than 0.03 per quarter metre across their edge, keeps each side's class away from it,
  has no crease at a grid line, clamps at the map's edge; the builder keeps the colours) and "Wet
  weather shaders: the forward pass and the trace class the land from the ground field" (the shared
  piece, the field read, no `wetVegetation(albedo)`, the trace's field use, the `veg * veg`
  threshold, the unit row, the binds). clang-format is clean on the changed C++.

## Still open

- Until the terrain map is built (about 0.6 s after an island loads on Malden) the land falls back to
  the per-pixel class, so the old seam can show for that moment.
- A land texture whose name matches a road token no longer pools as a road. None of the stock island
  textures do; a mod island that paints roads into its terrain textures gets bare-ground puddles there.
- B6's legacy ground path (rain with `wetWeather=0`) still takes its puddles' flatness from the
  per-triangle depth normal; its grass share now comes from the field. Not the shipped state.
- The ground field is the landscape texture's average colour (`GetColor`). A texture whose average
  is far from its look (large bright details) is classed by that average.
- Not measured: cost (one more texture read per wet land pixel and per trace texel) and motion.
