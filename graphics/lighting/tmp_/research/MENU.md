# Graphics research menu (2026-10-17)

This is the menu from the graphics research run for Uslu dur!. 22 research tracks each built a working prototype on the real Remaster data and took before and after pictures of the same spot and time. A second agent, the critic, then checked every claim against the files on disk and scored each track. A separate water study is listed at the end of the menu; no critic has reviewed it.

The goal was the owner's "RTX on" look: light that looks path traced, from the existing models and textures.

All paths below are relative to `D:\Arma_CWA_worktrees\lighting\tmp\research\`. The gallery folder `gallery\` holds one picture per track, named by rank, and `gallery\SHEET.png` shows them all on one sheet. In the pictures, panels labelled "Classic" show the original look.

## Read this first

- **No single track makes it look like a different game on its own.** The two closest are combat light at night (RTX-on 6 out of 10) and reflections on water and wet streets (5 out of 10).
- **Bounce light did not work anywhere.** Bounce light, where light reflects off one surface onto another and carries its colour, is the core of the RTX look. Four tracks tried it. All four made almost no visible change, because the game's existing hand-tuned fill light already covers that range. One bold experiment was never tried (see section 4, phase 4).
- **Four bugs make every current Remastered picture wrong.** (1) Vehicles spawned by script skip the new lighting and are drawn with the original look's lighting. (2) Trees, bushes and buildings cast no sun shadows at all. (3) A clear sky still puts cloud shadows on about 4% of the map. (4) The Remastered default ships with no anti-aliasing. Fix these first, then shoot the before pictures again.
- **Every GPU cost here is rough.** All tests ran on one RTX 4060 Ti shared with up to four other test games, at 1440x960 internal resolution. Nothing was measured at 1080p or on an RTX 3060. A clean timing run is still owed. The millisecond numbers printed on the comparison pictures are single noisy readings; ignore them.
- **Nothing was fabricated.** The critics found every cited file and confirmed every prototype really loaded in the game. They did find overclaims, listed in section 3.

## Words used below

- **Current Remastered**: the Remastered lighting as it ships on this branch today.
- **The original look**: the stock CWA lighting, used only as the before picture.
- **Worth building**: the critic's overall score out of 10. **RTX-on**: the critic's score out of 10 for how far the result moves toward the path-traced look.
- **Shader-only**: only the GPU programs change, no engine code. **C++**: engine code changes too.
- **Screen space**: the effect only uses what is already on screen, so it cannot see things off screen or hidden behind other things.
- **G-buffer**: extra per-pixel pictures (surface direction, colour, material) that the renderer saves so later effects can reuse them.
- **TAA** (temporal anti-aliasing): each frame is blended with the last few. This smooths edges and also cleans up cheap, noisy effects.
- **Half resolution**: an effect computed at half the screen size and scaled up, to save cost.
- **Effort**: S = shader-only or a small engine change, days. M = a few hundred lines of engine code, one to two weeks. L = new rendering passes across many engine files, several weeks.

## 1. Ranked menu

Ranked by the critic's "worth building" score. Ties are broken by the RTX-on score, then by how visible the gain is.

| # | Track | What the player sees | Area | Worth building | RTX-on | GPU cost (measured, shared GPU) |
|---|---|---|---|---|---|---|
| 1 | combat | Fires, flares and shell bursts light the men and cast long shadows | night and combat | 7 | 6 | too small to measure; est. 1-3 ms in fights |
| 2 | shade | Spawned vehicles finally get the new lighting; no grey veil; no phantom cloud shadows | light and shadow | 7 | 2 | about 0 |
| 3 | perf | Nothing visible; long views get much cheaper | budget | 7 | 2 | saves 7-17 ms CPU on long views |
| 4 | ssr | Calm water and wet streets reflect boats, hills and houses | water and shine | 6 | 5 | +0.35-1.9 ms |
| 5 | volumetric | Sun shafts through pine woods; headlight cones in night mist | sky and air | 6 | 4 | +0.4-2.4 ms |
| 6 | materials | Wet weather: puddles, wet cloth and stone, lamp streaks on wet ground | water and shine | 6 | 4 | +0.4 ms median |
| 7 | atmosphere | A physically based sky: golden glow at low sun, orange twilight band | sky and air | 6 | 4 | near 0 GPU; CPU hitch to fix |
| 8 | hfrt | Every tree casts a long golden-hour shadow, near and far | light and shadow | 6 | 3 | too high as built: +2.5 to +16 ms |
| 9 | haze | Distant hills fade into the real sky; pink morning mist in valleys | sky and air | 5.5 | 3 | unknown, probably high as built |
| 10 | shadows | Trees and buildings cast sun shadows again | light and shadow | 5 | 3 | +0.2-1.4 ms |
| 11 | night | Real low-beam headlights, stars, moon shadows, warm lamps | night and combat | 5 | 3 | +0.8-3.3 ms |
| 12 | clouds | Sky clouds glow on their sun side at dawn and dusk | sky and air | 5 | 3 | +0.2-0.9 ms |
| 13 | texsharp | Grass and rock grain on slopes instead of an airbrushed smear | surface detail | 5 | 2 | +0.5-2 ms |
| 14 | temporal | TAA: smoother edges, cheap noisy effects become clean | infrastructure | 5 | 2 | +0.07-0.14 ms |
| 15 | gbuffer | Nothing visible yet; groundwork for reflections and bounce light | infrastructure | 5 | 1.5 | est. +3-5 ms with its bounce light |
| 16 | foliage | Leaves glow when backlit; greener shade inside crowns | light and shadow | 4.5 | 3 | +0-1 ms |
| 17 | tiling | Fewer repeating ground patterns; desert gets real variation | surface detail | 4.5 | 2 | +0.5-2.5 ms |
| 18 | normals | Pebble relief on gravel at low sun | surface detail | 4 | 2 | +1-2 ms |
| 19 | aa | Anti-aliasing switched on; less shimmer on foliage | image quality | 4 | 1 | about 0 at native resolution |
| 20 | exposure | Dark vehicle sides readable at low sun; orange sun and fire cores | light and shadow | 3.5 | 1.5 | about 1 ms |
| 21 | ssgi | Invisible by day; a hull lit by a nearby headlight pool at night | light and shadow | 3 | 2 | +1.5-3.5 ms |
| 22 | wildcard | Soft dark pools under vehicles on overcast days | light and shadow | 3 | 2 | +1.6-3.9 ms, 160 MiB |
| W | water | Wave relief on the sea and a glittering sun path | water and shine | not reviewed | not reviewed | est. under 0.1 ms |

### 1. Combat light (track: combat)

- **What it does:** muzzle flashes, shell bursts, burning wrecks, campfires and flares become real light sources. They light the men and ground nearby and cast long shadows that are sharp at the feet and soft at the tips. Smoke is lit from inside.
- **Visible gain:** vs current Remastered large; vs the original look large. Night only. By day it is nearly invisible, as intended.
- **GPU cost:** below what the shared machine could measure (best run: -0.3 ms, noise of several ms). Estimated 1 to 3 ms at 1080p on a 3060, only while combat lights are near the camera.
- **Effort:** C++, L. The prototype is a 76 KB patch over 22 files (a new CombatLights module and hooks in weapons, explosions, wrecks, fires and flares). Production must move the shadow into its own half-resolution screen pass with temporal smoothing, and allow 2 to 4 shadowed lights instead of one.
- **Depends on:** nothing to work. TAA (14) removes the speckle. Shares a light list with the night lights (11).
- **Risks vs your taste:** the flash frame at eye level is darker than the original look's full-screen whiteout (mean 37 against 63), so the climax reads dimmer. Ground under fire light turns beige or khaki. A warm glow strip runs along the horizon behind fires. With two fires, the shadow jumps from one to the other between frames. Speckle shows in soft shadow edges. A seam of 3 to 4 brightness levels runs down the exact screen centre (a bug).
- **Best image:** `combat\BEST_ffh.png`
- **Critic's verdict:** A real, bold night-combat win: fires, flares and shell flashes now cast long contact-hardening shadows and light the men, so night battles look like a different game. The prototype has stipple, shadows popping between two fires, a centre-column seam it did not disclose, and a flash frame darker than the original look's whiteout.

### 2. Low-sun shade fixes (track: shade)

- **What it does:** three bug fixes plus a retune. (a) Vehicles spawned by script now get the new per-pixel lighting and receive shadows. Today they are drawn with the original look's lighting. (b) The sun-shaft effect no longer lays a grey veil over nearby vehicles. (c) A clear sky no longer casts cloud shadows over about 4% of the map. The retune gives warmer evening light and lighter shaded vehicle sides.
- **Visible gain:** vs current Remastered clear; vs the original look clear. Strongest on vehicles against a low sun: grey hazy cut-outs become solid painted vehicles. Side-lit views barely change.
- **GPU cost:** about zero (the new values are computed once per frame). Not cleanly measured.
- **Effort:** C++, M. The prototype touches 3 engine files (76 lines) plus two shaders. The lighting unit tests need new reference values.
- **Depends on:** nothing. Almost every other track needs the spawned-vehicle fix: Guerrilla Mode creates nearly every vehicle by script, so without it no lighting work reaches them. Four tracks found this bug independently (shade, gbuffer, materials, normals). Use this track's version: it skips the sky, sun and cloud shapes. The gbuffer track's version lit the whole night town like day.
- **Risks vs your taste:** at noon, spawned vehicle sides get darker than today (0.97 to 0.60), and darker than the original look, because today's pictures lit them with the old lighting by accident. Golden-hour tree crowns get darker than today. The sunrise foreground loses its sun-shaft streak. Shaded vehicle sides are lifted by a flat fill, not real bounce light.
- **Best image:** `shade\BEST_sinai_lowsun_fp.png`
- **Critic's verdict:** Real, verified bug fixes (spawned vehicles skipped the remastered lighting; the sun-shaft composite left a grey veil on near objects; cloud shadows under a clear sky) plus a calibrated low-sun rig. A clear gain on backlit hulls and a foundation every other track needs, but close to zero movement toward RTX on.

### 3. Frame cost (track: perf)

- **What it does:** makes frames cheaper without changing a pixel. Objects that look alike are drawn in groups, objects hidden behind hills are skipped, and a stock bug is fixed where grouped bushes took the wrong terrain fit. On the sunrise ridge view: 11,172 draw calls down to 1,728, and CPU frame time 32 to 12 ms.
- **Visible gain:** none, by design.
- **GPU cost:** frees time. About 7 to 17 ms of CPU saved on long view-distance views, 1 to 2 ms in forests. The key finding: the Remastered shading itself is cheap (0 to 1 ms in the world pass, 0.3 to 1.8 ms in post). The bottleneck is the number of draw calls. So there is GPU room for heavy effects once draw calls are fixed.
- **Effort:** C++, M. 31 KB patch, mostly in SceneDraw.cpp.
- **Depends on:** nothing.
- **Risks vs your taste:** none to the look. Engineering risks: the hill-hiding test is not proven safe in every case, shadows from hidden objects were never tested, and rebuilding its table costs 2.3 to 5.7 ms of CPU per camera move until it is made cheaper. On the noon fields view the full set was slower than the simpler grouping fix alone.
- **Best image:** `perf\BEST_sunrise_ridge.png`
- **Critic's verdict:** Real, verified and pixel-exact cost work. It cuts sunrise_ridge from 11,172 to 1,728 draws and its CPU frame from 32 to 12 ms, and fixes a genuine instancing bug. It adds nothing a player would see; its value is the budget it frees for the RTX-style effects.

### 4. Reflections on water and wet ground (track: ssr)

- **What it does:** screen-space reflections. Calm water mirrors boats, rocks and hills, broken up by moving ripples. Rainy streets get puddles that reflect houses, lamp posts and vehicles. Grass stays dry.
- **Visible gain:** vs current Remastered clear; vs the original look clear.
- **GPU cost:** +0.35 to 1.9 ms measured. Perhaps 2.5 to 3 ms worst case at 1080p.
- **Effort:** C++, L. 44 KB patch over 11 files. Production needs a real per-material flag, a half-resolution trace, and fades at the screen edges.
- **Depends on:** nothing to work. Production wants the material classes (6) or the G-buffer (15), TAA (14), and the water shader (W), so the sea reflection feeds the water shader instead of being laid on top.
- **Risks vs your taste:** the open sea at sunset turns pale lilac-grey and loses some of the warm sun column (a wash). Lamp-lit wet asphalt becomes a smooth grey sheet. Seams at the screen edges, and pale boxes under bushes and vehicles. Nothing at night: fires and lamps are not reflected. Paint reflections show nothing.
- **Best image:** `ssr\BEST_bay_offshore.png`
- **Critic's verdict:** Real and visible on calm water and wet daytime streets: the boat, the hill and the houses are mirrored. The sunset sea washes out, paint is null, nights get nothing, and there are more edge and box artefacts than reported.

### 5. Visible light in the air (track: volumetric)

- **What it does:** sun shafts slant through pine crowns at dawn and dusk, and headlight cones glow in night mist. The beams are blocked by the real trees through the shadow map.
- **Visible gain:** vs current Remastered clear; vs the original look clear.
- **GPU cost:** +0.4 to 2.4 ms, about 1 ms typical. Plus the shadow pass, which becomes real work once tree shadows are on (0.1 to 1.9 ms).
- **Effort:** C++, L. 44 KB patch over 4 files. Production needs a mist density driven by weather and time of day.
- **Depends on:** tree shadows (10); without them there are no beams. TAA (14) halves the cost. Best merged with the haze (9) into one "air" pass.
- **Risks vs your taste:** the mist strength that shows beams also lays a pale veil over backlit tree crowns and a glowing band along the horizon. The clear-weather setting removes the veil, but makes backlit vehicles 15 to 19% darker than today, because it also removes the old shaft glow. Blocky glow patches remain beside trees. Night lamp halos tint the whole sky brown (rejected).
- **Best image:** `volumetric\BEST_pinewood_morn.png`
- **Critic's verdict:** This is a real, working ray march occluded by the shadow map. It gives clearly visible god rays in woods at dawn and dusk and striking headlight cones at night. But the settings that show beams also veil the crowns, the clear-weather fix darkens backlit hulls, and blocky glow patches remain.

### 6. Wet weather and materials (track: materials)

- **What it does:** in rain, cloth and stone darken like wet surfaces, puddles form in flat hollows with rain ripples and reflections, and every street lamp and fire draws a long reflection streak across the wet ground. It also guesses a material for each texture (metal, glass, cloth, and so on) from the texture's file name.
- **Visible gain:** vs current Remastered clear; vs the original look clear. Wet scenes only. The dry look barely changes.
- **GPU cost:** +0.37 ms median; reflections add 0.15 to 0.41 ms in rain.
- **Effort:** C++, L. 36 KB patch. Production needs wetness as world state (things get wet in rain and dry afterwards), a config table of material names that mods can extend, and better puddle masks.
- **Depends on:** the spawned-vehicle fix (2), the reflection pass shared with item 4, and the night lights (11) for lamp streaks.
- **Risks vs your taste:** wet paint turns bluish at noon. The wet M113 gets strong vertical stripes and goes grey. Puddle edges are hard triangles that follow the terrain grid. The wet film also lands on a grass-textured town square. Far wet ground shows thin white dashes that will likely shimmer in motion.
- **Best image:** `materials\BEST_town_night_rain.png`
- **Critic's verdict:** Wet weather (lamp streaks across a rainy square, SSR puddles, low-sun wet glare) is a clear, player-visible step toward the RTX vision, and vbfix is a real, log-proven bug. Dry material reconstruction is almost invisible, and the wet path has blue-cast, corrugation and hard-edged puddle artefacts.

### 7. Physically based sky (track: atmosphere)

- **What it does:** computes the sky's colour from real air physics. A golden glow surrounds a low sun, twilight gets an orange band under a violet sky, and the flat band at the horizon now matches the sky. The original cloud textures are kept.
- **Visible gain:** vs current Remastered clear; vs the original look clear. Sky only: ground, vehicles and water do not change.
- **GPU cost:** near zero (under 0.3 ms estimated). CPU: an 8 to 12 ms table rebuild every few seconds, which would hitch. It must move to the GPU or a worker thread.
- **Effort:** C++, L. About 930 lines over 10 files.
- **Depends on:** nothing. It is the base for the haze (9), water reflections, the clouds (12) and later a physical sky light.
- **Risks vs your taste:** a muddy olive band in the sunset sky away from the sun. Noon cirrus streaks mostly erased. The fully physical colour cools dawn and dusk compared with the original pinks: use the half-way colour setting and keep saturation at 1.15 or less. The claimed water reflection is not visible, so a mauve sea sits under a gold sky. Using the sky to light the ground turned the ground blue (rejected).
- **Best image:** `atmosphere\BEST_lowsun_fp.png`
- **Critic's verdict:** This is a real, working physically based sky that clearly improves low-sun and twilight skies (aureole, golden horizon, orange civil-twilight band) at near-zero GPU cost. It is sky-only: ground, hulls and water do not change. It also has an olive horizon band, loses some noon cirrus and cools the frame with the physical colour, so ship it with the half-way colour and saturation of at most 1.15.

### 8. Long tree shadows at any distance (track: hfrt)

- **What it does:** every tree, bush and building on the island is drawn into a height map with 3 to 6 m cells. At low sun, each pixel looks toward the sun through that map, so every tree casts a long, soft shadow at any distance. The stock shadow maps never draw those.
- **Visible gain:** vs current Remastered clear; vs the original look clear. Low sun only.
- **GPU cost:** too high as built: +2.5 ms on the forest edge, +8 to 10 ms among big trees, +13 to 16 ms in a telephoto view over a wooded slope. A cheaper reduced-resolution version was planned but not measured.
- **Effort:** C++, L. 33 KB patch.
- **Depends on:** tree shadows (10) for the trees near the camera. TAA (14) to replace the fixed dither pattern.
- **Risks vs your taste:** it only removes light, with no bounce. Shade on pink rock turns cool blue-grey. Near shadows are soft slabs, not tree shapes. The fixed dither will crawl in motion.
- **Best image:** `hfrt\BEST_hill_1730.png`
- **Critic's verdict:** A real, verified find: a 3-6 m canopy heightfield cone-traced toward the sun gives every Abel tree a long, soft golden-hour shadow near and far, which the shipped shadow maps never draw. It is limited to low sun, it only darkens, it is cool in the shade, and as a forward per-pixel pass it costs too much (+13-16 ms in the telephoto view).

### 9. Distance haze and morning mist (track: haze)

- **What it does:** distant hills fade into the real sky behind them instead of one flat fog colour. At sunrise, pink mist pools on lakes and valley floors and burns off by 08:00.
- **Visible gain:** vs current Remastered clear; vs the original look clear. Mostly sunrise and early-morning valleys. Noon is unchanged.
- **GPU cost:** not known. The best single reading was +9 ms; it could be much more as built. A deferred half-resolution pass should land near 1 to 2 ms (critic's estimate).
- **Effort:** C++, M (93 lines of engine code plus shaders), more with a weather drive.
- **Depends on:** the atmosphere (7) for the sky colour, TAA (14) to remove a crosshatch pattern. Merge with item 5. It also needs a tie-in to AI sight: today the mist is visual only, so AI would see through a mist bank that blinds the player.
- **Risks vs your taste:** far hills turn bluer. At 06:00 to 06:30 the mid field gets a cool lift and about 9% less contrast (a wash). Mist hides the sun glint on lakes. A fixed crosshatch pattern in thick mist. A hard line where the horizon band meets the mist.
- **Best image:** `haze\BEST_valley_sunrise_mist.png`
- **Critic's verdict:** The real, verified sky-matched aerial perspective and terrain-pooled morning mist give one strong sunrise image and small gains elsewhere, at an unmeasured and probably large per-fragment cost, and they lean toward blue lift and wash in the 06:00 to 06:30 frames.

### 10. Tree and building shadows (track: shadows)

- **What it does:** turns tree, bush and building shadows back on. They were off in every capture because of a start-up order bug: the graphics settings are applied before the world exists. Forest floors get dappled sun and slopes get long golden-hour tree shadows. Also tested: soft sun shadows (sharp where an object touches the ground, softer farther away) and short contact shadows.
- **Visible gain:** vs current Remastered clear; vs the original look small.
- **GPU cost:** the shadow pass goes from 0.03 ms to 0.2 to 1.4 ms. Soft shadows 0 to 3 ms, unresolved.
- **Effort:** the start-up fix is C++, S. Soft shadows are M.
- **Depends on:** nothing. Needed by the beams (5) and the near trees of item 8. Woods need a light fill under the crowns (leaf transmission or bounce), or they go dark.
- **Risks vs your taste:** woods get 14 to 21% darker than the original look, evenings in the woods get cooler, and shade is grey. Town shade turns flat and dark. A tighter shadow setup put a wrong large shadow over the Sinai foreground. Crown self-shadows flicker. Every run of this track logged a GL warning about a shadow sampler bound to no texture.
- **Best image:** `shadows\BEST_woods_mixed_noon.png`
- **Critic's verdict:** Switching on the tree and building shadow casters (a boot-order bug fix) is the one change a player would see: dappled forest floors and long golden-hour tree shadows. PCSS and contact shadows are close to invisible, the woods get 14-21% darker and greyer than the original look, and the 4096/tight-tier setup has an unflagged near-tier shadow artefact on Sinai.

### 11. Physical night (track: night)

- **What it does:** the moon becomes the main light, with shadows. Street lamps are warm and fall off with distance. Headlights are real low beams with a hot spot and a sharp cut-off, aimed at the road ahead. Stars show, and dim areas lose colour the way the eye does at night.
- **Visible gain:** vs current Remastered clear; vs the original look clear. The big win is driving on a moonless road: today the landscape there is a bright grey, and the prototype shows a dark starry night with a warm beam on the road.
- **GPU cost:** +0.8 to 3.3 ms, perhaps 1.2 to 5 ms at 1080p. It needs light culling by screen tile to bring that down.
- **Effort:** C++, L. 769 lines over 9 files.
- **Depends on:** a shared light list with combat (1), the spawned-vehicle fix (2), and shadows for lamps (not built).
- **Risks vs your taste:** headlit roadside grass turns yellow-olive (mustard). Town views mostly just halve in brightness, and tree crowns next to lamps turn black. A jeep's rear is floodlit at 5 times the original look while it faces away from every light (a leak). Hill roads stay weak. Moonless country may be too dark to play. Lamp light passes through walls.
- **Best image:** `night\BEST_road_flat_drive_moonless.png`
- **Critic's verdict:** A real, verified physical night rig. The moonless low-beam driving view is a large, player-visible win, but the town views are mostly just darker, with black tree crowns, mustard verges and a jeep light leak, and the RTX-style pieces (bounce, volumetrics, local shadows) failed or are missing.

### 12. Lit clouds and matching cloud shadows (track: clouds)

- **What it does:** the painted sky clouds respond to the sun. At dawn and dusk their sun-facing edges glow peach and their bodies go slate. Cloud shadows on the ground now match the clouds you see, and they move with them.
- **Visible gain:** vs current Remastered clear; vs the original look clear. Golden hour only.
- **GPU cost:** +0.2 to 0.9 ms.
- **Effort:** C++, M. 851 lines over 12 files.
- **Depends on:** interacts with the clear-sky cloud fix (2). Ideally sits on the atmosphere (7).
- **Risks vs your taste:** at midday the clouds look embossed, with rainbow fringes on bright edges. In a still taken under a cloud, the frame just looks darker. The shadow pattern follows the camera, which aircraft would expose. Overcast barely changes.
- **Best image:** `clouds\BEST_sunrise_oc.png`
- **Critic's verdict:** The prototype is real and verified. Its lit clouds give a clear win at golden hour: the flat painted sky becomes salmon-lit cloud bands. Midday turns embossed and iridescent, overcast barely changes, and in stills the sky-matched shadows mostly show up as different placement or a darker frame.

### 13. Terrain detail (track: texsharp)

- **What it does:** each terrain square's own texture is re-sampled zoomed in, so slopes at 10 to 150 m show grass and rock grain instead of an airbrushed smear. Optional mild sharpening.
- **Visible gain:** vs current Remastered clear; vs the original look clear. On Abel and Eden slopes. None on Sinai sand.
- **GPU cost:** +0.5 to 2 ms.
- **Effort:** shader-only, S.
- **Depends on:** nothing. Needs a check at native 1080p and in motion.
- **Risks vs your taste:** a golden slope reads more like dry straw (khaki). Olive mottling on meadows. Dark blotches on paths from the detail boost. The sharpening touches the whole frame. Useful finding: texture filtering is already at its maximum and texture loading is not the cause of the blur, so no time should go there.
- **Best image:** `texsharp\BEST_forest_edge_golden.png`
- **Critic's verdict:** An honest, well-evidenced shader-only terrain micro-detail layer: the slope goes from airbrushed to grassy on Abel and Eden at 10 to 150 m for about 0.5 to 2 ms. It is invisible on Sinai sand and near gravel and adds no light at all, so it is a supporting remaster layer, not an RTX-on step.

### 14. TAA (track: temporal)

- **What it does:** blends each frame with the last few. Edges get smoother, and cheap noisy effects (soft shadows from 4 samples, ambient occlusion) come out clean within 2 to 4 frames.
- **Visible gain:** vs current Remastered small. The critic marked it large vs the original look, but most of that is the Remastered base itself.
- **GPU cost:** +0.07 to 0.14 ms.
- **Effort:** C++, L. Production needs per-object motion (so moving vehicles do not smear) and a mask for particles and muzzle flashes.
- **Depends on:** nothing. It makes items 1, 4, 5, 8, 9 and 21 cheaper and cleaner.
- **Risks vs your taste:** the whole frame gets 6 to 18% softer. Ghosting behind fast vehicles was not tested. The soft-shadow test used a sun 5 times wider than the real one, so its shadows were too soft.
- **Best image:** `temporal\final\forest_edge_golden__strafefx_motion.png`
- **Critic's verdict:** Real, working TAA infrastructure that removes the dotting from 4-tap stochastic PCSS and AO cheaply, but a player would barely see any change on its own: the only visible effect is a softer soldier shadow, exaggerated by the 2.5 deg sun.

### 15. G-buffer (track: gbuffer)

- **What it does:** saves per-pixel surface direction, material, colour and light during rendering, so later effects can reuse them. The bounce light built on top of it was invisible.
- **Visible gain:** vs current Remastered small; vs the original look small.
- **GPU cost:** the bounce light +1.5 to 2.5 ms; the G-buffer itself about 1 to 2 ms (estimate). A packed layout would roughly halve the G-buffer part.
- **Effort:** C++, L. 718 lines over 12 files.
- **Depends on:** nothing. Needed for better reflections (4), SSGI (21), any real bounce light, and a cleaner local exposure (20).
- **Risks vs your taste:** its version of the spawned-vehicle fix lit the whole night town like day; it must not ship. Its bounce light with occlusion made frames slightly darker.
- **Best image:** `gbuffer\BEST_lowsun_shadow.png`
- **Critic's verdict:** The G-buffer plumbing is real and clean, and it found a genuine stock bug: script-spawned shapes skip the Remastered shader. The GI consumer built on it is invisible at a glance (RTX-on about 1.5/10), and the late-VBO hack breaks night.

### 16. Tree crowns lit as volumes (track: foliage)

- **What it does:** leaves glow when the sun is behind them, crown cores stay darker, shade inside a crown is green rather than grey, and each tree gets a slightly different tint. A separate canopy map adds forest-scale shading.
- **Visible gain:** vs current Remastered small; vs the original look clear. Obvious only when looking toward the sun through trees.
- **GPU cost:** +0 to 1 ms; the canopy map adds 0.3 to 0.9 ms and 43 MB.
- **Effort:** the crown shader is S; the canopy map is C++, M.
- **Depends on:** nothing. Note: this track reported that the stock shadow maps show nothing when the camera faces a low sun. That is probably the same bug as item 10 (tree shadows off); confirm it there.
- **Risks vs your taste:** the green push recolours the authored foliage (blue-grey leaves turn yellow-green, backlit trees go lime), which is close to new art. The tree in front of the sun turns into one flat green mass. The canopy map mostly makes things darker.
- **Best image:** `foliage\BEST_backlit_hornbeam.png`
- **Critic's verdict:** The crown-volume shader is real, cheap and loaded correctly. Outside the into-the-sun hornbeam shot it mostly makes crowns a bit greener, and the C++ canopy map mostly makes things darker. It is a modest step toward the RTX goal, plus a valuable bug find: shipped shadow maps vanish when the camera faces the sun.

### 17. Terrain anti-tiling (track: tiling)

- **What it does:** hides repeating texture patterns on the ground, adds large lighter and darker patches that follow the terrain's shape, and adds fine relief.
- **Visible gain:** vs current Remastered small; vs the original look clear. A clear win on Sinai seen from height.
- **GPU cost:** +0.5 to 2.5 ms.
- **Effort:** shader-only, S. Production adds two small engine changes (a flag per terrain square and a proper noise texture).
- **Risks vs your taste:** pale fields get paler. The forest edge view gets 8% darker. The dark patches can read as cloud shadows on a clear day. Blending flattens the authored texture contrast. It does nothing for the khaki grass.
- **Best image:** `tiling\BEST_high_sinai_noon.png`
- **Critic's verdict:** The prototype is real and verified: anti-tiling plus a heightfield-tied macro plus relief from albedo gives a clear win on Sinai seen from height and crisper near-ground grit at low sun or on trails. On green islands at eye level the gain is small, with a mild wash on pale fields and darkening on forest_edge. This is texture work, not RTX-style light.

### 18. Surface relief from textures (track: normals)

- **What it does:** derives bumps from the existing textures. On vehicles nothing shows. On ground at low sun, gravel gets pebbles with tiny shadows.
- **Visible gain:** vs current Remastered small; vs the original look clear.
- **GPU cost:** about 1 to 2 ms.
- **Effort:** the useful part is shader-only, S. Drop the engine-side bump maps made at load time: they cost 5 to 6 s of load and 230 ms stalls, and show nothing.
- **Risks vs your taste:** Sinai sand turns into cobbles. Black specks on dirt roads and grass that will likely shimmer. The same grain on every surface reads as an overlay.
- **Best image:** `normals\BEST_lowsun_fp.png`
- **Critic's verdict:** Real, honestly documented, but small. The load-time normal maps are a negative result on CWA's flat-painted vehicles. The useful pieces are a shader-only, low-sun gravel micro-relief that is taste-risky on sand and grass, and a valuable side finding that runtime-spawned vehicles never get Remastered per-pixel shading.

### 19. Anti-aliasing (track: aa)

- **What it does:** the shipped Remastered default has no anti-aliasing at all. Turning on 4x MSAA calms the shimmer on leaves and edges. A better downscale plus mild sharpening helps when rendering above screen resolution.
- **Visible gain:** vs current Remastered small; vs the original look small. More noticeable in motion than in stills.
- **GPU cost:** 4x MSAA at native resolution is about free. Downscale and sharpen: +0.16 ms at 1080p. The cost of a 1.5x supersampling preset was not proven.
- **Effort:** a default setting plus small C++, S.
- **Risks vs your taste:** none to the light. The sharpening also hits HUD text until the drawing order is fixed.
- **Best image:** `aa\figures\SETTINGS_town_night_crop.png`
- **Critic's verdict:** The track is honest and every file checks out. The real win is turning on MSAA, which the shipped Remastered default lacks, plus a cheap Lanczos2+RCAS present pass. The result is less shimmer and crisper detail at crop scale, with no change to light, so it does almost nothing for the RTX-on vision.

### 20. Local exposure (track: exposure)

- **What it does:** brightens shade locally so crushed vehicle sides are readable at low sun. The eye adapts slowly to dark and bright views. A new highlight curve keeps the sun and fire cores orange instead of white.
- **Visible gain:** vs current Remastered small; vs the original look small.
- **GPU cost:** about 1 ms.
- **Effort:** C++, M.
- **Risks vs your taste:** the sunrise foreground washes out to a flat grey-green and loses its warm sun streak. Night only gets darker. The sun becomes a saturated orange ball. Worth keeping: the orange-keeping highlight curve (free inside the existing shaders) and the eye adaptation loop (for the night work).
- **Best image:** `exposure\BEST_lowsun_fp_crop.png`
- **Critic's verdict:** Honest, real tone-mapping work: local exposure makes the crushed Jeep and tent flanks readable at low sun, but it washes the sunrise, only darkens the night, and moves almost nothing toward an RTX-on look.

### 21. Screen-space bounce light (track: ssgi)

- **What it does:** traces bounce light through what is on screen. By day it is invisible. At night, an armoured vehicle next to a headlight pool gets lit from below by that pool.
- **Visible gain:** vs current Remastered small; vs the original look small.
- **GPU cost:** +1.5 to 3.5 ms, perhaps 4 to 6 ms at 1080p. Should only run where local lights are near.
- **Effort:** C++, M.
- **Depends on:** a real colour buffer from the G-buffer (15), TAA (14), a copy of the frame without smoke, and physical lights (11).
- **Risks vs your taste:** the lift is a flat grey. A soldier standing in the same pool stays black. Smoke leaks warm light onto walls. Its occlusion mode only darkens.
- **Best image:** `ssgi\BEST_headlights_bounce_crop.png`
- **Critic's verdict:** This report is honest and fully backed by files. Screen-space GI is invisible by day (you need 8x amplified change maps to see it) and gives one real but narrow night win: a hull beside a headlight pool goes from near-black to a grey-lit, textured face at x2. That is not an RTX-on look.

### 22. Sky visibility from the shadow casters (track: wildcard)

- **What it does:** draws the scene from 24 sky directions to work out how much sky each pixel sees, including things off screen, plus one grey bounce. Vehicles on overcast days sit in soft dark pools instead of floating.
- **Visible gain:** vs current Remastered small; vs the original look small.
- **GPU cost:** +1.6 to 3.9 ms and about 160 MiB of video memory.
- **Effort:** C++, L. 988 lines.
- **Depends on:** tree shadows (10). Would be cheaper evaluated at half resolution with the G-buffer (15) and TAA (14).
- **Risks vs your taste:** woods get darker and greyer. Blotchy streaks on large flat walls. The bounce is a fixed grey, so there is no colour bleeding. Keep the idea for the bounce-light experiment in section 4.
- **Best image:** `wildcard\BEST_overcast_close.png`
- **Critic's verdict:** This is a real, working world-space sky-occlusion system built from the shadow casters: soft contact pools under the Jeep and M113 on overcast close-ups. Elsewhere the frame is barely changed or just a bit darker and greyer, it is not path-traced, and it costs 2 to 4 ms.

### W. Sea waves (track: water, not reviewed by a critic)

- **What it does:** gives the sea wave relief from the sea texture the game already ships. Wave rows run to the horizon, the sunset sun lays a broken, glittering path instead of a blurred column, and wave faces reflect the sky.
- **Why the sea looks smooth today:** the sea is drawn by the terrain detail shader, not by the water shader. Current Remastered shades it as a flat surface with a very broad sheen, which smooths the texture away.
- **Visible gain (the builder's own numbers, not checked by a critic):** fine wave detail at sunset goes from 3.0 today to 23.7 with variant C (the original look: 3.7). At noon from 4.4 to 12.7 (the original look: 6.1).
- **GPU cost:** estimated under 0.1 ms (2 to 7 extra texture reads per sea pixel). The measurement was too noisy to use.
- **Effort:** shader-only for the look, plus small C++ to give the sea its own shader and to blend between the sea's animation frames. S.
- **Depends on:** nothing. Item 4's sea reflection should feed into it.
- **Risks vs your taste:** the far sea at sunset is lighter and greyer than the original look. Noon water is cooler and bluer than the original's yellow-green. The waves may pop each time the sea animation switches frame (every 0.14 s), and rows may crawl. Nobody has watched it in motion.
- **Where:** `water\NOTES.md`, and parked on branch `claude/remaster-water-research` (`arma_CWR/tests/rendering/water_research/README.md`).
- **Best image:** `water\images\triptych_coastline_sunset__C.png`

## 2. Tried and failed

These are honest negatives. Each one saves someone from trying it again.

**Bounce light and sky occlusion (the core RTX ingredient)**

- Bounce light from a G-buffer (gbuffer): invisible at normal size; with its own occlusion the frame got slightly darker.
- Screen-traced bounce light (ssgi): invisible by day, even at a non-physical double-counted upper bound. The game's hand-tuned fill light already supplies most of that light, and CWA ground is dark. Replacing the ambient occlusion made frames darker. In the lamp-lit town it made the wash worse. Around a campfire it changed nothing above the smoke animation.
- Terrain tracing for sky visibility and bounce (hfrt): the islands see 90 to 100% of the sky almost everywhere and the bounce was too dark to show, for +12 to 30 ms. Tracing the terrain per vertex was blobby and slower.
- Sky visibility maps (wildcard): mostly below what a player would notice. The physically straight setting only made woods darker.
- Lamp light bounced off the ground (night): no visible effect.
- Why they all failed: each was added on top of the existing fill light, at 1 to 3 times strength. Nobody turned that fill down to let the traced light carry the shade. See section 4, phase 4.

**Air and volumetrics**

- Lamp and campfire halos at night (volumetric, night): tinted the whole sky brown.
- Carving the beams out of the haze (volumetric): darkened the frame.
- Strongly blue distance haze (haze, rounds 1 to 5): blue haze, rejected.
- The 48-sample sun march inside the mist (haze): changed about 2% of pixels for a lot of cost.

**Sky**

- Lighting the ground from the physical sky (atmosphere): blue ground and blue shadows.
- One brightness setting for the whole physical sky (atmosphere, run 2): pale overcast and a grey sunrise.
- Stronger cloud relief at midday (clouds, variant B): glossy and painterly. Earlier rounds had a glare band at the horizon, blocky shapes and seams along the sky dome's triangles.

**Shadows**

- Screen-space contact shadows (shadows): the shadow map already covers almost all of them.
- Soft sun shadows (shadows, temporal): close to invisible at frame size. A sun 2 to 3 degrees wide looks mushy.
- Doubled shadow range (shadows): no visible gain for +5 to 8 ms.

**Surface detail**

- Texture filtering, texture streaming, rebuilt mip chains, sharper magnification, relief from the base texture, coarse detail layers (texsharp): no visible gain. Filtering is already at 16x. A negative mip bias with strong sharpening caused colour speckle and shimmer.
- Bump maps generated at load time for vehicles (normals): nothing visible, because CWA vehicle textures are flat paint with the lines already painted in.
- Relief from each terrain square's base texture (normals): seams every 50 m.
- Warping the terrain texture lookup (tiling): marbled liquid swirls and seams along triangle edges.
- Ground variation that also changes saturation (tiling): washed the ground pale.
- Leaf-card normals and a shadow floor for crowns (shade): no effect.
- A strong green push on leaves (foliage): lime, cartoon-like.

**Tone and exposure**

- A floating-point HDR frame (exposure): no gain by day; at night, smoke blew out to white.
- The AgX tone curve (exposure): muted colours and a grey sun.
- Eye adaptation toward a fixed brightness target (exposure): +2 stops on most frames and blown-out skies.
- Brightening shade everywhere (exposure): the sea went flat grey.
- Tuning only the existing lighting knobs (shade): cannot reach the target shade without flattening ground shadows.

**Reflections and materials**

- Reflections on vehicle paint (ssr, materials): nothing visible; matte hulls only get a little greyer.
- Lamp reflections using the ground's own roughness (materials, v7 and v8): white sheets over the whole wet square. Fixed in v9.
- Material guessing on dry surfaces (materials): almost invisible.
- Water variant A on its own, and the early water rounds with strong reflection (water): too pale or not enough.

**Other**

- A regular 2x2 dither pattern in the combat shadow (combat, round 8): printed a grid on the ground.
- Cheaper shader shortcuts for foliage (perf): saved at most 0.5 ms, inside the noise.
- TAA used only as anti-aliasing (temporal): a sideways move; the frame gets softer for only about 4% steadier town views.
- Sharpening before the final downscale (aa): blurred away. Strong sharpening (aa): looks processed.
- Leaf crowns reading the sky visibility maps (wildcard): +10 ms and heavy dark crowns.
- The canopy map's sky term on the forest floor (foliage): only darker.

## 3. Unverified or fabricated

**Fabricated:** none. The critics found every cited image, patch and note on disk, and the game logs confirm each prototype loaded and its settings took effect.

**Unverified across all tracks**

- **GPU cost.** No number was measured on an idle machine, at 1080p, or on an RTX 3060. Every reading came from an RTX 4060 Ti shared with up to four other test games, at 1440x960 internal with 4x MSAA. The same unchanged view often swung 2 to 10 times between shots.
- **Motion.** Almost every judgement is from still pictures. Shimmer, crawling patterns, ghosting and popping are untested in most tracks. Only short, slow camera moves were tried (volumetric, temporal, texsharp, ssgi, normals, shadows, clouds).
- **The before pictures are wrong in two ways.** Script-spawned vehicles were drawn with the original look's lighting, and trees and buildings cast no shadows unless a track switched them on. Some tracks' "current Remastered" panels also came from their own scratch build, not the latest branch.

**Overclaims the critics found, by track**

- combat: the shadow light flips between two fires every few frames; a seam down the screen centre in combat mode only; one picture caption names the wrong fire as the shadow source.
- shade: two headline pictures (forest edge at noon, coastline) compare runs with different cloud positions, so they oversell the change. On side-lit views the change is about 2 of 255 levels.
- perf: "world pass 34 to 2 ms" mostly measures removed CPU stalls; the honest number is the CPU frame, 32 to 12 ms. The hill-hiding test is not proven safe.
- ssr: right-edge seams and box artefacts were under-reported. The saved patch is missing one changed file (the settings registration). The best dusk picture uses an older shader.
- volumetric: "only adds light" is false; the clear-weather setting darkens backlit vehicles 15 to 19%. The blocky glow patches were said to be fixed and are not. One headline picture comes from older code.
- materials: the low-sun vehicle crops show no change from the vehicle fix. The soldier's puddle reflection is not visible. The blue cast and the stripes on wet hulls were understated.
- atmosphere: the water reflection is not visible; the distance colour barely reaches the ground; noon cirrus is mostly erased.
- hfrt: the timing table mixes runs; the cheaper setting was never shown to be cheaper; shadows near the camera are not sharp at contact.
- haze: "+9 ms" is the most favourable reading; the real cost may be much larger. The near slope does change, contrary to the claim.
- shadows: missed a wrong shadow on the Sinai foreground. The soft-shadow work is nearly invisible despite the title. The tree-shadow bug was partly documented in code already.
- night: the headline pictures were shot with lamp bounce on, not the recommended setting. The jeep-rear leak was wrongly blamed on the original look. Moving the moon makes no visible difference in town. "Silver crowns" are not visible.
- clouds: the overcast headline picture is a different view (looking up). The change in sea glitter was not mentioned. Some single shots read +5.5 ms.
- texsharp: the brightness rise and the sharpening cost were overstated on some views.
- temporal: the "before" panel of the convergence sheet is a glitched capture with the shadow cut off. The noise metric mostly measures blur. Some sheets come from an older build.
- gbuffer: the town headline picture mixes runs, and its bounce light sits inside run-to-run noise. The cause of the broken night is a guess, not proven.
- foliage: the frame-wide change is small; "dark crown cores" are not visible; the canopy map mostly darkens.
- tiling: the forest edge view is 8% darker, not 6%. The noon fields view moves further from the original look, not toward it.
- normals: the best variant does not use the load-time bump maps the track is named for; Sinai gets 3% darker; the best variant was never timed.
- aa: the new downscale alone is not sharper, only cleaner. The cost of the 1.5x preset is unproven.
- exposure: the sunrise sky is not unchanged (17 levels brighter). The real cost is about 1 ms, not 0.3.
- ssgi: the best night picture uses double-strength bounce.
- wildcard: video memory is about 160 MiB, not 85. The pine woods cost +3.9 ms. Trunk bases do not get darker.
- water: no critic has reviewed it. The cost is estimated, not measured. Motion is untested.

## 4. Road to RTX-on

**The honest picture.** The four RTX ingredients that worked here are emissive light with shadows (combat), reflections on water and wet ground (ssr, materials, water), visible light in the air (volumetric, haze), and a physical sky with long low-sun tree shadows (atmosphere, clouds, hfrt). The ingredient that did not work is bounce light. So the plan below stacks the working pieces on a corrected base. Shared groundwork lets each piece cost less. Real bounce light is the last step, as a research bet.

### Step 0: owed before any build

- A clean timing run: idle machine, 1080p, ideally an RTX 3060-class card.
- Fix the capture harness: wait for the view distance to finish loading before the first shot (the first original-look shot was taken mid-load), reset every lighting knob between variants (`triSetLighting remastered` keeps old knobs), pin the object detail level for A/B shots, and log draw counts next to each shot.

### Phase 1: correct the base (C++, S to M, almost no GPU cost)

1. Spawned-vehicle fix: build vertex buffers for shapes loaded after mission start (item 2's version). Skip sky, sun and cloud shapes and the gunner's interior view. Check night carefully. Find out why one vehicle's slats turn from grey to tan.
2. Tree and building shadows on at start-up (item 10's start-up fix).
3. No cloud shadows under a clear sky (item 2).
4. 4x MSAA on by default (item 19).
5. The frame-cost fixes (item 3): grouping only objects that have no lights, the bush terrain-fit bug, and a hardened hill-hiding test.
6. Shoot every before picture again. Earlier judgements of vehicles and trees were made on wrong pictures.

### Phase 2: shared groundwork

- **Sky table on the GPU** (item 7): one table for sky colour, horizon, fog colour and water reflection, and later a physical sky light. Use the half-way colour and saturation of 1.15 or less.
- **TAA** (item 14) with per-object motion and a particle mask, resolved before the tone curve. Every noisy effect below gets cleaner and cheaper.
- **Packed G-buffer** (item 15): surface direction, material class (item 6's name-based guess, moved into a config table), colour, and light stored after the night darkening. It replaces the stencil tags in item 4 and the colour-alpha tricks in items 20 and 21.
- **One shared light list**, culled per screen tile: used by combat (1), night lamps and headlights (11) and wet-street reflections (6).
- **One half-resolution air pass**: haze, valley mist and sun and headlight beams in a single march against the shadow map (items 5 and 9), driven by weather and time of day, and fed to AI sight.

### Phase 3: the visible layer, one piece per area

- **Night and combat:** combat light (1) with its shadow in a half-resolution pass, 2 to 4 shadowed lights and a brief flash brightening; night moon and low beams (11) on the shared light list.
- **Water and shine:** sea waves (W), reflections (4) feeding the water shader, wetness (6).
- **Light and shadow:** long tree shadows (8) at reduced resolution, the shade calibration (2), and a light fill under tree crowns so woods do not go dark.
- **Sky and air:** the air pass at dawn, dusk and in mist; lit clouds (12) only at low sun.
- **Polish:** terrain detail (13), downscale and sharpen (19), desert ground variation (17).

### Phase 4: the bounce-light bet

Try the experiment no track ran. Turn the hand-tuned sky and ground fill down hard. Let traced sky visibility (item 22's maps, or G-buffer screen tracing) plus one coloured bounce carry the shade, smoothed by TAA. This is the only route here toward shade that looks path traced, with colour bleeding and contact darkening. Bounce light added on top of the existing fill was invisible in four tracks, so this is a bet, not a plan.

### How the pieces depend on each other

```
Step 0 timing + harness
  -> Phase 1: vehicle fix, tree shadows, clear-sky fix, MSAA, frame-cost fixes
       -> re-shoot the before pictures
  -> sky table ---------> air pass (haze + beams) -> lit clouds
  -> TAA ---------------> cleaner combat shadows, cheaper beams, reflections, tree shadows
  -> G-buffer ----------> reflections + wetness -> later: bounce light (phase 4)
  -> shared light list -> combat + night + lamp reflections on wet ground
  -> tree shadows ------> beams (needs casters), long tree shadows near the camera
