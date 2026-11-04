# xp-nightlight: richer light at night

Lane xp-nightlight of the overnight experiment swarm. Base 243ef1e0 (graphics-remaster tip, base of graphics-experimental). Worktree D:\Arma_CWA_worktrees\sq-xp-nightlight, nothing committed. Patch: items\XP-xp-nightlight\patch.diff (git diff --binary against 243ef1e0, 8 files). Final frozen dist: arma_CWR\dist\xpnl-r10-rwdi in the worktree (r10 = r9, which the final stills and the timing ran, plus a one-frame state reset found in review; it changes no capture view).

## In one paragraph

Street lamps now cast shadows, and lamps, headlights and fires throw a little light back up off the ground they light. Both are cheap, off by default, and together they make the night town look grounded: the lamp throws the tree across the square, the sign and the post cast short shadows, the crown darkens inside, a burning hull is lit by its own fire. POSEIDON_XP_NIGHTLIGHT=1 turns both on. The third goal, lit windows, did not work on the stock houses: their windows are painted into the facade textures in the same dark, neutral-to-warm colours as the timber beams, so nothing in the texture tells a pane from a beam, and every colour rule I tried lit beams and frame corners instead of windows. The switch stays (off, not part of NIGHTLIGHT) with a diagnostic map, for data that marks its windows. Along the way I found a tip bug: in dry weather every texture is drawn with surface class 0, because the class table is only built once wet weather asks for it.

