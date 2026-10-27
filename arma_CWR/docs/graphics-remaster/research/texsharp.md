# Terrain detail (track: texsharp)

Rank 13 of 22. Critic scores: worth building **5/10**, RTX-on **2/10**. Area: surface detail.

## What the player sees

Slopes at 10 to 150 m show grass and rock grain instead of an airbrushed smear. Optional mild sharpening. No change to light.

## The diagnosis (where the blur comes from)

| Question | Finding |
|---|---|
| Anisotropic filtering on? | Yes, already at the maximum, 16x. Turning it off smears grazing ground visibly, so it works. |
| Mip LOD bias? | 0. A bias of -0.5 is barely visible and, with sharpening, turned motion into speckle. Not recommended. |
| Do textures load at full resolution? | Forcing every texture to its top level changed nothing visible. Texture loading is not the cause. |
| How are mips made? | Stored in the files and fine. Rebuilding them (Lanczos, coverage-preserving, box) changed nothing visible. |
| Terrain texture density | **The real cause.** Every terrain square is one 128x128 texture over 50 m (39 cm per texel). From 10 to 150 m one texel covers 5 to 15 pixels. |
| Detail texture | `data\detail_dx.paa`, 128x128 greyscale; its coarse mips average out, so the grain fades by about 20 m. |
| Supersampling downscale | At render scale 1.5 the frame is downscaled with one linear blit, which softens and aliases (not prototyped here; see `aa.md`). |

So no time should go into filtering, streaming or mip chains.

## The technique

**Self-detail** (shader only, `tsSelfDetail` in `psDetailRemasterMain.glsl`): each terrain pixel gets the brightness detail (high-pass) of its own square's texture, zoomed in 6 times and read from within about 2 m of that spot. Grass gets grass grain, rock gets rock grain, a cell next to a road reads road. Four cells around each pixel, each with a random rotation (this removed parallel "mowing stripes"), blended so the grain is not washed out, through a soft limiter. Only brightness relative to the local mean changes, so hue and average brightness stay as authored. The detail mip-maps away by itself at 150 to 300 m. 8 texture reads per terrain pixel.

Plus: **"detail kept"** (`K2`, the detail texture's contrast boosted at distance, up to 4x) with a second rotated sample against visible tiling (`K1`), and **CAS** (AMD contrast-adaptive sharpening) at 0.3 on brightness only.

## What real data showed

- **forest_edge_golden** (`BEST_forest_edge_golden.png`): the single biggest change. The smeared slope becomes dense grass grain.
- **Eden meadow (overcast):** gains turf texture, clear but moderate.
- **noon_fields:** a little grittier; most of it from the sharpening.
- **Sinai sand, near gravel:** near-identical. No gain.
- Ground mean brightness +0.1% with self-detail alone; hue ratios unchanged to three decimals.
- **Failed:** turning off filtering (diagnostic), forced full-resolution textures, LOD bias, rebuilt mip chains, Catmull-Rom magnification, relief from the base texture, coarse detail layers. Strength 2 looked scaly. CAS 0.5 on full colour turned pale ground into coloured speckle.
- **Motion:** a camera walk of 0.25 m per frame stayed within 0.65 to 1.24 times the stock shimmer ratio. No sparkle, coherent streaks.

## Cost

One self-detail evaluation costs about 0.35 ms at 1440x960 with 4x MSAA (from a probe that ran it 16 times per pixel). The whole final set: 0.5 to about 2 ms; direct pairs cannot resolve it on the shared GPU. CAS adds about 0.02 ms.

## Verdict

Critic: "An honest, well-evidenced shader-only terrain micro-detail layer." The slope goes from airbrushed to grassy on Abel and Eden. It is invisible on Sinai sand and adds no light at all, so it is a supporting layer, not an RTX-on step. The diagnosis is valuable: it closes off several dead ends.

## What production needs

1. Move the self-detail (rotated, strength about 1.2 to 1.5) into the remastered terrain shader in `EngineGL33_LightingShaders.cpp` with real uniforms and tuning keys.
2. Lower or drop the `K2` boost (it paints grime on paths).
3. CAS optional, on brightness only, default 0.3 or off.
4. Cost cut: 3 cells instead of 4, skip past 300 m.
5. The texture-residency estimate scaled by render scale squared (in the patch) as a correctness extra.
6. Validate at native 1080p without supersampling, in motion.

## Overclaims the critic found

- Most of the measured "sharpness" rise comes from the sharpening and the detail boost; self-detail alone adds 12 to 41%.
- "Ground mean brightness rises 1.0 to 1.5% with detail kept": the track's own numbers give 0.5 to 0.65% on three views, 1.4 to 1.5% on two.
- "Composite stable at 0.03 to 0.05 ms": true for two views only; `lowsun_fp` read 0.06 to 1.35 ms.
- The 0.35 ms per evaluation is extrapolated from the amplified probe, not measured.

## Taste risks

A golden slope reads more like dry straw (khaki). Olive and brown mottling on meadows. Dark blotches on paths from the detail boost. The sharpening touches the whole frame, objects too.

## Where the work lives

- Notes and patch: `D:\Arma_CWA_worktrees\lighting\tmp\research\texsharp\NOTES.md`, `prototype.patch` (31 KB, research knobs), final shaders `glsl7\`, offline mip analysis `offline\`, shimmer `run13\shimmer.md`.
- Scratch worktree: `D:\Arma_CWA_worktrees\gfx-texsharp`, build `arma_CWR\dist\gfx-texsharp-rwdi`.
- Best images (not committed): `D:\Arma_CWA_worktrees\lighting\tmp\research\texsharp\BEST_forest_edge_golden.png`, `ALT_selfdetail_only_forest_edge_golden.png` (no sharpening), `run8\BEST_unrotated_*.png`, `run13\forest_edge_golden__motion_diff.png`.
- Integration: a candidate for the new branch `graphics-remaster` the owner asked for on 2026-10-17 (shader only). That branch did not exist yet when this page was written.
