# Physical night (track: night)

Rank 11 of 22. Critic scores: worth building **5/10**, RTX-on **3/10**. Area: night and combat.

## What the player sees

The moon becomes the main light, with shadows. Street lamps are warm and fall off with distance. Headlights are real low beams with a hot spot and a sharp cut-off, aimed at the road ahead. Stars show, and dim areas lose colour the way the eye does at night. The big win is driving on a moonless road.

## Why CWA nights look the way they do (findings)

- At night the sun colour is black and the ambient is `BackgroundColor * 0.05`. The original look has no directional moonlight; current Remastered adds a weak moon with no shadows (`kMoonFull = 0.0035`).
- Local lights use a flat disc: full light out to `startAtten` (street lamp 50 m; headlight reflector 110 m inside a 12-degree cone), then a falloff out to 10 times that. Six lamps flat-light the whole Saint Louis square, and lamps hundreds of metres away still light open country: the moonless road at 23:00 is a bright grey landscape.
- The night-eye pass pulls dim pixels toward a brighter grey (the grey wash).
- The logs showed the Abel lamps uploading a white colour (1, 1, 1). The later bug fix found why: the engine read three-entry config colours as white (see `bugs.md`, night-bright, commit 72f32d4).

## The technique

Behind `triSetLighting night=1` and knobs. Recommended: `night=1, nightP1=0.3, nightP2=0.85, nightMoonSat=0.5, nightSpot=1.5, nightCamLights=1, nightLampWarm=0.7`, haze and bounce off.

- **Moon as the key light:** the sun slots carry the moon once the sun is 3 degrees under, so the shadow map, terrain horizon and cloud paths follow the moon. About 14 times the shipped moon fill, a cool colour mixed half-way to white, plus a moonlit sky fill.
- **Physical local lights:** lamps fall off with distance squared from the bulb (no flat disc). Headlights get a low-beam pattern (wide, a sharp cut-off just above the axis, a hot band just below).
- **Self-levelling headlight aim:** the hot band lands on the ground 25 m ahead.
- **Rough diffuse** for local lights (Oren-Nayar), so a road under a grazing beam reads lit.
- **Warm lamps** (toward 2700 K), headlights half-way.
- **One light list** of the 8 lights strongest at the camera, shared by every draw.
- **Purkinje shift:** very dim colour fades toward a slightly cool grey; lamp pools keep colour.
- **Smoke lit** by the same lights. **Stars** as soft round splats with a faint colour.
- A crash in the earlier haze code (stale light pointers after the fire was deleted) was fixed on resume.

## What real data showed

Ten runs on Abel.

- **Moonless country road from the driver's eye** (`BEST_road_flat_drive_moonless.png`): the strongest result. The original look and current Remastered light the whole landscape grey at 23:00. The prototype is dark, with a starfield, a warm low beam with a hot spot 20 to 35 m ahead and a hard top edge, and the town's lamps glowing on the horizon.
- **Moonlit views:** cool, low-saturation ground, stars, a faint jeep moon shadow. The original look draws moonlit trees black on a black sky (`BEST_moon_sky.png`).
- **Town:** a dark square, fire-lit smoke instead of white smoke. Pools are soft because the lamps hang 8 to 12 m up.
- **Night vision:** unchanged (1.7 of 255 mean difference).
- **Failed:** volumetric lamp haze (a brown wash over the town sky); lamp light bounced off the ground (0.21 of 255 mean difference, no visible effect).
- **Hill road:** Abel's test road climbs at 14 degrees. A sharp cut-off is very sensitive to aim; on the hill the beam passes over the road and lights only the verges. On a flat road the same beam is right.

## Cost

+0.8 to +3.3 ms (alternating run, medians of 3, shared GPU): town_lamps 3.90 to 4.70 ms, road_flat_drive_moonless 13.89 to 17.29 ms. Perhaps 1.2 to 5 ms at 1080p. The cost comes from the per-pixel loop over 8 lights on every lit pixel. Moon shadow pass 0.03 ms.

## Verdict

Critic: "A real, verified physical night rig." The moonless low-beam driving view is a large, player-visible win. The town views are mostly just darker, with black tree crowns, mustard verges and a jeep light leak. The RTX-style pieces (bounce, volumetrics, local shadows) failed or are missing.

## What production needs

1. A real light list: tiled or clustered culling (lights in a buffer, per-tile indices) instead of 8 uniforms shared by every draw. Share it with the combat lights (`combat.md`).
2. Shadows for the 2 to 4 lamps and headlights nearest the camera (light still leaks through walls).
3. Beam aim from the vehicle body with a softer cut-off, tested on hills; per-vehicle light colour and strength from config.
4. A night brightness policy agreed with the owner.
5. A neutral or cooler beam on vegetation, light through leaves near lamps, and a trace of the jeep-rear leak.
6. Delete the haze and bounce code.

Note: some of this is now on the branch in a smaller form. Commit 4515770 made the moon a real key light (no shadows), lowered the local-light cap to the original look's 1.0, and added the cool night grey; commit 72f32d4 fixed the lamp colour. See `bugs.md`.

## Overclaims the critic found

- The headline images were shot with lamp bounce on, not the recommended setting (low visual impact, since bounce is invisible).
- The jeep's rear is floodlit at 5 times the original look in the moonlit view (43.1 against 8.4) while it faces away from every light. The notes wrongly said the original look shows the same. The v9 fix did not change it.
- The moon's height term has no effect (the log reports the same key at 24, 36 and 69 degrees).
- Moving the moon makes no visible difference in town (no more than the smoke animation).
- "Silver crowns" are not visible: the crowns are a flat dim grey-green.
- No lamp pools are visible in town; facades are lit evenly, also far from any lamp.

## Taste risks

Headlit verges turn saturated yellow-olive (mustard). Town frames halve in brightness, with up to 7% pure black and a black birch crown that has a lamp inside it (flattened crowns, only darker). Moonless country may be too dark to play.

## Where the work lives

- Notes and patch: `D:\Arma_CWA_worktrees\lighting\tmp\research\night\NOTES.md`, `prototype.patch` (9 files, 769 lines), shader `ovr_v9\` (earlier `ovr_v4` to `ovr_v8`), timings `run_timing\timing_summary.md`.
- Scratch worktree: `D:\Arma_CWA_worktrees\gfx-night`, build `dist\gfx-night-rwdi`.
- Best images (not committed): `D:\Arma_CWA_worktrees\lighting\tmp\research\night\BEST_road_flat_drive_moonless.png`, `BEST_road_flat_head.png`, `BEST_town_night.png`, `BEST_moon_sky.png`, `BEST_road_head.png` (the hill case), `run7\cmp_town_lamps_haze.png` (the rejected brown wash).
- Integration: the moonless low beam, the falloff and the stars are candidates for the new branch `graphics-remaster` the owner asked for on 2026-10-17, built on top of the night fixes already landed. That branch did not exist yet when this page was written.
