# Shadows track: contact shadows, contact-hardening soft shadows, crisper and fuller shadow maps

Research prototype on a detached scratch worktree (`D:\Arma_CWA_worktrees\gfx-shadows`, at the
lighting branch tip 3c53141, build `build/gfx-shadows-rwdi`, game `dist/gfx-shadows-rwdi`). Nothing
committed, no branch. The C++ and GLSL change is `prototype.patch` in this folder. Every image named
below was produced by an in-game capture run through `run-captures.ps1` on the hidden desktop, on
the RemasterUD shadow, at 960x640, 150% render scale, 4x MSAA, presented frame (same rig as the
`20261017-final` baseline).

## Headline

1. **The biggest shadow win is not a filter, it is a missing caster class.** In every capture run
   (and in any fresh profile) trees, bushes and buildings cast no sun shadow at all, in Classic or
   Remastered. `Scene::_objectShadows` defaults to false; `LoadAndApplyGraphicsConfig()` runs before
   `InitializeWorld()` in `GameApplication.cpp` (lines 681 then 683), so its `if (GScene)` branch in
   `ApplyGraphicsConfigToEngine` that would switch object shadows on never runs at boot. Only the
   Graphics options page (or a legacy `objectShadows` entry in the user profile) turns them on. The
   sun-visibility debug view (`debug=1`) of `forest_edge_golden` shows it plainly: only the soldier
   casts (`run2/shots/forest_edge_golden__dbgsunvisrem.png`). With object casters on, a golden-hour
   forest edge gets long tree shadows raking across the slope, shade pooled under every crown, bushes
   anchored to the ground and crowns with a shaded side: `BEST_forest_edge_golden.png`,
   `breakdown_forest_edge_golden.png`. This is the one change in this track that makes a frame read
   differently at a glance.
2. **Contact-hardening soft shadows (PCSS) work and fix "floating" shadows.** With the sun as a disk
   (blocker search in the raw depth map, penumbra = blocker distance x tan(sun radius)), shadows are
   sharp where a caster touches the ground and soft at the far end. The visible gain is grounding:
   in `forest_edge_golden` the stock soldier shadow starts about a boot-length away from his feet;
   PCSS alone brings it closer, and `best` (PCSS + 4096 tight tiers + contact) joins the boots
   (`scratch/r6_soldier_feet_zoom.png` is the clearest crop; also
   `zoom_forest_edge_golden_rem_vs_best.png`). The effect is a few pixels at 960x640.
   Long tree shadows get soft, natural ends instead of cut-out leaf edges. A 1 degree sun is the
   sweet spot; 0.53 (physical) looks like stock with crisper edges; 2 and 3 degrees get mushy.
3. **Screen-space contact shadows are a near-negative result in these views.** The march works
   (debug view `run1/shots/lowsun_fp__dbgcontact.png`: wheels, boots, track skirt, tent hem), but at
   1440x960 with a 3 cm near shadow-map texel the shadow map already shadows almost everything the
   ray finds, and the post pass may only remove the direct-sun share that is still there. The final
   frames are nearly unchanged. The far-field variant (ray length scaled with distance) only adds
   streaks at tree-trunk bases; it cannot produce tree shadows tens of metres long, and alpha-to-
   coverage foliage cannot receive it. Cost is small (+0.07 ms composite). Keep as a cheap polish
   item at best.
4. **4096 maps and tighter near tiers** make near detail crisper (tent guy ropes, rifle, the soldier's
   arms now show in their shadows, `zoom_lowsun_fp_rem_vs_best.png`). On their own they do not change
   how a frame reads.
5. **Inside woodland the casters give dappled sunlight** (resumed session, round 8): sun patches and
   crown shadows on the forest floor at noon (`BEST_woods_mixed_noon.png`) and long raking tree
   shadows over a meadow at 17:30 (`BEST_woods_pine2_evening.png`). A cheap production candidate
   (`prod`: casters + PCSS 1 deg on the stock 2048 map, stock caster LOD, tighter near tiers) looks
   the same as the 4096 full-LOD `best` in these frames at a fraction of the shadow-pass cost.
6. **Motion** (resumed session, rounds 6-7): no measurable extra flicker in the colour frames during
   a slow dolly. The sun-visibility term alone does flicker on tree crowns (leaf-card self-shadow
   under alpha aliasing), but in colour it stays below the foliage's own aliasing.
