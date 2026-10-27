# Surface relief from textures (track: normals)

Rank 18 of 22. Critic scores: worth building **4/10**, RTX-on **2/10**. Area: surface detail.

## What the player sees

On vehicles, nothing. On ground at a low sun, gravel gets pebbles with tiny shadows. At noon it is nearly invisible.

## The technique

Normal maps (per-pixel surface directions) generated from the existing colour textures, no new art.

- The tangent frame is rebuilt per pixel from screen derivatives, so no mesh tangents are needed. The bump is a surface gradient subtracted from the geometric normal (Mikkelsen's form).
- Guards: the bump never lights a face the geometry turns away from the sun; the shadow lookup keeps the geometric normal; foliage keeps its crown normal; a chroma gate drops brightness steps that come with a colour step (paint, decals, numbers), so the M113's star stays flat.
- **Load-time normal maps** (C++, V5): every texture upload reads back its top level and builds a slope map plus a height channel on the CPU, uploaded as a companion texture.
- **Micro self-shadows** (V7): an 8-step march toward the sun over the height field.
- **Cavities** (V8): sky occlusion over the same height field.
- **Mean-preserving** (V6, the best): a linear bump in the sun term and the micro-shadow visibility divided by its expected value, so the ground does not get darker on average.
- The variant rides on the `saturation` knob. A runtime toggle (`O=1`) puts late-loaded shapes back on the CPU path, to show "Remastered as shipped".

## What real data showed

Six rounds plus diagnostics and motion runs.

- **Vehicles, interiors, tents:** almost nothing, even at double strength. CWA vehicle textures are flat paint with the dark lines already painted in.
- **Low-sun gravel** (`BEST_lowsun_fp.png`, `BEST_sinai_lowsun_fp.png`): sunlit gravel reads as gravel, pebbles throw small shadows away from the sun. With V6 lit gravel keeps its mean brightness on Abel (107.4 to 109.1) while local contrast rises about 2.5 times. Shade against lit stays 0.73 to 0.85 (not crushed).
- **Seams:** relief from each terrain square's base texture showed hard steps at the 50 m cell borders. Dropped; the terrain uses only the detail grain.
- **Motion:** two short strips (eye walking 0.25 to 0.35 m per frame) show no swimming.
- **Interior:** forcing the tank gunner interior onto the per-pixel path made it much darker (the hull shadows it and there is no interior light). Rejected.
- **A shader bug fixed on the way:** round 2 decoded signed slopes as unsigned, which would have tilted every normal.

## Side findings

1. **Script-spawned vehicles never get the per-pixel shading** (found here independently; a debug override painted the CPU path magenta in the shipped build, `evidence_cpu_path_magenta_shipped_build.png`). Pointer memory for "already tried" shapes is unsafe because shapes are freed and reloaded at the same address. The sun disc vanished when the sun halo got a buffer. Fixed on the branch in 747ffc8 (see `bugs.md`).
2. **The Zelda slat armour turns from grey to tan** on the per-pixel path. The spawned-vehicle bug fix later found the cause: the slat texture is tan, and on the CPU path the shaded face gets only the original look's blue ambient, which turns tan into grey.
3. **The gunner interior LOD always draws on the CPU path** (skipped as a view level above 900).

## Cost

At most about 1 to 2 ms at 1440x960 internal with 4x MSAA (noise floor about 1 ms), from V9's timing; V6 itself was never timed. About 44 texture reads per terrain pixel in V6. The load-time maps cost 5.0 s for 592 textures on Abel and 6.6 s for 552 on Sinai, up to 230 ms for one 1024x1024 texture, on the render thread. Textures streamed during play would hitch.

## Verdict

Critic: "Real, honestly documented, but small." The load-time normal maps are a negative result on CWA's flat-painted vehicles. The useful piece is a shader-only, low-sun gravel micro-relief that is taste-risky on sand and grass.

## What production needs

1. Drop the load-time normal maps entirely: they cost 5 to 6 s of load and 230 ms stalls and show nothing.
2. Terrain relief from the detail grain only, shader-only, with a per-surface strength (much weaker on sand and grass) and a fade beyond about 30 m.
3. A real shimmer test at 60 fps with camera motion.
4. Objects: skip.

## Overclaims the critic found

- The best variant does not use the load-time normal maps the track is named for (`kBumpTerrNmap = 0.0`): terrain relief comes from the detail texture alpha in the shader.
- "No swimming" is overclaimed: two 5-frame strips cannot show sub-pixel shimmer of near-black specks.
- Sinai lit ground gets 3% darker (78.3 to 75.7), not unchanged.
- V6 was never timed; the 1 to 2 ms is extrapolated from V9.
- The "as shipped" panels come from the prototype build with the runtime toggle, not the shipped build (the magenta diagnostic did run on the shipped build).

## Taste risks

Sinai sand turns into cobbles. Black specks on dirt roads and grass that will likely shimmer. The same grain on every surface reads as an overlay.

## Where the work lives

- Notes and patch: `D:\Arma_CWA_worktrees\lighting\tmp\research\normals\NOTES.md`, `prototype.patch` (8 files, 23 KB), shaders `override_v1` to `override_v6` (generated by `make_override.py`), `timings.md`, `runs\r6\ground_stats.txt`, `runs\r6\hull_stats.txt`.
- Scratch worktree: `D:\Arma_CWA_worktrees\gfx-normals`, build `dist\gfx-normals-rwdi`.
- Best images (not committed): `D:\Arma_CWA_worktrees\lighting\tmp\research\normals\BEST_lowsun_fp.png`, `BEST_sinai_lowsun_fp.png`, `evidence_terrain_cell_seams_r4.png`, `evidence_interior_per_pixel_too_dark_r5.png`, `evidence_zelda_cpu_path_vs_per_pixel_r6.png`, `runs\seq2_abel\lowsun_fp__motion2_strip.png`.
- Integration: only the shader-side terrain relief is a candidate for the new branch `graphics-remaster` the owner asked for on 2026-10-17, and it overlaps with the tiling track's relief. That branch did not exist yet when this page was written.
