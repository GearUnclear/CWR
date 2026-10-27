# TAA (track: temporal)

Rank 14 of 22. Critic scores: worth building **5/10**, RTX-on **2/10**. Area: infrastructure.

## What the player sees

On its own, very little: edges a little smoother and the frame a little softer. Its value is that cheap, noisy effects (soft shadows from 4 samples, ambient occlusion without a blur) come out clean within 2 to 4 frames.

TAA (temporal anti-aliasing) blends each frame with the last few. The camera is shifted by a fraction of a pixel every frame, so the history holds more samples than one frame.

## The technique

All behind `triSetLighting taa=1`, only in Remastered with post effects on.

1. **Sub-pixel jitter:** Halton(2,3), 16-frame cycle, added in every vertex shader, only for the world's 3D passes (the HUD and dialogs are never jittered).
2. **Optional texture LOD bias** (-0.5) while TAA runs, to win back texture detail.
3. **Resolve pass** (`psPostTaa`, after the composite): rebuild the current frame at the pixel centre, reproject with camera motion only (the matrix is built in double precision), sample the history with a 9-tap Catmull-Rom, drop the history where the depth does not match, clip it to the local colour range, blend at 10%.
4. **Output pass** (`psPostTaaOut`): a bounded sharpen (0.5, no halos) and a dither.
5. **Stochastic effects that lean on TAA:** soft sun shadows (PCSS) with a few rotated taps per frame; ambient occlusion with a rotated pattern and no blur; random alpha-test thresholds that TAA averages into partial coverage.

Knobs: `taa taaAlpha taaSharpen taaJitter taaGamma taaDepthReject taaMipBias taaStochAlpha aoStochastic aoTaps aoRadius shadowStochastic shadowTaps sunSize`.

## What real data showed

- **As anti-aliasing it is a sideways move.** Against today's 4x MSAA at 150% scale, TAA trades 6 to 18% of measured sharpness for a steadier image: about 25% less frame-to-frame change on the forest edge, about 3 to 5% on town and overcast views. Steadiness per unit of sharpness is roughly equal.
- **MSAA must stay under TAA:** without MSAA, TAA breaks the thin M113 antennas into fragments. TAA plus 4x MSAA at 100% keeps them whole.
- **Sharpen 0.5** is near today's look; 0.8 is crisp with no halos.
- **Moving objects:** a Jeep moved 0.12 m per frame left no visible ghost trail. Faster vehicles, rotors, the first-person weapon and particles were not tested.
- **The enabler result** (`final\forest_edge_golden__convergence.png`, `final\forest_edge_golden__strafefx_motion.png`): a 4-tap stochastic soft shadow is dotted without TAA. Under TAA it is smooth by frame 2 to 4, matches a 16-tap reference, and stays clean while the camera pans or strafes. The soldier's shadow goes from a uniformly sharp cut-out to sharp at the boots and soft at the head.
- **AO with 4 taps** stays faintly grainy even under TAA; it needs 6 to 8 taps or a light blur.
- **Failed:** TAA used only as anti-aliasing (softer for about 4% steadier town views).

## Cost

TAA resolve plus sharpen: **+0.07 ms at 960x640, +0.14 ms at 1440x960**, stable across all three timing runs. Memory: two RGBA16F, two R32F and one RGBA8 targets at render size. Frame and world totals were not usable (the same mode moved by up to 2 times between runs). The hoped-for saving from dropping 150% supersampling was never measured.

## Verdict

Critic: "Real, working TAA infrastructure that removes the dotting from 4-tap stochastic PCSS and AO cheaply, but a player would barely see any change on its own." Build it only together with an effect that uses it (soft shadows, reflections, the air pass, combat shadows).

## What production needs

1. Per-object motion vectors (previous world matrix per instance) for vehicles, rotors, animated soldiers and first-person weapons.
2. A mask for particles, muzzle flashes, tracers and glass (lower history weight there).
3. Resolve in HDR, before the tone curve.
4. Vehicle interiors: today the interior clears the world depth, so TAA turns off there.
5. Keep MSAA under TAA; 2x or 4x at 100% plus TAA as the replacement for 4x at 150%. Sharpen about 0.5 as a user setting.
6. A sun of 0.5 to 1 degree for the soft shadows, not 2.5.

## Overclaims the critic found

- The "before" panel of the convergence sheet is a glitched capture with the soldier's shadow cut off, which inflates the gain. The strafe sheet's bottom row is the honest reference.
- The "about 20% lower noise" metric mostly measures blur: it rates TAA lower than the noise-free current image and the 16-tap reference.
- On Sinai the crisp long leg shadows become faint smears; the change there is neutral to slightly worse, not "modest".
- The 2.5-degree sun is 5 times the real one, so the soft-headed shadow is stylised, not physical.
- Some sheets come from builds before the final rebuild.
- Production need "Classic bit-identity test" is moot after the owner decided the new lighting replaces the original look.

## Taste risks

The whole frame gets 6 to 18% softer, close to the rejected "wash over the frame". Low-sun shadows on Sinai lose contrast. Ghosting on fast movers and smeared particles are untested.

## Where the work lives

- Notes and patch: `tmp\research\temporal\NOTES.md`, `prototype.patch` (9 files, about 535 added lines), metrics `final\pan_metrics.json`, `final\r4_noise.json`, timings `final\timings_run12.md` to `timings_run14.md`.
- Scratch worktree: `gfx-temporal`, build `arma_CWR\dist\gfx-temporal-rwdi`.
- Best images (not committed): `tmp\research\temporal\final\forest_edge_golden__strafefx_motion.png`, `final\forest_edge_golden__convergence.png`, `final\thin_features_antennas.png`, `final\sinai_lowsun_fp__ghosting_grid.png`, `final\sinai_lowsun_fp__sharpen_sweep.png`.
- Integration: a candidate for the new branch `graphics-remaster` the owner asked for on 2026-10-17, as shared groundwork for other effects. That branch did not exist yet when this page was written.
