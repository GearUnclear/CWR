
## Iterations (what each run showed, all on real Remaster data)

| Run | Build / change | What the captures showed |
|---|---|---|
| diag1 (earlier agent) | shared lighting build | Today's noise-field shadows: invisible from eye level at overcast 0.5, km-scale blobs from 250 m up. |
| run1_eden | build 1: sky-mode uniform only | No sky change at all: the sky is drawn before `BeginPass` through the classic programs (this is how the cause was found). |
| run2_abel, run2_eden | build 2: remastered set forced around the sky draws, global saturation density | Sky lighting engages (pink dawn edges), but the pale horizon band counted as cloud: a glare band, blocky contours, and the sun disk dimmed under a clear sky. Shadows still absent (the broken-sky streaks are bluish, not white, so that density was about 0). |
| run3_abel_v2 | override `ovr_v2`: row-relative density, 16 row taps per fragment | Clear sky clean again; dusk edges better. Sunrise read 50 ms vs 7.8 ms in the same session, so the per-fragment row statistics were moved to the CPU. |
| run4_* | build 4: CPU density map (1 tap) | Cloud shadows visible and consistent with the sky; dawn clouds lit. Two defects: dome-triangle edges (the screen-derivative march jumps across the coarse dome mesh) and 8-bit palette banding. |
| run5_* | build 5 (final): march in direction space through the dome mapping, 5-tap blur of the density map, negative relief capped at -0.35, overcast belly contrast, sky drift knob | Clean: no triangle edges, no banding. The BEST images come from here. |

## Results (run5, 960x640, 150% scale, 4x MSAA, same game session per view)

Variants: **A** defaults (deck 1800 m, threshold 0.55, relief 1.8, silver
1.0); **B** stronger relief 2.8 and silver lining 1.8; **C** = A with a lower,
denser shadow deck (1200 m, threshold 0.42).

- `BEST_coast_oc_up.png` (sun 8 deg, overcast 0.5): the sun-facing undersides
  of the cloud bands glow peach and gold, the tops away from the sun go slate
  blue. The clouds read as lit volumes instead of a flat painted sheet. The
  strongest result of the track.
- `BEST_sunrise_oc.png` (sun 5 deg, overcast 0.5): cloud bases facing the sun
  catch pink sunrise light. Classic and today's Remastered show the same
  texture, flatly tinted.
- `BEST_broken_up.png` (sun 54 deg, overcast 0.5, looking up): the streaky sky
  becomes layered and sculpted, with shaded bases and bright sun-side rims.
  The most "different game" frame and also the riskiest: some edges read a
  little glossy or painterly (a bright rim line with faint colour fringes).
