# Screen-space bounce light (track: ssgi)

Rank 21 of 22. Critic scores: worth building **3/10**, RTX-on **2/10**. Area: light and shadow.

## What the player sees

By day, nothing. At night, an armoured vehicle next to a headlight pool gets lit from below by that pool. This is one of the four bounce-light attempts that failed by day (see README, "Why bounce light failed").

## The technique

SSGI (screen-space global illumination): rays are traced through what is on screen to find light that bounces off nearby surfaces. C++ prototype in the post pass, after the AO blur.

1. **Trace** at half resolution: 6 rays per pixel, 20 steps each, out to 3 to 25 m. A ray that passes behind the depth surface is a hit; its light is read from a mip chain of the frame, with interleaved noise that rotates every frame.
2. **Temporal accumulation:** reproject into last frame's history, reject where depth disagrees, blend 10%.
3. **Spatial blur:** 13-tap, depth-aware.
4. **Composite:** upsample, and recover the receiver's albedo from the sky-light share that the Remastered shaders store in the colour alpha. Capped so a lamp-lit pixel cannot claim a white albedo.

Modes (`triSetLighting "giMode=..."`): 1 adds only the light a hit sends back above what the rig's sky already assumes (the physical mode); 3 replaces the rig's sky and the AO with the traced result; 2 is a cheap SSDO-style variant; 6 is a non-physical upper bound (full hit light on top of the rig, double counting the rig's own ground bounce); 7 and 8 are diagnostics.

The resumed run (v7 to v9) tested night scenes and added: a "hue albedo" stand-in for night (the sky share carries nothing at night), a hit-normal test, a crossing cap, a distance fade, and (v9) a detail-keeping albedo so dark texels stay dark.

## What real data showed

**Daylight: failed.** The physical mode changes daytime frames by 0.0 to 0.5 levels on average (p99 at most +5, inside the shot-to-shot noise of about 3). Even the non-physical upper bound reaches only +9 to +13 levels on the lower halves of vehicles. Why:

- **The sky dominates the hemisphere.** The rig's sky fill is bright (shade at about 0.6 of lit, as the owner asked), so a hit only adds light if it is brighter than the sky it hides. Sunlit Abel and Sinai ground sits close to the rig's assumed ground albedo.
- **The acceptance views are backlit:** every face the camera sees looks down onto its own cast shadow (hull hit fraction 0.18 to 0.35).
- Not a missed-hit artefact (a more tolerant hit test changed nothing).
- Turning the rig's own ground bounce off changes daytime frames by only -0.01 to -0.49 levels; the traced bounce is about the same size.
- Mode 3 (replacing the AO) only darkens.

**Side finding:** vehicles, soldiers, the tent and tree crowns store alpha 1 under 4x MSAA, so the shipped Remastered AO never darkens them.

**Night with a headlight pool** (`BEST_headlights_bounce.png`, `BEST_headlights_bounce_crop.png`): a crewed Jeep's headlights light the ground next to an M113. The bounce lights the hull's rear and flank from below: +9.4 levels at x1, +15.1 at x2, on a hull whose mean is 25. With the v9 albedo, texture and panel lines stay intact (v8's flat albedo washed the hull grey). Stable in an 8-frame strafe.

**Campfire with the street lamps off:** +1 to +3 levels, inside the smoke-animation noise.

**Found on the way:** auto street lamps switch on several seconds into a capture (fixed later in the capture harness, 874638c), and `switchLight` over map objects crashed the game (fixed in ab61aa5). See `bugs.md`.

## Cost

+1.4 to +3.5 ms (the `ao` column, which holds AO plus the bounce); v8 and v9 at the top because of the extra hit-normal reads and the 25-tap albedo. Perhaps 4 to 6 ms at 1080p on a 3060. It fits the budget only if gated to scenes with local lights.

## Verdict

Critic: "This report is honest and fully backed by files." Invisible by day, one real but narrow night win: a hull beside a headlight pool goes from near black to a grey-lit, textured face at x2. That is not an RTX-on look.

## What production would need

- A real albedo and sky-share buffer (the G-buffer, `gbuffer.md`); every albedo here is a stand-in.
- A frame copy without particles (smoke sprites keep the depth of the house behind them, so the house "lights itself" warm).
- Scope it to local lights (headlights, lamps once physical, fires, muzzle flashes, explosions), gated off by day.

## Overclaims the critic found

- The headline night picture uses double-strength bounce; at x1 it is about half as strong.
- "Bottom-heavy falloff" is only partly true: at x2 the upper and middle hull get a nearly flat +12 to +13 levels, so the face reads as an even grey lift.
- "The soldier's lit side faces away" is doubtful: he stands in the pool with his legs to the camera and still gains only +1 to +2.
- The "current Remastered" baseline is the scratch build.
- The stability claim rests on one slow 8-frame strafe.

## Taste risks

The night lift is a flat grey, a mild local wash. Daylight mode 6 lifts running gear with a cool blue-grey. On the stock night town it brightens an already over-lit town and pales the crowns. Mode 3 only darkens.

## Where the work lives

- Notes and patch: `tmp\research\ssgi\NOTES.md`, `prototype.patch` (5 files, +542 lines), shaders `glsl_final\`, `override_v2` to `override_v9`, statistics `timings_and_deltas.txt`, `regions_noon_fixed.txt`, timings `timings.md`.
- Scratch worktree: `gfx-ssgi`, build `dist\gfx-ssgi-rwdi`.
- Best images (not committed): `tmp\research\ssgi\BEST_headlights_bounce_crop.png`, `BEST_headlights_bounce.png`, `BEST_lowsun_fp.png` (mode 6 upper bound), `BEST_town_dark_fire.png`, `motion_headlights_v8.gif`. The `cmp\abel1` images were shot with a units bug and must not be quoted.
- Integration: not recommended for daylight. A local-light bounce could come later on top of the G-buffer. If anything is taken to the new branch `graphics-remaster` the owner asked for on 2026-10-17, it would be the night-only path. That branch did not exist yet when this page was written.
