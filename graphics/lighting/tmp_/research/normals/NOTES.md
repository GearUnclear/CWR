# Normals track: normal maps generated from the existing albedo

TRACKID `normals`. Research prototype, not production code. Two sessions worked
on this track: rounds 1-2 by the first agent (stopped when the research was
paused), rounds 3-6 plus the diagnostics, timing and motion runs by the resumed
agent (2026-10-17, 18:25-19:25 local). Everything below was observed in
captures that are on disk in this folder.

## Verdict (short)

**Mixed.** Generated normal maps do almost nothing for CWA's vehicles,
interiors and tents: the textures are flat paint with dark lines already baked
in, and the relief luminance gives them is too weak to read even at double
strength. What does work is **terrain micro-relief at a low sun**: a bump
from the terrain detail grain plus a short height-field march for micro
shadows, small cavities, and a Smith-masking normalisation so the ground does
not get darker. Sunlit gravel then reads as gravel, with pebbles throwing small
shadows away from the sun (BEST_lowsun_fp.png, BEST_sinai_lowsun_fp.png). It is
stable in motion. At noon it is nearly invisible, and it does not make the game
look different overall. Relief from the per-cell terrain base textures had to
be dropped: it shows hard seams at the 50 m cell borders.

There are two side findings, and they matter more than the normal maps:
1. **Vehicles spawned at runtime never get the Remastered per-pixel shading in
   the shipped build.** Every capture view spawns its vehicles at runtime, and
   so does Guerrilla Mode. Details are in its own section below.
2. **The tank gunner interior LOD (resolution 1200) always draws on the CPU
   path.** Forcing it onto the per-pixel path made it much darker, so the
   interior needs its own light model first.

## Technique

