# G-buffer (track: gbuffer)

Rank 15 of 22. Critic scores: worth building **5/10**, RTX-on **1.5/10**. Area: infrastructure.

## What the player sees

Nothing yet. A G-buffer stores extra per-pixel pictures (surface direction, material, colour, light) during rendering, so later effects can reuse them. The bounce light built on top of it was invisible.

## The technique

Three extra MSAA colour targets written by the Remastered lit programs during the world pass, resolved for the post pass:

| Target | Format | Colour channels | Alpha |
|---|---|---|---|
| 0 (existing) | RGBA8 | lit colour | 1 minus sky share |
| 1 | RGBA16F | world-space normal (crown normal for foliage) | roughness from specular power |
| 2 | RGBA16F | non-sky light (sun, moon, local lights, emissive), linear | fog transmittance |
| 3 | RGBA8 | ambient response (albedo times material ambient) | sky share |

- Extra renderbuffers on the frame target, switched on only inside the world pass of a Remastered frame, so the HUD, dialogs and portraits never write them.
- A per-draw write mask (`glColorMaski`): only Remastered lit programs write.
- **The consumer:** screen-space one-bounce light plus ray-traced sky visibility at half resolution. 8 directions per pixel, 14 to 20 steps through the depth, radius 6 to 15 m. A ray that hits a surface facing it gathers that surface's light; a ray that leaves the screen counts as open sky. Then a temporal average (reset on any camera move) and a depth- and normal-aware blur.
- v1 painted a hard black wedge of false occlusion on near terrain (the coarse terrain grid's interpolated normal leans off the triangle). v2 rebuilds the geometric normal from depth; the wedge is gone.
- Knobs: `gbuffer`, `gi`, `giRadius`, `giSteps`, `giDirs`, `giAo`, `giTemporal`, `giSource`, `gbufMaskDebug`, `lateVbo`, `debug=5..11`.

## Side finding: script-spawned vehicles skip the new lighting

The first G-buffer captures had the Jeep, tent and M113 missing from every target. Every shape first loaded after mission load has no vertex buffer, falls back to the CPU path and draws pre-lit. `BEST_lowsun_shadow.png` shows it plainly: a Jeep standing in an M113's shadow is drawn sunlit. This track's fix (`lateVbo`, building the buffer inside `ShapeBank::New`) **broke the night**: the whole town rendered as if lit by day. The fix that landed on the branch (747ffc8) waits until the next drawn frame instead. See `bugs.md`.

## What real data showed

Change against Remastered with the vehicle fix, same run:

| Variant | lowsun_fp | town_day | sinai_lowsun_fp |
|---|---|---|---|
| bounce x1, radius 6 m, own occlusion | mean -0.34, 1.6% of pixels beyond 4 levels | -0.46, 3.2% | -0.60, 3.6% |
| radius 15 m, x3, whole-frame source, bounce only | +0.30, 5.9% | +0.68, 4.9% | +0.65, 5.6% |

- With its own occlusion the frame gets slightly **darker**.
- Even the strongest variant lifts shaded vehicle and soldier faces by only about 5 to 10 levels.
- **Why so small**, measured on the Jeep's shaded rear: sunlit grass carries about 0.11 linear of light; the bounce reaching the Jeep is about 0.01 (the ground right behind it is in its own shadow, sideways rays leave the radius, rays toward the camera leave the screen); the Jeep's response is about 0.16. One bounce adds about 0.0016 to a surface at about 0.05. The rig's own sky and ground fill already carry most of the indirect light, and dark military albedo eats the rest.
- **Night:** no visible gain. The G-buffer light was stored before the night-eye darkening, so at night it is about 13 times brighter than the frame.
- Roughness is almost empty: only glass has authored specular power.

## Cost

Bounce pass +1.5 to +2.5 ms at half resolution (the stock AO it replaces is 0.17 to 0.6 ms). The G-buffer itself is estimated at 1 to 2 ms (bandwidth); it could not be measured on the shared GPU. Total roughly +3 to +5 ms at 1440x960 with 4x MSAA. A packed layout would roughly halve the G-buffer part.

## Verdict

Critic: "The G-buffer plumbing is real and clean, and it found a genuine stock bug." The bounce light built on it is invisible at a glance, and the late-VBO hack breaks night. Keep the G-buffer as groundwork; do not ship this bounce consumer as a look.

The critic also named the bold move nobody made: switch the rig's hand-tuned sky and ground fill off (or scale it down hard) and let the traced visibility and bounce carry the shade. See README, "Road to RTX-on", phase 4.

## What production needs

1. Pack the targets: octahedral normal plus roughness, light in R11G11B10F, response in RGBA8 (8 to 12 bytes per sample instead of 20). Use multisample textures so consumers can read one sample instead of three resolve blits.
2. Store light after night-eye and fog, in the frame's own units.
3. Blended draws (vehicle glass) must not overwrite the surface behind them.
4. Reconstruct roughness per material class (see `materials.md`).
5. For any bounce: reprojected temporal accumulation, a far-field term, interleaved directions across 2x2 pixels (about 1 ms).
6. Drop the "keep the original look bit-identical" requirement (owner decision).

## Overclaims the critic found

- `BEST_town_day.png` mixes runs: its third panel combines the vehicle fix and the bounce and comes from a different run than the Remastered panel. The x2 bounce sits inside run-to-run noise (1.1% against 2.5%). The crop sheet separates the two changes correctly.
- The GPU numbers on the panels are single noisy shots (one even shows the bounce panel as cheaper).
- The cause of the broken night is a guess, not proven: the whole town lights up, not just the late-loaded fire pit, so the hack may be bypassing night-eye or exposure globally.

## Taste risks

Its own occlusion makes frames darker. Strong multipliers would tint shaded faces olive from the grass. The vehicle fix makes spawned vehicles' shaded faces darker (correct parity with placed ones).

## Where the work lives

- Notes and patch: `D:\Arma_CWA_worktrees\lighting\tmp\research\gbuffer\NOTES.md`, `prototype.patch` (12 files, about 720 lines), bounce shader `override_v2\psPostGi.glsl`, `timings.md`.
- Scratch worktree: `D:\Arma_CWA_worktrees\gfx-gbuffer`, build `dist\gfx-gbuffer-rwdi`.
- Best images (not committed): `D:\Arma_CWA_worktrees\lighting\tmp\research\gbuffer\BEST_lowsun_shadow.png` (the vehicle bug), `BEST_forest_edge_golden.png` (the targets on real scenes), `BEST_town_day_crop.png`, `run06_v2\wedge_forest_edge_golden.png`, `run10_night\cmp_town_night.png` (the broken night).
- Integration: the G-buffer plumbing is a candidate for the new branch `graphics-remaster` the owner asked for on 2026-10-17, as groundwork. Its late-VBO version must not ship. That branch did not exist yet when this page was written.
