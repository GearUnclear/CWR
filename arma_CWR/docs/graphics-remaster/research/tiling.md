# Terrain anti-tiling (track: tiling)

Rank 17 of 22. Critic scores: worth building **4.5/10**, RTX-on **2/10**. Area: surface detail.

## What the player sees

Fewer repeating patterns on the ground, large lighter and darker patches that follow the terrain's shape, and fine relief on near ground at low sun. A clear win on Sinai seen from height; small on green islands at eye level.

## How CWA terrain is textured (findings)

- Every land grid cell (50 m on the stock islands) draws one land texture over the cell, with a small random UV jitter per vertex. Tileable textures continue across neighbouring cells of the same type; transition textures are clamped.
- Every cell is drawn with the detail texture (`detail_dx.paa`, 128 px), one repeat every 1.56 m, through `psDetail` (`psDetailRemasterMain.glsl` in Remastered).
- The grass layers redraw the land mesh with `psGrass`, so they must take the same lookups as the ground or they ring the camera.
- Filtering is already at 16x anisotropy.

## The technique

Shader overrides only (no C++), on the shared build.

1. **Hex-tiled land texture:** a world-anchored triangle lattice (30 m) gives each vertex a random UV offset; three reads blended with sharpened weights in a way that keeps the texture's contrast. Only for tileable cells (detected by a research hack; production passes a flag).
2. **Hex-tiled detail, two octaves:** the 1.56 m detail and a coarse 12.5 m octave, each randomly rotated or mirrored per lattice vertex.
3. **Patch macro, brightness only:** three octaves of the cloud noise (700, 190, 60 m) minus terrain convexity (ridges lighter, hollows darker) minus slope. The "balanced" version is symmetric about the authored brightness and gated by colour, so pale fields do not bleach.
4. **Relief from albedo:** a tangent-free bump from the detail and land-texture brightness, faded in only once the texels are minified.

## What real data showed

Six runs on Abel, Eden and Sinai (with @LoBo).

- **High Sinai at noon** (`BEST_high_sinai_noon.png`): the flat beige plateau turns into varied desert ground with darker gravel-like patches and lighter sand that follow the land. The far dunes gain form. The one result an owner would notice.
- **Repetition** of distinctive features (the dark specks of Sinai sand, dark blobs in Eden grass) is gone (`cmp_hex_land_texture_sinai.png`).
- **Low sun:** the relief makes near ground read as gravel (`BEST_lowsun_fp.png`).
- **Eye level at noon on Abel and Eden:** small; some views slightly worse.
- **Failed:** warping the texture lookup to break up the smeared slope texture (marbled liquid swirls and seams along triangle edges, `rejected_warp_forest.png`); a macro that also changed saturation (washed pale fields, `rejected_bold_lush_noon_fields.png`).
- Eden numbers mix the macro with drifting cloud shadows (the same shader read -10% in one run and -1.5% in the next).

## Cost

About +0.5 to +2.5 ms per frame at 1440x960 internal with 4x MSAA, more when terrain fills the frame (interleaved test, four repetitions; the ranges straddle zero). The four tileability probe reads can be removed in production.

## Verdict

Critic: real and verified; a clear win on Sinai from height and crisper near-ground grit at low sun or on trails. On green islands at eye level the gain is small, with a mild wash on pale fields and darkening on the forest edge. "This is texture work, not RTX-style light."

## What production needs

- Pass the cell's clamp flag and the grid size as uniforms; a proper mip-mapped noise texture (the cloud noise has no mips, so the 60 m octave will alias in motion).
- Anti-tiling plus relief everywhere; the patch macro per island (desert maps gain most, green Eden least), with crisper, terrain-shaped boundaries and an amplitude that cannot pull a whole view darker or paler.
- A motion test; check seams where hex-tiled cells meet clamped transition cells.
- The relief overlaps with the normals track (`normals.md`).

## Overclaims the critic found

- The forest edge view is 8% darker, not 6%, and the balanced macro does not fix it.
- On `noon_fields` the near field gets paler and brighter, moving further from the original look, not toward it.
- `BEST_lowsun_fp.png` is presented as the full package, but on that view the relief alone gives nearly all of the change (0.37 of 255 difference without the macro).
- The near slope smear is partly reduced after all (crisper grass detail on the forest edge slope).
- The GPU numbers on the pictures are meaningless.

## Taste risks

Pale fields get paler (a mild wash). Sinai near sand 11% brighter. The forest edge view 8% darker. The dark patches can read as cloud shadows on a clear day. Blending flattens authored texture contrast; ploughed rows lose their regularity. It does nothing for the existing khaki grass.

## Where the work lives

- Notes: `tmp\research\tiling\NOTES.md`. This track has no `prototype.patch` (it is shader-only). The work is the shader overrides: final `ovr_final2\` (`psRemasterCommon.glsl`, `psDetailRemasterMain.glsl`, `psGrassRemasterMain.glsl`), earlier `ovr_v1` to `ovr_v5` and `ovr_final`. Statistics `stats_run5.txt`, `stats_run6.txt`.
- No scratch worktree (runs used the shared `dist\lighting-rwdi` build).
- Best images (not committed): `tmp\research\tiling\BEST_high_sinai_noon.png`, `BEST_sinai_desert_noon.png`, `BEST_lowsun_fp.png`, `cmp_hex_land_texture_sinai.png`, `rejected_warp_forest.png`, `rejected_bold_lush_noon_fields.png`.
- Integration: anti-tiling and relief are candidates for the new branch `graphics-remaster` the owner asked for on 2026-10-17; the macro per island only. That branch did not exist yet when this page was written.
