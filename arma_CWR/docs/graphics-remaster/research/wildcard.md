# Sky visibility from the shadow casters (track: wildcard)

Rank 22 of 22. Critic scores: worth building **3/10**, RTX-on **2/10**. Area: light and shadow.

## What the player sees

Vehicles on overcast days sit in soft dark pools instead of floating. Elsewhere the frame barely changes, or gets a little darker and greyer. This is one of the four bounce-light and sky-occlusion attempts (see README, "Why bounce light failed").

## The pick, and why

The other tracks owned screen-space bounce, heightfield tracing, soft shadows and reflections. The screen-space bounce track concluded that the path-traced jump would have to come from large-scale sky occlusion that reaches objects, on and off screen. So this track built a compute-free GL 3.3 way to trace the sky against real nearby geometry.

## The technique

**Sky visibility maps.** K orthographic "shadow maps" around the camera, one per direction spread evenly over the sky above 6 degrees, drawn from the same caster set the sun shadow cascades already collect. Each texel stores the coverage and depth moments of the nearest surface toward the sky, plus the caster colour, with full mip chains.

In the lit shader, every pixel walks each direction's cone down the mip chain: all points along a ray toward that direction land on the same texel of that direction's map, so a cone trace is reading footprints of growing size at one position. The result: per-pixel, normal-aware, colour-weighted sky visibility (blocking the warm sunward horizon leaves a cooler fill), plus a one-bounce estimate. In effect 16 to 24 cone-traced rays per pixel against real geometry, with no compute shaders, no BVH and no screen-space holes.

- 512x512 maps, a quarter of the directions redrawn per frame (round robin), each layer anchored to where it was drawn.
- A 6-step cone march per direction (a single wide blocker search under-counted contact occlusion by about 2 times).
- Front faces stored (back faces let the inside of a building darken its own facade).
- Leaf receivers skip the maps (they cost +10 ms in woods and darkened crowns).
- Final setting: `skyOcc=1 skyBounce=1 skyAlbedo=1 skyLeaf=0.45 skyBleed=0.08 skyGround=1 skyDirs=24 skyCone=1.3 skyRadius=64 skyGain=1.15 skyCrown=2`. "final_phys" drops the 1.15 sky gain.
- It copied the shade track's spawned-vehicle fix so vehicles receive the maps, and turned on object shadows (otherwise trees and buildings are not in the caster set; see `shadows.md`).

## Finding on the way: NaN crown normals in the shipped shader

Some tree crowns (the Saint Louis birch, several conifers) got NaN lighting normals and silently fell back to ground-only light. This track blamed leaf cards with zero normals and degenerate crown ellipsoids. The later bug fix measured it: only the zero normals were the cause (`normalize(0)` is NaN on this driver). Fixed on the branch in d57b712. See `bugs.md`.

## What real data showed

Change against this build with the maps off (r16):

| View | final: mean / mean abs / share over 4 levels | Read |
|---|---|---|
| overcast_close | +0.9 / 2.9 / 12% | the clearest win: a soft dark pool under the Jeep and along the M113's skirt; vehicles sit on the grass (`BEST_overcast_close.png`) |
| woods_mixed_noon | -1.7 / 4.2 / 26% | understory darker, sun patches keep their brightness |
| woods_overcast | -2.8 / 6.0 / 32% | forest floor darker and flatter grey; reads mostly as "darker" |
| town_street | -2.0 / 4.0 / 25% | facade bases darker; blotches on the church wall from the 24 directions |
| town_day | -0.3 / 1.4 / 6% | a pool under the birch, the Jeep grounded |
| lowsun_fp, noon_fields, forest_edge_golden, overcast_broken | about +-1 | near zero |
| town_night | nothing visible | local lights dominate |

The visibility field itself (`cmp_debug_final.png`) looks like a ray-traced sky-visibility pass, but the final frames carry it only partly: the sky is a fraction of the light, and the cone-averaged occlusion around a 2 m object is 10 to 40%.

**Motion:** a dolly of 0.4 m per frame shows no popping or swimming; moving objects would lag up to 3 frames (not tested).

**Failed:** leaf crowns reading the maps (+10 ms and heavy dark crowns); a leaf tint (no visible change); the physically straight setting (`final_phys`) only made woods darker (-5 to -7.5 levels).

## Cost

+1.6 to +3.1 ms at 1440x960 with 4x MSAA on most views, +3.9 ms in the pine woods: about 1 ms to draw the maps and the rest for the per-pixel march (24 directions times 6 steps). About 160 MiB of video memory. Perhaps 2.5 to 5 ms at 1080p on a 3060.

## Verdict

Critic: "a real, working world-space sky-occlusion system built from the shadow casters", with soft contact pools under the Jeep and M113 on overcast close-ups. Elsewhere the frame is barely changed or just a bit darker and greyer; it is not path-traced, and it costs 2 to 4 ms. Worth keeping as infrastructure for the bounce-light experiment (README, phase 4).

## What production needs

- Tree and building casters on by default, and a caster set around the camera regardless of what is on screen (casters beyond about 50 m fade out today).
- Half-resolution deferred evaluation from the G-buffer's depth and normal (`gbuffer.md`), 4 cone steps, culling directions by the normal.
- Per-pixel rotation of the direction set with TAA (`temporal.md`) against the wall blotches.
- A real albedo for opaque casters, so the bounce can carry colour.
- An exposure answer, so woods and streets do not just get darker.

## Overclaims the critic found

- Video memory is about 160 MiB, not 85.
- The rounds agree within 0.3 ms only for the final rows; the maps-off rows spread by up to 2 ms.
- The cost range left out the pine woods (+3.9 ms).
- "Trunk bases darker" is not visible: trunks count as foliage receivers and skip the maps; the floor dims broadly.
- The "one bounce" from opaque casters uses a fixed grey, so there is no colour bleeding.

## Taste risks

Woods darker and greyer (the bright floor patches compressed). Blotchy streaks on large flat walls. The 1.15 sky gain is a calibration fudge.

## Where the work lives

- Notes and patch: `tmp\research\wildcard\NOTES.md`, `prototype.patch` (988 lines), shaders `glsl_final\`, `ovr_cone\`, `ovr_nofol\`, measurements `r16\measure_remastered.md`, timings `rt7\timing_summary.md`, `rt6\timing_summary.md`.
- Scratch worktree: `gfx-wildcard`, build `dist\gfx-wildcard-rwdi`.
- Best images (not committed): `tmp\research\wildcard\BEST_overcast_close.png`, `BEST_town_street.png` (the wall blotches), `BEST_woods_overcast.png`, `cmp_debug_final.png`, `r14\crop_church_faces.png`, `rd\dolly_overcast_close.png`.
- Integration: not as a look. Keep the idea for the bounce-light bet; if it goes to the new branch `graphics-remaster` the owner asked for on 2026-10-17, it goes as groundwork behind a switch. That branch did not exist yet when this page was written.