7. **The "only darker" risk is real in woods**: two of the three woodland frames with casters are
   14% and 21% darker than Classic in mean brightness (`run8prod/frame_brightness.md`); exposure
   x1.25 recovers a quarter to a half of that gap. Town and open views stay at Classic's brightness.

Honest verdict: **promising, but the star is object shadows + PCSS at 1 degree, not contact
shadows.** It is a real "lived-in light" improvement in wooded and built-up terrain at low sun, and
nearly invisible on open ground at noon or on the Sinai camp ground (no static casters there).
Whether the owner accepts it depends on the woodland darkening (point 7), which needs a canopy
transmission term or a scene-aware exposure meter before it ships.

## Resumed session (2026-10-17 evening): rounds 6-8

Same scratch build and patch as before (the exe in `dist/gfx-shadows-rwdi` was built 14:50 from the
sources in `prototype.patch`; no C++ change in this session). All runs on the hidden desktop with
their own junctions (`RemasterUD-shadows6/7/8`), sharing the GPU with 3-4 other agents' games, so
GPU numbers are single noisy samples again.

### Round 6, motion in colour (`run6motion/`, `gen_motion.py`, `motion_analyse.py`)

`lowsun_fp` and `forest_edge_golden`, variants `rem`, `obj` (casters, stock filter) and `best`;
4 frames with the camera fixed, then 16 frames dollying sideways 2 cm per frame, sim clock pinned
per frame index so every variant sees the same camera and clock on frame k.

| view | variant | static mean abs diff | moving mean abs diff | 2nd temporal diff, mean | 2nd diff in shadow crop |
|---|---|---|---|---|---|
| forest_edge_golden | rem | 0.00003 | 0.01407 | 0.01880 | 0.01719 |
| forest_edge_golden | obj | 0.00004 | 0.01369 | 0.01849 | 0.01670 |
| forest_edge_golden | best | 0.00004 | 0.01380 | 0.01872 | 0.01712 |
| lowsun_fp | rem | 0.00002 | 0.01096 | 0.01718 | 0.02881 |
| lowsun_fp | obj | 0.00003 | 0.01097 | 0.01716 | 0.02876 |
| lowsun_fp | best | 0.00003 | 0.01095 | 0.01720 | 0.02885 |

Static frames are stable (the PCSS rotation is fixed per pixel, not per frame). In motion the three
variants are within 2% of each other; the heat maps (`run6motion/cmp/*__jerkmap.png`) show the
temporal energy sits on foliage alpha edges and the ground texture, the same in all three. GIF crops
for a human look: `run6motion/cmp/<view>__side.gif`.

### Round 7, sun visibility in motion and woodland stills (`run7woods/`, `gen_r7.py`, `motion_sunvis.py`)

The debug sun-visibility term (`debug=1`) on the same `forest_edge_golden` dolly isolates the
shadows from texture aliasing (`run7woods/motion_sunvis.md`, `run7woods/cmp/sunvis_jerkmap.png`):

| variant | mean abs 1st diff | mean abs 2nd diff | pixels with mean 2nd diff > 0.05 |
|---|---|---|---|
| vrem | 0.00108 | 0.00146 | 0.8% |
| vobj | 0.01317 | 0.01755 | 14.1% |
| vbest | 0.01625 | 0.02339 | 16.4% |

The extra instability is almost all on tree crowns, not on the ground shadows. The cascades are
world-snapped (`SceneShadowPass.cpp` line 387), so this is not cascade crawl; it is leaf-card
self-shadowing seen through alpha-test aliasing (as the camera moves, a pixel flips between an
outer, sunlit card and an inner, shaded one). In the colour frames of round 6, inside a mask of
those crown pixels, the 2nd temporal difference is rem 0.0276, obj 0.0258, best 0.0269 (absolute)
and, relative to the darker crowns, 7-10% higher with casters. Not a visible new artefact at this
speed; a TAA or a crown self-shadow fade with distance would remove the residue.