The tangent frame is rebuilt per pixel from screen derivatives (`dFdx/dFdy` of
the camera-relative position and of the UV), so no mesh tangents are needed.
The bump is a surface gradient subtracted from the geometric normal
(Mikkelsen's surface-gradient form). Guards: the bump never lights a face the
geometry turns away from the sun; the shadow-map lookup keeps the geometric
normal; foliage (forced-diffuse materials) keeps its crown normal; a soft limit
stops a paint edge from folding the normal; a Retinex chroma gate drops
luminance steps that come with a hue step (paint, decals, numbers), so the
M113's star stays flat.

Variants. The saturation knob selects them at runtime
(`saturation = 100*O + 10*V + 1.12`), so every variant of a view is shot in one
session at the same camera and sim time:

| V | what | where it runs |
|---|---|---|
| 0 | no bump | |
| 1 | screen-space luminance bump: `dFdx/dFdy` of the shaded albedo luminance | shader only |
| 2 / 3 | texture-space taps, 2 scales, chroma gate (x1 / x2) | shader only |
| 5 | **load-time normal map**: every texture upload reads back mip 0 and builds a 3-scale gated slope map plus (round 4) a gated high-pass height on the CPU, uploaded as an RGBA16F companion with mips and 8x aniso, bound on unit 6 whenever the albedo is bound on unit 0 | C++ |
| 7 | V5 + micro self-shadows: 8-step march toward the sun over the height field (companion `b` on objects, detail grain `tex1.a` on terrain), penumbra widening with distance | C++ + shader |
| 8 | V7 + cavities: horizon-based sky occlusion over the same height field (6 directions x 3 radii, cosine weighted), on the sky term only | C++ + shader |
| 9 | V8 with the bump and the height doubled | C++ + shader |
| 6 (round 6) | **V9, mean-preserving**: linear (unnormalised) bump in the sun term, so a symmetric relief keeps the mean, and micro-shadow visibility divided by its Smith GGX expectation `G1 = 2/(1+sqrt(1+a^2/tan^2 E))` from the local slope variance | C++ + shader |
| O=1 | runtime toggle: late-loaded shapes back on the CPU path, i.e. the look the shipped build gives spawned vehicles ("Remastered (as shipped)" in every sheet) | C++ |

Terrain uses only the detail grain for relief from round 5 on (`--ktnmap 0`).
Objects use the companion map.

## What the rounds showed

- **r1** (first agent, shared build, override_v1). V1-V4 on all views. Only the
  terrain and the soldier had per-pixel shading. The vehicles, tent and tank
  interior did not, as the debug-normal shots show
  (`runs/r1_abel/lowsun_fp__all6.png`). The terrain gravel gained a faint
  relief.
- **diag** (first agent). A debug override colours CPU-path fragments magenta:
  the M113, Jeep and tank interior are magenta in the shipped build
  (`evidence_cpu_path_magenta_shipped_build.png`).
- **r2** (first agent, cut off after one view). The first late-VB hack made the
  sun disc vanish, because the sun halo got a vertex buffer.
- **r3** (resumed). The sky shapes are now skipped (special light or fog hints)
  and the late-VB toggle is added. **Bug fixed:** the round-2 shader decoded
  the raw signed RG16F slopes as unorm (`*2-1`), which would have tilted every
  normal by -k. V5/V6 had never been captured with that bug. Result: at
  matched strength every bump is subtle. Only sunlit gravel at a low sun
  changes. The integrated-slope micro shadow was a no-op: luminance slopes are
  far below tan(sun elevation). The screen-space bump (V1) looks much like the
  texture-space ones (`evidence_screen_space_vs_texture_bump_r3.png`).
- **r4.** The height channel is added, with the height-field micro shadows and
  cavities. Micro shadows become visible (`evidence_micro_shadow_debug_r4.png`;
  the debug sun-visibility panel shows pebble shadows near the camera).
  **Seams:** relief from the per-cell terrain base texture (V2 taps and the V5
  map alike) gives hard steps at the 50 m cell borders on Sinai
  (`evidence_terrain_cell_seams_r4.png`). V9 darkens sunlit ground by 7%.
- **diag2.** Logging finds why the M113 at 7.5 m still drew on the CPU path
  after the hack. The vehicle was deleted and spawned again, and its new
  LODShape reused the freed address, which the hack's "already tried" pointer
  set had marked. The M60 gunner LOD (resolution 1200) is skipped by
  `LODShape::OptimizeRendering` as a ">900 view level". Fixed by
  `ResearchEnsureVB`, which converts the drawn level itself with no pointer
  memory.
- **r5.** Terrain relief comes from the detail grain only: the seams are gone
  (`evidence_seams_gone_detail_only_r5.png`). Motion strip with the eye walking
  0.35 m per frame: the relief is texture-locked and does not swim
  (`evidence_motion_strip_r5.png`). The tank interior on the per-pixel path
  goes much darker than Classic or as-shipped (the hull shadows it and there is
  no interior light), which is a rejected look
  (`evidence_interior_per_pixel_too_dark_r5.png`). The bump does nothing
  visible there.
- **r6.** V6, mean-preserving: lit gravel keeps the as-shipped mean brightness
  (lowsun_fp lit 107.4 -> 109.1, Sinai 78.3 -> 75.7) while local contrast rises
  about 2.5x (sd 9.8 -> 24.2). The shade/lit ratio stays 0.73-0.85, so the shade
  is not crushed (`runs/r6/ground_stats.txt`). Best result of the track.
- **seq2.** Motion strip, as-shipped vs V1 (screen-space) vs V6, eye walking
  0.25 m per frame: neither swims (`runs/seq2_abel/lowsun_fp__motion2_strip.png`
  and GIFs).

## Side finding 1: runtime-spawned vehicles skip the Remastered shading

`Shapes.OptimizeAll()` creates vertex buffers once, at mission load
(`WorldImpl.cpp:2382`, `WorldInit.cpp:796`). Any shape first loaded later has
no buffer, so `Shape::Draw` takes the CPU T&L path and the fragment shader gets
pre-lit vertex colour (`vLitMode = 0`). That means no per-pixel sun, no
shadow-map receiving, no bump and no specular. The capture harness and
Guerrilla Mode spawn vehicles at runtime. **So most vehicle hulls in the
20261017-final baseline were judged on that fallback.**

Moving those shapes onto the per-pixel path (prototype: `ResearchLateOptimize`
plus `ResearchEnsureVB` on first draw, Remastered only) has only a small
effect at 16 m. At 7.5 m (LOD 0) the shaded faces turn warmer: on the Zelda's
slat armour, mean RGB goes from 67.8/64.0/65.5 (neutral grey) to
77.2/66.9/57.2 (tan); on the M113 side, from 44.8/45.0/43.6 to 46.6/46.6/40.7
(`runs/r6/hull_stats.txt`, `evidence_zelda_cpu_path_vs_per_pixel_r6.png`).
That bears directly on the owner's rule that evening hulls must be at least as
warm as Classic. Classic itself shows the Zelda slats grey, so the lighting
owner should check whether the per-pixel path is dropping some per-section
material or tint, or whether the CPU path is the one that is wrong.

The sky dome, sun, halo, clouds and stars must stay on the CPU path (the sun
disc vanished in r2). Pointer memory is unsafe because shapes are freed and
reloaded at the same address. The interior LOD needs a light model before it
moves.

## GPU cost (honest)

The GPU was shared with up to four games from other agents, so single-shot
numbers swing by 5-20 ms (see the labels on the sheets). The interleaved
timing run `runs/tm3_abel` takes six rounds per variant; `timings.md` has the
minimum and median. Min-of-6 against the late-VB base: lowsun_fp V5 -0.4, V8
-0.6, V9 +1.0 ms; noon_fields V5 +1.1, V8 +1.6, V9 +1.4 ms. **The cost is at
most about 1-2 ms at 960x640 x1.5 with 4x MSAA, and the noise floor is about
1 ms.** V6 adds 6 taps to V9. Shader taps per terrain pixel in V6: 1 companion
+ 8 detail-bump + 9 shadow + 19 cavity + 7 Smith, about 44, all at the
pixel's own mip.

The load-time cost is real: CPU generation on the render thread took 5.0 s for
592 textures (Abel) and 6.6 s for 552 (Sinai), up to 230 ms for one 1024x1024
texture. Textures streamed in during play would hitch.

## What a production version needs

1. The late-VB fix on its own. Build buffers at load in the shape bank, or a
   lazy per-level `ConvertToVBuffer` like `ResearchEnsureVB`, Remastered only,
   so Classic keeps its CPU path bit-identical. Sky shapes and the interior LOD
   stay excluded. Then review the colour change on close vehicles.
2. Generate the companion on the GPU (an FBO pass at upload, or a compute
   shader) or cache it on disk, never with `glGetTexImage` on the render
   thread. Store it as BC5 or RG8 snorm slopes plus R8 height instead of RGBA16F
   (8 B/texel).
3. Terrain: relief from the detail grain only, at a per-surface strength (sand
   weaker than gravel), faded out beyond about 30 m so the 44 taps only run
   where the relief is visible.
4. Objects: probably skip. There is no visible gain at x1 or x2 on any vehicle
   in the views.
5. Classic stays bit-identical: everything lives in the Remastered shaders and
   the late-VB path is gated on `LightingRemasterActive()`.

## Risks against the owner's taste

- **"Only darker":** V9 darkens lit ground by 7%; V6 fixes this (mean kept).
  The cavities still take 1-4% off the shade.
- **Crushed shade:** no. Shade/lit stays 0.73-0.85.
- **Embossed / fake:** the chroma gate keeps decals flat. At x2 the ground
  reads as coarse gravel, and on Sinai the hardpan reads more pebbly than
  sandy. A uniform grain everywhere can look like a texture overlay: the detail
  map repeats every few metres (about 1.56 m in the lighting lab's model of the
  terrain texgen; not measured in game).
- **Swimming:** none seen in two motion strips. Both are texture-space except
  V1.
- **Seams:** relief from the terrain base texture shows cell seams. Avoided by
  using the detail-only path.
- No colour shift, haze or grading.

## Files

- `make_override.py`: generates the override GLSL (`override_v3`..`override_v6`,
  plus the first agent's `override_v1`, `override_v2`, `override_diag`);
  `make_override_r2.py` is the round-2 generator as the first agent left it.
- `gen_tests.py`, `gen_seq.py`: research capture tests, written into the
  scratch worktree under `arma_CWR/tests/rendering/research/normals/`.
- `compare.py`, `analyze_r3.py`, `make_best.py`, `strip.py`, `timing_summary.py`,
  `diffstat.py`: sheets and statistics.
- `prototype.patch`: the C++ prototype against `claude/youthful-lovelace-8imbdh`
  @ 3c53141 (8 files; `git apply --check` passes on the lighting worktree).
  Scratch worktree: `D:\Arma_CWA_worktrees\gfx-normals`, build
  `dist/gfx-normals-rwdi`.
- `BEST_lowsun_fp.png`, `BEST_sinai_lowsun_fp.png`, `BEST_hull_close.png`:
  Classic | Remastered as shipped | + V6, full frame and a 2x crop.
  `three_*.png`: the same for the other views (the interior one uses V8).
- `evidence_*.png`: copies of the key diagnostic sheets named above.
- `runs/`: every capture run (r1/r2/diag from the first agent; r3-r6, diag2,
  tm2, tm3, seq2 from the resumed agent). r2 was cut off by the pause and is
  incomplete (r2_abel 4 shots, r2_sinai none, no sheets). tm_abel is empty: I
  stopped my own tri while it was still waiting for a slot (no game had started)
  and re-ran the timing as tm2/tm3 on the round-4 build.
