# Track: exposure (local exposure and HDR range)

Research prototype, not production code. Resumed run: the earlier agent left
only a base build of the scratch worktree and the first C++ hunks (tuning keys,
RGBA16F frame target, struct fields); the post pass, the shaders, every capture
and every image here are from this run.

## Short verdict

**Mixed, leaning promising for one piece, negative for two others.**

- **Local exposure (bilateral-grid local tone mapping in the post pass) works
  and fixes a known complaint:** at low sun the crushed shaded faces of vehicles,
  tents and soldiers become readable (Jeep flank, tent folds, M113 camouflage)
  while the sky and sunlit ground keep their level. Best image:
  `BEST_lowsun_fp_crop.png`. It is not a "different game" change on its own; it is
  a targeted readability fix, and it only stays inside the owner's taste because
  of a *gate* that this engine cannot yet compute cleanly (see Risks).
- **Eye adaptation over time works mechanically** (`adapt_sequence.png`,
  `adapt_sequence.gif`), and the meter put a number on a known problem: the
  moonlit town meters **-2.62 log2**, brighter than every low-sun day view
  (-3.2 to -4.3) and only 0.5 EV under noon (-2.08). A night-aware key (darken only
  at night, brighten only by day for dark compositions) is the useful form.
- **Hue-keeping shoulder:** since the shipped per-fragment Neutral shoulder is
  exactly invertible, the post pass can undo its whitening. Effect: the sun disc
  turns from white to a yellow-orange sun at sunrise/sunset and the campfire core
  turns orange instead of white (`HUE_sunrise_ridge.png`, `BEST_town_night.png`).
  It does nothing to dark olive hulls (measured: identical RGB), so it does not
  help the "warm evenings judged on hulls" rule.
- **Negative: HDR float frame (RGBA16F, world shaders skip the tonemap, post
  tonemaps once).** No visible gain on any day view (H1 vs L1 nearly identical,
  because the remastered exposure meter keeps the lit world near the LDR range),
  and at night it breaks pre-lit particles: smoke expanded through the inverse
  shoulder blows out white (`NEG_hdr_frame_town_night.png`).
- **Negative: AgX base curve:** desaturated, grey sun, lower contrast
  (`NEG_agx_coastline.png`). Rejected.
- **Negative: un-gated local exposure** lifts the dark sunset sea to flat grey,
  a wash (`NEG_ungated_lift_coastline.png`).
- **Negative: adaptation with an uncalibrated key** (-2 log2) pushes every
  non-noon frame up about 2 EV and blows the sky out (`NEG_adapt_key_sunrise.png`).

## Technique

All in `EngineGL33::EndWorldScene` (post pass), after AO/bloom/shafts, before the
composite. Patch: `prototype.patch` (scratch worktree
`D:\Arma_CWA_worktrees\gfx-exposure`, detached at 3c53141, build
`arma_CWR\build\gfx-exposure-rwdi`, exe `arma_CWR\dist\gfx-exposure-rwdi`).

1. **Scene-linear recovery.** The shipped frame is display-encoded and tonemapped
   per fragment with the toe-less Neutral curve: identity below 0.8 linear, and the
   shoulder keeps the max channel at `newPeak`, so the input peak and the
   desaturation weight can be solved back exactly (`neutralInverse`). The post pass
   gets scene-linear values up to ~5.6x (8-bit limit) with no G-buffer.
2. **psPostLum** (quarter res, RGBA16F, mipmapped): log2 luminance, plus a
   centre-weighted metering channel (sky pixels, depth = far, weighted 0.25).
3. **psPostGrid**: bilateral grid (Chen, Paris, Durand 2007), 45 x 30 cells x 16
   bins over log2 -12..4, built by *gathering* (GL 3.3 has no compute/atomics):
   each grid vertex reads the 16x16 quarter-res texels around it, tent-weighted in
   xy and z. Then a separable 3-axis blur (psPostGridBlur).
