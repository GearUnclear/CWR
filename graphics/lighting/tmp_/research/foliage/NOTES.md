# Foliage track: volumetric crowns, leaf translucency, canopy map

Research prototype for Uslu dur! Remastered lighting. Trees and bushes are
lit as participating volumes instead of as surfaces: every leaf fragment knows
how deep it sits inside its own crown toward the sun, the sky and the viewer,
and the light it gets follows from that. A second, C++ part rasterises every
tree and forest block of the island into a canopy map so that ground, trunks
and distant forests are shaded by the canopy around them.

All results below come from in-game captures on the full Remaster data
(packages/RemasterUD-foliage junction to D:\Arma_CWA\tmp\remaster-shadow-67),
960x640, 150% render scale, 4x MSAA, taken with the scratch build
`D:\Arma_CWA_worktrees\gfx-foliage\arma_CWR\dist\gfx-foliage-rwdi\PoseidonGame.exe`
and shader overrides from this folder.

## Best images

| File | What to look at |
|---|---|
| `BEST_backlit_hornbeam.png` | Camera into a 12 deg sun through a hornbeam/aspen grove. Current Remastered: pale grey-olive crowns. vH2: deep green crowns, the tree in front of the sun glows yellow-green, per-tree colour variety, bark stays bark. vHc adds the trees' shadows on the foreground that the shadow maps never draw in this view (see Findings) |
| `BEST_backlit_birch.png` | Cypresses and fig shrubs against the sun: crowns glow at their thin edges and near the sun, the far side of each bush stays dark |
| `BEST_forest_edge_golden.png` | The "pale grey-green crown at golden hour" case, front lit: the birch is 7% darker, 12% more saturated and warmer (R/B 1.84 vs 1.72), with a darker core |
| `BEST_overcast_broken.png` | Distant Everon forest blocks: with the canopy map (vHc) the grey "wall of trunks" along the forest edge becomes a dark forest interior |
| `BEST_noon_grove.png`, `BEST_noon_fields.png`, `BEST_overcast_forest.png` | Top-lit and sky-lit crowns: lit top, darker underside, brighter rims, more saturated |
| `BEST_pine_stand_noon.png`, `BEST_fields_golden.png`, `BEST_eden_forest_golden.png` | Canopy map cases: the first is a negative result (forest floor only gets darker), the other two show little change |
| `BEST_town_night.png` | Night: essentially no change (the night look belongs to the night track) |

Each BEST sheet is Classic | Remastered (current) | Remastered + prototype, same
view and sim time, GPU frame time on each panel, with a 2x crop row. All from
run `r9` (final shader `ov8`). `best_r8_ov7/` holds the same sheets from run r8
(shader ov7, before the flower-colour fix).

## Technique (shader `ov8/psRemasterCommon.glsl`, `shadeFoliageResearch`)

The engine already uploads, per shape, an ellipsoid fitted to the bounding box
(`CrownEllipsoid`, used by the shipped foliage shading for its crown normal).
The vertex shader override (`ov8/vsTransformRemaster.glsl`, plus the matching
varyings in `vsScreenRemaster`/`vsShadowRemaster`) also passes the leaf's
position in unit-sphere space `e` and the matrix `A` that maps a world
direction into that space. A world ray `p + t d` is `e + t A d` there, so one
ray/sphere intersection gives the distance in metres from the leaf to the crown
boundary along any direction. Depths are used as a fraction of the crown
diameter along that ray (so a 3 m bush and a 20 m hornbeam behave alike).

Per leaf fragment (half-lit material with a crown, research knob f0 = 1):

1. Sun self-shadowing: `T = exp(-tau * dSun)`, tau = optical depth across a
   whole crown (2.0). The front sun visibility is `min(shadowMap, T)`, so crowns
   keep their dappled shadow-map detail near the camera and still get a dark
   core and far side beyond the shadow-map range.
2. Translucency: leaves facing away from the sun transmit it toward the viewer,
   `m * tintT * exp(-0.75 tau dSun) * back * (0.2 + 1.8 cos^6)`, where cos is the
   angle between the view ray and the sun. Thin crown edges and leaves near the
   sun glow; the crown core and leaves far from the sun direction do not. The
   transmitted light is checked against other occluders with a 4-tap shadow
   lookup at the point where the sun ray leaves the leaf's own crown, so the
   crown never shadows its own glow but a tree in front of it does.
