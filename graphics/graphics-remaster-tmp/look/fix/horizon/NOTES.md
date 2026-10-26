# Horizon fixer: dusk firefights (firefight_1, _2, _3)

Scratch copy `D:\Arma_CWA_worktrees\look-fix-horizon` (fec1075 + `LOOK/fix/base.patch`), build
`build/look-fix-horizon-rwdi`, game `dist/look-fix-horizon-rwdi`, junction `RemasterUD-fix-horizon`.
Nothing committed, no branch. Patch: `fix.patch` (7 files, +207 / -18, only my delta over the base
tree 07d0ae09; no probe tests in it).

## Look first

- `sbs_horizon_strip.png`: firefight_2 and firefight_3, before (director final) | after, and the
  horizon band at 2x, before above after.
- `sbs_muzzle_pools.png`: firefight_1 and firefight_2, the riflemen at 2x, before above after.
- AI fire is not pinned, so muzzle flashes and smoke differ between before and after. The shell
  bursts are at the same age (5 and 6 frames after the shell) in both.

## Cause (with the probes)

Both problems are B4 combat light on the ground, in the forward shader (`combatLights`).

- `probe_1/` (P mode: one paused frame shot with each setting): the strip and the pools are the
  same with `fogLights=0`, `volFog=0`, `combatScatter=0` and `ssr=0`. `combatFire=0` removes the
  right half of the strip (the burning Ural). `debug=6` shows the burst and the fire listed in
  the tiles under the strip. So: not the fog, not the composite's scatter, not SSR.