Woodland stills (spots chosen from `PoseidonTools terrain objects abel.wrp`): `woods_pine_morning` /
`woods_pine_evening` (6888, 9944, heading 210) turned out to be blocked by a near pine crown and are
not used for judgement. `woods_mixed_noon` (2864, 6656, heading 300, 14:00, mixed aspen/birch/pine,
`scratch/r7_mixed_noon_grid.png`) shows dappled sun on the forest floor with casters. Measured on
the ground (lower 190 rows) through the sun-visibility mask: sunlit ground is identical in `rem`
and `best` (RGB 144/137/100), shaded ground is 75/74/56, about 0.52 of lit and cooler (R/B 1.34
against 1.44). The shaded floor reads greyer than the sunlit sand; see Risks.

### Round 8, a cheaper production candidate and exposure (`run8prod/`, `gen_r8.py`)

The first attempt (`run8_INVALID_exposure_leak/`) is invalid: `triSetLighting "remastered"` does
not reset tuning knobs, so the `exposure=1.25` of the `prodexp` shot leaked into every later shot
of that run, `rem` included. The generator now sets `exposure=1` on every variant and the rerun is
`run8prod/`. (Rounds 1-7 never set `exposure`; every knob they set is in the reset list, so they
are not affected.)

Variants: `prod` = casters + PCSS 1 deg on the stock 2048 map, stock caster LOD (3), near tiers
0.04 / 0.15; `prodexp` = `prod` with exposure x1.25 (a stand-in for a scene-aware meter); `best` as
in round 4. Six-panel grids `run8prod/cmp/<view>__grid6.png` (Classic, rem, prod, prodexp, best,
prod sun visibility).

- `woods_pine2_evening` (6940, 9964, heading 240, 17:30, sun 20 deg front-right): long tree
  shadows with lit streaks over the meadow, the jeep's nose in a gap of sun. Strong mood change,
  and the darkest frame of the set.
- `woods_mixed_noon`: dappled floor as in round 7.
- `woods_pine2_morning` (sun behind): tree shadows on the right, the meadow stays sunlit; moderate.
- `town_golden`: the lawn in house shadow; frame brightness equals Classic's.
- `forest_edge_golden`, `lowsun_fp`: as in round 4.
- `prod` and `best` are hard to tell apart at 960x640 in every view.

Mean frame brightness (display value of the mean linear luminance; `run8prod/frame_brightness.md`):

| view | Classic | rem | prod | prodexp | best |
|---|---|---|---|---|---|
| woods_pine2_morning | 0.352 | 0.367 | 0.357 | 0.386 | 0.357 |
| woods_pine2_evening | 0.336 | 0.368 | 0.265 | 0.284 | 0.265 |
| woods_mixed_noon | 0.362 | 0.383 | 0.310 | 0.334 | 0.310 |
| town_golden | 0.352 | 0.366 | 0.353 | 0.359 | 0.352 |
| forest_edge_golden | 0.372 | 0.434 | 0.425 | 0.443 | 0.425 |
| lowsun_fp | 0.525 | 0.552 | 0.552 | 0.564 | 0.551 |

Shadow-pass GPU time per shot (ms; single samples on a shared GPU, `run8prod/timings.md`):

| view | rem | prod | best |
|---|---|---|---|
| woods_pine2_morning | 0.06 | 0.22 | 1.39 |
| woods_pine2_evening | 0.04 | 1.41 | 3.20 |
| woods_mixed_noon | 0.03 | 0.54 (1.09 / 1.41 in the next two shots) | 2.37 |
| town_golden | 0.04 | 0.22 | 1.15 |
| forest_edge_golden | 0.03 | 0.18 | 1.81 |
| lowsun_fp | 0.14 | 0.06 | 0.11 |

Whole-frame GPU numbers in the same run swing by 2-10 ms between neighbouring shots of one variant
(for example `woods_pine2_morning` prod 11.48, its debug twin 16.04) and are not interpreted.

## What was built (prototype.patch)

