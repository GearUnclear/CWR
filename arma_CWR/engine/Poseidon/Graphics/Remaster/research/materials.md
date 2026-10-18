# Wet weather and materials (track: materials)

Rank 6 of 22. Critic scores: worth building **6/10**, RTX-on **4/10**. Area: water and shine.

## What the player sees

In rain, cloth and stone darken like wet surfaces, puddles form in flat hollows with rain ripples and reflections, and every street lamp and fire draws a long reflection streak across the wet ground. In dry weather the change is almost invisible.

## The technique

RTX Remix rebuilds physically based materials from old textures. CWA has no material data beyond a texture, a colour set and sometimes a specular power, so the prototype guesses everything at run time.

1. **A material class per draw (C++).** Each textured section is classified from its texture file name against Czech and English word lists (stock data is Czech: `sklo` glass, `okno` window, `kolo` wheel, `plachta` tarp, `xicht` face, `silnice` road, and so on), plus the kind of object being drawn (car, tank, soldier, ship, weapon, static). 14 classes: paint, metal, glass, rubber, fabric, skin, road, masonry, wood, leather, weapon, lamp lens, roof, unknown. Terrain is classified per pixel from the ground colour.
2. **A response per class** (`shadeMaterials` in GLSL): roughness, metalness, porosity, sheen. A bump from the texture's own brightness. A satin sheen at grazing angles on hard objects.
3. **Wetness** follows the engine's rain density. Wet surfaces darken and saturate by porosity, vertical faces get run-down streaks, ground under a canopy stays drier.
4. **Puddles** on flat ground, filling hollows first, with animated rain rings. None on vegetation.
5. **Puddle reflections** through a screen-space reflection pass (`psPostSsr`).
6. **Lamp reflections** (v9): a highlight per local light (up to 8), reflected by a smooth water film on top of the dry surface. That gives the long streak toward the viewer.

## Side finding: script-spawned vehicles skip the new lighting

This track found the spawned-vehicle bug independently (logs show `draw sim car ... buffer 0` until its `vbfix` knob builds the buffer). It was later fixed on the branch (747ffc8). See `bugs.md`.

## What real data showed

Nine capture rounds.

- **Rainy night in Saint Louis** (`BEST_town_night_rain.png`, v9): each lamp and the fire throw a separate streak toward the camera, the road shines, puddles mirror the half-timbered houses. The original look and current Remastered show a dry lawn under rain. The strongest result, and the one a player would call a different game.
- **Low sun on wet gravel** (`BEST_lowsun_fp_wet.png`): a strong wet glare sheet toward the sun.
- **Rain on the Eden meadow** (`BEST_overcast_heavy_rain.png`, v7): wet, darker grass, no puddles on grass.
- **Dry weather**: near nothing at full frame (`noon_fields`: 1.9% of pixels change by more than 8 levels; gunner optics unchanged). Glass reflects some sky; ground near the camera gets a little relief.
- **Failures fixed on the way:** v6 turned the noon jeep blue-grey (rough paint added a sky reflection); r6 put flat grey puddle sheets over olive grass (the colour test missed it); v7 and v8 put white sheets over the whole wet square at night (lamps reflected through the ground's own roughness; fixed in v9 with the separate water film).

## Cost

Median +0.37 ms per frame over 33 paired shots (interquartile range -0.27 to +1.75 ms), on the shared GPU at 1440x960 internal. Reflections add +0.15 to +0.41 ms in rain and about 0 when dry. The critic says it fits an RTX 3060 at 1080p.

## Verdict

Critic: wet weather is "a clear, player-visible step toward the RTX vision", and the spawned-vehicle bug is real and log-proven. Dry material guessing is almost invisible. The wet path has a blue cast, stripes and hard-edged puddles.

## What production needs

- Material classes from a config table of name patterns that mods (such as @LoBo) can extend, baked at texture load, with per-model overrides for the misses (rifles land in fabric).
- Wetness as world state: things get wet in rain and dry over minutes; puddles fill and drain slower than surfaces.
- One shading path (fold `shadeMaterials` into `shadeRemastered`).
- Reflections at half resolution with temporal smoothing, shared with the reflections track (`ssr.md`).
- Puddle masks with soft borders that do not follow the terrain triangles; a per-texture terrain class table instead of colour tests.
- Check that wet paint never goes bluer or colder than dry.
- A motion capture to check shimmer.

## Overclaims the critic found

- The low-sun crops cited for "ghostly to solid" hulls show no visible change from the vehicle fix (box differences under 0.5). The fix shows only on the sunlit noon jeep (70.8 to 49.7).
- No soldier reflection is visible in the puddles. At Abel the puddles read more as dark wet stains than as mirrors.
- The wet M113 has strong regular vertical stripes and turns grey, more than "slightly corrugated".
- Wet paint at noon turns blue (blue above red), more than "picks up grey sky".
- Not reported: a black sliver at the base of a bush (v7 Eden), hard triangular puddle edges along the terrain grid, thin white dashes on far wet ground that will likely shimmer.
- "Dry night looks like current Remastered": the change is small but real (8.4 mean difference against a 4.7 noise floor).

## Taste risks

Blue cast on wet paint (the rejected blue direction). Wet low-sun hulls colder than the original look. The spawned-vehicle fix makes the noon jeep side about 30% darker. The wet film also lands on the grass-textured town square, which then looks like wet asphalt.

## Where the work lives

- Notes and patch: `D:\Arma_CWA_worktrees\lighting\tmp\research\materials\NOTES.md`, `prototype.patch` (36 KB), final shaders `override_v9\`, every texture's class in `classification_log_r6.txt`, timings in `timings.md`.
- Scratch worktree: `D:\Arma_CWA_worktrees\gfx-materials`, build `arma_CWR\dist\gfx-materials-rwdi`.
- Best images (not committed): `D:\Arma_CWA_worktrees\lighting\tmp\research\materials\BEST_town_night_rain.png`, `BEST_crop_town_night_wet.png`, `BEST_lowsun_fp_wet.png`, `BEST_overcast_heavy_rain.png`, `BEST_crop_noon_jeep.png`, `FAIL_v8_night_white_sheets.png`.
- Integration: the wet path is a candidate for the new branch `graphics-remaster` the owner asked for on 2026-10-17. That branch did not exist yet when this page was written. Note that the night is now darker (bug fix 4515770), so the lamp streaks were never judged in a real night.