3. Leaf tint: transmitted and inter-leaf light is coloured by the leaf: the
   texture chroma (mixed 70% toward white) leaned toward a chlorophyll
   yellow-green, but only for texels that are saturated and greener than blue
   (`leafy`), so olive or brown leaf textures glow green instead of mustard,
   while white flowers, blue-grey leaves and bark keep their colour.
4. In-crown scattering: sun and sky light lost inside the crown is partly
   re-scattered by the leaves (msK), tinted by the leaf: the shade of a crown is
   a saturated green instead of grey.
5. Crown sky occlusion: sky light falls off with the crown depth toward the sky
   (average of the up and outward rays), replacing the shipped 0.6..1 ramp.
6. Sky rim: leaves at thin crown edges transmit the sky behind them (depth along
   the view ray), a soft bright rim against the sky.
7. Per-tree variation: a hash of the object origin gives each tree a +-10%
   value, a hue swing between yellow-green and blue-green and +-15% saturation.
8. Trunk mask: alpha test turned out to be on for whole tree draws, trunks
   included (debug shot `r5/backlit_hornbeam__dbgleaf.png`, all yellow), so bark
   is told apart geometrically: the thin column on the crown axis below the
   crown base (`r6/backlit_hornbeam__dbgleaf.png`, trunks green). Bark gets no
   translucency, tint or rim.
9. Local lights on crowns use the crown normal with a wrapped terminator and a
   glow-through term for lamps behind leaves (no visible effect in town_night).

The lit side of a crown keeps the shipped brightness (reflection term
`m * wrap(N.L) * visibility` equals the shipped `kd * 1 + forced` for a sun-facing
leaf). Measured on the crowns (work/r9_regstats.txt), vH2 against current
Remastered: golden birch luma -7%, saturation +12%; backlit hornbeam luma -1%,
saturation +21%; noon bushes luma -4%, saturation +16%; overcast spruces luma
-1.5%, saturation +24%. So it adds colour and depth, not just darkness.

### Knobs (prototype build, `triSetLighting "fN=value"`)

| Knob | Meaning | vH2 | vJ |
|---|---|---|---|
| f0 | 1 = research model (0 = shipped path, bit-identical shading) | 1 | 1 |
| f1 | crown optical depth across a diameter | 2.0 | 2.0 |
| f2 | translucency strength | 1.3 | 1.0 |
| f3 | per-tree variation amplitude | 1.0 | 1.0 |
| f4 | crown sky occlusion (1 = depth based) | 1.0 | 1.0 |
| f5 | in-crown scattering | 0.6 | 0.5 |
| f6 | packed: units debug view, tens green lean tenths, hundreds sky rim | 150 | 130 |
| f7 | weight of the analytic self-shadow | 1.0 | 1.0 |

Debug views (f6 units): 1 sun depth, 2 translucency only, 3 crown sky, 4 tree
hash, 5 exit-point visibility, 6 front sun visibility, 7 leaf/trunk mask.

## Canopy map (C++, `prototype.patch`, knobs c0..c7)

`EngineGL33::BuildCanopyMap` runs when the terrain horizon map is uploaded: it
walks every landscape object list, takes MapTree/MapSmallTree/MapBush (taller
than 2.5 m) and the three MapForest block types, and rasterises them (16
subsamples per texel; discs for trees, the oriented model box for forest
blocks) into a 2048x2048 RGBA16F texture over the whole island (6.25 m texels
on Abel and Eden): cover, canopy top and crown base above the terrain, both
premultiplied by cover so mipmaps average correctly. Abel: 68764 trees/bushes
and 1424 forest blocks in 123 ms; Eden: 29374 and 6273 in 131 ms (game log).
Bound on texture unit 6 (`canopyMap`), uv = world xz / island size.

Shader use (`ov8`, c0 sky, c1 sun, c2 debug, c3 extinction):
- Sky occlusion: the cover right above (6 m texels, not for leaves) and the
  stand around (25 m and 100 m mips), each counted while the point is below
  that canopy's top.
- Sun shadow beyond the shadow-map range: a 10-step march toward the sun over
  the canopy map and terrain height (steps 3 m growing x1.35, about 170 m),
  Beer-Lambert through the canopy layer; faded in where the cascades fade out,
  and used everywhere when the shadow maps are inactive.

vHc = vH2 + c0 = 1, c1 = 1, c3 = 0.12. vHs = sky part only.

## Runs (all on disk; `<run>/<view>__all.png` shows every label of a view)