- `BEST_broken_high.png`, `BEST_overcast_broken.png`: cloud shadows that belong
  to the clouds drawn in the sky lie across the hills (long soft bands, from
  the broken-sky streaks). Today's noise field also shows shadows from 250 m
  up, but they have no relation to the sky; from eye level the prototype
  shows lit and shaded hills where today's frame is uniformly lit. In the
  matched overcast_broken frame the camera itself ends up under a cloud (the
  dome has cloud at the sun's position), so the foreground is shaded: ground
  band mean luminance 0.341 to 0.247 (`luminance_check.txt`).
- `BEST_overcast_heavy.png` (overcast 0.9): deeper bellies and lifted thin
  gaps in the deck. A modest change.
- `SEQ_broken_high_drift.png` / `.gif`: with the sky drift knob stepped 0 to 15
  deg, the sky turns and the shadow bands sweep across the slope (static knob
  steps standing in for a wind-driven drift).
- `cmp_clear_sanity.png`: the matched clear-sky views (coastline_sunset,
  sunrise_ridge at overcast 0) are unchanged by the prototype.
- Sky brightness is kept: the sky-band mean luminance is within 0.01 of
  today's at midday and 0.02 to 0.04 brighter at golden hour; its standard
  deviation (cloud contrast) rises 30 to 45% (`luminance_check.txt`). Not
  "only darker".

## GPU cost (honest)

The machine ran 3 to 4 other agents' games throughout, so single shots are
noisy (Classic broken_high read 51 ms in one run and 30 ms in the next).
Interleaved test `run5_perf` (same view, modes alternated, 120 frames each,
GPU frame ms):

| view | Remastered now | proto A | sky only | shadows only | now again | proto A again |
|---|---|---|---|---|---|---|
| overcast_broken | 9.08 | 9.93 | 9.80 | 7.85 | 7.26 | 7.48 |
| broken_high | 13.69 | 14.25 | 14.17 | 14.73 | 17.59 | 17.95 |
| sunrise_oc | 5.38 | 37.09 | 33.46 | 44.96 | 41.62 | 37.62 |

Back-to-back pairs give **+0.2 to +0.9 ms** per frame. The sunrise view's world
time jumps from 5 to about 40 ms within the session *with the prototype off*
(`remcur2` 41.6 ms), so the earlier "+18 to +40 ms at sunrise" readings were
that drift (the known sunrise_ridge world-pass problem or GPU contention), not
the prototype. The density map is rebuilt twice per session (on weather
change: a 256x256 CPU decode and blur, logged as `sky cloud-density map ...
rebuilt`).

## Verdict

Promising. Lit sky clouds are the biggest visible win of this track: at dawn
and dusk the existing painted sky becomes sun-lit cloud volumes with warm
undersides, for under a millisecond, using only the shipped sky textures.
Sky-consistent cloud shadows work and are cheap (one texture tap per lit
fragment), but they move with the camera (see production needs) and their
shapes are the long streaks of the broken-sky texture, so they read as bands
rather than cumulus patches.

## What a production version needs

- Keep the sky-mode hook, but give the sky draws a proper remastered sky pass
  instead of forcing the program set inside the screen pass. Classic stays
  bit-identical because every new path is gated by `_lightingActive` (the
  `DrawClouds` split is a pure rename); this prototype was not pixel-diffed
  against Classic.
- Move the density-map builder from the GL backend into a pure-math
  `Graphics/Remaster` unit with a test (row baseline, deck flag, blur), built
  on weather change off the render thread.
- Drive the sky drift from the wind integrator that already moves the noise
  field, and decide on purpose whether the visible dome may move (Classic's
  dome never does).
- Shadow anchoring: the dome is camera-centred, so its shadow pattern
  translates with the camera. Anchor the projection to a slowly following
  point (re-centre after a few km with a cross-fade) or turn the cloud layer
  into a real world-space deck. At infantry speed this is invisible; in an
  aircraft or a fast car it would show.
- The `mrak1-4` billboards (world-anchored, 6 to 11 km across) get the lit
  shading but no shadows yet. `Landscape::CollectCloudSheets` (scaffolding from
  the earlier agent, in the patch, unused) lists them at their true positions
  for a sheet shadow pass.
- Tests: the LitConstants layout test for the new slots (`Sky`, `SkyVp`,
  `SkyRight/Up/Fwd`, `Knob0/1`), glslang compile, a density-map unit test on
  the three weather skies, and a capture view looking up at a broken sky.

## Risks against the owner taste list

- No blue wash (sky mean unchanged), and not only darker (sky mean kept; the
  ground is darker only where a cloud really shadows it). Dusk clouds take the
  rig's sun chroma, so they are warm, not neutral.
- The midday broken sky (`broken_up`) can read glossy or painterly: bright rim
  lines with faint colour fringes. Variant A is the safer gain; B is too
  strong at midday but good at dusk. Scaling the silver lining down with sun
  elevation would help.
- A frame where the camera sits under a cloud is darker on the ground (the
  existing cloud transmission of 0.2). With drift this changes over time, but
  a still can look "darker" to the owner.
- The shadow shapes follow the painted texture's streaks: long bands, not
  cumulus patches.

## Files

- `prototype.patch`: `git diff` of the scratch worktree (engine only).
- `glsl_final/`: the built GLSL pieces (dumped from the patched source).
- `ovr_v2/`, `ovr_drift/`: intermediate shader overrides (v2 row-relative
  density, superseded by the CPU map; the drift override, now built in).
- `tests/`: the research capture tests (`clouds_eden`, `clouds_abel`,
  `clouds_seq`, `clouds_perf_*`) and `capture_lib.sqf`.
- `compare3.py`, `strip.py`, `collect.py`: comparison and collection scripts.
- `assets/`: extracted sky and cloud models and textures as PNG, density
  studies (`density_v3.png`, `oblacno_q_lum.png`). Game data, never commit.
- `run1_eden` to `run5_*`: capture runs (each with `timings.csv`/`.md`,
  `udcap.log`, `tri/`). `run5_*` is the final build.
- `BEST_*.png`, `SEQ_*.png`/`.gif`, `cmp_*.png`, `luminance_check.txt`.