```

### GPU budget at 1080p on an RTX 3060 (rough)

Rule of thumb: about 1.5 to 2 times the measured numbers. 1080p has 1.5 times the pixels of the test resolution, and a 3060 is slower than the test card. Everything here is an estimate built on noisy readings.

| Piece | Measured (shared 4060 Ti, 1440x960) | Estimate, 1080p on a 3060 | When it runs |
|---|---|---|---|
| Tree and building shadows | +0.2-1.4 ms | 0.5-2.5 ms | day |
| Sky table | under 0.3 ms (estimate) | about 0.5 ms | always |
| Air pass (haze + beams, half res) | beams 0.4-2.4 ms; haze unknown | 2-4 ms | dawn, dusk, mist, headlights |
| TAA | 0.07-0.14 ms | about 0.3 ms | always |
| G-buffer (packed) | 1-2 ms (estimate) | 1.5-3 ms | always |
| Reflections | 0.35-1.9 ms | 1-3 ms | water or wet ground on screen |
| Wetness | +0.4 ms | about 0.8 ms | rain |
| Long tree shadows (reduced res) | unmeasured; forward version +2.5-16 ms | target 1.5-3 ms | low sun |
| Lit clouds | 0.2-0.9 ms | 0.5-1.5 ms | day |
| Sea waves | under 0.1 ms (estimate) | about 0.2 ms | sea on screen |
| Terrain detail | 0.5-2 ms | 1-3 ms | always |
| Downscale and sharpen | 0.16 ms at 1080p | about 0.2 ms | always |
| Combat light | under 1-2 ms (unresolved) | 1-3 ms | fights |
| Night lights (shared list) | +0.8-3.3 ms | 1-2 ms | night |

- **Base frame:** after the frame-cost fixes, current Remastered measured 2.5 ms (sunrise ridge) to 7.5 ms (noon fields) of GPU time on the shared card. That is perhaps 4 to 12 ms at 1080p on a 3060.
- **Day, typical:** about 8 to 10 ms of effects. **Day, worst case** (low sun, water and woods in one view): about 20 ms, which does not fit.
- **Night, typical:** about 6 ms. **Night, worst case** (a fight in the rain): about 13 ms.
- **Verdict:** the 16.6 ms budget for 60 fps holds only if the effects are gated by time of day, weather and what is on screen, and the heavy ones run at half resolution. Many effects never run together: long tree shadows and beams only at low sun, combat light only in fights, reflections only where water or wet ground is in view.

### The first three to build

1. **The phase 1 base fixes** (spawned vehicles, tree shadows, clear-sky cloud shadows, MSAA by default), then re-shoot the before pictures. Three of these are bugs that make every current picture wrong, and four tracks hit the vehicle bug on their own. Guerrilla Mode spawns nearly every vehicle by script, so without this fix no lighting work reaches them. Beams and long tree shadows need the shadow fix. The engine changes are small and cost almost no GPU time.
2. **The frame-cost fixes** (item 3). They change no pixels, they cut the CPU frame on long views from 32 to 12 ms, and they fix a real stock bug. They also pay for the extra shadow work that step 1 switches on and for the heavy effects that follow. They should land before anything adds cost.
3. **Combat light, production version** (item 1). It has the best critic scores (7 for worth building, 6 for RTX-on) and gives the clearest "different game" picture. It needs no G-buffer. Rebuilding its shadow as a half-resolution pass with temporal smoothing also builds the first piece of the TAA and history groundwork, which reflections, the air pass and long tree shadows reuse next. Fix the two bugs the critic found (the shadow jumping between fires, the centre seam) and the dim flash frame.

**Cheap side task:** the sea waves (W) are shader-only and answer a complaint you already made about smooth water. They need a critic check and a look in motion first, because the sea texture switches frames every 0.14 s and the waves may pop.

## 5. Bugs and findings to act on anyway

- **Spawned vehicles skip the new lighting** (shade, gbuffer, materials, normals). See phase 1.
- **Tree and building shadows are off at start-up** (shadows, volumetric). The graphics settings are applied before the world exists (GameApplication.cpp lines 681 and 683), so `Scene::_objectShadows` stays false.
- **Cloud shadows under a clear sky** (shade): `setOvercast 0` still gives 4% cloud cover in the shadow model.
- **No anti-aliasing in the Remastered default** (aa).
- **A stock instancing bug** (perf): grouped bushes take the first bush's terrain fit, which also affects missions without lights.
- **The sun-shaft composite lays a grey veil on nearby vehicles** (shade).
- **"Shadow maps vanish when facing the sun"** (foliage): probably the same as the tree-shadow bug; confirm.
- **The Remastered night is over-bright** (exposure, materials, night): the moonlit town meters brighter than low-sun daytime views.
- **NaN crown normals in the shipped shader** (wildcard): some tree crowns silently fall back to ground-only light.
- **A GL undefined-behaviour warning** (shadows, clouds): a shadow comparison sampler bound to texture 0.
- **`switchLight` on some map street lamps crashes the game** (ssgi): access violation in `StreetLamp::SimulateSwitch` calling `EntityHit::GetHit`.
- **A dead water shader path** (water): `misc\waveBumpMap.paa` named in `remaster.cpp` does not exist, `GetWaterBumpMap()` has no callers, and the README's Water row describes a shader the sea does not use.
- **Mist is visual only** (haze): AI spotting ignores it, which would be unfair to the player.
- **Owner decision 1 makes some advice stale:** many track notes say "keep the original look bit-identical". The new lighting replaces it, so those steps can be dropped.