- `probe_2/` (live override `ovr_tint/`: each combat light's ground light drawn in one colour):
  the strip is green (the shell burst) and red (the wreck's embers, a Fire light), the pool under
  a rifleman is magenta (the muzzle light, window 3 m or less). `scratch/p2_tint.png`.

Why they were so bright at dusk:

1. **The wrong night.** The combat lights are scaled by Classic's night effect
   (`CombatDayScale`, the flash response `FlashExposureGain`). Classic's night is 1 as soon as
   the sun is under the horizon, so at 19:35 (sun 1.7 degrees under) every fire, burst and muzzle
   flash ran at full night power. The photoreal camera still treats that as a twilight
   (`CameraNight` is 0 until 4.5 degrees under) and the fog's lamp halos already follow the
   camera's night (round 2). The combat lights did not.
2. **The ball's wrap.** The ground took `(cos + core/d) / (1 + core/d)` of a light. For a light
   low over flat ground that lights the plain far off as if the light stood `core` metres high:
   about 1.4 to 1.5x a sphere light's real value 12 to 20 m from a shell burst (core 2.6 m), up
   to 5x for a ball sitting on the ground.
3. **The held flash.** A burst's white-hot flash holds 0.15 s at its peak to match Classic's
   whiteout, which the flash response (exposure) reproduces. At dusk the response is off, but the
   held flash still lit a 20 m band of plain in every frame of it (firefight_2 is 0.13 s after
   its burst).

## The change

- `CombatNight(classicNight, photoCamera, cameraNight)` (CombatLights.hpp/.cpp): under the
  photoreal camera the combat lights' night is min(Classic's, the camera's). It drives the day
  scale (`Evaluate`, EngineGL33_Lighting.cpp) and the flash response (EngineGL33_PostCombat.cpp,
  stored in `CombatState::night`). Without the camera nothing changes. At full night (town_night,
  ffh) it is 1 as before.
- `CombatBallCosine(cos, core, d)` (C++) and `combatBallCosine` (GLSL, same math): the share of
  the ball above the surface's horizon, the horizon-clipped sphere light of Lagarde and de
  Rousiers (2014). cos while the ball is wholly above the horizon, (1 + cos)/2 at its surface and
  inside it (the shooter's arm still lit all round), 0 wholly below. It replaces the wrap for
  non-foliage surfaces in `combatLights`. Leaves keep their crown wrap.
- `CombatTuning::flashHold` (default `kExplosionFlashHold`, 0.15 s): under the camera it is
  0.15 s times the combat night, so at dusk the flash falls from its birth (same peak, about half
  a step later, under a fifth of the held flash 0.13 s after the burst). At night it holds as
  before.

Constants touched (name: old -> new):
- `kExplosionFlashHold`: 0.15 -> 0.15 (unchanged; moved above `CombatTuning`).
- New `CombatTuning::flashHold`: 0.15 (camera: 0.15 x combat night).
- Day scale input under the camera: Classic night (1 at dusk) -> camera night (0 at dusk).
  `CombatDayScale` itself unchanged (fires 0.12, bursts 0.2 by day).
- Ground wrap `(cos + core/d)/(1 + core/d)` -> horizon-clipped sphere (no constant).
- Round 2's camera gains (muzzle 0.05 x / 2.5 m, burst 0.3 x / 20 m, fire 20 m) unchanged.

## Results

Firefight views (`shots_1/`, the view 7 rhythm, overhaul half only; 960x640 at 1.5 scale):
- **Muzzle pools: gone** at thumbnail and full size (`sbs_muzzle_pools.png`). The flash sprite
  and the shooter remain; at dusk the muzzle light runs at the burst day scale 0.2.
- **Strip: gone at thumbnail size.** Behind the riflemen the burst band is gone (firefight_2), and
  the fire band is mostly gone. Column x=800 (12 m from the wreck), peak over the sky (64):
  firefight_1 153 -> 88, firefight_3 132 -> 80; a paused frame of the same scene (`probe_4`,
  "on") 57. The ground there is 30.
- The burst's cloud reads its own white and yellow more (no held white flash lighting it).
- Night re-check (`shots_2_night/`, 1920x1080): **ffh**: the camp fire lights the ground with a
  soft falloff and long shadows, the burning Ural its own pool; **town_night**: the same fire and
  plume, the lit ground in front of the fire 2.6 levels darker than with the old wrap on the same
  build (box mean 82.1 -> 79.5, A/B in `probe_5_oldwrap/`, `scratch/tn_ab.png`). The camp fire's
  ball (0.6 m up, core 0.45 m) is wholly above the ground, so the exact cosine is the point
  light's; the wrap had lit it about 1.4x. A night firefight (`p6n` stills): muzzle flashes light
  only the shooter; a burst's first frame still lights a band of plain, as a flash at night does
  (with the flash response).
- AI sight and fog link: Target.cpp (FogSightActive / FogSightTransmittance) and
  VolumetricFog.cpp's AI sight are untouched (no diff in either file).

## Tests

- `[CombatLights]`: 26 cases, 1066 assertions, all pass. New: the camera's night (dusk is the
  day scale, night unchanged, never more than Classic's), the ball cosine (above, inside, below,
  continuity at the horizon and the surface, monotone, 2 core/(3 pi d) for a ball on the ground,
  the firefight burst), the flash without its hold; the shader contract pins `combatBallCosine`
  and its call and that the old wrap is gone.
- Gate tags (`[remaster],[FeatureSwitches],[FeatureGates],[PostStages],[fog],[SurfaceLight],
  [AirMedium],[ssr],[wet],[CombatLights],[Shaders],[StateCacheAudit],[PhotoCamera]`): 303 cases,
  954,264 assertions, all pass (glslang compiles every program).
- clang-format: the changed lines are clean.

## Still open

1. **A faint fire line at full size.** Near the burning wreck a thin warm line at the horizon
   remains in the brightest flicker, up to about 20 levels over the sky 12 m from the wreck
   (2-3 px tall, over about 10 m each side). Not visible at thumbnail size. Cause: the rig's dusk
   ground is metered as dark as a night field (camera exposure log2 +2.46 at the dusk firefight,
   meter mean -9.15; ffh at night -1.00, -9.99; town_night -2.07, -4.00), so a fire at the day
   scale (0.12) still reads about as strong against the dusk ground as at night. The real
   twilight at 1.7 degrees under is about 1000x a night's light. Knob probe (`probe_4`,
   `scratch/p4_s3.png`): `combatFire=0.25` puts the line under the sky, 0.1 removes it. Options:
   a twilight-based scale for continuous lights under the camera (the real twilight falls about
   tenfold per 2.5 degrees of sun from the horizon to 12 under), or a brighter twilight ground
   fill in the light lane. Not done here: it changes the blue hour and the day for every
   continuous light, a call for the owner.
2. A shell burst in its first frame or two at dusk (age under 0.05 s, firefight_1's timing) still
   lights a band of plain about 15 m each side, much dimmer than before (no exposure pump, day
   scale 0.2, no hold). A real flash does that for a frame.
3. The fire fixer (`LOOK/fix/fire`) works on fire light and smoke: my patch touches the smoke
   block's neighbour in EngineGL33_Lighting.cpp (`_combat.night` is computed before it) and
   CombatLights.*. Expect a small merge.
4. In my runs the town_night plume reads white at its foot, against the director final's orange
   foot. The same with the old wrap on my build, so it is not this patch (staging or the base).
5. In ffh the Ural is not always burning when the shot is taken (staging timing; one of my two
   runs).

## Files

- `fix.patch`, `NOTES.md`, `sbs_horizon_strip.png`, `sbs_muzzle_pools.png`.
- `shots_1/`: firefight_1..3 after. `shots_2_night/`: town_night, ffh, night firefight stills.
- `probe_1/` (switch probes), `probe_2/` (tint), `probe_3/` (first fix, before the hold change),
  `probe_4/` (fire levels, night), `probe_5_oldwrap/` (old wrap A/B), `scratch/` (crops).
- `tools/`: `strips.py`, `sbs.py`, `build.ps1`, the probe generators `gen_probe_ff.py` and
  `gen_probe_night.py` (copy into `tests/rendering/captures/` to use), the tint and old-wrap
  overrides in `ovr_tint/`, `ovr_oldwrap/`.
