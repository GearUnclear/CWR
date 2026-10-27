# Lit clouds and matching cloud shadows (track: clouds)

Rank 12 of 22. Critic scores: worth building **5/10**, RTX-on **3/10**. Area: sky and air.

## What the player sees

The painted sky clouds respond to the sun. At dawn and dusk their sun-facing edges glow peach and their bodies go slate. Cloud shadows on the ground match the clouds in the sky and move with them.

## Findings about the existing sky

- **The sky never reached the new shaders.** The sky dome, stars, sun, moon and cloud sheets are drawn before the world pass begins, through the original-look programs, whatever the Lighting setting. The first build only set a uniform and changed nothing.
- **Two cloud systems:** the sky dome (`data3D\obloha.p3d`) with the weather sky texture, which dominates the visible sky, and four huge flat cloud billboards (`mrak1-4.p3d`, 6.7 to 10.8 km across) on a 6 km grid.
- **Why overcast 0.5 shows almost no cloud shadows:** cover 0.3, and the noise field's blobs are 1 to 3 km across, so from eye level the whole valley usually sits in one lit gap. The field also has no relation to the clouds drawn in the sky.
- **A "clear" overcast 0 is not cloud-free** (cover 0.041, the bug later fixed in 5cf85cf; see `bugs.md`).

## The technique

C++ scratch build, behind `triSetLighting "skyClouds=0|1"` and `"cloudShadowSource=0|1"`, plus research knobs `r0` to `r7`.

1. **New-lighting programs for the sky:** `Engine::SetRemasterSkyMode` forces the new program set around the dome, the cloud sheets and the sun disk.
2. **A cloud-density map from the weather sky texture** (CPU, 256x256, rebuilt on weather change): whiteness above the clearest sky of the same row is cloud; a row that is grey all the way round is an overcast deck. No new art.
3. **Lit clouds:** a 6-step march toward the sun in texture space. "Relief" is the density here minus the mean density toward the sun: positive on sun-facing edges (lit, warm at dusk), negative on the far side. The sky keeps its average brightness. A silver lining near the sun.
4. **Sky-matched cloud shadows:** a lit point is projected along the sun to a cloud deck at 1800 m, then mapped to the dome's texture. The cloud you see is the one whose shadow you stand in. One texture read per pixel.
5. **Sky drift:** the dome texture turns about the zenith; clouds, their light and their shadows move together.

## What real data showed

Five runs. Run 2 had a glare band at the horizon, blocky contours and a dimmed sun under a clear sky; run 4 had seams along the dome's triangles and palette banding. Run 5 is clean.

- **Coast at sun 8 degrees, overcast 0.5** (`BEST_coast_oc_up.png`): the sun-facing undersides of the cloud bands glow peach and gold, the tops go slate blue. Clouds read as lit volumes. The strongest result.
- **Sunrise, overcast 0.5** (`BEST_sunrise_oc.png`): cloud bases catch pink sunrise light.
- **Midday, looking up** (`BEST_broken_up.png`): layered and sculpted, but some edges read glossy and painterly.
- **Cloud shadows** (`BEST_broken_high.png`, `BEST_overcast_broken.png`): soft bands across the hills that belong to the clouds in the sky. In the matched eye-level overcast frame the camera ends up under a cloud, so the foreground is shaded (ground brightness 0.341 to 0.247).
- **Drift** (`SEQ_broken_high_drift.gif`): the shadow bands sweep across the slope.
- **Clear sky:** unchanged.
- **Failed:** stronger relief at midday (variant B): glossy, painterly.

## Cost

+0.2 to +0.9 ms in back-to-back pairs (6 density reads on sky pixels, 1 per lit pixel). Some single shots read +5.5 ms; the sunrise view's world time jumped from 5 to about 40 ms with the prototype off too (draw calls climbing from 1,989 to 11,172 within the session).

## Verdict

Critic: "Its lit clouds give a clear win at golden hour: the flat painted sky becomes salmon-lit cloud bands." Midday turns embossed and iridescent, overcast barely changes, and in stills the sky-matched shadows mostly show up as different placement or a darker frame. It is an emboss on a painted panorama, not a volumetric light model.

## What production needs

- A proper sky pass with the new programs instead of forcing the program set inside the screen pass.
- The density-map builder as a pure-math unit with a test, built off the render thread on weather change.
- Lit clouds only at low sun: scale relief and silver lining down hard as the sun rises.
- Drive the drift from the existing wind.
- **Anchor the shadows in the world.** The dome is centred on the camera, so the shadow pattern moves with the camera. Aircraft and fast vehicles would expose it.
- Shadows from the cloud billboards (scaffolding exists in the patch, unused).
- Sits on the clear-sky fix (5cf85cf) and ideally on the physical sky (`atmosphere.md`).

## Overclaims the critic found

- `BEST_overcast_heavy.png` is a view looking up at the deck, not the matched ground view; the matched view shows almost no change.
- Not mentioned: the sun glitter on the sea changes a lot in the coast view (a broad bright sun column instead of a dim streak), because the sun is no longer covered by the noise field.
- Some single shots read +5.5 ms; the interleaved pairs are the better evidence.
- Sky mean at midday is slightly darker (0.583 to 0.567), not unchanged.

## Taste risks

In a still under a cloud, the frame just looks darker (the jeep loses its sun shadow). Midday rims have pastel green, yellow and pink fringes, like an oil film, and white wisps turn grey-blue. Shadow shapes are long bands from the painted streaks, not cumulus patches.

## Where the work lives

- Notes and patch: `D:\Arma_CWA_worktrees\lighting\tmp\research\clouds\NOTES.md`, `prototype.patch` (851 lines over 12 files), shaders `glsl_final\`, `luminance_check.txt`.
- Scratch worktree: `D:\Arma_CWA_worktrees\gfx-clouds`, build `dist\gfx-clouds-rwdi`.
- Best images (not committed): `D:\Arma_CWA_worktrees\lighting\tmp\research\clouds\BEST_sunrise_oc.png`, `BEST_coast_oc_up.png`, `BEST_broken_up.png`, `BEST_overcast_broken.png`, `SEQ_broken_high_drift.gif`, `cmp_clear_sanity.png`. The `assets\` folder holds extracted game textures; never commit it.
- Integration: the low-sun lit clouds are a candidate for the new branch `graphics-remaster` the owner asked for on 2026-10-17. That branch did not exist yet when this page was written.