| Piece | Where | What |
|---|---|---|
| Knobs | `LightRig.hpp` `LightingTuning`, `GameStateExtTestRender.cpp` `triSetLighting` | `pcssSun` (sun disk diameter, deg; 0 = stock filter), `pcssSearch`, `pcssMin`, `contact`, `contactLen`, `contactThick`, `contactFar`, `contactFarThick`, `contactFarMax`, `contactSteps`, `contactFade`, `objShadows` (Scene object casters), `casterLod` (shadow caster LOD bias), `debug=5` (contact occlusion view) |
| Raw depth sampler | `EngineGL33_Lighting.cpp/.hpp`, `EngineGL33_Shaders.cpp` | The shadow map array bound a second time on unit 6 with a non-comparison NEAREST sampler object (`shadowDepth`), for the blocker search |
| PCSS | `EngineGL33_LightingShaders.cpp` `pcssLit` | Centre tap + 16-tap rotated disk blocker search (radius = 40 m x tan(sun radius), clamped 2-48 texels); mean blocker depth to metres via the cascade VP's z row; PCF radius = d x tan(r) in texels (min 0.6); Vogel spiral of 12/24/32 hardware 2x2 comparison taps by penumbra width; per-pixel rotation (interleaved gradient noise). LitConstants slot 58 `litPcss` |
| Contact shadows | `EngineGL33_PostFX.cpp` composite | Full-res march toward the sun in the resolved depth buffer (16 or 24 steps, jittered), thickness test, softens along the ray, fades with distance; removes `c.a` (the non-sky share the remastered shaders already store) x occlusion, only on opaque lit pixels (`c.a < 0.999`) |
| Shadow map setup | existing tri verbs | `triShadowSetRes 4096`, `triShadowSetOmniR0 0.04`, `triShadowSetOmniR1 0.15` (at VD 1500 the near spheres go from 31 and 61 m radius at 3.0 cm per texel to 17 and 63 m at 0.8 cm per texel), `triShadowSetDistance` |

Classic is untouched in the prototype except that `objShadows=1` also turns on Classic's own object
shadows; every Classic shot in these runs had it off (the default).

## Runs and variants

All runs used the scratch build, so "Remastered (current)" in every comparison is the stock remastered
path of the same binary (all new knobs at 0, 2048 map, object casters off), shot in the same game
session, same view, same sim clock as the variant.

| Run | Folder | Variants | Notes |
|---|---|---|---|
| r1 | `run1/` | contact; PCSS 0.53 / 2 deg + 4096 + contact; PCSS 2 + full-LOD casters; debug views | Abel only (my test lacked `triEndTest`; I stopped my own game after the last shot, Sinai never ran) |
| r2 | `run2/` | PCSS 1; PCSS 1 + tight tiers; far-field contact; combos with 1 and 3 deg; debug views on every view | Abel + Sinai |
| r3 | `run3/` | object casters on (stock filter); + PCSS 1 + 4096 tight; + PCSS 2 + contact; debug | Abel (+ new views `forest_side_golden`, `town_golden`) + Sinai |
| r4 | `run4/` | `best` = object casters + PCSS 1 + 4096 tight + full-LOD casters + contact; `bestlong` = best with shadow range x2; debug; plus a 3x alternation cost test | Abel + Sinai |
| cost2 | `run5cost/` | 8x alternation of rem / pcssonly / contactonly / obj / best on two views, timings only | see Cost |

Each run folder: `shots/<view>__<variant>.png` (copied captures), `cmp/<view>__<variant>.png`
(Classic | Remastered current | variant, GPU frame ms on each panel) and `__zoom` crops,
`timings.md/json`, `tri/` (game logs with every `UDCAP` line).

## What the real data showed, per view

- **forest_edge_golden** (sun 12 deg behind left): stock has no tree shadows at all. Object casters
  give long diagonal tree and bush shadows up the slope and shade under the crowns
  (`breakdown_forest_edge_golden.png`: Remastered | +casters stock filter | +PCSS 1 deg 4096 |
  +PCSS 2 deg + contact). The stock filter makes them crisp, leaf-cut; PCSS 1 deg softens their far
  ends the way a real low sun does; 2 deg is too soft. The pines are darker overall because their
  inner leaves are now in their own shade (foliage keeps 45% of the sun in shadow, so not black).
  The soldier's shadow joins his boots with PCSS.
- **forest_side_golden** (new view, same spot, heading 40): casters add shade streaks on the right
  slope and under the near tree; moderate change.
- **town_golden** (new view, Saint Louis, 18:15): with casters on, the houses and the big tree put
  most of the street and the jeep in shade. Physically right, but the frame is mostly darker. This
  is the owner's "only darker" risk; see Risks.
