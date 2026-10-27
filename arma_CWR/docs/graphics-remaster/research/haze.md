# Distance haze and morning mist (track: haze)

Rank 9 of 22. Critic scores: worth building **5.5/10**, RTX-on **3/10**. Area: sky and air.

## What the player sees

Distant hills fade into the real sky behind them instead of into one flat fog colour. At sunrise, pink mist pools on lakes and valley floors and burns off by 08:00. Noon is unchanged.

## The idea

Current Remastered fog is the original linear fog with the sky's sun glow added toward the sun. Distance only fades toward one flat colour, so far hills look cut out against the sky, and the air has no structure.

## The technique

Single-scattering aerial perspective, per pixel, in every Remastered pixel shader:

1. **Haze that thins with height** (scale height 350 m), slightly stronger in blue, in closed form.
2. **The colour hills fade into is the real sky behind them** (C++): after the sky and clouds are drawn, the engine copies the frame into a mip-mapped texture (`Engine::CaptureSkyBackdrop`). Each pixel samples that copy in the direction just above the horizon behind it. Far hills dissolve into the clouds, the sunrise glow or the overcast grey that is actually there.
3. **Sky light and sun light in the air:** a forward glow toward a low sun, cool away from it.
4. **Sun visibility in the air** (variants "vol"): 12 samples along the ray read the terrain horizon map and the cloud shadows, so air in a ridge's shadow scatters less.
5. **Morning valley mist:** a flat-topped layer about 40 m above the lowest ground within about 850 m (14 height-map reads), so it pools in valleys and on water and leaves ridges clear. It forms around sunrise, is thickest around 06:00 and is gone when the sun reaches 38 degrees.
6. **Nothing changes inside 150 m** by construction (not quite true in practice, see below).

The variant number rides on the `saturation` knob. Variant 0 is current Remastered bit for bit. The recommended look is v43 (air, mist, sun visibility, and a fix for a seam at the right edge).

## What real data showed

Eight rounds on Abel and Eden.

- **The hero image** (`BEST_valley_sunrise_mist.png`, Abel valley 05:15, sun 6 degrees): pink mist lies on the lake and the low ground, islands and ridges stand out of it, and the hard line where the sea met the flat horizon band is gone. It reads like a real dawn photograph and is still recognisably Malden.
- **06:30** (`BEST_valley_morning.png`): mist banks on the lake, the far island dissolves into the sky. The lake under the mist goes white-grey.
- **Time of day** (`BEST_valley_mist_timeofday.png`): forms with the sun, thickest at 06:00, thins by 07:15, gone at 08:00.
- **Sunrise ridge:** the 2 km massif sits in the dawn sky instead of being cut out against it. Some will read it as washed out.
- **Overcast:** hills at 1 to 3 km go lighter and blue-grey. The closest thing to "blue haze" in the set. Most of the cool shift comes from the real sky colour behind the hills.
- **Noon:** almost nothing changes (an honest null result).
- **Failed:** the first family (stronger blue, rounds 1 to 5) turned far hills blue-teal. The 12-sample sun march changes only about 2% of pixels at sunrise and nothing at 06:30.

## Cost

Not known. The best single reading was about +9 ms; round 7 minimum readings were 33.6 against 5.3 ms (sunrise) and 44.2 against 18.1 ms (valley). It may be much more than +9 ms as built, because it runs per pixel in every pixel shader, and alpha-tested forest overdraw multiplies it. The critic estimates that a deferred half-resolution pass would land near 1 to 2 ms at 1080p on a 3060.

## Verdict

Critic: the sky-matched haze and the terrain-pooled mist "give one strong sunrise image and small gains elsewhere, at an unmeasured and probably large per-fragment cost", and they lean toward a blue lift and a wash in the 06:00 to 06:30 frames.

## What production needs

- **One full-screen pass** after the opaque world, reading the resolved depth (no overdraw). Blended surfaces keep a cheap analytic version.
- **March at half or quarter resolution** with blue noise and temporal accumulation (removes the crosshatch and most of the cost).
- **A precomputed mist floor** per island (the lowest nearby ground, baked once at load), so 16 reads become 1.
- **Drive the mist from weather and time** (humidity, `setFog`, season, wind), not only from an eastern sun.
- **Tie the mist into AI sight.** Today the mist is visual only, so AI would see through a mist bank that blinds the player.
- Grey extinction (v38) as the default and the thinner mist (v36).
- Best merged with the volumetric beams into one "air" pass (see `volumetric.md`).
- Drop the "keep the original look bit-identical" step (owner decision: the new lighting replaces it).

## Overclaims the critic found

- "The near slope is unchanged" at 05:15 is not true: the near slope moves 5.7 levels on average and leans blue. At 06:30 the bottom third lifts +7/+8/+14 RGB.
- The sun march as "a clear win at low sun": only 2.2% of pixels change by more than 16 levels.
- "+9 ms" is the most favourable reading, not an estimate.
- In the hero sheet the left forest renders as a flat dark block in one panel and as canopy in the other (a level-of-detail difference), so that third of the image does not compare like with like.
- The GPU numbers printed on the panels are single contended readings.

## Taste risks

A cool lift and about 9% less contrast at 06:00 to 06:30 (a wash). Far hills bluer under overcast. Mist hides the sun glint on lakes, which works against "water and shine". A fixed crosshatch in thick mist will read as a screen-door pattern in motion. A hard line where the horizon band meets the mist.

## Where the work lives

- Notes and patch: `tmp\research\haze\NOTES.md`, `prototype.patch` (5 files, 93 lines of C++: the sky backdrop), haze code `src\haze_block.glsl`, final shaders `shaders_r8\`, timings `timings_r6.md`, `timings_r7.md`, `timings_r9.md`.
- Scratch worktree: `gfx-haze`, build `arma_CWR\dist\gfx-haze-rwdi`.
- Best images (not committed): `tmp\research\haze\BEST_valley_sunrise_mist.png`, `BEST_valley_morning.png`, `BEST_valley_mist_timeofday.png`, `BEST_sunrise_ridge.png`, `BEST_overcast_broken.png`, `cmp_vol_sunrise.png`, `view\t0515_zoom.png` (the crosshatch).
- Integration: a candidate for the new branch `graphics-remaster` the owner asked for on 2026-10-17, as part of a deferred air pass. That branch did not exist yet when this page was written.
