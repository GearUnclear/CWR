# Combat light (track: combat)

Rank 1 of 22. Critic scores: worth building **7/10**, RTX-on **6/10**. Area: night and combat.

## What the player sees

Muzzle flashes, shell bursts, burning wrecks, campfires and flares become real light sources. They light the men and the ground near them and cast long shadows that are sharp at the feet and soft at the tips. Smoke is lit from inside. By day the effect is almost invisible, as intended.

## How the original look does it

In the original look a flash or an explosion is a point light added per vertex with almost no falloff. On the flash frame the whole visible ground whitens evenly, with no direction, no colour and no shadow. By day the engine drops every local light (`Scene::SelectLights`, `NightEffect() < 0.01`). Current Remastered keeps the same pool size and falloff.

## The technique

- A new module, `Graphics/Rendering/Lighting/CombatLights.{hpp,cpp}`. Each light carries linear HDR light (light values above display white) with a soft ball falloff, `E(d) = I / (1 + d^2 / r0^2)`, faded to zero where it reaches the night ambient level.
- Kinds of light, each with its own timing: muzzle flash (30 ms), cannon, explosion (a white flash cooling to an orange fireball), fire (smooth flicker instead of a random brightness each frame), burning wreck embers, tracer, flare.
- Hooks in the weapon, explosion, wreck, fireplace, tracer and flare code.
- A **screen-space shadow**: the strongest combat light in view traces 24 steps per pixel toward the light through the previous frame's depth buffer. How deep a blocker sits gives a soft edge that hardens at contact. This was the breakthrough (round 6).
- Smoke particles take the combat light (on the CPU).
- A little lit air (in-scatter) around each light in the post pass.
- All behind `triSetLighting "combatLights=1"` and six more knobs.

## What real data showed

Abel, nine rounds of captures, shell sequences shot frame by frame.

- **Shell at night, camera 5 m up** (`BEST_ffh.png`): the original look and current Remastered whiten the ground evenly. With combat light the men throw long radial shadows toward the camera, the crater glows orange and the truck is backlit.
- **Parachute flare** (`BEST_flr.png`): the original look lights the field flat grey. Combat light gives a falloff from the flare's side and long shadows that swing as the flare sinks. The builder called it the most "RTX" result of the set.
- **Steady fires** (`BEST_ffn_fires.png`): the campfire and the burning truck light the battlefield warmly and cast shadows of the men.
- **Flash frame at eye height is darker** than the original look's whiteout: mean 37 against 63 (0 to 255). Physically correct (ground near the camera is lit at a grazing angle), but it breaks the "only darker" rule at the most dramatic moment.
- **Muzzle flashes** light the shooters' arms and a pool on the ground, but only for one captured frame per shot.
- **Noon**: close to invisible after the round 9 day scaling.
- Round 8 tried a regular 2x2 dither in the shadow. It printed a grid on the ground and was dropped.

## Cost

Below what the shared machine could measure. The best run shows -0.32 ms against a noise of several ms. The critic estimates 1 to 3 ms at 1080p on an RTX 3060, only while combat lights are near the camera. The worst case (a big explosion near the camera) was not captured by the averaged timers.

## Verdict

Critic: "A real, bold night-combat win." Fires, flares and shell flashes cast long shadows and light the men, so night battles look like a different game. It has stipple, shadows that jump between two fires, a seam down the screen centre, and a flash frame darker than the original look.

## What production needs

- Move the shadow into its own half-resolution screen pass on the current frame, with a blur and temporal smoothing (this removes the stipple and the one-frame lag). Allow 2 to 4 shadowed lights instead of one.
- Pick the shadowed light with hysteresis, so it does not flip between two fires every frame.
- Find and fix the centre-column seam.
- A brief eye adaptation or flash fill so the flash frame does not read dimmer than the original look.
- A light budget (merge one squad's muzzle flashes into one light) and a light list shared with the night lights.
- Calibrate per ammo and explosion class from config instead of a hard-coded table.
- Light smoke per vertex or in a small 3D grid, with self-shadowing.

## Overclaims the critic found

- Not disclosed: the shadow-casting light flips between the campfire and the burning truck on consecutive frames (`ShadowCaster()` picks the strongest flicker with no hysteresis).
- Not disclosed: a vertical seam of 3 to 4.5 brightness levels at the exact screen-centre column, in combat mode only, present since round 5.
- The `BEST_ffn_fires` caption names the truck as the shadow source; the frames shown have campfire shadows.
- "By day nearly invisible" is a little generous: small orange glows stay at the campfire base and behind the truck at noon.

## Taste risks

The flash frame is darker than the original look. Ground under fire light turns beige or khaki (red/blue 1.41 to 1.48), close to the rejected khaki look. A warm glow strip runs along the horizon behind fires. Ground next to a flare goes almost white.

## Where the work lives

- Notes and patch: `tmp\research\combat\NOTES.md` and `prototype.patch` (round 9, 76 KB, 20 changed files plus the new `CombatLights` files). Earlier states: `prototype_r3.patch`, `prototype_r4.patch`, `prototype_r5.patch`.
- Scratch worktree: `gfx-combat`, build `dist\gfx-combat-rwdi`.
- Best images (renders of game data, not committed): `tmp\research\combat\BEST_ffh.png`, `BEST_flr.png`, `BEST_ffn_fires.png`, `zoom_shadow_stipple.png`, `zoom_r8_grid_vs_r9.png`.
- Integration: the owner asked on 2026-10-17 for the ideas that worked to be compiled on a new branch, `graphics-remaster`. This patch is a candidate for it. That branch did not exist yet when this page was written; check it for the production state.
- Note: the bug fix 4515770 lowered the night local-light cap from 1.5 to 1.0 (the original look's clamp). A combat-light rework that wants bright HDR flashes must take that into account.
