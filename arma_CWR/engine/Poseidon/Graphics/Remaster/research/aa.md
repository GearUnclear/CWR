# Anti-aliasing (track: aa)

Rank 19 of 22. Critic scores: worth building **4/10**, RTX-on **1/10**. Area: image quality.

## What the player sees

Less shimmer on leaves and edges, crisper bark, needles and grass at close range. No change to light. More noticeable in motion than in stills.

## The main finding

**The shipped Remastered default has no anti-aliasing at all.** `GraphicsConfig` has `msaaSamples = 0` and `renderScale = 1.0`, so cutouts (leaves, nets) get a plain `discard` at alpha 0.753 and no alpha-to-coverage. Turning on 4x MSAA (with the alpha-to-coverage path that already exists) cuts foliage shimmer hard. On the town tree, the leftover frame-to-frame change in a sub-pixel pan goes 6.96 (no AA) to 5.39 (4x MSAA at 1.0x scale) to 3.54 (4x MSAA at 1.5x), and its 99th percentile 82.7 to 34.4 to 23.0.

A fix for this was built but **not landed**: its migration also added MSAA on top of a supersampling scale a user had already chosen. A second attempt was in progress when this page was written. See `bugs.md`.

## How the pipeline works (findings)

- The frame target is always an offscreen buffer, with `samples` MSAA at `renderScale` times the window. Defaults: no MSAA, scale 1.0.
- Cutouts use the alpha test at `0xc0` (0.753). With MSAA they use alpha-to-coverage with edge sharpening already in place, plus alpha-to-one in Remastered.
- Blended alpha draws (camouflage nets, glass, flares) also run the alpha test, with a reference of 1/255. Any coverage trick must skip them.
- The window output (`ResolveSSAAToDefault`) is an MSAA resolve plus a `GL_LINEAR` downsample blit, then the gamma pass. It runs after the last GPU timer mark, so its cost was never measured before.
- The shadow-depth cutout test is at 0.5.

## The technique

- **A. FXAA and CAS in the composite** (shader override): FXAA 3.11 and AMD CAS on the resolved frame.
- **B. A present pass** (C++): in Remastered, resolve into a texture and draw a Lanczos2 downsample (separable: horizontal pass, then vertical) plus FSR1 RCAS sharpening at window resolution, replacing the linear blit. It also adds a timer around the whole present step.
- **C. Cutout coverage** (shader, "j3"): a study of 55 CWA foliage textures showed the authored mips are 1-bit and already coverage-preserving. What thins distant crowns is the engine's 0.753 threshold applied to filtered alpha: on average 21% less coverage than authored. j3 moves the threshold to 0.5 once the texture is minified (the shadow pass already cuts at 0.5, so crowns then match their own shadows).
- **D. Shimmer measurement:** sub-pixel pans, the global shift found by phase correlation and removed, the leftover reported.

## What real data showed

- **4x MSAA:** stair-stepped leaf cards become soft edges (`figures\SETTINGS_town_night_crop.png`, `figures\PLAYER_DEFAULT_*.png`).
- **Lanczos2 plus RCAS 0.5** at 1.5x scale: crisper bark, needles, grass grain, net weave and vehicle edges, no colour or brightness shift. RCAS 1.0 looks processed.
- **Do not sharpen at 1.0x scale:** RCAS after a 1.0x frame sharpens the aliasing (forest 3.79 to 4.38).
- **FXAA** only matters without MSAA (cuts shimmer 16 to 21% there).
- **j3** is near invisible (1.8% of lower-frame pixels change); pines read slightly fuller.
- **Bug found and fixed:** an earlier coverage version applied to every alpha-tested draw and made the Sinai camouflage net vanish (`figures\BUG_v2_net_vanishes.png`).
- **Methodology finding:** the adaptive object detail (`Scene::AdjustComplexity`) switched distant forest blocks between two LODs from shot to shot. The track pinned it; the capture-harness fix later made that a tri verb (`bugs.md`).

## Cost

- 4x MSAA at native resolution: near free.
- Present pass (separable Lanczos2 plus RCAS 0.5): **+0.055 ms** at 960x640 output and **+0.155 ms** at 1080p output over the stock blit (minimum over 90 frames, three rounds).
- FXAA or CAS: +0.02 to 0.03 ms. j3: not measurable.
- The cost of a 1.5x supersampling preset at 1080p was not proven (contended 1080p frames ran 18 to 29 ms).

## Verdict

Critic: "The track is honest and every file checks out. The real win is turning on MSAA, which the shipped Remastered default lacks, plus a cheap Lanczos2+RCAS present pass." Less shimmer, crisper detail at crop scale, no change to light, so almost nothing for the RTX-on vision.

## What production needs

1. 4x MSAA on by default in Remastered (with a migration that respects a user's own choice). 1.5x scale only as an optional preset.
2. The present pass, used only when render scale is above 1, **before the HUD** (today the HUD is drawn into the scaled target first, so the prototype sharpens HUD text too). A sharpness slider, default 0.5. A GPU timer mark for it.
3. j3 in the remastered `psNormal` main, opaque cutouts only.
4. FXAA when MSAA is off.

## Overclaims the critic found

- "Lanczos2 alone is crisper than the blit" is not true: it is slightly softer but cleaner. The crispness comes from RCAS.
- Present-pass minima are stable to 0.001 ms only at 960x640; at 1080p the delta is good to about 0.04 ms.
- The separable against 2D difference figure is for the lower 55% of the frame; the whole frame differs more, still below noise.
- The fps cost of the 1.5x preset is not proven.

## Taste risks

None to the light. RCAS 1.0 looks processed; sharpening adds a little shimmer on far forest. Fuller distant crowns move toward the authored texture coverage, away from the original look's thinner trees.

## Where the work lives

- Notes and patch: `D:\Arma_CWA_worktrees\lighting\tmp\research\aa\NOTES.md`, `prototype.patch` (present pass, separable variants, present timer, LOD pin; 19 KB), `prototype_v3.patch` (earlier state), shaders `overrides\v3\`, shimmer `shimmer_v4v6.txt`, present cost `run_v5\present_cost.md`, texture study `tree_texture_mip_coverage_summary.txt`.
- Scratch worktree: `D:\Arma_CWA_worktrees\gfx-aa`.
- Best images (not committed): `D:\Arma_CWA_worktrees\lighting\tmp\research\aa\figures\SETTINGS_town_night_crop.png`, `figures\PLAYER_DEFAULT_forest_edge_golden_crop.png`, `figures\BEST_forest_edge_golden_1080p_present_1to1.png`, `figures\forest_edge_golden_1080p_present_zoom2.png`, `figures\PAN_DMH_town_night.gif`, `figures\BUG_v2_net_vanishes.png`.
- Integration: the MSAA default is a bug fix (second attempt in progress, see `bugs.md`). The present pass and j3 are candidates for the new branch `graphics-remaster` the owner asked for on 2026-10-17. That branch did not exist yet when this page was written.
