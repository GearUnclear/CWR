# Remastered lighting: change log

One entry per change to the Remastered lighting, oldest first: an id, what changed, the captures that show it, the GPU cost, and the owner's verdict where the owner gave one. The design is in `README.md` next to this file; the October 2026 research run is in `research/README.md`.

"The original look" means the Remaster engine's stock lighting (the "Classic" option of the Lighting row, labelled "Classic" in every comparison picture). Pictures are renders of game data and are never committed; they are referenced by their path on the development machine. GPU costs were measured on one RTX 4060 Ti at 1440x960 internal (960x640 window at 150% render scale) with 4x MSAA, often shared with other test games, so treat them as rough.

Earlier history: the first lighting attempt (Codex, branch `origin/graphics/lighting-improvements`, draft PR #210) kept its own log, `arma_CWR/tests/rendering/ITERATIONS.md` on that branch (entries A to W). This work started from `main` instead and used that log as the list of looks the owner had already rejected: blue haze or a wash, mustard or khaki ground, crushed shade on vehicles and soldiers, flattened tree crowns, results that are only darker, and shadow-map darkening that crushed the shade.

## Summary

| Id | Date | Commit(s) | Change | Owner verdict |
|---|---|---|---|---|
| R1 | 2026-10-17 | 77ea85d | Remastered lighting mode (per-pixel sun and sky, shadows, terrain and cloud shadows, post pass) | none (lab only) |
| R2 | 2026-10-17 | d035650 | GPU timestamp timers, live shader reload | none |
| R3 | 2026-10-17 | 2c7339f | Ground bounce takes the colour of the ground below | none |
| R4 | 2026-10-17 | 7536557, fda57e9, 63123e8 | Three rounds of adversarial review fixes | none |
| R5 | 2026-10-17 | f8ba59e | Evenings take the original look's sunset hue | prompted by the owner: evenings looked "about 5600 K" |
| R6 | 2026-10-17 | 29ceed5 | clang-format only | none |
| V1 | 2026-10-17 | bf69b6b, 3c53141 | Scripted capture run; **first in-game validation** | "marginally better, at most"; water "so much less defined" |
| D1 | 2026-10-17 | none | Owner direction: "RTX on"; interview decisions | (owner's own direction) |
| W1 | 2026-10-17 | 935cb5e (branch `claude/remaster-water-research`) | Sea wave study | not yet judged |
| X1 | 2026-10-17 | none (research only) | 22-track research run with critics | not yet judged; owner asked for bug fixes first, then this documentation |
| B1 to B10 | 2026-10-17 | 874638c to 72f32d4, 84408f8 | Bug fixes found by the research | not yet judged |
| B11, B12 | 2026-10-17 | none | Tree shadows at start-up, MSAA by default: built, **not landed** | none |
| P1 | pending | none | New branch `graphics-remaster` with the ideas that worked; a side-by-side video in motion | requested by the owner |

Dates are as git records them: R1 to R5 were committed in UTC, everything after in local time (UTC-7). Owner quotes are verbatim except for one corrected typo ("marginally").

## R1. Remastered lighting mode (77ea85d)

**What changed.** A second lighting model for the GL 3.3 backend, behind a new Options row (Lighting: Classic or Remastered). The original look sums sun and ambient (times 1.6 brightness) and clamps to 1 before texturing, so the sunlit and shaded sides of most surfaces both clip to bare texture colour. Remastered lights per pixel in linear light and tonemaps per pixel (Khronos PBR Neutral without its toe):

- a light rig (`LightRig.cpp`): sun colour by sun height, a sky hemisphere with a warmer sunward horizon, an exposure meter that keeps sunlit ground at the original look's brightness by day, a twilight cap;
- shadow maps that dim only direct sun, following the true sun down to 4 degrees;
- a terrain horizon map built off-thread (hill shadows at any range, valley sky occlusion);
- drifting cloud shadows whose mean equals the original overcast dimming;
- foliage lit with a crown ellipsoid normal; fog that takes the sky's sun glow; water that reflects the sky; local lights per pixel;
- a post pass: half-resolution AO on the sky-light share only, bloom from near-white, screen-space sun shafts.

Also fixed the shadow depth pass restoring the wrong framebuffer on its first run.

**Captures.** None in game: the full build needed vcpkg downloads that were not reachable where it was written. Validated in the offline lab (`tests/rendering/lighting_lab`, engine GLSL on Mesa llvmpipe with procedural stand-in scenes) and by `test_remaster_lighting.cpp`.

**GPU cost.** Not measured.

**Owner verdict.** None on R1 itself.

## R2. GPU timers and live shader reload (d035650)

**What changed.** Timestamp queries around the frame, the world pass (with the shadow pass inside) and each post step, read back late so nothing waits (`EngineGL33_GpuTimer.cpp`, `Graphics/Core/GpuTimings`, the Lighting tab's *Measure GPU time*, `triGpuTimes`). `CWR_SHADER_OVERRIDE_DIR` now covers the post shaders and the split Remastered pieces, and *Reload shaders* rebuilds every program without a restart.

**Captures, GPU cost, owner verdict.** None; this is tooling.

## R3. Ground-coloured bounce (2c7339f)

**What changed.** The light bounced off the ground took one near-neutral colour everywhere. The horizon-map bake now reads the average colour of the landscape texture at every terrain vertex and turns it into a luminance-neutral tint: warm shade on sand, green over grass, never darker or brighter.

**Captures.** Lab only. **GPU cost.** Rides in spare channels of an existing texture; no new texture unit. **Owner verdict.** None.

## R4. Review fixes, three rounds (7536557, fda57e9, 63123e8)

**What changed.** From adversarial reviews:
- the HUD, map and dialogs dimmed to shadow darkness when the camera stood in shade (the original shadow test was left armed after the world pass);
- a soft reset leaked every program and lighting resource;
- bloom and shafts leaked over cinema borders and the gunner optics mask;
- roads and roofs slid their shadow edges metres off at a low sun;
- a cockpit's depth was fed to AO and shafts;
- a `pow` NaN; MSAA changing cutout AO;
- long low-sun shadows clipped by the cascade depth range (now depth-clamped);
- the horizon build pegged every core and could block the main thread;
- night vision lost its sun shadows;
- a failed Remastered build fell back with the wrong shadow state;
- a shader reload on a 2D-only frame; constant-buffer reuse after a reset;
- the ground tint sat half a texture square off.

**Captures.** Lab only. **GPU cost.** None measured. **Owner verdict.** None.

## R5. Evenings take the original's sunset hue (f8ba59e)

**Owner verdict that prompted it** (roadmap log): evenings looked "about 5600 K next to the original (the vehicle light most of all)".

**What changed.** The original look tints its sun with an art-directed sunset colour applied in display space; the Remastered sun colour is linear, so the same evening showed far less red. Below 16 degrees (in full by about 7) the rig's sun now takes 0.65 of the original sunset hue, taken to linear (`EveningSunChroma`). The full hue overshot into copper. 14 degrees and above are unchanged, so olive textures do not turn mustard.

**Captures.** Lab golden hour: the sunlit APC hull went from red/blue 1.54 to 2.06 against the original look's 1.99 (test view) and 1.56 to 2.19 against 2.29 (close-up).

**GPU cost.** None (computed once per frame). **Owner verdict on the fix.** None recorded.

## R6. Formatting (29ceed5)

clang-format on five files. No logic change.

## V1. First in-game validation (bf69b6b, 3c53141; run 2026-10-17 13:53 to 13:55 local)

**What changed.** `tests/rendering/captures/` shoots matched original-look and Remastered pairs on the Remaster data (Abel, Eden, Sinai with @LoBo) at 960x640, 4x MSAA, 150% render scale, records `triGpuTimes` per mode, and builds pairs, a contact sheet and a timings table. `smoke_*` tests toggle the lighting both ways. 3c53141 lets each run take its own data junction and a shader override directory. The views: `lowsun_fp` (the owner's acceptance view stand-in: Jeep, M113, soldier, camouflage net, 08:00 clear), `sunrise_ridge`, `noon_fields`, `forest_edge_golden`, `coastline_sunset`, `town_night`, `firefight_1` to `_3`, `tank_gunner_interior`, `tank_gunner_optics`, `overcast_broken`, `overcast_heavy`, `sinai_lowsun_fp`, `sinai_desert_noon`.

**Captures.** `D:\Arma_CWA_worktrees\lighting\tmp\captures\20261017-final\` (`<view>__pair.png`, `contact_sheet.png`, `timings.md`); earlier attempts in `smoke-1`, `run-1`, `run-2-abel`, `zoom`. Both modes worked on all three islands with no shader build failures. The terrain horizon map built in 607 to 639 ms per island. One low-severity GL message (131204, texture 0 on unit 2) per island, later fixed in B3.

**GPU cost** (one run, frame ms, original look against Remastered, from `timings.md`):

| View | Original look | Remastered | Post pass |
|---|---|---|---|
| lowsun_fp | 3.80 | 4.90 | 0.83 |
| noon_fields | 6.04 | 8.23 | 0.39 |
| forest_edge_golden | 3.56 | 4.73 | 0.46 |
| coastline_sunset | 2.93 | 4.44 | 1.79 |
| town_night | 5.63 | 5.57 | 0.54 |
| sinai_lowsun_fp | 1.92 | 4.47 | 1.13 |
| sunrise_ridge | 7.17 | 18.44 | 0.75 |

The `sunrise_ridge` gap is a measurement artefact: the original-look shot was taken while the adaptive object detail was still held coarse after the camera jump (found by the perf track, root-caused and fixed in B1).

**What the pictures were later found to show wrongly.** Every vehicle in these pairs was spawned by script and so drew with the original look's lighting (fixed in B8); trees and buildings cast no sun shadows (B11, not landed); a clear sky could still drop a cloud shadow (B4); the first shot of a view could be at coarser detail (B1).

**Owner verdict** (2026-10-17): "these are marginally better, at most?" and, about the sea, "the water is so much less defined looks so smooth".

## D1. Owner direction (2026-10-17)

Not a change; it set the bar for everything after.

- "aim for the stars we want this to be like RTX on haha maybe not raytracing but you see the vision. a remaster without changing model or texture"; "dont be afraid to try and fail".
- "'classic' engine code is no longer used in this project, we target the remaster engine exclusively".
- Interview answers: the new lighting **replaces** the original look (no switch; the original stays only as the before picture); target **an RTX 3060 class card at 60 fps in 1080p**; **all four areas** matter (light and shadow, sky and air, water and shine, night and combat); results are judged on **side-by-side pictures**.

## W1. Sea waves (935cb5e, branch `claude/remaster-water-research`)

**What changed.** Research only: shader overrides that derive a wave normal from the shipped sea texture and add a Fresnel sky reflection and sun glints (variants A, B, C; C is the pick). The study found the sea is drawn by the terrain detail shader, not the water shader. Full write-up: `arma_CWR/tests/rendering/water_research/README.md` on that branch; summary in `research/water.md`.

**Captures.** `D:\Arma_CWA\tmp\water-research-images\` and `D:\Arma_CWA_worktrees\lighting\tmp\research\water\images\` (`overview_all.png`, `triptych_<view>__C.png`). Wave detail at sunset 3.0 today against 23.7 with C (original look 3.7).

**GPU cost.** Estimated under 0.1 ms; not measurable on the shared GPU.

**Owner verdict.** Not yet judged. The owner asked for it on its own branch "with good docs to review later". Known risk: noon water is cooler and bluer than the original look's green-yellow.

## X1. Research run: 22 prototypes with critics (2026-10-17)

**What changed.** No production code. 22 tracks each built a prototype in a scratch worktree on the real Remaster data; an independent critic verified every claim and scored each track for "worth building" and "RTX-on". Paused once at the owner's request ("pause all verifiers") and resumed ("gogo").

**Captures.** Per track in `D:\Arma_CWA_worktrees\lighting\tmp\research\<track>\`; one picture per track in `research\gallery\` and all of them in `research\gallery\SHEET.png`.

**Results.** The ranked table, the failures and the road to RTX-on are in `research/README.md`. Top three: combat light (7 and 6), low-sun shade fixes (7 and 2), frame cost (7 and 2). Bounce light failed in four tracks. Four bugs made every current picture wrong.

**GPU cost.** Per track (rough, shared GPU); the budget picture is in `research/README.md` section 9.

**Owner verdict.** Not judged on looks yet. The owner's response: "Fix bugs with opus agents first then do this: Document all of the findings. Compile all ideas that worked into a new branch graphics remaster and have an opus agent make a video with a side by side in motion comparison of each approach".

## B1 to B10. Bug fixes (874638c to 72f32d4; roadmap log 84408f8)

Each bug got a fix agent in a scratch worktree, an independent reviewer and an integrator. Each fix has a unit test that fails without it and passes with it. Details, root causes and evidence: `research/bugs.md`.

| Id | Commit | Change | Visible effect |
|---|---|---|---|
| B1 | 874638c | Capture rig pins object detail, settles each mode, resets lighting knobs, logs draws; `setDate` syncs street lamps like `skipTime` | Before and after pairs now draw the same objects |
| B2 | d57b712 | No NaN lighting on leaves without a mesh normal | Affected crowns gain a lit and a shaded side (about 2 levels) |
| B3 | f102499 | The shadow-map unit always holds a complete depth array | None (removes GL undefined behaviour) |
| B4 | 5cf85cf | A clear sky casts no cloud shadows | `forest_edge_noon` 74.5% of the frame no longer under an invisible cloud |
| B5 | 463506f | Sun shafts brighten the air, not the surfaces near the eye | Near-vehicle veil +36 to +4 levels; the warm near-ground beam is gone too |
| B6 | d80f868 | Instanced runs keep each member's terrain fit | None on Abel today (instancing is off there); exact once instancing runs |
| B7 | ab61aa5 | `switchLight` on a non-lamp no longer crashes | None |
| B8 | 747ffc8 | Script-spawned vehicles get the Remastered lighting | Spawned vehicles match placed ones pixel for pixel; noon vehicle sides darker (the old ones were lit with the original look by accident) |
| B9 | 4515770 | The night takes the original look's local-light clamp and a real moon | Night town at the original look's brightness, the yellow-green gone, moonlit country a readable cool grey |
| B10 | 72f32d4 | Three-entry config colours read as `{r,g,b}` | Street lamps warm, their wash over the country near towns gone (also in the original look and night vision) |

**Captures.** Per bug in `D:\Arma_CWA_worktrees\lighting\tmp\bugfix\<bug>\`. The full set after all fixes: `D:\Arma_CWA_worktrees\lighting\tmp\captures\20261018-bugfix\` (pairs, `contact_sheet.png`, `timings.md`, `cmp_vs_20261017-final.txt`, `vs_20261017-final\<view>__stack.png`). Every mode settled; 131204 and 131222 no longer appear (only GL performance and info messages remain); `town_night` Remastered lower-frame brightness 157.5 to 121.4 (original look 120.4).

**GPU cost.** B2, B5 and B9 measured no change beyond the shared-GPU noise (a few shader instructions). B3, B4, B7 and B10 add no per-pixel work (a bind at init, a per-frame constant, or CPU-side logic). B6 adds one virtual call per object (not measured). B1 makes captures pin the finest object detail, so long-view timings are higher than in older runs. B8 costs about 1.2 to 1.4 ms once, on the frame after a burst of 8 new vehicle classes, and afterwards spawned vehicles cost what placed ones cost. After all fixes, for example: `lowsun_fp` Remastered frame 3.77 ms, post 0.71 ms; `noon_fields` 7.66 ms, post 0.40 ms (single run, finest object detail now pinned, so not comparable with V1).

**Owner verdict.** Not yet judged.

## B11, B12. Tree shadows at start-up, MSAA by default (built, not landed)

- **B11:** trees, bushes and buildings cast no sun shadows in any fresh profile, because the graphics settings are applied before the world exists. A fix was built and tested (scratch worktree `fix-object-shadows`) but not landed: it also applied the terrain grid tier at boot, which changes physical terrain.
- **B12:** the Remastered default ships with no anti-aliasing (`msaaSamples = 0`). A fix (default 4x, a config migration) was built and tested (`fix-msaa-default`) but not landed: its migration also added MSAA on top of a supersampling scale the user had chosen.

Second attempts were in progress in `D:\Arma_CWA_worktrees\fix2-object-shadows` and `fix2-msaa-default` (uncommitted) when this log was written. Details: `research/bugs.md`, items 10 and 11.

## O1. The photoreal overhaul (8476b9b to f4d57cb, 2026-10-19)

The owner found the integrated features (A0 to A7, B1 to B11 on `graphics-remaster`) too subtle ("nearly no difference") and asked for a real visual overhaul in a modern photoreal direction, judged at thumbnail size against the vanilla upstream build. It was built in detached scratch copies by parallel workflows (lanes light, fog and surface, a look director, then fixer rounds with critics) and landed as five commits, one per round:

| Commit | Round | What it brings |
|---|---|---|
| 8476b9b | 0 to 2 | Metered camera with a hue-keeping filmic curve (`photoLight`), froxel volumetric fog that replaces the stock fog and B8's air (`volFog`), GTAO and surface light (`surfaces`); AI sight follows the drawn fog; a GL bind-cache fix also covering B6 and B8 |
| 2c957bb | 3 | The owner's wet-ground seam fixed at the root (a continuous ground-colour field instead of per-square albedo), dusk combat light without horizon strips or muzzle pools, grey-day air without a plate or halos, a clear near field at noon |
| 1fcc8a4 | 4 | Black-body fireball glow, twilight-scaled combat light, a blue noon sky, a white-hot sun disc, lighter mist, the AI fog medium computed in the simulation |
| 46b47a0 | 5 | Sun-in-frame metering, speck-free mist under sights, dawn haze by the phase function, particles lit by the photoreal rig, AI link end-to-end tests; the owner's rain rule: only puddles reflect (option 4 of a six-way puddle study) |
| f4d57cb | explosions | Explosions scale with the munition's charge from CfgAmmo (detonation physics, option 1 of a six-way study): a brief white-hot flash, a lingering dark earth column, fuel fires as sooty deflagrations |

**Owner decisions.** The overhaul is the final look, with nothing kept from the as-landed column (three-way sheet, round 3). There is no toggle requirement: deep changes that cannot be switched off are fine. AI vision must follow the rendered fog. The vanilla side of every comparison comes from a build of the vanilla upstream repo. Rain: only puddles may reflect.

**Checks.** Each round: unit gate tags all green (round 5: 331 cases), the full capture set shot four games at a time, two critics (realism at thumbnail size; artefacts and cost), both "ready for the owner" after round 5. Cost on an RTX 4060 Ti at 1080p, 4x MSAA, quiet machine: +1.52 ms mean over 21 views (round 3: +1.45 ms); the low-sun day views grew about 0.3 to 0.5 ms (dawn haze phase).

**Known leftovers.** The light mist is paler than vanilla and greys its trees; a rainy day reads more misty than rainy at thumbnail size; the low sun is a small white disc rather than a large glare; the HE flash is one or two frames (physically right, a pop in motion); the night fuel soot reads pale; the ground field arrives about 0.6 s after an island loads, so the old seam can flash once; script-fired rockets (LAW, RPG) never armed in the capture tests.

**Where the evidence is (not in git).** `graphics-remaster\tmp\look\`: `director\` (sheets and notes per round, `round2` to `round4`), `fix`, `fix4`, `fix5` (fixer notes and patches), `puddle\1..6` and `boom\1..6` (the two six-way studies), `critic_r3_*` to `critic_r5_*`, `ref\vanilla` and `ref\landed` (the reference shots), `owner\` (everything shown to the owner).

## V2. The trailer (2026-10-19)

"Uslu dur! trailer, part 1: graphics": 14 scenes from the Uslu dur world (resistance, occupiers, checkpoints, convoys, the occupied town), each shot twice at 60 fps from one camera path, on a vanilla upstream build and on the O1 build, and composited full size behind a moving divider that sweeps the whole frame; calm dissolves and wipes, Oswald and Roboto text with the HUD green as the only accent, a typed opening line, and a calm female narration (local Breeze TTS, every line checked by transcription). 2:13.4, 1080p60, -16 LUFS. Tooling 111e563; the files are in `graphics-remaster	mpideo2inal\` (not in git). The NPC head flicker in early takes was the AI look-around fast-forwarded by the capture's real time per frame; the fix is capture-only (an exact 1/60 s step and a held gaze).

## P1. Pending

- A clean timing run on an idle machine at 1080p, ideally on an RTX 3060.
- Re-shoot every before picture once B11 and B12 land.