- **lowsun_fp** (open field, vehicles): PCSS and 4096 sharpen the near shadows (tent guy ropes show,
  jeep frame detail), soften the far tip of the jeep shadow, and turn the blocky, broken far end of
  the stock jeep shadow into one continuous soft tip (`scratch/r1_sunvis.png`, sun visibility:
  stock | PCSS 2 deg). Change in the final frame: small.
- **noon_fields**: almost no change. Distant trees now cast short noon shadows, but from eye height
  they are hidden under the crowns. Contact shadows invisible here.
- **sinai_lowsun_fp**: almost no change (no static casters at the camp; PCSS sharpens the HMMWV and
  rifleman shadows a little).

## Cost (honest)

GPU timing was heavily contaminated: up to four games from other agents shared the one RTX 4060 Ti
during every run (tri slot holders logged in `run*/tri.log`), so absolute frame times drift by 2-3x
between consecutive shots (noon_fields Classic measured 19-24 ms here against 6.0 ms in the quiet
baseline). Numbers below are medians of paired differences (variant minus the stock shot taken in
the same alternation), from `run4/cost_paired.md` (3 alternations) and `run5cost/cost_paired.md`
(8 alternations). Treat everything but the shadow-pass column as plus or minus a millisecond or two.

8 alternations, `run5cost/cost_paired.md` (variant names: `pcssonly` = PCSS 1 deg on stock 2048 map; `contactonly` = contact shadows only; `obj` = object casters, stock filter; `best` = casters + PCSS 1 + 4096 tight + full-LOD casters + contact):

| view | variant | n | median frame (ms) | median paired delta vs rem: frame | world | shadow pass | composite |
|---|---|---|---|---|---|---|---|
| forest_edge_golden | best | 8 | 13.03 | -0.02 | -0.21 | +1.37 | +0.07 |
| forest_edge_golden | contactonly | 8 | 9.22 | -1.33 | -1.36 | +0.00 | +0.07 |
| forest_edge_golden | obj | 8 | 10.61 | -0.51 | -0.47 | +0.58 | +0.00 |
| forest_edge_golden | pcssonly | 8 | 11.04 | -0.05 | +0.01 | +0.65 | +0.00 |
| forest_edge_golden | rem | 8 | 11.66 | +0.00 | +0.00 | +0.00 | +0.00 |
| lowsun_fp | best | 8 | 17.05 | +0.85 | +0.82 | -0.20 | -0.03 |
| lowsun_fp | contactonly | 8 | 14.20 | -1.25 | -1.07 | +0.02 | -0.22 |
| lowsun_fp | obj | 8 | 13.67 | +0.08 | +0.35 | +0.15 | -0.24 |
| lowsun_fp | pcssonly | 8 | 16.07 | +1.32 | +1.26 | -0.34 | +0.04 |
| lowsun_fp | rem | 8 | 15.80 | +0.00 | +0.00 | +0.00 | +0.00 |

3 alternations, `run4/cost_paired.md` (adds `best2048` and `bestlong`, and `town_golden`):