4. **psPostAdapt**: 1x1 ping-pong, exponential approach of the metered log
   luminance, frame dt clamped to 0.1 s.
5. **Composite**: base = mix(bilateral slice, bicubic blurred mip, 0.3);
   gain = (base - mid) x (contrast - 1) + detail term; contrast < 1 below mid lifts
   shade, > mid compresses highlights; then adaptation EV; then the tonemap curve
   (Neutral, hue-keeping, or AgX) and the existing AO/bloom/shaft composite.
6. **Gates (round 2-5, override GLSL):** the lift is weighted by the pixel's
   *sky-light share* (the frame alpha the remastered shaders already store for AO:
   true shade is mostly sky light, dark paint or water is not), OR'd with a
   near-field weight from the AO pass depth (< 25-70 m) because alpha-tested and
   alpha-to-coverage draws (vehicles, men) store alpha 1, and faded out beyond
   250-900 m so fogged hills do not haze. The pre-lit sky (alpha exactly 1) is kept
   out of the highlight compression.
7. **HDR path (hdr=1, negative):** RGBA16F frame and post colour target,
   `litLightCount.y` tells the world shaders to skip the tonemap and to expand
   display-referred colours (pre-lit, fog) through the inverse shoulder.

Why it suits CWA: no new art, no G-buffer, one post pass; the engine's own
Neutral shoulder being invertible is what makes a post-pass tone operator possible
on an 8-bit frame.

## Variants and rounds (all real captures, Classic | Remastered | variant, same view/time)

| Round | Override | What | Result |
|---|---|---|---|
| r1 | none (C++ GLSL) | L1 local exp, L2 + detail 1.3, A1 adapt key -2, H1/H2/H3 HDR frame + Neutral/hue/AgX, G1 debug | L1 lifts Jeep/tent and sunrise foreground; washes the sea. A1 too bright. H* = L1 by day, smoke blowout at night. AgX muted. |
| r2 | `override_r2` | sky-share gate (L3), no gate (L4), stronger (L5), meter bars (M1), gain map (G2) | Gate protects the sea, but the Jeep flank is no longer lifted (alpha-tested draws read as pre-lit). Meter readings below. |
| r3 | `override_r3` | sky kept out of highlight compression, hue-keeping (K1), punch = exposure 1.3 + LTM (P1/P2), adaptation with dead band (AD/PA) | Hue shoulder: yellow sun. Punch: brighter ground, little else. Adaptation by day brightens the sky too (global). **Engine flake:** every Remastered coastline shot in r3 (including the unmodified baseline, knobs neutral) rendered the shore and near sea in sun shadow; r1, r2, r4, r5 with the same script did not; Classic unaffected. Looks like an intermittent shadow-map problem, not this prototype. |
| r4 | `override_r4` | near-field gate (N2/N3), hue (N2K), gain map (NG); night set: hue only (NK), meter (NM), lamp highlight compression (NH), night darken (NA) | Near gate lifts vehicles; costs a mild lift on the near sea strip. Night meter -2.62. |
| r5 | `override_r5` (final) | F1 final local exp (mid -2.8, sky-share + near gate, far fade), F2 stronger, F1K + hue, F1A + night-aware adaptation, FG gain map | Final set; BEST images use F1A. |
| r6 | `override_r5` | adaptation time sequence only | After fixing two sequence bugs (r3: snap on long frame gaps; r5: camera not re-pinned before settling), the ramp is visible. |

Meter readings (r2/r4, centre-weighted scene-linear log2, sky weight 0.25):
noon_fields -2.08, town_night -2.62, coastline_sunset -3.22, tank interior -4.07,
lowsun_fp -4.15, forest_edge_golden -4.32, sunrise_ridge -6.45.