| Run | Shader | What it tested | Outcome |
|---|---|---|---|
| r1 | ov2 | first volumetric model, depth in metres (sigma 1/m) | worked on thin crowns only; big hornbeams got dSun > 10 m and no glow (`r1/backlit_hornbeam__dbgdepth.png` mostly white). town_night lamps were off (switch-on delay), fixed later with a 900-frame settle |
| r2 | ov3 | depth as a fraction of the crown diameter | glow appears; texture-chroma tint made olive shrubs glow mustard (`r2/backlit_birch__vC.png`) |
| r3 | ov4 | | FAILED: `packed` is a reserved GLSL word, the override did not compile and the game used the built-in shaders; folder renamed `r3_FAILED_ov4_compile_error`, images there are not the prototype |
| r3b | ov4 fixed | green lean on the tint, sky rim | green instead of mustard, but too lime and uniform (`r3b/backlit_hornbeam__all.png`) |
| r4 | ov5 | separate transmission/fill tints, cos^6 lobe, deeper falloff | glow gathers around the sun, dark crown cores; trunks turned green |
| r5 | ov6 | alpha-test gate for bark | gate useless (alpha test on for trunks too) |
| r6 | ov7 + canopy build | geometric trunk mask, canopy map v1 | trunk mask works; canopy counted 10k grass tufts as canopy |
| r7 | ov7 + canopy build | canopy v2 (bushes under 2.5 m skipped), new views, shipped sun-visibility debug | forest-block interiors and into-sun tree shadows good; pine floor only darker |
| r8 | ov7 | full set, vJ / vH2 / vHc | white flower bush turned green (`work/r8_ob_flower.png`) |
| r9 | ov8 | final: gated leaf tint | flowers stay white (`work/r9_ob_flower.png`); source of the BEST sheets |
| t1, t2, t3 | ov3, ov7, ov8 | alternating GPU timing, no screenshots | below |

## GPU cost

Timestamp queries, alternating Remastered / vH2 / vHc on the same view, 120
settle frames each, 4 repetitions (work/t2_summary.txt, work/t3_summary.txt).
Other agents' games share the GPU, so single shots (the panel labels) are
noisy by several ms; the medians:

| View | Remastered | vH2 | vHc |
|---|---|---|---|
| backlit_birch | 5.53 | 5.92 (+0.40) | 6.21 (+0.69) |
| backlit_hornbeam | 5.81 | 5.78 (-0.04) | 6.68 (+0.87) |
| forest_edge_golden | 3.91 | 4.89 (+0.99, noisy) | 5.87 (+1.96, noisy) |
| pine_stand_noon | 5.39 | 5.42 (+0.03) | 5.75 (+0.36) |
| overcast_broken (Eden) | 5.49 | 5.17 (-0.32) | 5.33 (-0.16) |
| eden_forest_golden | 2.94 | 3.92 (+0.98) | 2.55 (-0.39) |

Honest reading: the crown model costs between nothing and about 1 ms at this
resolution (the samples where the machine was quiet, the backlit views, show
+0.0 to +0.4 ms); the canopy march adds roughly another 0.3 to 0.9 ms. The
exit-point shadow lookup is 4 taps per leaf fragment; the canopy march is up to
20 texture reads for fragments beyond the shadow range.

## Findings outside the prototype

- In camera-toward-the-sun views the shipped Remastered sun visibility is 1
  everywhere (`r7/backlit_hornbeam__dbgsunvis.png`, `r8`/`r9` same view: all
  white; `triGpuTimes` shows the shadow pass at 0.00 ms in these views, and the
  exit-point lookups in `r2/backlit_birch__dbgexit.png` are all 1). Trees cast
  no shadows toward the camera when it looks into a low sun, where a real
  scene would put the foreground in their shade. The canopy-map march fills
  that in (vHc), but the cascades themselves should be checked by the shadows
  track (light view built nearly parallel to the camera forward?).
- `pine_stand_noon`: the shipped shadow maps put the whole clearing in shade at
  noon (`r7/pine_stand_noon__dbgsunvis.png`, flat grey), so the forest floor has
  no sun flecks; the canopy map can only darken it further.
- Whole tree draws (leaves and trunk) run with alpha test / alpha-to-coverage
  on, so material state cannot tell bark from leaves.

## Verdict

Promising for the crown model (vH2), mixed for the canopy map.

- vH2 is a shader-only change that makes trees read as lit volumes: glowing
  translucent leaves against a low sun, dark crown cores, saturated green shade,
  bright rims against the sky and per-tree colour variety. The backlit views
  look like a different, lusher game while keeping CWA's textures and shapes.
  The golden-hour birch is less pale and warmer. Night shows no change.
