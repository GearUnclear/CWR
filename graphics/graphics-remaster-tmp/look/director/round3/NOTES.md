# Look director: the combined photoreal look, round 3

Scratch copy `D:\Arma_CWA_worktrees\look-director`, detached at **fec1075**, nothing committed, no branch.
Round 3 merges the five fixers (`LOOK\fix\seam`, `horizon`, `plate`, `veil`, `fire`) over round 2
(`fix\base.patch`) and adds one merge fix of its own. `combined.patch` is `git diff --cached --binary fec1075`
after `git add -A arma_CWR` (105 files, +16,744 / -266). It applies with `git apply --cached --check` on a
fresh index of fec1075 and gives the same tree as the scratch copy's index (d2f866e4). Build
`arma_CWR\build\look-director-rwdi`, game `arma_CWR\dist\look-director-rwdi`.

Round 2 (sheet, shots, notes, patch, cost, metrics) is kept in `round2\`; round 1 in `round1\`.

## Look first

- `THREEWAY.png`: Original (vanilla 3.05) | graphics-remaster as landed | Overhaul, 320 px a panel, 28 views,
  the owner's view `town_night_rain` first. One strip per view at 640 px a panel in `threeway\<view>.png` (39
  strips: the 28, the 1080p copies of the shared views and `optics_mist_sight_f1`). Original and landed come
  from the reference set (`LOOK\ref`, see `ref\NOTES.md`). Every vanilla shot exists, so no panel uses the
  features-off fallback.
- `SHEET.png`: Original (vanilla) | Overhaul at 480 px a panel, the same 28 views in the same order.
- `final\<view>__overhaul.png`: the 50 round 3 shots, with round 2's file names.

## Round 3: cause and fix per cluster

**Seam (town_night_rain).** Cause: B7's wet shader classed each land pixel from its terrain square's own
texture (`wetVegetation(albedo)`, `wetGroundPorosity(albedo)`), and both are steep in colour. Two neighbouring
squares whose textures differ by 2 to 3 levels got different classes, so the film, the darkening and the
puddles stopped at the square's edge. Fix: the land's class comes from a continuous world-space field, the
land's own colour per terrain vertex (`TerrainHorizonMap::groundColor`, uploaded on unit 12 as
`kGroundFieldUnit`). The forward pass and the SSR trace both read it with smoothstep interpolation. The
puddle threshold mixes by `veg * veg`. In the round 3 shot the straight line from the tree base is gone
(checked at full size against round 2).

**Horizon (dusk firefights).** Cause: B4 combat light on the ground. The combat lights used Classic's night
(1 as soon as the sun is down) while the camera still sees twilight. The ball wrap lit far grazing ground up to
5x too much. A burst's flash held its white peak for 0.15 s at dusk. Fix: `CombatNight` (the lower of Classic's
and the camera's night) drives the day scale and the flash response. A horizon-clipped sphere light
(`combatBallCosine`) replaces the wrap on non-foliage surfaces. `CombatTuning::flashHold` scales the hold by
the combat night. The muzzle pools are gone and the fire and burst strips are much shorter. The 99th
percentile of the horizon band (rows 300 to 340) is 95 / 158 / 94 in firefight_1 / 2 / 3, against 115 / 210 /
131 in round 2 (the shots differ in AI timing, see open items).

**Plate (overcast_heavy, rain_day, mist).** Cause: under a grey sky the horizon ring sampled the classic
horizon polygon's warm band, so every far ridge converged to one flat warm colour with a hard seam. The dry
grey haze was too thick (gain 3.5). The composite's GTAO and contact shadow darkened whole fogged pixels (dark
rims under fogged trees). Rain took the full classic extinction, so nothing was left past 1 km. Fix: the ring
starts above 3.8 degrees under a grey sky. The apply replaces the band with the air and sinks the deck into
it. Grey haze gain 1.2 (`kHazeGreyGain`). Rain's weather fog takes 0.5 of the classic extinction
(`kRainFogShare`). The fog apply writes a veil target (the surface's share of the light after the fog), and
the composite applies it to the surface shares before the occlusion. overcast_heavy now fades its ridges in
layers; rain_day's gap is air continuous with the sky.

**Veil (noon_vista, low sun glow).** Cause: the clear haze kept 0.35 of itself at the eye, so 70% of a dark
wood's light at noon was air. The fog injection weighted the ring by its taps' mean confidence, which drew a
pale strip along the screen edge. The dome's single broad Mie lobe drew a flat column from the sun to the
horizon. Fix: `kHazeNearKeep` 0.06, `kHazeNearTo` 3000 m; the ring weight comes from the freshest tap; the Mie
phase is 0.7 CS(0.75) + 0.3 HG(0.97) in the C++ mirror, the dome and the sky table pass. The noon woods are
dark green again and the sunset glow gathers round the disc.

**Fire (town_night fire, golden hour).** Cause: the fire's light on smoke saturated per channel and clipped in
the 8-bit colour (white steam, no flame). The stones were lit by a 0.45 m point light several stops past white.
The golden warm gain was a diagonal gain over the whole frame (teal sky). Fix: `CombatParticleLight` keeps the
light's hue; `KeepHueUnderOne` and `SetDecalGlow` carry the light past white as the puff's glow;
`CombatFlameEmission` restores the flame body; the fire's surface light comes from a 1.3 m body and is deeper
orange; the golden gain (`camGolden`) applies by each pixel's warmth. town_night__1080 fire box: pixels with
every channel at 200 or more went from 1450 (round 2) to 171; plume foot (179,166,153) to (236,193,145).
forest_edge_golden keeps golden land under a blue sky.

## Merge decisions

- Patches applied in the order seam, horizon, plate, veil, fire with `git apply --3way --index`. Three textual
  conflicts, all resolved by hand.
- `EngineGL33_Lighting.hpp`, unit 12: seam renamed the free `kLightDataUnit` to `kGroundFieldUnit` (the world
  programs and the SSR trace read the ground field there, and the fog floor shares it). Plate put the fog veil
  on the same free unit for the composite. I kept seam's name and comments and added
  `kFogVeilUnit = kGroundFieldUnit`. Each post pass (fog floor, SSR trace, composite veil) binds unit 12 itself
  and unbinds it, and the world rebinds the ground field at each frame's lighting upload. Plate's test
  (`kFogVeilUnit == kFogFloorUnit`, not a post unit) and seam's test (`kFogFloorUnit == kGroundFieldUnit`, the
  bind strings) both hold.
- `test_volumetric_fog.cpp`: plate and veil both appended test cases at the end of the file. All are kept.
- `EngineGL33_Lighting.cpp`, the combat block: horizon computes `_combat.night` (CombatNight), sets
  `ct.flashHold` and evaluates with it; fire removed round 2's 3x night smoke gain. The merged block does all of
  that and its comment carries both reasons.
- No two fixers changed the same constant. Plate's `kHazeGreyGain` (grey deck) and veil's `kHazeNearKeep` /
  `kHazeNearTo` (clear air near the eye) act on different terms of the same haze. The grey deck keeps its own
  near air (`kHazeNearKeepGrey`), so both fixers' unit bounds pass together.
- Merge fix (mine): `CombatParticleMix` (CombatLights.hpp/.cpp, called from `Object.cpp CombatLitParticle`).
  The fire fixer made every combat light keep its hue on smoke, but its intent and tests are about a fire's
  light. Now a fire's light keeps its hue (with the tint), and every other combat light (a burst's flash, a
  muzzle, a flare, a tracer) goes back to the per-channel soft saturation that whitens an incandescent core,
  as in round 2. New unit case in `test_combat_lights.cpp`; the source contract test now pins the split.
  Result: it changes little in the shots (firefight_2 burst cloud (183,157,82) before, (175,146,79) after) and
  does not cure the yellow burst cloud (open item 1).
- The AI sight and fog link is intact. `Target.cpp` lines 639 to 646 still multiply `vis` by
  `FogSightTransmittance` while `FogSightActive()`. `FogSightExtinctionAt` and `SetFogSight` in
  `VolumetricFog.cpp` are unchanged by every fixer and by the merge. Rain's new share acts on the rendered
  weather fog only; the AI's weather part stays the classic tactical fog.
- Capture tests: `look_final_*` are regenerated overhaul-only (`gen_look_director.py --modes
  "overhaul=remastered" --tag final`, same views and sim clocks); round 2's classic and overhaul versions are
  in `round2\combined.patch`. The run-captures tests (`abel_lighting`, `eden_lighting`, `sinai_lobo_lighting`)
  and `look_director_optics` are unchanged. New sheet script `tests\rendering\captures\make_threeway_sheet.py`.
  The scratch tests `look_r3_*` (overhaul-only cuts with the Classic block dropped, as in the landed reference)
  and `look_probe_r3_*` are untracked and not in the patch.

## Tests

- Unit gate tags `[remaster],[FeatureSwitches],[FeatureGates],[PostStages],[fog],[SurfaceLight],[AirMedium],
  [ssr],[wet],[CombatLights],[Shaders],[StateCacheAudit],[PhotoCamera],[TerrainHorizon],[StateOwnership],
  [atmosphere]` on the final build, run from D:\: all pass, 319 cases, 957,469 assertions (glslang compiles
  every program).
- Full suite on the final build: 3395 cases, 3385 pass, 4 skipped, 6 fail. The 6 are the known localisation
  cases (`test_equipment_localization`, `test_map_legend_localized`, `test_mp_version_message_localization`),
  which need a `packages\Remaster` junction the scratch copy does not have.
- clang-format is clean on every file the merge touched by hand.

## Capture

Four games at once, each with its own junction (`RemasterUD-look-director-a` to `-d`) and output folder in
`r3_shots\`: (a) the 960x640 Abel set; (b) and (c) the 1080p Abel views in two halves (8 and 7 views, sim
clocks kept); (d) Eden and Sinai at 960x640, `look_final_eden`, `look_final_sinai` and the optics test, five
games in a row. Final run (after the merge fix): wall time **184 s** from start to the last game; (a) 125 s,
(b) 132 s, (c) 133 s, (d) 184 s; every test passed. The first run (before the merge fix, 201 s) is kept in
`r3_shots_try1\` and `final_try1\`. `r3_probe\` holds the firefight probe (the merged build, the merged build
with `combatSmoke=0`, the fire fixer's build, the horizon fixer's build). Only my games were stopped
(run-captures' scoped cleanup; `tri slots` showed none left).

## Open items

1. **The dusk shell burst reads as a flat yellow cloud** in `firefight_2`, visible at thumbnail size. With no
   held white flash at dusk (horizon), the cloud shows the cooled fireball light, about (175,146,79), evenly
   over the whole cloud. In the probe, where the burst is younger (`r3_probe\`), the merged build, the horizon
   build and `combatSmoke=0` all show a yellow-white cloud with white patches, so it depends on the burst's age.
   The per-channel merge fix did not cure it. Likely cause: a burst's light reaches every puff of its cloud with
   no self-shadowing, so the whole cloud glows one colour. A root fix for a fixer: let a burst's light cross its
   own cloud (as `CombatSmokeTransmittance` does for a fire) and fall faster on its smoke.
2. **Firefight timing**: the overhaul-only cut shoots each firefight earlier in the fight than round 2 did
   (round 2 shot Classic first). It matches the landed reference's timing. The round 2 to round 3 firefight
   numbers are therefore indicative only. A faint fire line at the wreck remains at full size (horizon open
   item 1: the dusk ground is metered as dark as night).
3. **Grey-day skies are flat**: rain_day, overcast_heavy and the morning mist read as an almost featureless
   white sky at thumbnail size; the classic cloud texture is gone under the grey veil (round 2 behaviour,
   unchanged here). The mist in the optics views also erases the right-hand tree line.
4. **The low-sun glow is broad** at 960x640: lowsun_fp, sinai_lowsun_fp, sunrise_ridge and dawn_valley carry a
   wide cream glow over the sun side of the sky. It is smaller than in round 2 after the veil fix, but still a
   large warm area at thumbnail size.
5. Seam: for about 0.6 s after an island loads, the land falls back to the per-pixel class, so the old seam
   can flash. Mixed town ground is now evenly wet rather than mottled.
6. Plate: under a clear sky the classic horizon polygon is unchanged. The veil target's cost is not timed.
7. Fire: the flame core reads cream-white with orange edges. The C++ curve mirror still lacks round 2's chroma
   keep.
8. Cost: not re-timed this round. New per-frame work: one ground-field fetch per wet land pixel and trace texel
   (seam), one R8 full-resolution write and one fetch (plate). `round2\cost.md` is the last measurement.

---

# Round 2 notes (carried over; round 2's SHEET.png, final\, cost.md and metrics.txt are now in round2\)

Scratch copy `D:\Arma_CWA_worktrees\look-director`, detached at **fec1075** (still the graphics-remaster
tip when this round ended). Nothing committed, no branch. `combined.patch` is
`git diff --cached --binary fec1075` after `git add -A arma_CWR` (96 files, +15,429 / -191);
`git apply --cached --check` passes on a fresh index of fec1075. Build `arma_CWR/build/look-director-rwdi`,
game `arma_CWR/dist/look-director-rwdi`, junction `RemasterUD-look-director`.

Round 1 (sheet, shots, notes, patch, cost) is kept in `round1\`.

## Look first

`SHEET.png`: 28 Original | Overhaul pairs at 480 px a panel: the full `run-captures.ps1` set (15 views,
960x640 at render scale 1.5), the lanes' 11 views at 1920x1080, and a rifle in morning mist (HUD and
iron sight). Full-size shots in `final\` (`<view>__classic.png` / `__overhaul.png`; 1080p copies of shared
views are `<view>__1080__*.png`). `r2\compare_<view>.png` shows Original | Round 1 | Round 2 for the
views the critics blocked; `r2\hud_proof.png` crops every HUD element in both modes.

What the owner should look at first:
1. `r2\compare_rain_day.png` and `r2\compare_optics_mist.png`: rain and mist now fade the land with
   distance into the weather's air. The cause was a real bug, see "The fog's air was the wrong colour".
2. `r2\compare_firefight_2.png`: the dusk firefight is now a dark plain under a dusk sky.
3. `town_night` fire (`r2\compare_town_night.png`): an orange flame and a fire-lit plume, no white slab.
4. `r2\compare_pine_rays.png`, `compare_woods_pine2_evening.png`: shade keeps its green and brown.
5. `forest_edge_golden`, `overcast_broken`, `noon_fields`, `tank_gunner_interior`.

## What the look is

Modern photoreal, one camera over everything:
- **Light and camera** (`photoLight`): scene light and a metered camera with eye adaptation, a filmic
  hue-keeping curve, a daylight white balance that follows the light a little, a physical sun and sky, a
  dark night with lamps and fires, a weather veil for grey days. Round 2 adds time-of-day exposure
  (golden hour darker and warm, high clear sun 0.25 EV under, dusk exposed for the sky) and a curve that
  rolls saturated light off in hue (flames stay orange).
- **Air** (`volFog`): a froxel volume for haze, aerial perspective, weather fog, rain air, valley mist,
  sun beams, lamp halos and night glow, now also under weapon sights and optics.
- **Surfaces** (`surfaces`): GTAO with temporal accumulation, canopy occlusion, a deeper clear-day shade,
  specular, texture relief and small cavities, interior occlusion; relief and cavity eased at a low sun.

## Round 2: every critic point and what was done

Numbers are from `metrics.txt` (my numpy pass over `final\`, the critics' regions where they gave them).

**The fog's air was the wrong colour (the root of the rain and mist failures).** `PostFog`'s shared
binder did raw `glActiveTexture` binds and then `BindSkyTable()` through the GL bind cache, whose active
unit was stale: the sky table landed on unit 13, the horizon ring's, and the ring's sampler read a sky
table texel. So the air's colour in every fog column was never the sky above the land, and in rain or
mist (where the air is most of what you see) far land converged to a dark slate. Fixed by invalidating the
cache and binding the sky table first (`EngineGL33_PostFog.cpp`). The same raw-bind-then-cache pattern
was in B8's air apply (`EngineGL33_PostAir.cpp`) and B6's SSR trace (`EngineGL33_PostSsr.cpp`); both get
the same one-line guard. This is a bug on the graphics-remaster branch itself (B6, B8), not only here.

- **rain_day**: right hill (1 to 2 km) luma 66 in round 1, now **182** against the 211 sky band; the
  gap plate now carries the same distance gradient (the ring samples just above the land under a grey
  sky, and the deck above the fill sinks into the weather's air over about 5 degrees, so no hard top
  edge); rain haze no longer stacks a grey deck's haze times 3 on top of rain's own fog (the near hill
  300 m away keeps its shape). Rain streaks: the overlay draws at 0.45 of its density under the camera
  (`SceneDraw.cpp`).
- **optics_mist_hud / optics_mist_sight**: setFog is 2.8x denser in the volume (`kSetFogBoost`), so
  the mist is a fog that thickens with distance: the right-hand hill fades out, the near ground stays.
  The fog now also runs under a screen overlay: the world is copied before the first overlay of the
  frame (`SnapshotWorldBeforeOverlay`), the fog applies to that copy and the overlay's darkening (and any
  reticle or brackets brighter than the world) is laid back over it. No pop when aiming:
  `optics_mist_sight` (rifle at the eye) and `optics_mist_hud` show the same mist. The test now arms the
  same rifle in both modes and records each shot (HUD state matched).
- **Dusk firefights**: bottom-band luma **26 to 28** (Original 17, round 1 44 to 63), sky over the
  horizon 79 to 89: the ground is about a third of the sky. Three pieces: the twilight's sky fill drops
  45% from 4 degrees of sun to the horizon (`DuskFillScale`), the camera keeps 0.62 of a dark scene's
  darkness around sunset instead of 0.4 (`CameraDuskKeep`), and combat light is reworked under the
  camera (`EngineGL33_PostCombat.cpp`): a muzzle flash lights 2.5 m at 0.05 of its power (it was the
  flat ellipse under every rifleman), bursts end at 20 m at 0.3, the fire at 20 m, all warmer. The fog's
  lamp and muzzle halos follow the camera's night, not the classic night effect (yellow halos in the
  dusk air are gone).
- **Fire** (town_night): 255-white pixels in the fire box **0** (round 1: 547). The curve's whitening
  is scaled by 1 - 0.8 chroma^2 (`kChromaKeep`), the night's extra additive gain is gone, and the fire
  lights its own smoke 3x and tints it with its hue near the flame (`CombatTuning::smokeTint`), so the
  plume is fire-lit orange at its foot and grey above. Fire light on the ground: R-G 5.8, R-B 18 (warm,
  not olive). The R-G 30 asked for is not reached on that grass (texture is grey-green); see problems.
- **Frost-grey shade**: ground R-B pine_rays **14.7** (classic 26, round 1 2), woods **21.3** (40, 7),
  sinai_lowsun **20.9** (10, 2), lowsun_fp **22.2** (13.5, 7). Changes: dark-tone desaturation 0.88 to
  0.97, the low-sun cool shade 0.55 to 0.04, the sky fill 75% toward grey at equal luminance
  (`kPhysicalSkyWhiten`), white balance follow at high sun 0.55 to 0.35. pine_rays and woods are at 56%
  and 53% of the classic's warmth, just under the 60% asked.
- **Sunrise and dawn foreground stipple**: relief and cavity keep 0.3 of themselves under 4 degrees of
  sun, all from 14 (`kSurfLowSunGrain`); sunrise foreground is a warm dark olive (R-B 16, luma 41).
- **forest_edge_golden**: grass (129,107,73) against round 1's (151,131,101); sky (114,138,159)
  against (119,149,193). The sun's aerosol is 0.13 (was 0.08), the golden hour takes 0.7 EV off and a
  warm balance (1.1, 1, 0.76) from 5 to 16 degrees, the deeper low-sun blue only under 14 degrees.
- **dawn_valley**: the murky band under the horizon is gone: luma 185 under a sky of 174 (round 1:
  about 90 under 116). The far surface under the horizon is lighter and its air thicker; valley mist
  2e-3 (was 3e-3) and the forward lobe 0.2 (was 0.3) keep the sun side from one wash.
- **overcast_broken**: sky saturation **0.243** (Original 0.241, round 1 0.106). The weather veil now
  starts at a sky-through of 0.62 (full at 0.3), so setOvercast 0.5 keeps its blue gaps and sun;
  overcast_heavy and rain still get the grey day.
- **Noon**: noon_fields foreground luma **136** (Original 163, round 1 162), sinai 134 (170),
  jeep_close_noon 154 (167; its view is mostly near-white ground with the Jeep). A bright scene is now
  exposed under the average grey (`kBrightKeep` -0.45) and a high clear sun takes 0.25 EV more.
- **town_noon / foliage**: grass saturation 0.398 against 0.362 (1.1x, under the 1.3x cap; camSat 1.15
  to 1.05, the low-sun saturation drop halved); plaster (90,85,72), R-B 18 against the classic's 15.
- **Sky hatching**: the composite's dither is now hashed white noise, half an LSB, turned per frame;
  the fog writes a half-float target (no 8-bit store and no dither before the curve); the dome adds
  half an LSB of hashed noise. 3 px high-pass std in the 1080p coastline sky 1.22 (round 1) to 0.49;
  the 960 px sunrise and coastline skies 0.42 and 0.50 (Original 0.24 and 0.21).
- **HUD**: `r2\hud_proof.png`. The gunner's optic: compass, tank icon, ammo line and action menu are
  the same pixels in both modes (mean colours identical); the target brackets are present, 8 to 25
  levels dimmer where they are blended over a darker scene. The interior's HUD and the rifle's weapon box
  are present and within a few levels. The optic shot was re-staged: each mode puts the player back in the
  seat and waits 400 frames, so the action menu and brackets are in both (round 1 had them in one only).
- **Tank interior**: luma 66 against 98 (round 1: 89). The camera grades a vehicle interior: half a
  stop down, a deeper toe, double vignette (`camDark.w`, the composite).
- **Night cost**: at night under the camera the fog runs at quality 0 and GTAO takes 1 slice.

## Cost (`cost.md`)

RTX 4060 Ti, 1920x1080, render scale 1, 4x MSAA, TAA. Tip = `remastered,photoLight=0,surfaces=0,volFog=0`,
Overhaul = `remastered`; two interleaved rounds per view in one game; run with `run-timing.ps1`, whose
GPU sampler logged **0 of 153** samples with a foreign PoseidonGame. "3060 est." is 1.4x the 4060 Ti
overhaul frame (a rule of thumb, not measured; there is no 3060 here). The final rain haze tweak came
after this run (a constant, no new pass).

| view | GPU frame tip -> overhaul (ms) | delta | 3060 est. |
|---|---|---|---|
| noon_fields | 9.37 -> 10.96 | +1.60 | 15.3 |
| lowsun_fp | 8.73 -> 10.52 | +1.78 | 14.7 |
| forest_edge_golden | 7.13 -> 8.98 | +1.85 | 12.6 |
| sunrise_ridge | 10.26 -> 10.37 | +0.11 | 14.5 |
| coastline_sunset | 5.72 -> 6.03 | +0.31 | 8.4 |
| woods_pine2_evening | 11.27 -> 12.50 | +1.23 | 17.5 |
| blue_hour_coast | 5.24 -> 6.11 | +0.87 | 8.5 |
| dawn_valley | 30.69 -> 33.33 | +2.64 | 46.7 |
| pine_rays | 6.33 -> 7.33 | +1.00 | 10.3 |
| town_night | 11.44 -> 12.88 | +1.43 | 18.0 |
| town_fog_night | 12.97 -> 14.10 | +1.13 | 19.7 |
| town_night_rain | 13.18 -> 14.83 | +1.65 | 20.8 |
| jeep_close_noon | 7.20 -> 8.87 | +1.68 | 12.4 |
| town_noon | 7.20 -> 9.11 | +1.91 | 12.7 |
| forest_morning | 6.58 -> 7.89 | +1.32 | 11.0 |
| overcast_broken | 10.05 -> 11.52 | +1.46 | 16.1 |
| overcast_heavy | 9.52 -> 11.45 | +1.93 | 16.0 |
| rain_day | 6.58 -> 8.20 | +1.61 | 11.5 |
| noon_vista | 28.49 -> 30.36 | +1.86 | 42.5 |
| sinai_lowsun_fp | 6.38 -> 7.00 | +0.62 | 9.8 |
| sinai_desert_noon | 7.66 -> 9.08 | +1.42 | 12.7 |

Per stage (cost.md): world +0.2 to +1.2 ms (surface terms), ao 0.1 to 0.35, air (fog) 0.4 to 1.0,
composite +0.1. The night gates cut ao at night from about 0.35 to 0.21 ms; the night views' frame delta
is still +1.1 to +1.7 ms, and the night town views are over 16.6 ms at the 3060 estimate (18 to 21 ms)
mostly because the tip itself is 11.4 to 13.2 ms there. dawn_valley and noon_vista are over budget at
the tip already (world pass 27 to 31 ms).

## Knobs and defaults (A1 registry; `triSetLighting "remastered,<key>=<v>"`)

Switches (all default 1): `photoLight`, `volFog`, `surfaces` (plus every landed A/B feature).
Changed this round in bold.

| knob | default | note |
|---|---|---|
| camEv | 0.6 | exposure compensation, EV |
| camAdapt | 1 | eye adaptation, s |
| camCurve | 0 | hue-keeping filmic |
| camContrast | 1.2 | toe |
| camShoulder | 0.95 | |
| camWhite | 32 | |
| camSat | **1.05** (was 1.15) | scene saturation |
| camWb | 0.45 | white balance follow |
| camSky, camSkyHue | 1, 1 | |
| camNight | -3.8 | EV at full night |
| camVignette | 0.18 | doubled in vehicle interiors |
| camBloom | 1 | |
| camHaze | 6 | sky table haze |
| camScotopic | 1 | |
| camAdditive | 1 | |
| camAerial | 6 | inert while volFog runs |
| camLocal | 2.5 | |
| fogDensity, fogQuality, fogMist, fogSun, fogLights | 1 each | quality drops to 0 at night under the camera |
| surfAo 3, surfAoRadius 4, surfCanopyAo 1.0, surfSkyLight 0, surfSkyTint 0.25, surfSkyLevel 0, surfSpec 1, surfContact 1, surfRelief 1.5, surfShade 0.65, surfCavity 2 | unchanged | |
| combatFlash | 0.6 | combatFire, combatSmoke, combatScatter 1 |

Constants changed this round:
- PhotoCamera.hpp: `kBrightKeep` 0.15 -> **-0.45**; new `kDuskDarkKeep` 0.62 with `CameraDuskKeep`;
  new `kCameraGoldenEv` -0.7, `kCameraGoldenWarm` (1.1, 1, 0.76), `CameraGolden` (5 to 16 degrees,
  gone by 32); new `kCameraHighSunEv` -0.25 (35 to 55 degrees, not under the veil);
  `kWhiteBalanceShareHighSun` 0.55 -> 0.35; `kLowSunSaturationDrop` 0.2 -> 0.1; `kCameraShadeCool`
  0.55 -> 0.04; `kCameraDarkSaturation` 0.88 -> 0.97; `kCameraAdditiveNight` 1.35 -> 1.0; new
  `kChromaKeep` 0.8 (in the composite's curve).
- LightRig.hpp: `kPhysicalSkyWhiten` 0 -> **0.75**; `kSunAerosolTau` 0.08 -> 0.13 with
  `kSunAerosolMaxAirMass` 14; `CameraWeatherVeil` from sky-through 0.62 (none) to 0.3 (full), was 0.9 to
  0.6; new `DuskFillScale` (`kDuskFillCut` 0.45 from 4 to 0 degrees, back by -9).
- VolumetricFog.hpp: `kFogLobeGain` 0.3 -> 0.2; `kMistSigma` 3e-3 -> 2e-3; new `kSetFogBoost` 2.8;
  `kNightFogBoost` 1.5 -> 1.0 (the boost above carries it); rain haze `(1 + 3.5 grey (1 - 0.7 rain))(1 +
  0.5 rain)` (was `(1 + 3.5 grey)(1 + 2 rain)`).
- EngineGL33_PostFog.cpp: `kFogNightAirShare` 0.18 -> 0.35, `kFogGlowGain` 1.6 -> 2.6, `kFogGlowWide`
  0.45 -> 0.7; fog output RGBA16F; overlay snapshot; night quality gate.
- SurfaceLight.hpp: new `kSurfLowSunGrain` 0.3.
- Combat light under the camera (PostCombat): muzzle 0.05 x, 2.5 m; tracer 0.5 x; burst and cannon
  0.3 x, 20 m; fire 20 m; all warmer at equal luminance. Smoke lit 3x and tinted (0.85) at night.
- PostStages.cpp: the fog (only the fog; B8 and the glow keep their rule) runs under a screen overlay.

## Tests

Unit gate tags (`[remaster],[FeatureSwitches],[FeatureGates],[PostStages],[fog],[SurfaceLight],
[AirMedium],[ssr],[wet],[CombatLights],[Shaders],[StateCacheAudit],[PhotoCamera]`): 301 cases, 953,805
assertions, all pass (glslang compiles every post program). Full suite: 3381 cases, the 6 known
localisation cases fail (no `packages\Remaster` junction in the scratch copy), as in round 1. Updated
expectations: the veil's thresholds, the fog's optics gate (runs with an overlay), the setFog density
bound, the mist density bound, rain's air. clang-format applied to the changed lines.

Capture changes: `abel_lighting.test.sqf` re-seats the gunner in each mode before the optic shot (same
HUD state); `look_director_optics.test.sqf` arms the rifle in each mode, records each shot, and formats
its labels with a counter (`_forEachIndex` did not format in this SQF dialect, so round 1's labels
were broken). Probe tests used while tuning are in the scratch copy's `tmp\probe_tests\`, not in the
patch.

## Known problems

1. **Firefight horizon**: a thin warm line remains along the ground at the burning wreck and, in
   firefight_2, under the shell burst (the fire's and the burst's 20 m of light on the plain behind them,
   seen edge-on). Much shorter than round 1's strip, still visible at 1080p.
2. **Fire light on grass** reads warm (R-B 18) but R-G is 5.8, not the 30 asked: the lit ground there is
   grey-green, and pushing the hue further turned it khaki in the B4 research.
3. **Shade warmth** in pine_rays and woods is 56% and 53% of the classic's R-B, just under the 60% asked.
4. **town_fog_night**: houses lose about 72% of their contrast in my box (std 64 -> 18), but at 480 px
   the view still reads more as a dark town than a foggy one.
5. **Dither**: the sky's 3 px high-pass is still about 2x the Original's (0.42 to 0.50 against 0.21 to
   0.24); it is white noise now, no diagonal pattern, but not as clean as the 8-bit original gradient.
6. **Overlay snapshot**: the fog under an overlay costs one full-frame blit per frame with an overlay;
   untested with NVG, binoculars and vehicle commander optics.
7. **Cost**: not measured on a 3060; night town views project to 18 to 21 ms there, mostly the tip's own
   cost. The timing run was before the last rain haze constant (same passes).
8. The fixed bind-cache bug also affected B6's SSR trace and B8's air apply on the branch: their sky
   table lookups could have read the depth texture (unit 5). Worth landing that guard on its own.
9. Pairs differ by more than 5% in drawn objects in nine run-captures views (object streaming between
   the modes, as in round 1); firefight flashes differ by AI timing.
