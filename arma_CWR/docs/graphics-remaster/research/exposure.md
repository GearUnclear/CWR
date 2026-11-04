# Local exposure (track: exposure)

Rank 20 of 22. Critic scores: worth building **3.5/10**, RTX-on **1.5/10**. Area: light and shadow (tone).

## What the player sees

Shade is brightened locally, so crushed vehicle sides are readable at low sun. The eye adapts slowly between dark and bright views. A new highlight curve keeps the sun and fire cores orange instead of white.

## The technique

All in the post pass (`EngineGL33::EndWorldScene`), before the composite.

1. **Recover scene-linear light.** The shipped tone curve (Khronos PBR Neutral without its toe) is exactly invertible: identity below 0.8, and the shoulder can be solved back. So the post pass gets light values up to about 5.6 times display white from the 8-bit frame, with no G-buffer.
2. **A luminance pyramid** at quarter resolution, with a centre-weighted meter.
3. **A bilateral grid** (Chen, Paris and Durand 2007), 45 x 30 cells x 16 brightness bins, built by gathering (GL 3.3 has no compute shaders), then blurred. This gives a local "base" brightness that respects edges.
4. **Eye adaptation:** an exponential approach to the metered brightness.
5. **Composite:** contrast below 1 under the mid level lifts shade, above it compresses highlights; then the adaptation; then the tone curve (Neutral, a hue-keeping version, or AgX).
6. **Gates:** the lift is weighted by the pixel's sky-light share (true shade is mostly sky light; dark paint or water is not), OR'd with a near-field weight (because vehicles and men draw with alpha 1 and would otherwise not be lifted), and faded out beyond 250 to 900 m so fogged hills do not haze.
7. **An HDR float frame** (RGBA16F; world shaders skip the tone curve, the post pass applies it once). Negative, see below.

## What real data showed

Six rounds, final variant F1A.

- **Low sun with vehicles** (`BEST_lowsun_fp_crop.png`): a clear win. The Jeep flank, tent folds, M113 camouflage and the soldier are readable; the sky is unchanged.
- **Sunrise ridge:** the backlit foreground goes from near black-green to readable grass, but reads flatter, and the warm sun streak on the ground loses contrast. Mood against readability.
- **Coastline sunset:** neutral to slightly negative (the near-sea strip lifts greyer).
- **Town at night:** about 0.75 EV darker (a night key), fire core orange.
- **Gunner interior and forest edge:** almost no change claimed (the critic disagrees on the forest, below).
- **Adaptation over time** (`adapt_sequence.gif`): tipping the camera from sky to ground, the ground brightens over about 1.2 s.
- **The meter put a number on a known problem:** the moonlit town meters -2.62 log2, brighter than every low-sun day view (-3.2 to -4.3) and only 0.5 EV under noon (-2.08). The night-bright bug fix later found the cause (see `bugs.md`).
- **Hue-keeping shoulder** (`HUE_sunrise_ridge.png`): a yellow-orange sun disc and an orange fire core instead of white. No change on dark olive hulls.
- **Negative results:** the HDR float frame (no gain by day; at night smoke blew out to white, `NEG_hdr_frame_town_night.png`); the AgX curve (muted colours, a grey sun, `NEG_agx_coastline.png`); local exposure without the gate (the sunset sea went flat grey, `NEG_ungated_lift_coastline.png`); adaptation to a fixed brightness target (+2 EV on most frames, blown skies, `NEG_adapt_key_sunrise.png`).
- **An engine flake seen on the way:** in one round every Remastered coastline shot, including the unmodified baseline, had the shore and near sea in shade. The capture-harness bug fix saw the same once and could not reproduce it at the same pinned time, so it stayed unresolved there. After all fixes, the roadmap log reports the shore cloud shadow in `coastline_sunset` gone (see `bugs.md`).

## Cost

About 1 ms at 1440x960 with 4x MSAA: the composite delta is a steady 0.9 to 1.1 ms on every exterior view. The track's own "best estimate" of 0.25 to 0.35 ms came from the cheap interior view. About 1 MB of extra memory.

## Verdict

Critic: "Honest, real tone-mapping work: local exposure makes the crushed Jeep and tent flanks readable at low sun, but it washes the sunrise, only darkens the night, and moves almost nothing toward an RTX-on look."

## What production needs

1. **A light buffer, not the alpha and depth hack:** an extra render target with the irradiance per pixel, so local exposure lifts only the illumination and never touches dark paint or water. This also helps other tracks (it is part of the G-buffer plan, `gbuffer.md`).
2. **The hue-keeping curve inside the world shaders,** where it costs nothing.
3. **The adaptation loop with the meter,** given to the night work, with a reset on teleports and cutscenes.
4. Drop the HDR float frame unless a future effect produces light above 1 (and then do not expand pre-lit particles).
5. A cheaper grid build and its own GPU timer.

## Overclaims the critic found

- "Sky unchanged" at sunrise is false for the final variant: the top 30% of the frame goes from 143.0 to 160.5 (the day adaptation is global).
- "Almost no change" on the forest edge undersells it: 55% of pixels change by more than 8 levels; the crowns brighten about 0.27 EV.
- The real cost is about 1 ms, not 0.3.
- The adaptation numbers do not reproduce exactly with Rec.709 luma (the ratios do), and the cap is reached by about 1.2 s.

## Taste risks

The sunrise foreground washes out to a flat grey-green and loses its warm sun streak (against the warm-evenings rule). Night only gets darker. The sun becomes a saturated orange ball, which may read as a cartoon.

## Where the work lives

- Notes and patch: `tmp\research\exposure\NOTES.md`, `prototype.patch` (7 files, 43 KB; the final look also needs `override_r5\psPostComposite.glsl`), `cost_summary.md`.
- Scratch worktree: `gfx-exposure`, build `arma_CWR\dist\gfx-exposure-rwdi`.
- Best images (not committed): `tmp\research\exposure\BEST_lowsun_fp_crop.png`, `BEST_sunrise_ridge.png`, `HUE_sunrise_ridge.png`, `adapt_sequence.gif`, `NEG_hdr_frame_town_night.png`, `NEG_agx_coastline.png`, `NEG_ungated_lift_coastline.png`, `NEG_adapt_key_sunrise.png`.
- Integration: the hue-keeping curve and the adaptation loop are candidates for the new branch `graphics-remaster` the owner asked for on 2026-10-17; the local exposure post pass should wait for a light buffer. That branch did not exist yet when this page was written.
