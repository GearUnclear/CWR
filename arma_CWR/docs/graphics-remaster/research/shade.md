# Low-sun shade fixes (track: shade)

Rank 2 of 22. Critic scores: worth building **7/10**, RTX-on **2/10**. Area: light and shadow.

## What the player sees

Vehicles spawned by script finally get the new lighting. The grey veil over nearby vehicles against a low sun is gone. A clear sky no longer throws invisible cloud shadows. Evening light is warmer, and the shaded sides of vehicles are lighter and readable.

## The main finding

Current Remastered looked no better than the original look on real data mostly because of **three bugs**, not because of the light model:

1. **Script-spawned vehicles skip the new lighting.** Vertex buffers are built only for shapes loaded at mission start. A shape first loaded later (any `createVehicle` of a class the mission did not place) is drawn on the CPU path with the original look's per-vertex light, no per-pixel light and no shadows. Guerrilla Mode spawns nearly every vehicle this way. Evidence: `cmp_rootcause_prelit.png` (a debug view paints pre-lit draws magenta; the Jeep and the M113 are magenta).
2. **The sun-shaft composite veils near objects grey.** The shaft term is added at full weight whatever the depth. In `lowsun_fp` the Jeep side goes from 27 to 68 (of 255) with shafts on.
3. **A clear sky casts cloud shadows.** `setOvercast 0` still gives 4% cloud cover inside the shadow model, so about 4% of the map loses 80% of its sun with no cloud in the sky. When the camera sits under one patch, the whole view is dim (`forest_edge_noon`: 74% of the frame, frame brightness -14.5%).

All three were later fixed on the branch as separate commits (see `bugs.md`). The fix that landed for bug 1 is a reworked version (a deferred queue, not this track's lazy buffer at first draw), and the fixes for bugs 2 and 3 differ in detail from this prototype.

## The technique (the calibration on top)

Measured with a trick: a debug view returns the surface's own texture colour (albedo), so each box gets the fraction of texture brightness on screen and the colour of the light on it.

- **Evening hue earlier:** the original look starts warming the sun from 25 degrees; current Remastered only below 16. The prototype blends in more of the original's sunset hue from 25 degrees down.
- **Fill on vertical faces at low sun:** the anti-sun face of a hull got too little sky. A larger horizon fill at low sun only.
- **Crown term:** tree crowns at golden hour were too pale. A sun path through the crown and less sky toward the core, at low sun only.
- **Depth-weighted shafts:** the shaft term times `1 - exp(-depth / 300 m)`.
- **Clear-sky floor:** cloud shadows only above 5% cover.
- **Lazy vertex buffer** for spawned shapes, built on first draw.

## What real data showed

Hull shade against lit (same paint, shaded face over lit face):

| View, sun height | Original look | Current Remastered | Prototype |
|---|---|---|---|
| lowsun_box 12.7 deg, M113 | 0.40 | 0.46 | 0.62 |
| sinai_box 12.2 deg, Zelda | 0.49 | 0.49 | 0.61 |
| dawn_box 7.5 deg, M113 | 0.32 | 0.38 | 0.60 |
| dawn_box5 4.8 deg, M113 | 0.25 | 0.31 | 0.58 |

The owner's target for shade is about 0.6; 0.42 was called "crushed".

- In `BEST_lowsun_fp.png` and `BEST_sinai_lowsun_fp.png`, current Remastered hulls are grey shapes in a haze. Prototype hulls are solid, painted, detailed.
- Crowns at golden hour go from brightness fraction 0.78 (pale) to 0.67 (original look 0.57), and read greener. Their internal contrast does not change.
- Noon frames are unchanged in brightness, except spawned vehicle sides (see risks).
- A knob-only grid could not reach shade 0.6 without flattening ground shadows. The best knob-only set was `bounce=2.5,warmth=1.5` (0.54 / 0.50).

## Cost

About zero. The rig terms are computed once per frame. The crown term adds about 20 instructions per foliage pixel; the depth weight adds one texture read per pixel. Not cleanly measured (the shared GPU swung up to 4 times between rounds).

## Verdict

Critic: real, verified bug fixes plus a calibrated low-sun rig. "A clear gain on backlit hulls and a foundation every other track needs, but close to zero movement toward RTX on." The fill is a hand-tuned ambient boost, not bounce light.

## What production needs

- The spawned-vehicle fix, with the sky, sun, halo and cloud shapes excluded (without that the sun disc vanished in an early run). This landed as 747ffc8 in a different form.
- The hue, fill and cloud-floor blocks in `LightRig.cpp` without the research gate (`moonlight=1.01`).
- The crown term and the depth-weighted shafts in the shaders.
- New reference values in `test_remaster_lighting.cpp` and in the README calibration table.
- Keep an albedo debug view: it made this calibration possible.

## Overclaims the critic found

- `BEST_forest_edge_noon.png` and `BEST_coastline_sunset.png` compare runs with different cloud positions, so they oversell the change. A same-run check gives +17% on the forest view but only +0.5% on the coastline.
- "Shipped hull pixels equal the original look exactly" holds on Sinai, not on Abel.
- The third panel of `cmp_rootcause_prelit.png` is captioned "hulls now lit per pixel" but is an albedo debug view.
- "Clearly better" is overstated on side-lit views: the change there is about 2 of 255 levels.
- The GPU numbers on the pictures are noise.

## Taste risks

- At noon, spawned vehicle sides drop from 0.97 to 0.60, darker than the original look, because today's pictures lit them with the old light by accident. `bounce=2.5` is the lever.
- Golden-hour crowns get darker than today, with no gain in volume.
- The sunrise foreground loses its sun-shaft streak.
- Shaded hull sides are lifted by a flat fill, close to the rejected "Lambert-only" look.

## Where the work lives

- Notes and patch: `tmp\research\shade\NOTES.md`, `prototype.patch` (lazy buffer plus the C++ rig, 3 engine files, 76 lines), shader half in `ovr_final\`.
- Scratch worktree: `gfx-shade`, build `dist\gfx-shade-rwdi`.
- Best images (not committed): `tmp\research\shade\BEST_sinai_lowsun_fp.png`, `BEST_lowsun_fp.png`, `BEST_dawn_box5.png`, `BEST_crowns_golden_crop.png`, `cmp_rootcause_prelit.png`.
- Integration: the three bugs are already fixed on `claude/youthful-lovelace-8imbdh` (747ffc8, 463506f, 5cf85cf). The calibration (hue, fill, crown term) is a candidate for the new branch `graphics-remaster` the owner asked for on 2026-10-17. That branch did not exist yet when this page was written.
