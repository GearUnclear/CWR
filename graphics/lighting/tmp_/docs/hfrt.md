# Long tree shadows at any distance (track: hfrt)

Rank 8 of 22. Critic scores: worth building **6/10**, RTX-on **3/10**. Area: light and shadow.

## What the player sees

At low sun, every tree, bush and building on the island casts a long, soft shadow, near and far. The stock shadow maps only cover objects near the camera.

## The technique

The name stands for heightfield ray tracing. It had two parts.

1. **Tracing the terrain itself** (shader only): march the terrain height texture toward the sun per pixel, a per-pixel sky visibility (8 slices), and one bounce of light off the terrain. **Mostly negative** (below).
2. **A canopy heightfield** (C++ plus shader): at island load, every landscape object that can shade the ground at a distance (trees, bushes, forest blocks, buildings, churches, towers, rocks) is drawn top-down into a texture. On Abel that is 84,395 of 98,198 objects, gathered in 16 to 22 ms and rasterised on a worker thread in about 1 s. Each texel holds the height of the object top above the ground and how dense it is. The shader cone-traces from each pixel toward the sun through that texture. Far occluders use coarser mip levels, so their shadows blur and lighten with distance.

The winning setup, mode 15/20: the shipped baked horizon map for the terrain, the canopy trace everywhere, 6.25 m texels (k = 2) with a "crossing density" of `sqrt(coverage)`.

## What real data showed

- **Terrain tracing failed.** Sky visibility from the terrain is 0.9 to 1.0 almost everywhere: the hills are gentle for their spacing. One bounce off the terrain was invisible, even at three times strength, because terrain albedo is low and the sky dominates the fill. Traced terrain sun shadows differ from the baked map only where the ground faces away from the sun anyway. Per-vertex tracing was blobby and slower. It cost +12 to 30 ms.
- **The canopy trace works at golden hour** (`BEST_hill_1730.png`, `BEST_slope_tele_1730.png`, `BEST_ground_b_1730.png`): every tree on the sunlit slopes casts a long shadow away from a 9-degree sun, trees stand on the ground instead of floating, distant forest blocks get a lit and a shaded side. In `ground_b` the near trees put the foreground rock slope in shade.
- **It also found that the shipped shadow maps cast no tree shadows in any captured view.** That is the start-up bug the shadows track found (see `shadows.md`).
- Shade against lit about 0.73 to 0.84 per channel: not crushed.
- 6.25 m texels with the square-root density match 3.1 m texels in strength with a quarter of the memory.
- Canopy sky occlusion added grey halos at tree bases in the debug view and only 0.2 to 0.4 levels in the frame: dropped.
- No change at sunrise (the trees' shadows fall behind the crest) or under overcast.

## Cost

Too high as built, because the trace runs per pixel in the forward pass, including every alpha-tested leaf. Paired deltas inside one run, shared 4060 Ti, 1440x960:

| View | Added GPU time |
|---|---|
| forest_edge_golden | about +2.5 ms |
| hill_1730 | +3 to +7 ms |
| ground_b (big trees near) | +8 to +10 ms |
| slope_tele (telephoto over a wooded slope) | +13 to +16 ms |

A reduced-resolution version was planned but not measured (the render-scale setting clamps below 1.0). Memory: about 21 MiB at k = 2, 85 MiB at k = 4.

## Verdict

Critic: "A real, verified find": every Abel tree gets a long, soft golden-hour shadow near and far, which the shipped shadow maps never draw. It is limited to low sun, it only darkens, it is cool in the shade, and as a forward per-pixel pass it costs too much.

## What production needs

- Move the trace off the forward pass: a half- or quarter-resolution sun-visibility buffer from a depth pre-pass (or the last frame's depth), with a bilateral upsample or temporal accumulation.
- One texture read per step (pack the terrain height into the canopy texture).
- Keep the canopy current when trees and buildings are destroyed (local re-rasterise).
- Crown base heights for broadleaf trees and a thin trunk term, so near shadows hold up next to shadow-mapped objects.
- Decide the near range with the shadow maps: now that tree shadows are fixed at start-up, the canopy could cover only the far field.
- Blue noise plus TAA instead of the fixed screen-space dither, which will crawl in motion.
- A warmer shade fill or some bounce, so it is not only darker.

## Overclaims the critic found

- The timing table mixes runs (each minimum from a different run). The honest numbers are the paired deltas above.
- "Mode 20 costs the same as mode 15" and "k = 2 is the cheap setting" are not shown; only the memory saving holds.
- "A long shadow at any range" and "contact-hardening" are overstated near the camera: there the shadows are soft wedges and slabs with no tree shape.
- No town, midday or vehicle view, and no camera-motion test.

## Taste risks

It only removes light (no bounce, no colour bleeding), which borders on "only darker". The shade on pink rock turns blue-grey (blue over red rises from 0.53 to 0.63). The fixed dither will crawl.

## Where the work lives

- Notes and patch: `D:\Arma_CWA_worktrees\lighting\tmp\research\hfrt\NOTES.md`, `prototype.patch` (33 KB, canopy gather, rasterisers, mips), shaders `override_v1` to `override_v10` (v10 holds every mode), `timings_best_across_runs.md`.
- Scratch worktree: `D:\Arma_CWA_worktrees\gfx-hfrt`, build `arma_CWR\dist\gfx-hfrt2-rwdi`.
- Best images (not committed): `D:\Arma_CWA_worktrees\lighting\tmp\research\hfrt\BEST_hill_1730.png`, `BEST_slope_tele_1730.png`, `BEST_ground_b_1730.png`, `SQRT_*.png`, `DEBUG_hill_0700.png`, `PERVERTEX_vs_PERPIXEL.png`.
- Integration: the canopy part is a candidate for the new branch `graphics-remaster` the owner asked for on 2026-10-17, at reduced resolution. The terrain-tracing parts should not be integrated. That branch did not exist yet when this page was written.
