# Wildcard track: sky visibility maps (rasterised "ray-traced" sky light)

Research prototype, not production code. Scratch worktree `D:\Arma_CWA_worktrees\gfx-wildcard`
(detached at 3c53141, the tip of claude/youthful-lovelace-8imbdh), build `build/gfx-wildcard-rwdi`,
game `dist/gfx-wildcard-rwdi/PoseidonGame.exe`. Nothing committed, no branch. The whole C++ and
GLSL change is `prototype.patch` (988 lines, `git apply --check --reverse` passes on the worktree).
Every image below was shot in game on the RemasterUD shadow through `run-captures.ps1` on the hidden
desktop, 960x640, 150% render scale, 4x MSAA, presented frame. This track started from an empty
folder (no earlier agent work existed for it).

## The pick, and why

The other tracks own SSGI, heightfield tracing, contact/soft shadows and SSR. The SSGI track's own
conclusion was that screen-space bounce is invisible in CWA daylight and that the "path traced" jump
would have to come from "large-scale sky occlusion that actually reaches objects". That is what a
path tracer gives a 2001 outdoor scene that no screen-space trick gives: the sky is a light, and every
object near a point blocks part of it, on and off screen, at metre scale. So the pick is a
compute-free GL 3.3 way to ray trace the sky against the real nearby geometry:

**Sky visibility maps.** K orthographic "shadow maps" around the camera, one per direction spread
evenly (equal solid angle) over the sky hemisphere above 6 degrees, rendered every frame from the
exact caster set the sun cascades already collect (opaque triangles plus alpha-cut foliage, already
uploaded, so each extra direction is one solid draw plus the alpha batches). Instead of depth, each
texel stores coverage-weighted depth moments of the nearest surface toward the sky, `(c, c*z, c*z^2,
foliage flag)`, plus the caster colour in a second target, all with full mip chains. In the lit
shader every receiver walks each direction's cone down the mip chain: all points of the ray `p + d*t`
project onto the same texel of that direction's orthographic map, so a cone trace is just reading
footprints of growing radius (`t * tan(cone)`) at one uv and counting the occluders whose mean depth
puts them near `t`, front to back. The result is per-pixel, normal-aware, colour-weighted sky
visibility (each direction weighted by the rig's own sky light from there, so blocking the warm
sunward horizon leaves a cooler fill) and a one-bounce estimate (occluder colour, lit by the sun where
the cascades say it is sunlit, plus sky). It is in effect 16 to 24 cone-traced rays per pixel against
real geometry, contact-hardening by construction, with no compute shaders, no BVH and no screen-space
holes.

## What was built (prototype.patch)

| Piece | File | What |
|---|---|---|
| Sky maps | `EngineGL33_ShadowDepth.cpp` | `RenderSkyMaps`: RGBA32F moments + RGBA8 colour arrays (K layers, 512^2, full mips), front faces stored (what the sky sees), depth clamp, round robin (a quarter of the directions per frame, `CWR_SKYVIS_STRIDE`), per-layer box-filter mip blits, each layer anchored to the camera position it was drawn from (`skyOff`). `SkyVis` UBO on binding 4, textures on units 6 and 8. Runs inside the shadow GPU timer. |
| Lit shader | `EngineGL33_LightingShaders.cpp` | `skyVisibilityMaps`: 6-step cone march per direction, foliage transmission (alpha casters pass `skyLeaf` of the sky), optional leaf tint, one-bounce, terrain receivers without normal offset, NaN-proof normal. Applied to the sky half of the ambient (`skyGround` extends it to the ground-bounce half), with a sky gain while on. Leaf receivers keep the crown term and skip the maps (`skyCrown=2`). Debug views `skyDebug=1/2`. |
| Knobs | `LightRig.hpp`, `GameStateExtTestRender.cpp` | `triSetLighting` keys: skyOcc, skyBounce, skyCone, skyBleed, skyRadius, skyDirs, skyDebug, skyCrown, skyAlbedo, skyLeaf, skyShell, skyFloor, skyTint, skyGain, skyGround, skyFaces. All off by default (skyOcc=0). |
| Night | `SceneDraw.cpp` | The depth pass also runs at night while skyOcc > 0 (the maps ride on its caster set). |
| Spawned vehicles | `ShapeDraw.cpp`, `Shape.hpp` | **Not mine**: the shade track's lazy vertex buffer fix (script-spawned vehicles otherwise bypass the remastered lighting through CPU T&L), copied so vehicles can receive the maps. Shown as its own panel. |

