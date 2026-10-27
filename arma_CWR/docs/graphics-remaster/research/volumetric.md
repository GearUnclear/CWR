# Visible light in the air (track: volumetric)

Rank 5 of 22. Critic scores: worth building **6/10**, RTX-on **4/10**. Area: sky and air.

## What the player sees

Sun shafts slant through pine crowns at dawn and dusk. Headlight cones glow in night mist. The beams are blocked by the real trees, because they read the shadow map.

## The technique

Ray-marched single scattering (light that bounces once off the air toward the eye) of the sun and of local lights. It replaces the old screen-space sun shafts. It runs at the end of the world pass in `EngineGL33_PostFX.cpp`.

1. **March at half resolution** (720x480 for a 1440x960 target): 48 samples per pixel, dense near the camera, out to 250 m (about the reach of the shadow cascades), jittered per pixel.
2. **Sun visibility per sample**: one shadow-map tap, the terrain horizon map (for ridges) and the cloud shadow field.
3. **Medium**: a thin haze plus a ground mist that thins with height above the terrain. `setFog` thickens the mist.
4. **Sun light** uses the rig's own sun colour, so the beams take the same warm evening colour as the hulls.
5. **Local lights**: the 16 nearest scene lights. Headlights are marched through their cone. (Probe 6 fixed a reversed phase sign that no earlier shot had exposed.)
6. **Filter**: a depth-aware blur, a bilateral upsample, an optional temporal path.
7. **Composite**: `volGain` (probe 6) boosts a lit shaft that stands above its local, depth-matched mean. This is what made the beams readable. `volEdge` (probe 8) stops thin foliage from borrowing the sky's glow.

Knobs: `vol`, `volHaze`, `volMist`, `volMistHeight`, `volAniso`, `volDistance`, `volSteps`, `volLocal`, `volSpot`, `volCarve`, `volHighpass`, `volGain`, `volEdge`, `volTemporal`, `volBlur`, `volDebug`.

## Big side finding: no tree or building shadows in any capture

Tree, building and wall shadows were off in every capture run, including the `20261017-final` baseline. Only spawned vehicles and soldiers cast. Probe 1 therefore showed no beams at all. From probe 2 the research tests called `triEnableShadows`, and the shadow pass went from about 0.05 ms to a real 0.1 to 1.9 ms. The shadows track found the cause (a start-up order bug). See `shadows.md` and `bugs.md`.

## What real data showed

Nine probes on Abel.

- **God rays through a pine wood at 06:00** (`BEST_pinewood_morn.png`): diagonal shafts through the crowns onto the grass, a glow behind the crowns. The strongest daytime result.
- **Misty morning** (`BEST_pinewood_mist.png`, fog 0.35): shafts and a warm glow in the crowns, where current Remastered shows almost no mist.
- **Evening beam through a cypress row** (`BEST_pine_eve.png`) and **beams between village houses** (`BEST_town_eve.png`).
- **Sunrise** (`BEST_sunrise_ridge.png`): a cypress's shadow volume cuts through warm mist.
- **Headlights at night** (`BEST_hl_front.png`): a Jeep's cones glow in the mist. The biggest "different game" change of the track. From the side the beam is only a faint glow.
- **Sun behind the camera or overhead**: little change.
- **The trade-off:** the default mist (1.5) shows beams but lays a warm-white band along the horizon toward a low sun and a pale veil over backlit crowns. A clear-weather mist (0.3) removes the band and keeps narrow beams through gaps, but loses most of the wide fan in woods.
- **Rejected:** street-lamp and campfire halos (the fire tinted the whole night sky brown, lamp halos were invisible); "carving" the beams out of the haze (darkened the frame); an unfloored high-pass (darker objects in front of glow).

## Cost

Pass time 0.4 to 2.4 ms on the shared GPU at 1440x960, about 1 ms typical (probe 9 median 0.72 ms). The old shafts cost 0.15 ms. Turning on tree shadows (needed for any beams) makes the shadow pass cost 0.1 to 1.9 ms. Halving the steps did not halve the cost, which points at contention from other games.

## Verdict

Critic: "a real, working ray march occluded by the shadow map." Clearly visible god rays in woods at dawn and dusk, striking headlight cones at night. But the settings that show beams also veil the crowns, the clear-weather fix darkens backlit hulls, and blocky glow patches remain. Worth building as groundwork; headlights in mist are nearly ready.

## What production needs

- A mist density driven by weather and time of day: thin in clear weather (mist 0.3, gain 4), denser at dawn, dusk, in fog and in rain (mist 1.5, gain 2.5).
- Make up for the lost lift of the old shafts in open terrain, so the clear-weather setting does not darken backlit vehicles.
- Tree and building shadows on by default (the start-up bug).
- A different design for lamp and fire halos (sized by the lamp head, flicker-free).
- Cheaper: always-on temporal accumulation with 24 steps (matched 48 steps on a static view), quarter resolution, or a 3D grid (froxels) shared with local lights.
- Fix the box artefacts and the half-resolution edges.
- Best merged with the haze (see `haze.md`) into one "air" pass.

## Overclaims the critic found

- "Only adds light" is false: the pass replaces the old shafts. The clear-weather variant darkens backlit hulls by 15 to 19% against current Remastered (jeep 74.6 to 63.3, tent 73.4 to 59.6, APC 44.4 to 35.2).
- The blocky glow patches beside the right-hand cypress were said to be fixed in probe 6. They are still in both final pine_eve images.
- "Nothing changes with the sun behind the camera" is overstated: sunlit crowns lift about 7 levels and turn about 20% bluer.
- `BEST_pine_mist.png` comes from probe 5 code, not the final.
- The "current Remastered" panels have tree shadows forced on, so they are not the same before picture the other tracks used.

## Taste risks

A pale veil over backlit crowns and bushes (flattened crowns, a wash). A glowing horizon band on open ground. Translucent "ghost" cypresses at sunrise. Pink-white headlight beams with a hard upper edge. Night halos tinted the sky brown (rejected).

## Where the work lives

- Notes and patch: `tmp\research\volumetric\NOTES.md`, `prototype.patch` (44 KB, 4 files), shaders in `glsl\`, probe 5 state in `prototype_probe5.patch` and `glsl_probe5\`, timings in `timings.md`.
- Scratch worktree: `gfx-volumetric`, build `dist\gfx-volumetric-rwdi`.
- Best images (not committed): `tmp\research\volumetric\BEST_pinewood_morn.png`, `BEST_pinewood_mist.png`, `BEST_pine_eve.png`, `BEST_hl_front.png`, `BEST_lowsun_fp.png`, `cmp\probe8\lowsun_glow_check.png`, `cmp\probe8\town_night_fog_8.png` (the rejected brown sky).
- Integration: a candidate for the new branch `graphics-remaster` the owner asked for on 2026-10-17 (the sun and headlight parts, not the lamp halos). That branch did not exist yet when this page was written.