Final knobs (F1A), set with `triSetLighting key=value` on the prototype build
with `-ShaderOverrideDir override_r5` (some knobs are repurposed by the override,
noted in brackets):
`ltmShadow=0.6 ltmHighlight=0.9 ltmDetail=1.15 ltmBlur=0.3 ltmMaxGain=2.5
ltmMid=-2.8 ltmMidAdapt=2 [gate: sky share + near field] bloomHdr=1 [sky kept out of
highlight compression] tmCurve=2 [hue-keeping shoulder] adapt=1 adaptKey=-4
hdrWhite=0.75 [adaptation dead band, EV] adaptMin=-0.75 adaptMax=0.5 adaptSpeed=30
[captures; 0.4-1.5 for play]`. Local exposure fades out at night (ltmNight=0);
the night key is 2 EV lower and only darkens.

## What real data showed (per view, final F1A)

- **lowsun_fp** (`BEST_lowsun_fp.png`, `BEST_lowsun_fp_crop.png`): clear win. Jeep
  flank, tent fabric, M113 and the soldier are readable; sky unchanged; sunlit
  ground slightly lighter. The cast shadow on the ground is also lifted (bluish
  shade a little lighter).
- **sunrise_ridge** (`BEST_sunrise_ridge.png`): backlit foreground lifted from
  near-black-green to readable grass, sky unchanged, sun disc yellow-orange. Costs:
  the foreground reads flatter and the warm sun streak across the ground loses
  contrast; the far hill is very slightly lighter. Mood vs readability trade.
- **coastline_sunset** (`BEST_coastline_sunset.png`): the far sea stays dark (gate),
  but the near-sea strip inside ~70 m is lifted a little greyer (near-gate cost).
  Sun disc yellow. Net: neutral to slightly negative.
- **town_night** (`BEST_town_night.png`): about 0.75 EV darker overall (night key),
  fire core orange, lamps unchanged. Local exposure is off at night by design.
  Darkens, which fixes "night too bright" but is the night track's call.
- **tank_gunner_interior** (`BEST_tank_gunner_interior.png`): almost no change
  (the interior is lit flat; hands lifted slightly). Adaptation stays neutral
  (meter -4.07, inside the dead band).