Cost, measured (request 3, results\quick-r1: timed quick set on this machine's RTX 4060 Ti and i7-14700KF, ring mode, clean 0 of 45 foreign samples, one A/B pair per row, same exe, POSEIDON_XP_NIGHTLIGHT=1 against everything off):

| view | CPU work (median) | GPU frame (median) | GPU pass1 | B GPU frame p95 | B CPU work p95 |
|---|---|---|---|---|---|
| town_night_rain (street lamp, fire, rain) | +0.11 ms | +0.48 ms | +0.33 ms | 7.72 ms | 3.26 ms |
| combat_fires (fires, no street lamp) | -0.04 ms | 0.00 ms | +0.01 ms | 3.20 ms | 2.11 ms |

So about half a millisecond of GPU where a street lamp is near (the extra mask channels and the per-fragment lookups and bounce), nothing measurable where only fires are, and both views stay far inside the 15 ms p95 budget. One pair per row on a rain view, and the object counts differ between the sides (608 against 764, sampling), so read the town number as indicative, not a verdict.

## What each switch does

| variable | effect |
|---|---|
| POSEIDON_XP_NIGHTLIGHT=1 | lamp shadows and bounce (gain 1) together; not the window glow |
| POSEIDON_XP_LAMPSHADOWS=1 | up to two street lamps within 60 m of the camera borrow free channels of the combat shadow mask |
| POSEIDON_XP_LIGHTBOUNCE=g | the ground's bounce of lamps, headlights and fires, gain g (fires at a quarter) |
| POSEIDON_XP_WINDOWGLOW=g | experimental: lit windows and rooms at night, gain g (see "Window glow") |
| POSEIDON_XP_WINDOWANY=1 | window glow treats every static texture as a facade with painted windows (rejected, kept for diagnosis) |
| POSEIDON_XP_NIGHTDEBUG=1 | diagnostic map of the window glow's inputs (not a look) |
| POSEIDON_XP_NIGHTLOG=1 | logs each texture's first draw with its class, kind and window hint |
| POSEIDON_XP_SURFTABLE=1 | builds the surface class table before the first draw (the tip bug's fix, as a probe) |

All are read once at start. With none set the shaders read a constant slot that stays zero and every frame is the tip's (see "Off is the tip").

### Lamp shadows (how)

The combat lights already have a screen-space shadow mask: a half-resolution march toward each shadowed light through last frame's linear depth, four channels, one bilinear tap per fragment. When a street lamp is within 60 m, the lamps take the mask's last one or two channels (chosen by intensity over distance squared, with the same swap hysteresis the combat lights use) and the combat lights keep the rest. The lamp's physical light is multiplied by its channel's visibility. The mask is also built when only lamps need it (no combat light). Cost: the same pass with one or two more channels; when no fire or flash is near, it adds the mask pass itself.

### Bounce (how)

For each lamp, headlight and combat light already shaded at a fragment, the shader finds the ground the fragment faces (under it, pushed out along its horizontal normal by about its height), takes that light's irradiance there, and returns 0.2 of it in the ground's tint, weighted by how much of the fragment's lower hemisphere sees the ground (none on flat ground, half on a wall, all on an overhang). No occlusion is known for it. Fires count a quarter: at full strength a burning hull read pale, as if lit directly, and at half it still lost its shape.

## Results (stills, judged by eye; same exe, A off and B on unless noted)

Final look (request 11, results\stills-r8-final, with the fires' bounce at half; request 13 repeats combat_fires, town_night, town_night_rain and noon_fields on the final patch with a quarter: POSEIDON_XP_NIGHTLIGHT=1 against everything off, deterministic world, fixed 300-frame settle):

| view | vgate class | max | px over 8 | what changed |
|---|---|---|---|---|
| town_night | CHANGED | 65 | 269,467 | lamp shadows of the tree trunk and crown across the square, the road sign and the lamp post; the crown darker inside; faint warm bounce on walls and signs near the fire and lamp; metering +0.09 EV |
| town_fog_night | CHANGED | 49 | 84,320 | the same shadows through fog, softer; the headlights' pool lifts the nearest walls a little |
| town_night_rain | CHANGED (perceptual, rain) | 55 | 208,967 | the trunk's shadow across the wet ground and puddles; same crown and wall changes |
| combat_fires | NEAR | 78 (67 at a quarter) | 3,453 (2,315) | the burning hull lit warm by its own fire off the ground (pale at half, shape kept at a quarter); soldiers a shade lighter on their ground side |
| road_flat_drive_moonless | CHANGED (mild) | 49 | 22,516 | a roadside tree crown faintly lit from below by the headlights' pool; metering +0.03 EV |
| noon_fields | IDENTICAL | 0 | 0 | nothing runs by day |

Judgement: better, not just different. The square reads grounded instead of evenly washed (showcase final_town_night_A_off.png against final_town_night_B_nightlight.png, and the trunk shadow crop r1_town_night_lampshadow_crop.png); fog and rain keep it (r3_town_fog_night_*, r3_town_night_rain_*). combat_fires needed two rounds. At full strength the fire's bounce washed the burning hull pale (r1_combat_fires_bounce_too_strong_crop.png); at half it still flattened its shape (final_combat_fires_A_off.png against final_combat_fires_B_nightlight.png, request 11). At a quarter (the final patch, request 13, results\stills-r9-final: NEAR, max 67, 2,315 px over 8) the hull is lit warm by its own fire off the ground and keeps its shape (final2_combat_fires_A_off.png against final2_combat_fires_B_nightlight_quarterfire.png, crop final2_combat_fires_hull_quarter_bounce_crop.png). In that job combat_fires was the boot's first shot, so its fire state differs from request 11's; A and B of one job always match. Most of the whole-frame difference in the diff pictures is the metering: the shadows darken the square, so the exposure rises 0.07 to 0.10 EV. Measured on the town_night pair (display levels): the tree crown 46.1 to 39.7 (-14%, the lamp's own leaves shadow it), the left of the square 43.9 to 42.0 (-4%, shadow minus the exposure lift); the burning hull at half bounce 34.6 to 40.8 (+18%), at a quarter 30.1 to 33.8 (+12%, request 13's own fire state).

Earlier jobs, in order: r1 (all on, gain 1) and r1-strong (glow 2.5, bounce 3) located each effect; r2 halved the fire bounce; r3 moved the constants to slot 91 (pictures within a few hundred pixels of r2: the move is clean); r3-surftable is the tip bug probe below; r4 to r7 debug maps and window-any attempts for the window glow; r8 final and off identity.

## Window glow: what was tried and why it is not on

1. Glass sections (the materials step's glass class on static objects) and textures whose name names a window (okno, okna, okenice, sklo, window, glass, black_sum): on Abel the only such sections in view are a few window frames (okno_ram) and doors; the panes are not separate sections.
2. Rooms: the stock houses do have inside textures (hruzdum_in1 to 4, stena_in_2, strop, podlaha), lit as a room light per building and storey. They are not what the windows show from outside.
3. Every static texture as a facade, panes by colour (near black and not warm, then dark and cool): the debug maps (showcase r4_ and r5_) show the hash, the lit sides and the pane test working, but the painted panes are dark neutral-to-warm grey, like the beams. Near black lit the beams; dark and cool lit frame corners and specks (showcase r6_town_night_windowany_noise_rejected.png). Pre-lit and kind-0 sections were probed too: there are none in these views.

Conclusion: window glow on the stock data needs authored input, for example a small per-texture table of window rectangles in UV, or a window mask texture per facade. The shader side (per building, side and storey hash; warm, soft warm and television tints; metered against the night's own ambient; colour kept under the night eye) is in the patch and would take such a mask in a few lines.

## A tip bug found on the way (not this lane's change)

In dry weather every texture reads surface class 0 (Unknown). EngineGL33::EnsureSurfaceTable builds the class table only when wet weather shades the frame, or a decal batch or a fog interior asks; TextureGL33::SurfaceClassFor treats table generation 0 as already classified and returns its zeroed classes. The NIGHTLOG texture list showed class 0 on every logged texture, and the tip-side game log builds the table only just before the fifth shot (town_night_rain), so town_night, town_fog_night, combat_fires and noon_fields are all shot with no glass, metal, paint, roof or road class. POSEIDON_XP_SURFTABLE=1 builds it before the first draw. Its stills job (request 6, results\stills-r3-surftable): town_night_rain IDENTICAL (the table was already built there), combat_fires PI-T, town_night and town_fog_night NEAR (about 100 px over 8), noon_fields NEAR (max 85, 2,815 px over 8): the Jeep's windscreen now reads as glass with a reflection line (showcase tipbug_surftable_noon_fields_windscreen_crop.png). The fix is one call at the top of ApplySurfaceClass (or once at world load) and belongs on graphics-remaster after an owner look, since it changes the approved dry look a little.

## Off is the tip

Request 12 (results\stills-r8-offidentity): the final exe with no switch set against the tip's own dist (integ7, 243ef1e0), claim exact. combat_fires, noon_fields, town_fog_night and town_night_rain are byte-identical to the tip (vgate prints REFUSED there only because its cross-exe guard sees the same file hash on both sides). town_night is PI-T: max 6, 6 px over 2, none over 8, scattered single-level specks; that boot reached the first shot one frame later (frame 875 against 874, 841 metering reads against 817), a boot-timing offset on the first shot of the session, not anything the lane computes. So the default frame is the tip's, but this is not a proof of pixel identity on every view. (Request 12 ran the r8 exe; r9 differs from it only in a shader constant that is read only while the bounce is on.)

## Integration notes

- The lane's constants live in a new LitConstants slot (LitConst::XpNight = 91, Count 92; the layout unit test knows it). The free padding slot 72 was the first choice, but xp-particles (already on graphics-experimental) writes slot 72 with projection terms halfway through the frame, which would have switched this lane's window glow on for every lit draw after the first particle. On graphics-experimental xp-water already took slot 91, so the integrator renumbers this one to 92.
- The transform vertex shader now includes the LitConstants block (it reads litXpNight and litCamPos to hash the object's position into vLitMode, 1 to 1.4, only while the window glow is on; exactly 1 otherwise).
- The per-draw surfClass uniform carries a fourth value (the window hint), 0 unless the window glow runs.
- Formatted with clang-format 22.1.8; 21.1.8 pending. No new build warnings. Unit gate on the final build (r10): PoseidonTests 3525 of 3535 passed, 6 failed (the known data six: map legend, equipment and MP version localization), 4 skipped; PoseidonCoreTests 823 of 827 (4 skipped) and PoseidonFoundationTests 475 of 475 on r9. The glslang cases in PoseidonTests compile every changed shader.

## Risks and limits

- Lamp shadows are screen space: an occluder off screen casts nothing, so a shadow can appear or vanish as the camera turns, and the march noise turns per frame under TAA. Motion must be judged by the owner from a clip.
- With lamps near, at most two combat lights (instead of four) get shadows. In a firefight under street lamps that is a trade.
- The lamp-only case (lamps, no fire or flash) is in no capture view: every town view has a fire. The code builds the mask for lamps alone, but no picture shows it.
- Bounce has no occlusion: a surface under an overhang or behind a wall still takes the ground's light.
- The lamp channel hysteresis counts frames at a fixed 1/60 s step, so at high frame rates a lamp swap happens sooner in real time.
- The window hint is kept per texture pointer and cleared when the programs are rebuilt (experiment grade).

## Recommendation

- Lamp shadows and bounce: a night quality tier (High and above), after the owner's clip passes for motion and the timing above. Not default yet: it changes approved night views and is temporal.
- Window glow: drop for the stock data; revisit only with authored window masks.
- SURFTABLE: land the one-line fix on graphics-remaster after an owner look (separate from this lane).

## Files

items\XP-xp-nightlight\: notes.md (checkpoints), patch.diff, request-1..12.json, results\ (every job), showcase\ (the pictures named above). Same report: tmp\perf-squad\xp\xp-nightlight.md.
