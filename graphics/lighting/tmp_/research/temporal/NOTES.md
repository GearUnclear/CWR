# Temporal track: TAA and temporal accumulation as the enabler

Track id `temporal`. C++ prototype in the scratch worktree `D:\Arma_CWA_worktrees\gfx-temporal`
(detached at 3c53141, the tip of claude/youthful-lovelace-8imbdh), built to
`arma_CWR\dist\gfx-temporal-rwdi`. Patch: `prototype.patch` (regenerated with
`git diff --output` on 2026-10-17 18:27; `git apply --check --reverse` passes against the worktree).

This folder was started by an earlier agent that was stopped before writing notes.
The resumed session reused its build and runs 01 to 13 and its final sheets, and added:
run 14 (third interleaved timing run), run 15 (convergence sequence and effects under
motion), new sheets (convergence, motion, ghosting grid, long-shadow crops, sharpen sweep,
smoke crop), the noise metric, and these notes.

## Verdict in one paragraph

TAA on its own is not the "different game" moment: against the current 4x MSAA at 150%
it trades about 6 to 18 % of measured sharpness for a temporal-stability gain that is real
on foliage (about 25 % less frame-to-frame residual on the forest edge) and negligible
elsewhere. Without MSAA it erodes sub-pixel antennas. TAA does work as an
enabler, and that part is a clear positive result. A 4-tap stochastic contact-hardening sun
shadow (PCSS) and a 4-tap unblurred AO look broken up and dotted on their own. Under TAA they settle
in about 2 to 4 frames to the look of a 16-tap reference, and they stay clean while the camera
pans or strafes. The soldier's shadow on the forest slope goes from a uniformly sharp
cut-out to a shadow that is sharp at the boots and soft at the head, which is the
most "path traced" cue this track produced. The TAA resolve and sharpen cost
0.07 ms at 960x640 and 0.14 ms at 1440x960. Production needs per-object motion
vectors, a particle/reactive mask, HDR-domain resolve and an answer for vehicle interiors.
**Self verdict: promising as infrastructure, mixed as an AA replacement.**

## Technique (what the patch does)

All of it is gated by `triSetLighting taa=1` (default 0) and only runs in Remastered with
post effects on. With TAA off, the jitter offset is 0 and the new code paths are skipped.

1. **Sub-pixel jitter.** Halton(2,3), 16-frame cycle, in render-target pixels. It rides in
   the free `vpScale.zw` slot of the shared VS constants. Every vertex shader (Remastered
   and Classic sources, TL and non-TL, instanced) adds `vpScale.zw * gl_Position.w`.
   `ApplyTaaJitter()` sets it only for the world's 3D passes (`BeginPass`), and sets it to zero for the
   HUD, dialogs and portraits (`EndWorldScene`), so the UI is never jittered.
2. **Optional texture LOD bias** (`taaMipBias`, best -0.5) on the material sampler objects
   while TAA runs, to win back texture detail the accumulation blurs.
3. **Resolve pass** (`psPostTaa`, after the composite). The composite now draws into `curTex`
   (RGBA8, i.e. after the existing tonemap/composite). The resolve then:
   - reconstructs the current frame at the unjittered pixel centre (3x3 Gaussian, Karis 2014),
   - takes the nearest depth of the 3x3 neighbourhood (moving silhouettes keep foreground motion),
   - reprojects with camera motion only, using depth, this view and the previous view rotation
     and position. The matrix is built in C++ in double precision. A camera move over 50 m or a big turn drops the history,
   - samples the history with a 9-tap bilinear Catmull-Rom,
   - **depth rejection**: the previous frame's view depth is stored in a second R32F
     attachment of the history. If the reprojected point is outside the 3x3 depth range
     around the history texel (with a tolerance of 3 % + 0.15 m), the history is dropped,
   - clips to the YCoCg variance box (gamma 1, intersected with min/max),
   - blends with alpha 0.1, raised slightly with motion, using luma-weighted blending against fireflies,
   - writes RGBA16F ping-pong history and view depth.
4. **Output pass** (`psPostTaaOut`): a bounded unsharp mask (`taaSharpen`, best 0.5; overshoot
   limited to a quarter of the local cross range, no halos), plus a +-1 LSB dither back into the
   frame target. With MSAA on, the composite and TAA run on the MSAA-resolved image, so
   "TAA + MSAA" means MSAA edges first, then temporal accumulation.