Final setting ("final" in the images): `skyOcc=1 skyBounce=1 skyAlbedo=1 skyLeaf=0.45 skyBleed=0.08
skyGround=1 skyDirs=24 skyCone=1.3 skyRadius=64 skyGain=1.15 skyCrown=2`.
"final_phys" is the same without the 1.15 sky gain and with skyLeaf 0.25 (darker, more physical).

## Test conditions that matter

- **Object shadows are on in every run of this build** (`triEnableShadows`). The shadows track found
  that a fresh profile never turns on static object casters, so trees and buildings cast nothing and
  are absent from the caster set. My maps are built from that set, so without it the sky maps contain
  only vehicles and men (run r4d `dbg_leaf` shows the town houses and the birch missing). Every
  comparison therefore has four panels: Classic (this build, object shadows on) | Remastered shipped
  exe (`dist/lighting-rwdi`, run r6s/r8s: object shadows off, cloud shadows off on clear views) |
  this build with maps off (object shadows + lazy vertex buffers) | + sky maps. Panel 3 to 4 is the
  effect of this track alone.
- Cloud shadows are forced off on clear-weather views (`cloudShadows=0`): the shade track found the
  cloud field shading a whole clear view at 0.2 transmission (town_day had no visible sun shadows
  until this was off).

## Bugs and engine facts found on the way

1. **NaN crown normals (shipped path too).** Some tree crowns (the Saint Louis birch, several conifers
   in forest_edge_golden and woods_mixed_noon) have NaN `nLight`: leaf cards with zero mesh normals
   give `normalize(0)` in the vertex shader, and degenerate crown ellipsoids give inf/NaN `vCrown`.
   The shipped shader hides it because GLSL `clamp`/`max` drop NaN on this driver, so those crowns
   silently fall back to forced diffuse plus ground-only ambient. My first runs rendered those crowns
   black (`r1/strip_town_day.png`, `r3/strip_forest_edge_golden.png`); `r5d/strip_*` `dbg_nan` paints
   them green. Worth a look by the foliage track: those crowns are lit wrong in the shipped exe.