| view | variant | n | median frame (ms) | median paired delta vs rem: frame | world | shadow pass | composite |
|---|---|---|---|---|---|---|---|
| forest_edge_golden | best | 3 | 12.46 | +2.13 | +2.02 | +1.30 | +0.07 |
| forest_edge_golden | best2048 | 3 | 12.96 | +2.63 | +2.53 | +0.20 | +0.07 |
| forest_edge_golden | bestlong | 3 | 13.72 | +6.01 | +5.93 | +1.24 | +0.07 |
| forest_edge_golden | contactonly | 3 | 7.90 | -0.89 | -0.95 | +0.00 | +0.07 |
| forest_edge_golden | obj | 3 | 11.19 | +0.86 | +0.84 | +0.82 | +0.00 |
| forest_edge_golden | pcssonly | 3 | 11.55 | +1.22 | +1.21 | +0.64 | +0.00 |
| forest_edge_golden | rem | 3 | 10.33 | +0.00 | +0.00 | +0.00 | +0.00 |
| lowsun_fp | best | 3 | 18.02 | +4.21 | +3.93 | -0.21 | +0.23 |
| lowsun_fp | best2048 | 3 | 17.76 | +4.89 | +4.59 | -0.30 | +0.31 |
| lowsun_fp | bestlong | 3 | 18.64 | +8.19 | +7.67 | -0.20 | +0.28 |
| lowsun_fp | contactonly | 3 | 14.75 | -0.06 | +0.02 | +0.31 | -0.08 |
| lowsun_fp | obj | 3 | 11.52 | -0.01 | +0.00 | +0.07 | -0.01 |
| lowsun_fp | pcssonly | 3 | 15.06 | +3.43 | +3.33 | -0.26 | +0.10 |
| lowsun_fp | rem | 3 | 12.87 | +0.00 | +0.00 | +0.00 | +0.00 |
| town_golden | best | 3 | 11.24 | +6.43 | +6.45 | +1.53 | -0.01 |
| town_golden | best2048 | 3 | 10.51 | +6.98 | +6.31 | +0.18 | +0.75 |
| town_golden | bestlong | 3 | 8.49 | +4.82 | +4.51 | +1.70 | +0.34 |
| town_golden | contactonly | 3 | 7.21 | +0.00 | +0.01 | +0.00 | -0.06 |
| town_golden | obj | 3 | 8.29 | +1.66 | +1.28 | +0.34 | +0.41 |
| town_golden | pcssonly | 3 | 6.21 | -1.00 | -0.23 | +0.00 | -0.53 |
| town_golden | rem | 3 | 6.31 | +0.00 | +0.00 | +0.00 | +0.00 |

Reading: the frame and world deltas swing by more than a millisecond in both directions between runs (contact shadows cannot make a frame 1.3 ms faster; that is contention), so the whole-frame cost of `best` is not resolved by these runs: the 8-alternation medians say about 0 to +0.9 ms, the 3-alternation medians +2.1 to +6.4 ms. The honest statement is "somewhere between free and about 6 ms at 1440x960 4x MSAA on an RTX 4060 Ti, most likely 1.5 to 3 ms (depth pass plus PCSS taps)", to be timed again on a quiet GPU. The shadow-pass column is the stable signal: +1.3 to +1.7 ms for `best` in the wooded and town views in every run, +0.6 to +0.8 ms for casters at 2048. Even that column carries about 0.5 ms of noise: `pcssonly`, which does not touch the depth pass, reads +0.65 ms on it in `forest_edge_golden`. Composite +0.07 ms for contact shadows reproduces in every run where shafts are off. `bestlong` (range x2) measured +4.8 to +8.2 ms frame in run4 and showed no visible gain.

What is solid: the shadow depth pass goes from ~0.03 ms (vehicles and soldiers only) to ~0.8 ms with
tree and building casters at 2048 and ~1.3-1.7 ms at 4096 in the wooded views; the contact-shadow
composite adds ~0.07 ms at 1440x960. PCSS itself (blocker search + up to 32 taps per receiving
fragment, with foliage overdraw) is the most uncertain number: +1.3 ms median over 8 alternations
on `lowsun_fp`, +3.4 ms over 3 on the same view, about zero on `forest_edge_golden`; call it 0 to 3 ms
until it is timed on a quiet GPU. 4096 maps cost 4x the memory of 2048 (4 layers of 4096^2 depth: 256 MiB if the driver
stores DEPTH24 in 32 bits, against 64 MiB). CPU: caster collection for trees is cached per static
object (`s_staticCasterCache`), but the first frames after a camera move into a forest rebuild it.

## Risks against the owner's taste list

- **"Only darker"**: object casters darken frames that are full of trees or houses at low sun
  (`BEST_town_golden.png`, the pines in `BEST_forest_edge_golden.png`). The shade itself keeps the
  sky fill (~0.6 of lit per the calibration table), it is not crushed, but the average frame drops.
  A production version should judge this on more views and may want the exposure meter to see shade
  coverage, or foliage self-shadow kept lighter than ground shadow. Round 8 measured it: woodland
  frames 14-21% below Classic's mean brightness, town and open ground unchanged.
- **Grey shade**: shaded forest floor is cooler and less saturated than the sunlit sand next to it
  (R/B 1.34 against 1.44 in `woods_mixed_noon`), because the shade is lit by the blue sky hemisphere
  only. Physically right, but close to the "greyer ground" complaint. Real canopy shade is greener
  and brighter (light through leaves); see production item 7.