- **forest_edge_golden** (`BEST_forest_edge_golden.png`): almost no change (trunks
  and the soldier's shadow lifted slightly). No haze.
- **Adaptation over time** (`adapt_sequence.png`, `.gif`, run_r6_seq): settled on
  the sky (adapted -4.7), tip down to the ground (meter -7.3): adapted -4.8 at the
  cut, -5.2 at +0.4 s, -5.8 at +1.2 s, -7.2 at +9.8 s; the ground brightens from
  lower-half mean 43.6 to 48.3 to 56.4 (cap +1 EV) while Remastered stays at 30.6.
  The red/green bars in the frames are the debug readout (adapted / meter).

## GPU cost

`cost_summary.md`: local exposure (lum pyramid + grid gather + 3 blurs +
adaptation + the larger composite) measured as composite-section delta against
the shipped composite in the same run and view. Frame 1440x960 (960x640 at 1.5x),
4x MSAA. **Best estimate +0.25 to +0.35 ms** (tank interior a steady +0.27 ms over
16 shots); median over 140 shots +0.8 ms, max +1.9 ms, because every run shared
the GPU with up to three other research games (the world pass of an unchanged
scene swung from 23 to 43 ms between shots). The grid gather (16x16 texels per
grid vertex, 21,600 vertices) is the likely dominant part; no per-pass timer was
added. Memory: ~1 MB extra (quarter-res RGBA16F mip chain, two 720x30 RG32F grids).
The HDR frame doubles the frame target and resolve memory (RGBA16F).

## What a production version needs

1. **A light buffer, not the alpha + depth hack.** The gate needs to know shade
   from dark albedo for every draw, including alpha-tested / alpha-to-coverage ones
   that store alpha 1 today. An MRT (R16F) from the remastered shaders with
   log2(irradiance / reference) (0 = neutral for pre-lit and particles, so alpha
   blending and additive blending pull toward neutral), with `glColorMaski` off for
   classic programs. Local exposure on the *illumination* layer would lift shade
   without touching dark paint or water, which removes both the sea wash and the
   near-sea strip.
2. Cheaper grid build (fewer texels per vertex, or build from a 1/8 res lum), a
   per-pass GPU mark, and an MSAA-aware slice.
3. Adaptation tied to the exposure meter and the night track's targets; real dt
   (done, clamped 0.1 s); a reset on teleports/cutscenes; UI knobs.
4. The hue-keeping shoulder does not need the post pass: replacing
   `tonemapNeutral` with the hue-keeping peak curve in the world shaders gives the
   same yellow sun and orange fire at zero cost (pre-lit sprites aside).
5. Drop the HDR float frame unless a future track (emissive muzzle flashes, fire
   bounce) actually produces >1 radiance; if kept, pre-lit particles must not be
   expanded.
6. Classic stays bit-identical: all changes are behind `remastered` plus research
   knobs; with neutral knobs the Remastered frame is also unchanged (static
   lowsun_fp: 0.002-0.003 mean abs difference vs `tmp/captures/20261017-final` in
   both modes; other views differ by the same amount in Classic as in Remastered,
   i.e. scene noise from smoke, waves and HUD).

## Risks against the owner's taste list

- **Wash / flattening:** any shade lift risks it. Un-gated it is a wash (sea);
  gated, the sunrise foreground and the near-sea strip still read flatter. Keep the
  lift modest (contrast 0.6, max +1.3 EV) and gate on illumination in production.
- **Only darker:** not the case by day (adaptation is brighten-only, local
  exposure lifts). At night the night-aware key darkens by design; present it to
  the night track, not as a standalone change.
- **Blue haze:** the far fade keeps fogged hills from lifting; the lifted cast
  shadows are slightly bluish because the sky fill is cool (unchanged hue, more of
  it).
- **Warm evenings:** local exposure scales luminance and keeps hue; the
  hue-keeping shoulder keeps highlights warm; neither changes hull warmth.
- **Yellow sun disc:** the hue-keeping shoulder's most visible effect is a
  saturated yellow-orange sun; some will read it as "real low sun", some as a
  cartoon ball. Owner call.

## Files

- `prototype.patch`: the C++ change (7 files, post pass + shaders + knobs + float
  frame). The C++ composite is the round-1 version; the final look needs
  `override_r5\psPostComposite.glsl` on top (gates, sky keep, night-aware
  adaptation, debug bars).
- `override_r2..r5\psPostComposite.glsl`: per-round composite overrides.
- `tests\`: copies of the research capture tests (`exposure_r1..r6.test.sqf/.toml`,
  `capture_lib.sqf` with `t=` sim time added to shot lines). Live copies are in the
  scratch worktree under `arma_CWR\tests\rendering\research\exposure\` (git-excluded).
- `gen_test.py` (test generator), `compare.py` (collect + labelled sheets +
  three-ways), `crop.py`, `readbars.py` (meter readout), `seqstrip.py`,
  `seqstrip2.py`, `diffruns.py`.
- `run_r1..r5\`, `run_r6_seq\`: full capture runs (tri tree, every shot,
  `<view>__sheet.png`, `timings.csv/.md`, contact sheet).
- `BEST_*.png`, `HUE_sunrise_ridge.png`, `NEG_*.png`, `adapt_sequence.png/.gif`
  (`adapt_sequence_r3.png`, `_r4.png` and `_r5.png` are the buggy sequence
  attempts, kept as evidence: r3 snapped on long frame gaps, r4 used a fixed 1/60 s
  step but the game renders unlogged frames between tri commands so it had already
  converged, r5 never re-pinned the camera before settling), `cost_summary.md`, `build*.log`, `run_r*.console.log`.