2. Script-spawned vehicles skip remastered lighting (shade track's finding) reproduced: without the
   lazy vertex buffer the Jeep and M113 in lowsun_fp would not receive the maps at all.
3. A single wide blocker search (mean depth over a 6 m footprint, then one cone fetch) under-counts
   contact occlusion by about 2x: the mean mixes a whole hull's depths, so the cone footprint is
   too wide and straddles the occluder's edge (`r9d`, `r11d`, `r12d` per-direction probes). The
   6-step cone march fixed it (r13 onward).
4. Storing back faces (as the cascades do) lets the inside of a building mix into a footprint's mean
   depth and darkens the facade itself (`r14/crop_church_faces.png`, right two panels). Storing front
   faces (what the sky sees) removes most of it.

## What real data showed

Mean display luminance change against this build with maps off (`r16/measure_remastered.md`; 0-255;
"abs" is the mean absolute change, then the share of pixels changing by more than 4 levels):

| view | final: mean / abs / >4 | final_phys: mean / abs / >4 | read |
|---|---|---|---|
| overcast_close | +0.9 / 2.9 / 12% | -0.5 / 3.0 / 16% | the clearest win: a soft dark pool under the Jeep and along the M113's skirt, the Jeep's underside and wheel wells darker; vehicles sit on the grass instead of floating (`BEST_overcast_close.png`, crop row) |
| woods_mixed_noon | -1.7 / 4.2 / 26% | -5.0 / 5.0 / 30% | understory and trunk bases darker, sun patches keep their brightness, more contrast (`BEST_woods_mixed_noon.png`) |
| woods_overcast | -2.8 / 6.0 / 32% | -7.5 / 7.6 / 30% | forest floor darker and flatter grey; reads mostly as "darker" (`BEST_woods_overcast.png`) |
| town_street | -2.0 / 4.0 / 25% | -4.8 / 4.8 / 33% | facade bases and the bush darker; the church wall shows soft blotches from the 24 discrete directions (`BEST_town_street.png`, crop row) |
| town_day | -0.3 / 1.4 / 6% | -1.9 / 2.0 / 13% | pool under the birch, Jeep grounded; houses beyond about 50 m fade out of the maps (`BEST_town_day.png`) |
| woods_pine2_evening | -0.1 / 1.6 / 8% | -3.1 / 3.1 / 30% | small (`BEST_woods_pine2_evening.png`) |
| lowsun_fp | +1.0 / 1.3 / 3% | -0.3 / 0.4 / 2% | near zero: open ground, objects far apart, and the sun term dominates (`BEST_lowsun_fp.png`) |
| noon_fields, forest_edge_golden, overcast_broken | about +-1 | about +-0.6 | near zero (objects are few or beyond 50 m) |
| town_night | -0.1 / 3.6 / 14% | | nothing visible: local lights dominate, the abs change is the fire's smoke differing between shots (`r16/strip_town_night.png`) |

The sky visibility field itself (`skyDebug=1`, e.g. `cmp_debug_final.png`, `r16/strip_*`
`dbg_final`, `r14/strip_town_day.png`) looks like a ray-traced AO/sky-visibility pass: contact pools
under vehicles and men, darker ground under crowns, darker facades near the ground and under eaves,
sky light through foliage. The final frames carry that only partly, because the sky is a fraction of
the light and the cone-averaged occlusion around a 2 m object is 10 to 40%.

Motion: `rd/dolly_overcast_close.png` and `rd/dolly_town_day.png` (camera dollies 0.4 m per frame for
8 frames, maps updating a quarter of their directions per frame): no visible popping or swimming of
the pools at this scale; rotation cannot change the maps at all (they are world-anchored around the
camera position). Moving objects would lag up to 3 frames (not tested).

## GPU cost (this machine's GPU, which the SSGI track names an RTX 4060 Ti, shared with other agents' games)

Interleaved, three rounds of 120 frames per variant (`rt7/timing_summary.md`):

| view | maps off | 16 dirs, 48 m | final (24 dirs, 64 m) | sky-map rendering inside the shadow pass |
|---|---|---|---|---|
| lowsun_fp | 5.15 | 6.70 (+1.55) | 7.85 (+2.70) | ~0.85 / 1.25 |
| overcast_close | 5.20 | 6.16 (+0.96) | 6.80 (+1.60) | ~0.75 / 1.15 |
| town_day | 4.41 | 5.93 (+1.52) | 6.84 (+2.43) | ~0.8 / 1.1 |
| town_street | 4.09 | 6.27 (+2.18) | 7.21 (+3.12) | ~0.8 / 1.1 |
| woods_mixed_noon | 4.97 | 6.26 (+1.29) | 6.81 (+1.84) | ~0.9 / 1.3 |

So +1.0 to +3.1 ms per frame at 1440x960 with 4x MSAA: about 1 ms to draw the maps (a quarter of the
directions per frame) and the rest for the per-fragment cone march (24 x 6 RGBA32F trilinear fetches).
Letting leaf receivers use the maps too costs far more in woods: woods_mixed_noon 15.0 ms against 3.8
off (+11.2) without the foliage skip, 6.1 (+2.4) with it; woods_pine2_evening 15.3 / 5.6 / 9.6
(`rt6/timing_summary.md`, two rounds; leaf cards are the overdraw). The earlier 2-fetch estimator (rt4, rt5) was cheaper but under-counted contact.

## Variants tried (all images on disk)

| run | what | result |
|---|---|---|
| r1 | 16 dirs, 2-fetch blocker estimator, all layers per frame, glGenerateMipmap | works; ~8-9 ms map cost; black crowns (NaN); weak |
| r2, r4d, r5d | debug views | mapping verified (near-zenith map puts the pool under the soldier); houses and trees absent without object shadows; NaN normals found |
| r3, r6 | + leaf transmission, crown shell offset, round robin (4x cheaper maps) | effect small; NaN crowns still black |
| r7, r8 | NaN-proof normal, ground-bounce half, sky gain, leaf tint, near-first search | tint invisible; occlusion still under-counted |
| r9d, r11d, r12d | per-direction and per-mip probes | found the mean-depth blocker problem |
| r13 | 6-step cone march (override `ovr_cone`) | contact occlusion now strong; woods -11, street -8.5 levels; church wall blotches |
| r14 | front vs back faces | front faces halve the facade self-darkening |
| r15 | 24 dirs, wider cones, 64 m, calibrated gain | smoother; woods still -6.5 with leaf receivers included |
| rt6 | foliage receivers skip the maps | woods cost 15.3 -> 6.1 ms, crowns keep their look |
| r16 | final and final_phys on 11 views | the BEST images |

## Honest verdict

**Mixed.** The technique is real and works on real data: a compute-free, world-space, per-pixel
"ray-traced" sky visibility with one bounce, built from geometry the engine already submits, at a
reasonable 1 to 3 ms. It gives the one thing screen space cannot: correct grounding and sky
occlusion from off-screen and large occluders. On overcast close-ups it is a clear, owner-safe
improvement (vehicles sit in soft occlusion pools instead of floating, without darkening the frame).
In woods and streets it changes the frame visibly but mostly as darker shade, which is on the owner's
rejected list unless an exposure or local-exposure answer comes with it. On open sunny ground, which
is most of CWA, it changes almost nothing. It is not "a different game" on its own; it is the sky
half of what a path tracer would add, and it would pair naturally with the shadows track's object
casters (which it needs anyway) and a local exposure pass.

## What a production version needs

- Object casters on by default (shadows track) and a caster set that includes the region around the
  camera regardless of what is on screen; casters beyond about 50 m fade out today.
- Cheaper evaluation: half-resolution deferred (depth + normal, which the G-buffer track is building)
  instead of per-fragment forward, or per-vertex for terrain; 4 cone steps; RGBA16F moments relative
  to the receiver plane; direction culling by the normal before any fetch.
- Banding of discrete directions on large flat walls (`BEST_town_street.png`): per-pixel rotation of
  the direction set with temporal accumulation (the temporal track's TAA), or more directions.
- A real albedo for opaque casters (today they bounce a grey 0.45 display; only alpha-cut foliage
  carries its texture colour into the maps), if the bounce should colour-bleed.
- Moving-object lag from the round robin (update dynamic casters' layers every frame or render a
  separate dynamic layer).
- Exposure: an exposure or local-exposure term so woods and streets do not just get darker.
- The NaN crown normals fixed at the source (vertex shader / crown ellipsoid), independent of this.

## Risks against the owner's taste list

- "Only darker": real in woods and streets with final_phys (-3 to -7.5 levels); final (sky gain
  1.15) keeps the frame mean within about -2.8 to +1.0 levels but woods_overcast still reads darker
  and greyer.
- Crushed shade: the maps never touch the sun term; open shade keeps the rig's calibration (final
  raises it by the 1.15 gain), shade near objects drops further. No crowns are darkened (foliage skip).
- No haze, no grading, no hue shift (leaf tint was tried and is off; it made no visible change).
- Classic: untouched in shading; the prototype's object-shadow switch also changes Classic's
  projected shadows in these test runs, and the lazy vertex buffer is only used while Remastered is
  active.

## Files

- `prototype.patch`: the full C++/GLSL change (scratch worktree diff).
- `glsl_final/`: the final remastered GLSL extracted from the patched source (needs the patched C++
  for the SkyVis block and units 6/8; it is not a stock-exe override). `ovr_cone/`, `ovr_nofol/`,
  `ovr_dbg*/`: the override files used for r13, rt6 and the debug runs.
- `BEST_*.png`: Classic | Remastered shipped | Remastered + object shadows (maps off) | + sky maps
  (final), GPU frame ms per panel, with a 1.5-2.5x crop row where useful. `cmp_debug_final.png`:
  maps off | final_phys | final | the visibility field.
- `r*/strip_<view>.png`: every variant of a run; `r*/timings.md`; `r16/measure_remastered.md`;
  `rt4..rt7/timing_summary.md`; `rd/dolly_*.png`.
- `scripts/`: `gen_tests.py` (research capture tests), `collect.py`, `build_best.py`,
  `measure.py`, `timing_summary.py`, the patch helpers; `tests/`: copies of the final generated tests.
- Run folders r1-r16, r3s/r6s/r8s (shipped exe baselines), rt4-rt7 (timing), rd (dolly). r1 and r2
  hung at their end (my generated tests lacked `triEndTest`; their images are complete, their tri
  status is a failure); every later run ended cleanly.