- **Flattened crowns**: the opposite happens here; crowns gain a lit and a shaded side from their
  own leaves. Watch for leaf-card acne: the sun-visibility debug shows halftone patterns on crowns
  (`scratch/r3_forest_dbg.png`); they are not visible in the colour frames I looked at
  (`scratch/r3_crown.png`), but may shimmer in motion.
- **Grain**: PCSS uses per-pixel rotated taps without temporal accumulation; wide penumbrae show a
  fine dither at 1:1 (`scratch/r1_sunvis.png`, right). Fixed in screen space, so it can crawl when
  the camera moves. A TAA or a blur pass would hide it.
- No blue wash, no grading, no new art. Classic untouched (object casters are a shared toggle and
  must stay off for Classic in production, or Classic's projected object shadows appear).

## What a production version needs

1. Fix the boot order so the Graphics shadow tier really drives `SetObjectShadows` (apply the scene
   part of `ApplyGraphicsConfigToEngine` after `InitializeWorld`, or have Scene read the tier), and
   decide per lighting mode: Remastered on, Classic as the player had it.
2. PCSS as the remastered default at a 1 degree sun, with the blocker search on a min-depth mip or
   a coarser search for tiers 2-3, and a temporal or bilateral smoothing of the penumbra.
3. Foliage receivers: a larger normal offset or slope bias against leaf-card acne; possibly a
   lighter self-shadow factor for crowns.
4. 4096 only on a High/Ultra shadow tier; tighter omni tiers (0.04/0.15) are free and help first
   person.
5. Contact shadows: optional. If kept, write a real direct-sun share (not one minus the sky share)
   so local lights and fog are not darkened, and let A2C foliage receive.
6. Do not ship `bestlong` (range x2): no visible gain in these views for ~+4-8 ms of noisy cost.
7. Canopy transmission: leaf-card casters block 100% of the sun in a depth-only map. A
   transmittance layer for alpha casters (a colour attachment in the shadow pass storing the
   minimum leaf transmittance, tinted by the leaf texture's average colour) would let 15-25% of the
   sun through crowns, green the shade and lift the woodland frames back toward Classic. Not
   prototyped here.
8. Start from `prod` (casters + PCSS 1 deg, 2048, stock caster LOD, tight near tiers): it matches
   `best` visually in rounds 7-8 at a shadow pass of roughly 0.2-1.4 ms instead of 1.2-3.2 ms.
9. Capture tooling: `triSetLighting "remastered"` keeps every tuning knob; any A/B script that
   sets a knob must reset it explicitly (round 8's first attempt was spoiled by this).

## Files

- `prototype.patch` - the full C++/GLSL change against 3c53141.
- `gen_tests.py` - generates the research capture tests (rounds r1-r4, cost, cost2); `tests/` - the
  generated `.sqf/.toml` and the extended `capture_lib.sqf` (`capVariant`).
- `collect.py` - copies shots, parses UDCAP timings, builds the labelled three-way comparisons;
  `crop.py` - zoomed crops; `cost_paired.py` - paired cost medians.
- `BEST_<view>.png` - Classic | Remastered (current) | best (object casters + PCSS 1 deg + 4096 tight
  + full-LOD casters + contact), from run4.
- `breakdown_forest_edge_golden.png`, `zoom_*.png` - the detail comparisons quoted above.
- `scratch/` - working crops referenced in this file; `scratch/r6_soldier_feet_zoom.png` is a 4x
  crop of the `forest_edge_golden` soldier (rem | PCSS 1 deg only | best): the stock shadow starts
  a few pixels off his boots, PCSS brings it closer, `best` joins it.
- Resumed session: `gen_motion.py` + `motion_analyse.py` (round 6), `gen_r7.py` + `motion_sunvis.py`
  (round 7), `gen_r8.py` + `brightness.py` + `grid.py` (round 8); run folders `run6motion/`,
  `run7woods/`, `run8prod/` (valid) and `run8_INVALID_exposure_leak/` (kept only as a record, do
  not use); `BEST_woods_mixed_noon.png`, `BEST_woods_pine2_evening.png` (Classic | Remastered
  current | prod).