- The canopy map gives two real wins (forest-block interiors instead of grey
  trunk walls; tree shadows in into-the-sun views where the shadow maps fail)
  and one clear failure (forest floors under dense pines only get darker,
  which the owner has rejected). It is a good base for forest-scale sky
  occlusion and far-field tree shadows, but needs tuning per case before it is
  shown as a look.

## What a production version needs

- Crown model: fold `shadeFoliageResearch` into `shadeRemastered` (or a
  foliage-only branch) with fixed constants instead of knobs; pass `e` and `A`
  per instance (flat varyings or a per-draw uniform: they are constant per
  shape and instance) rather than computing `inverse(mat3)` per vertex; extend
  the unit-test glslang compile to it. Classic stays untouched (separate program
  set), and the research path is a branch inside the remastered shader only.
- A better bark test than the axis heuristic (multi-trunk birches are only
  partly caught): a per-texture flag computed at load (opaque texture vs
  alpha-cut) or a per-section material property.
- The crown ellipsoid is the whole shape's bounding box; forest blocks get one
  50 m ellipsoid. Works, but a per-LOD crown box (excluding the trunk) would
  place the crown better.
- Canopy map: build it on the horizon-map worker thread, keep it per landscape,
  cap memory (2048x2048 RGBA16F plus mips is about 43 MB on the GPU), skip the
  sky term on forest floors (or pair it with sun flecks), and keep the march for
  beyond-shadow-range and for the into-the-sun failure case.
- Owner A/B on vH2 vs vJ (milder) before tuning; both are in every r8/r9 view.

## Risks against the owner taste list

- Greener foliage: crowns become noticeably greener and more saturated than
  current Remastered and Classic; at the strong end (`r3b` vE/vF) it went lime
  and cartoonish, so the green lean is gated and moderate in vH2. Could still be
  read as "too green" for Malden's dry look; vJ is the milder option.
- Darkness: vH2 keeps crown luma within -7%..0% of current; vHc darkens the
  foreground in backlit scenes (physically right, but "darker") and forest
  floors (wrong look). Do not ship the canopy sky term on floors as is.
- No blue haze, no grading: all changes are per-fragment lighting on foliage.
- Golden-hour warmth: the birch's R/B rises 1.72 -> 1.84 (warmer than current
  and Classic's 1.65), but the overall hue moves toward green-yellow.
- Remaining colour issue: the blue-grey leaves of the flowering bush in
  overcast_broken turn darker green (`work/r9_ob_flower.png`); flowers are fine.

## Files

- `ov2` .. `ov8`: shader override dirs per iteration (`ov8` final; `ov4` was
  fixed in place after the r3 compile error, and r3b used the fixed file).
  `ov1` is the earlier agent's start (vertex-shader varyings only).
  `ov8_vs_shipped.diff`: the final shaders against the shipped dump
  (`glsl_dump`, extracted from the scratch worktree source).
- `prototype.patch`: the C++ diff of `D:\Arma_CWA_worktrees\gfx-foliage`
  (research knobs f0..f7 and c0..c7 via triSetLighting, LitConstants slots
  58..61, texture unit 6, `EngineGL33::BuildCanopyMap`).
- `scripts/`: `gen_tests.py` (test generator: views, variants, timing runs),
  `collect.py` (per-run sheets and timings.md), `make_best.py`, `zoom.py`,
  `regstats.py`, `objs.py`, `find_backlit*.py` (spot finders over
  `PoseidonTools terrain objects` dumps `abel_objs.txt`, `eden_objs.txt`),
  `patch_canopy_cpp.py`, `make_ov7.py`, `make_ov8.py`, and
  `research_tests/` (copies of the generated .test.sqf/.toml and capture_lib).
- `r1`..`r9`, `t1`..`t3`: capture and timing runs (each has `tri/` with the game
  logs, `<view>__<label>.png`, `<view>__all.png`, `timings.md`).
- `work/`: zoom crops, region statistics and timing summaries.
- Logs: `build*.log` (scratch builds), `r*_run.log`, `t*_run.log`.

Reproduce: build the scratch worktree with `prototype.patch`, then from its
`arma_CWR`: `run-captures.ps1 -Tests tests/rendering/research/foliage/fol_abel_r9.test.sqf,tests/rendering/research/foliage/fol_eden_r9.test.sqf -GameDir dist/gfx-foliage-rwdi -DataJunction RemasterUD-foliage -ShaderOverrideDir <this folder>\ov8 -OutDir <out>`.