5. **Stochastic effects that lean on TAA:**
   - `shadowStochastic=1, shadowTaps=N, sunSize=deg`: per-pixel, per-frame rotated Vogel
     disc. A blocker search in raw depth (new sampler `shadowRaw` on unit 6, the same
     shadow array without compare) gives the occluder distance, and the filter radius grows with
     distance times the apparent sun size (PCSS / contact hardening). Each tap is still a hardware
     2x2 compare. A slope-aware bias stops self-shadowing at a low sun.
   - `aoStochastic=1, aoTaps=N, aoRadius=m`: the existing SSAO with a per-frame rotated
     pattern and **no bilateral blur**. TAA integrates the pattern instead.
   - `taaStochAlpha=1` (no-MSAA only): alpha-tested cutouts discard against a per-pixel,
     per-frame random threshold derived from the A2C coverage ramp. TAA averages it back into
     partial coverage, giving an A2C look without MSAA.
6. Debug: `debug=5` shows motion (rg) and rejected/invalid history (b).

Tri knobs (all via `triSetLighting key=value`): `taa taaAlpha taaSharpen taaJitter taaGamma
taaDepthReject taaMipBias taaStochAlpha aoStochastic aoTaps aoRadius shadowStochastic
shadowTaps sunSize`.

"BEST" TAA config used in the sheets: `taa=1 taaSharpen=0.5 taaMipBias=-0.5 taaStochAlpha=1`.
"FX" = `shadowStochastic=1 shadowTaps=4 sunSize=2.5 aoStochastic=1 aoTaps=4 aoRadius=2.5`.

## Why it suits CWA content

CWA's look is dominated by alpha-tested foliage, thin wire geometry (antennas, fences,
the camo net) and big low-sun shadows. All three either shimmer or need many samples. The
expensive, physically motivated effects the owner wants (soft contact-hardening shadows,
large-radius AO, SSGI, SSR, volumetrics) are all integrals that only become affordable at 1
to 4 samples per pixel per frame plus temporal accumulation. This prototype shows the
accumulation half works on the real content and real camera motion.

## Variants tried (all real captures, 960x640 window)

| id | config | where |
|---|---|---|
| taa15 | TAA, no MSAA, 150 % | runs 02-11, aa_crops, foliage_alpha |
| taa15_sa | + stochastic alpha (A2C emulation) | foliage_alpha |
| taa15_mip | + mip bias -0.5 | foliage_alpha |
| best15 / best10 | BEST, no MSAA, 150 % / 100 % | aa_crops, pans |
| best10msaa4 | BEST + 4x MSAA at 100 % | 3way_aa, pans, aa_crops |
| best10msaa2 | BEST + 2x MSAA at 100 % | thin_features_antennas |
| best15_g15 | clip gamma 1.5 | thin_features_antennas |
| taa15_s0 / s50 / s80, taa125_s50, taa10_s50 | sharpen and scale sweep | sharpen_sweep |
| ghosttaa / ghosttaa_norej | moving Jeep, depth rejection on/off | ghosting_grid |
| shadowStoch (4 taps, 2.5 deg), pcss_sun1/5, sun5_t2 (2 taps) | stochastic PCSS +- TAA | pcss_* sheets |
| aodbg_stoch (4 taps, r 2.5 m, no blur) | stochastic AO +- TAA | ao_converge |
| conv_f000..f064 | FX, TAA switched on from an empty history | convergence |
| panfx / strafefx | FX under yaw pan and strafe, +- TAA | *_motion |

## What the real data showed

### AA quality (stills and pans)
- `final/<view>__aa_crops.png`, `final/<view>__3way_aa.png`. TAA at 100 % without MSAA is
  visibly soft. At 150 % it is close to current but a little softer. TAA + 4x MSAA at 100 % is
  closest to current, with leaf edges slightly smoother and texture slightly softer.
- Pan metric (`final/pan_metrics.json`: consecutive captures 0.1 deg apart, registered by integer
  shift, mean residual, so lower means steadier; sharpness = mean neighbour difference):

  | view | residual MSAA4 150 % | TAA 150 % | TAA+MSAA4 100 % | sharpness change TAA 150 % / TAA+MSAA4 100 % |
  |---|---|---|---|---|
  | forest_edge_golden | 0.00788 | 0.00596 (-24 %) | 0.00593 (-25 %) | -8 % / -17 % |
  | sinai_lowsun_fp | 0.00416 | 0.00322 (-23 %) | 0.00381 (-8 %) | -11 % / -18 % |
  | town_night | 0.02544 | 0.02419 (-5 %) | 0.02464 (-3 %) | -8 % / -16 % |
  | overcast_broken | 0.00287 | 0.00277 (-3 %) | 0.00329 (+15 %) | -6 % / -11 % |

  Read the columns together. Blur lowers both residual and sharpness. Residual per unit of sharpness is
  roughly equal to the current setting (`residual_per_sharpness` in the json). So as pure AA,
  TAA is a sideways move against the expensive 150 % SSAA + 4x MSAA it would replace. The win is in the foliage
  views, which are where CWA shimmers most.
