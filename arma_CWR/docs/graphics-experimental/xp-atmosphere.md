# xp-atmosphere: low-sun glare (POSEIDON_XP_SUNGLARE)

Lane goal: sky and aerial perspective, sun disc and haze, sunrise and sunset colour, at small cost. Base 243ef1e0 (graphics-remaster tip). Worktree D:\Arma_CWA_worktrees\sq-xp-atmosphere (uncommitted), patch `patch.diff` in this folder (sha256 c18c6b37...).

## What I found before changing anything

The tip's atmosphere is already mature, and the code and the integ7 stills confirm it: a Hillaire-style sky table, froxel volumetric fog with Rayleigh, haze and ground air, a horizon ring, a sun disc drawn round on the screen and reddened toward the horizon (camSunDisc, PhotoCamera.hpp kSunDisc*), and aerial perspective that already gives broken_high and valley_0515 their depth (distant land goes blue grey away from the sun and warm toward it). I found no cheap aerial-perspective change that would clearly look better, so the lane went after the one thing a camera shows at a low sun and the tip does not: the glare round the sun.

## What the switch does

`POSEIDON_XP_SUNGLARE=1` (unset or 0: off, and the frame is the tip's; another number scales the strength, up to 8). Tuning without a rebuild: `POSEIDON_XP_SUNGLARE_SHAPE=coreDeg,coreLevel,haloDeg,haloLevel` (default 0.6,1.5,3.0,0.4).

- A 1x1 pass taps the frame at 24 points over the sun's disc and measures how much of it still burns, smoothed over about four frames. The glare fades when a ridge, a tree crown or a cloud covers the sun and never paints a white ball over an occluder (sunrise_ridge, sun behind cypresses: barely changes).
- The composite adds, before the tone curve and after the meter (exposure does not change), a soft core plus a long faint halo round the sun's screen point, in the sun's colour, times that visible share. Full strength up to 15 degrees of sun elevation, a third from 40 degrees, none at night or below the horizon.
- The glare is round on the screen, like the disc. The first version used the true angle, which drew an oval near the frame's edge (lowsun_fp).

Files: `engine/PoseidonGL33/EngineGL33_XpAtmosphere.cpp` (new), `EngineGL33.hpp`, `EngineGL33_PostFX.cpp`, `EngineGL33_PostShaders.cpp`.

## How it looks (stills, same exe on vs off, deterministic world)

Four shapes were tried. Crops and full frames in `showcase\`.

- Strong (1.5 degree core x6): rejected. The core clips into a flat pale ball with a visible rim and veils the Sinai camo net (`crop_coast_off_strong_med_soft_round1.png`, `crop_sinai_off_strong_r4final.png`).
- Medium (0.8 x2.5): rejected, the rim still shows in lowsun_fp.
- Soft (0.6 x1.5 core, 3 degree x0.4 halo): kept, now the default. A wider, softer warm bloom round a low sun, closer to a photo (`crop_coast_off_r3soft_r4final.png`, `coastline_sunset_off/glare.png`, `valley_0515_off/glare.png`).
- Final r4 (soft and round), vgate on six views: broken_high IDENTICAL (sun out of frame), sunrise_ridge NEAR (max 36, 150 px over 8), valley_0515 max 17 (105 px over 8), coastline_sunset max 35, lowsun_fp max 24, sinai_lowsun_fp max 54. The Sinai change is a mild veil over the dark jeep under the sun: real lens veiling glare, but it costs some foreground contrast (`sinai_lowsun_fp_off/glare.png`).

Honest verdict: a small positive change at the sunset views (coast, valley), neutral to slightly worse at Sinai depending on taste. It is a look polish, not a depth leap. It is temporal (the visibility smoothing when leaves or a ridge cross the sun), so the owner should judge it in a clip.

## Cost (timed quick, same exe on vs off, 2 pairs per row, clean 0/41)

| view | d CPU work | d GPU frame | d GPU post |
|---|---|---|---|
| valley_0515 | +0.05 ms | +0.03 ms | +0.02 ms |
| coastline_sunset | +0.02 ms | +0.02 ms | +0.02 ms |
| lowsun_fp | +0.01 ms | +0.04 ms | +0.01 ms |

At the noise floor, well under 0.1 ms. Job: results\quick-r2-glare.

## Build and tests

Build green, 0 new warnings. Formatted with clang-format 22.1.8; 21.1.8 pending. Unit gate: PoseidonTests 3535 cases, 3525 passed, 6 failed (the known external-data six: equipment localization x4, map legend, MP version message), 4 skipped; PoseidonCoreTests 823 of 827 (4 skipped); PoseidonFoundationTests 475 of 475.

## Recommendation

Quality tier (High and Ultra), off until the owner has seen it in motion. Not pixel-identical with the switch on, by design. With it off no pass runs and the composite skips the glare branch (a uniform branch on a zero gain); the off side was not separately proven byte-identical to the tip exe.

## Jobs and evidence

- results\r1-glare1 (strong, 6 views), r2-glaremed, r3-glaresoft (4 views each): round 1 bracket, exe 1e981cef (dist sq-xp-atmosphere-r1-rwdi).
- results\r4-soft-round: final shape, exe 0be86728 (dist sq-xp-atmosphere-r2-rwdi).
- results\quick-r2-glare: timed cost.
- cmp\ and showcase\: side by side crops at 1:1.

## Open notes

- The tip's own aureole round the sun is drawn by angle, so it is slightly oval near the frame edge (visible in lowsun_fp with the switch off). Not touched.
- Distant sea in coastline_sunset stays much darker than the horizon sky; at grazing angles it should reflect closer to the sky's brightness. That is water shading (xp-water lane), not this switch.
- Squad tooling: `bash` on PATH from PowerShell is WSL here; squad-heavy commands that run a shell script need `C:\Program Files\Git\bin\bash.exe`.
