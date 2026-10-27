# Physically based sky (track: atmosphere)

Rank 7 of 22. Critic scores: worth building **6/10**, RTX-on **4/10**. Area: sky and air.

## What the player sees

The sky's colour comes from real air physics. A golden glow surrounds a low sun, twilight gets an orange band under a violet sky, and the flat band along the horizon now matches the sky behind it. The original cloud textures are kept. Ground, vehicles and water do not change.

## How the original look draws the sky (findings)

- The sky is one dome object (`data3d\obloha.p3d`, 53 points), drawn first through the software lighting path. Each vertex gets one colour; the shader sees no view direction.
- The sky textures are 256x256: `jasno.pac` (clear) is a plain blue gradient with no clouds, `oblacno.pac` is blue with cirrus, `zatazeno.pac` grey overcast. Overcast 0 uses 86% clear plus 14% cirrus.
- The flat band between the horizon and the sky is not the dome. It is a separate polygon (`Landscape::DrawHorizont`), fully fogged.
- The dome is drawn before the world pass begins, so on this branch it always drew through the original-look programs. Run 1 found that the new sky shader never ran for that reason.

## The technique

- A clear-sky atmosphere model after Hillaire (Rayleigh and Mie scattering, ozone, multiple scattering), computed on the CPU into small lookup tables: transmittance and multiple scattering once at load (about 100 ms), and a sky-view table rebuilt when the sun moves 0.02 degrees (8 to 12 ms).
- The dome vertices carry a marker; the vertex shader rebuilds an exact per-pixel view direction from screen position and the camera axes.
- Each sky row keeps the original look's brightness; the atmosphere supplies how a direction differs from its row's mean (sun glow, aureole, darker anti-sun side). `atmoHue` 1 is the physical hue, 0 keeps the original hue, 0.5 is half-way.
- Clouds come from the original texture: a texel brighter than the clearest texel of its row counts as cloud, and is lit by the sun as seen from 2.5 km up.
- Fog colour for every world draw (aerial perspective) uses the same sky function near the horizon. The far horizon polygon now matches the sky.
- Night: the whole thing fades out between -3 and -9 degrees of sun height.

Knobs: `atmosphere`, `atmoFog`, `atmoLight`, `atmoSun`, `atmoHaze`, `atmoExposure`, `atmoClouds`, `atmoHue`, `atmoSat`, `atmoDebug`.

## What real data showed

Five runs.

- **Coastline sunset:** the strongest result. The pink-cyan wash and the flat pink horizon band become a golden horizon glow under a blue sky with lit cirrus.
- **Dusk** (sun -1.7 degrees): a real civil twilight, an orange band on the horizon under a violet-blue sky, instead of a uniform salmon.
- **Low sun** (`BEST_lowsun_fp.png`): a warm aureole around the sun and lit cirrus. The props are lit exactly as before.
- **Sunrise:** reads as real dawn, but cooler than the original look's pink. The half-way hue keeps a mauve-pink sky with a coral glow (`ALT4_sunrise_ridge.png`).
- **Noon:** a clean graded blue; strong at saturation 1.3.
- **Overcast:** small change, no regression.
- **Rejected:** lighting the ground from the physical sky (`atmoLight`): blue ground and blue shadows. One global brightness for the whole sky (run 2): pale overcast, a grey sunrise.

## Cost

GPU: below the noise (by construction about 3 table reads and 40 instructions per world pixel; estimated well under 0.3 ms). CPU: 8 to 12 ms per sky-table rebuild every few seconds of game time, which would be a visible hitch.

## Verdict

Critic: "a real, working physically based sky that clearly improves low-sun and twilight skies at near-zero GPU cost." It is sky only. It has an olive horizon band, loses some noon cirrus and cools the frame with the physical hue. Ship it with the half-way hue and saturation of at most 1.15. On its own it is not a "different game" moment, because the lit world under the new sky is unchanged.

## What production needs

- Build the sky-view table on the GPU, or on a worker thread with double buffering.
- A real flag for the dome through the draw description instead of the vertex marker; handle a cropped (pillarboxed) view.
- Open the new programs for draws before the world pass properly (the current hook is a hack).
- Bake the cloud mask and row reference once per sky texture change instead of 12 reads per pixel.
- Fix the olive band, keep the noon cirrus, make the water reflect the sky for real.
- Light the 3D cloudlets the same way.
- Tie haze to fog density and overcast; add the moon.

## Overclaims the critic found

- "Water reflects the atmosphere": not visible. The sea differs by under 3 levels and stays mauve under a gold sky.
- At noon the cloud mask erases most of the cirrus (sky detail down about 18%).
- Aerial perspective barely reaches the ground: the bottom 40% of the frame changes by 0 to 1.15 levels.
- "No darkening by construction": at sunrise the upper frame is about 9% darker and moves from warm pink to neutral lavender.

## Taste risks

A muddy olive band in the sunset sky away from the sun. The physical hue cools sunrise, golden hour and dusk compared with the original look's pinks. Saturation 1.3 at noon comes close to a colour grade (flat cyan).

## Where the work lives

- Notes and patch: `D:\Arma_CWA_worktrees\lighting\tmp\research\atmosphere\NOTES.md`, `prototype.patch` (10 files, about 930 lines), shaders `glsl_final\`, standalone model `offline\`.
- Scratch worktree: `D:\Arma_CWA_worktrees\gfx-atmosphere`, build `dist\gfx-atmosphere-rwdi`.
- Best images (not committed): `D:\Arma_CWA_worktrees\lighting\tmp\research\atmosphere\BEST_lowsun_fp.png`, `BEST_coastline_sunset.png`, `ALT4_sunrise_ridge.png`, `REJECTED_run2_overcast_broken_globalK.png`, `VARIANTS_run4_lowsun_fp.png`, `OFFLINE_sky_panoramas.png`.
- Integration: a candidate for the new branch `graphics-remaster` the owner asked for on 2026-10-17, as the base for haze, clouds and water. That branch did not exist yet when this page was written.