- Thin features (`final/thin_features_antennas.png`): TAA without MSAA breaks the
  sub-pixel M113 antennas into fragments, and a wider clip (gamma 1.5) only partly helps. TAA + 4x
  MSAA at 100 % keeps both continuous, more complete than the current dashed look. With 2x MSAA one
  antenna is whole and the other still has a gap.
  **MSAA must stay under TAA for CWA's wire geometry.**
- Foliage alpha (`final/forest_edge_golden__foliage_alpha.png`): stochastic alpha under TAA
  approximates the A2C look without MSAA, but in the stills the difference from plain alpha test
  under TAA is small. Mip bias -0.5 recovers some needle detail.
- Sharpen sweep (`final/sinai_lowsun_fp__sharpen_sweep.png`): sharpen 0 is soft, 0.5
  is near current on the camo net, and 0.8 is crisp with no halos (the bounded mask works).
- Static camera flicker of two consecutive TAA frames is small (0.0005 to 0.0018 mean abs luma on
  forest/sinai; `cmp/metrics.json`, `pan_metrics.json`). town_night measured 0.009 because of the
  live fire and smoke.

### Moving objects
- `final/sinai_lowsun_fp__ghosting_grid.png`, `final/forest_edge_golden__ghosting_grid.png`:
  a Jeep teleported 0.12 m per frame across the M113. With camera-only motion vectors, there
  is no visible ghost trail with or without depth rejection; the variance clip carries it. The
  Jeep is slightly softer than under MSAA. In the forest grid the Jeep stays at the crop edge, so the\n  Sinai grid is the useful one. Faster vehicles, rotors, and first-person weapon
  sway were **not tested**.
- Particles: `final/town_night__smoke_particles.png` cannot answer it. Every capture shows
  the smoke at a different simulation moment. **Untested**, a known TAA weak spot (no reactive mask).

### The enabler result (the reason to do TAA)
- `final/forest_edge_golden__convergence.png` (soldier shadow, 2x): without TAA the 4-tap
  stochastic PCSS is dotted. After TAA starts from an empty history the shadow is smooth by about frame 2
  to 4, and frames 8 to 64 match the 16-tap no-TAA reference. The current PCF shadow is
  uniformly sharp, while the converged PCSS is sharp at the boots and soft at the head.
  The frame numbers are nominal: shots are taken at present, so about one frame later.
- `final/forest_edge_golden__strafefx_motion.png` / `__panfx_motion.png` (+ GIFs): under a
  strafe (0.03 m per frame) and a yaw pan, the TAA rows stay clean and soft while the no-TAA rows stay
  dotted. Noise proxy (`final/r4_noise.json`, high-frequency energy in the shadow crop,
  dominated by ground texture so only relative): FX + TAA is about 20 % lower than FX without TAA
  in both pan (0.0129 vs 0.0162) and strafe (0.0103 vs 0.0128). In the convergence it is 0.0153
  without TAA, 0.0163 at frame 1 (first resolved frame plus sharpening), 0.0132 at frame 2, and stays
  at 0.0129 to 0.0134 through frame 64; the 16-tap reference is 0.0141.
- `final/<view>__pcss_converge.png`, `final/sinai_lowsun_fp__pcss_longshadow*.png`,
  `final/sinai_lowsun_fp__pcss_sunsize.png`: on Sinai's long low-sun shadows the soldier's
  leg shadows soften with distance from the boots. The effect is real but low contrast on
  that view. A 2-tap / 5 degree variant still converges, so the budget can go lower.
- `final/<view>__ao_converge*.png` (AO buffer, debug view 4): 4-tap unblurred AO with a
  2.5 m radius is speckled alone. Under TAA it is much smoother but keeps a faint grain at
  this sample count, and it is less smooth than the current blurred 12-tap AO. The AO
  needs either 6 to 8 taps or a light spatial pass before TAA.

### Full frames
`BEST_<view>.png` = Classic | Remastered (current) | Remastered + TAA + 4x MSAA at 100 % + FX.
At full-frame scale the visible difference is the softer, contact-hardening soldier and
vehicle shadows. Everything else is close to the current Remastered look, which is
expected: this track does not change the lighting. Treat these as context; the evidence is in the crops above.

## GPU cost (honest)

- **TAA resolve + sharpen: +0.07 ms at 960x640, +0.14 ms at 1440x960** (the `composite`
  section, which contains them: 0.08 vs 0.01 ms and 0.17 vs 0.03 ms medians). This number is
  stable across all three timing runs. Memory: 2x RGBA16F + 2x R32F + 1x RGBA8 at render size.
- **Frame and world-pass totals are not usable for small deltas.** Three interleaved runs
  (`final/timings_run12.md`, `timings_run13.md`, `timings_run14.md`; 18 to 24 samples per mode) ran while
  three to four other agents' games shared the GPU. The same mode's world-pass median moved by up to 2x
  between runs: plain TAA + MSAA4 at 100 % was 8.97 ms in run 13 and 11.61 ms in run 14,
  while MSAA4 at 100 % without TAA was 9.45 and 6.84. In all three runs MSAA4 at 100 % never timed
  faster than MSAA4 at 150 % (medians equal or slower, minimums 4.5 to 5.1 ms vs 2.6 to 3.3 ms),
  which cannot be a real GPU cost. Do not read the frame column as the cost of TAA.
- Stochastic PCSS cost lands in the world pass and could not be separated from that noise. By
  construction the 4-tap variant does 4 raw blocker reads + 4 compare taps per cascade sample
  against the current 12 compare taps, so it should cost about the same or less than today's PCF. The
  12/16-tap PCSS reference (24/32 fetches) is the expensive one TAA lets us avoid. AO with 4
  taps and no blur: ao section 0.32 / 0.21 ms (runs 12 / 14) vs 0.30 / 0.23 ms for the current AO at
  the same 100 % scale, i.e. no measurable difference in this noise.
- Promised headroom: if TAA + 4x MSAA at 100 % replaces 4x MSAA at 150 %, the frame target
  shrinks 2.25x in pixel count. On a quiet GPU that should free a large part of the world pass for the
  stochastic effects. **This was not measured cleanly here** and needs a quiet-machine run.

## What a production version needs

1. Per-object motion vectors (previous world matrix per instance, a velocity target) for
   vehicles, rotors, animated soldiers and first-person weapons. Camera-only + clip is
   enough for slow movers, not for a helicopter or a muzzle.
2. A reactive/transparency mask for particles (smoke, fire, muzzle flash, tracers) and for
   alpha-blended glass. Lower history weight there, or composite particles after the resolve.
3. Resolve in HDR (before tonemap) with proper luma weighting. The prototype resolves the
   composited RGBA8 frame because that was the least invasive cut.
4. Vehicle interiors: the cockpit frame clears world depth (`_worldDepthCleared`), so the
   prototype turns TAA off there and drops the history. That needs a depth-preserving interior pass
   or a separate interior history.
5. Keep MSAA under TAA (wire geometry). Pick 2x or 4x MSAA at 100 % plus TAA as the
   replacement for 4x MSAA at 150 %. Mip bias -0.5. Sharpen around 0.5 (expose it as a user setting).
6. Classic bit-identity: the jitter add is `+ 0 * w` when TAA is off, and Classic never
   enables it, but this was **not verified by an image diff**. My Classic captures differ from
   `tmp/captures/20261017-final` only by scene state (pose, particles; the same branch tip).
   Production needs a Classic golden-image test, or GLSL `#ifdef`s so the Classic shaders are byte-identical.
7. Make the stochastic effects the default path once TAA is on: PCSS (2 to 4 taps, sun 1 to 2.5 deg)
   and AO (6 to 8 taps, larger radius). This is also where other tracks (SSGI, SSR, volumetrics)
   should plug in: 1 to 2 rays per pixel per frame and let the history integrate.

## Risks against the owner taste list

- **Softness**: TAA without enough sharpening reads as a blur over the frame. The owner rejected
  "a wash over the frame". Keep sharpen near 0.5 and MSAA under it. The 100 % no-MSAA TAA is
  visibly too soft (aa_crops, sharpen_sweep).
- **Tree crowns**: TAA smooths the leaf cut-outs (convergence_trees, foliage_alpha). This is not
  the "flattened crowns" look by itself, but heavier accumulation could get there. Watch the crowns.
- **Not darker, not a grade**: none of this changes colour or exposure. PCSS slightly reduces
  shadow coverage far from casters (softer edges). It does not crush shade.
- Ghosting on fast movers and smeared particles are the classic TAA complaints, and both are untested here.

## Files

- `prototype.patch`: the C++ / GLSL prototype (9 files, about 535 added lines).
- `scripts/`: test generators (`gen_tests_*.ps1`, `gen_funcs.ps1`), sheet builders
  (`make_compare.py`, `make_final.py`, `make_r4.py`, `ghost_grid.py`, `sinai_shadow_crop.py`,
  `sharpen_sheet.py`, `smoke_crop.py`), metrics (`pan_metrics.py`, `r4_noise.py`,
  `timing_summary.py`), helpers (`taalib.py`, `sheets.py`).
- The research tests live in the scratch worktree, under
  `arma_CWR\tests\rendering\research\temporal\` (git-excluded).
- `run01..run15*`: capture runs (shots/, tri/, timings.json, contact sheets; the
  `run*.log` files next to them are the run-captures output). run01 was a smoke run;
  run12/13/14 are timing only.
- `cmp/`: first-round comparison sheets. `final/`: final sheets. `BEST_<view>.png`: the three-way per view.
