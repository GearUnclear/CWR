# B4 (combat lights) shelved

Status: not accepted at the second review. The change is kept as
`SHELVED.patch` next to this file so it can be picked up again.

## The patch

- Saved from the lane's scratch copy `D:\Arma_CWA_worktrees\gr-B4`
  (detached at 706b9a3, "graphics: one tiled light list feeds every local
  light") with `git add -N` on the 8 new files and
  `git diff --binary --output=SHELVED.patch`; the intent-to-add entries were
  dropped again afterwards, so the scratch copy's index is as it was.
- 48 files: 40 modified, 8 new (CombatLights.hpp/.cpp,
  EngineGL33_PostCombat.cpp, EngineGL33_PostCombatShaders.cpp,
  test_combat_lights.cpp, combat_ab.test.sqf/.toml, gen_combat_ab.py);
  4125 insertions, 64 deletions. 244874 bytes, SHA-256
  250FDBC47209A3AB40CD2246CBED6CACE4753C8AF0B6275D8ADDD11BE635D501,
  byte-identical to `B4.patch` in this folder.
- Applies cleanly on 706b9a3 (`git apply --cached --check` on a temporary
  index read from 706b9a3).
- Does NOT apply on the current graphics-remaster tip 06c7e4d. Six lanes
  landed since the base (e91b3f3, fcf12c5, 9a40228, 7b7e3e6, 7c21bc7,
  06c7e4d) and 23 files fail a plain `git apply --check`; with `--3way`
  the same 23 merge with conflicts:
  Engine.hpp, GpuTimings.cpp, GpuTimings.hpp, FeatureSwitches.cpp,
  LightRig.hpp, PostStages.cpp, PostStages.hpp, INTEGRATION-PLAN.md,
  FrameLights.cpp, FrameLights.hpp, Object.cpp, EngineGL33.hpp,
  EngineGL33_Lighting.cpp, EngineGL33_Lighting.hpp,
  EngineGL33_LightingShaders.cpp, EngineGL33_LightingShadersLights.cpp,
  EngineGL33_PostFX.cpp, EngineGL33_PostShaders.cpp, the unit
  CMakeLists.txt, test_feature_switches.cpp, test_frame_lights.cpp,
  test_post_stages.cpp, test_remaster_lighting.cpp.
  Most are shared registries (the switch list, GPU sections, post stages,
  the frame light block, shader sources) where both sides append; expect
  mechanical merges plus a recount of switch, slot and section numbers.
- The draft commit message is `COMMIT_MSG.txt` in this folder.

## What was tried

Plan section 5 B4 (night and combat): fires, flares, shell bursts, muzzle
flashes, tracers and missile motors become real HDR light sources.

- `Graphics/Rendering/Lighting/CombatLights` (pure): one set of combat
  lights, aged by the simulation step in `World::Simulate`. Bursts (muzzle,
  cannon, explosion) are spawned and forgotten; fires, wreck embers,
  tracers, flares and motors are owned through a `CombatLightHandle`. Per
  kind envelopes (`EvaluateCombatLight`), ball falloff
  `E = I * window / (1 + d^2 / r0^2)` capped at the light's range, a day
  scale (a fifth for bursts, 12% for fires).
- Sources are the existing gameplay hooks (WeaponLightSource,
  WeaponFireSource, SmokeSourceVehicle, Explosion, ShotBullet, Missile,
  IlluminatingShell, Fireplace).
- Up to 12 HDR entries at the head of A6's tiled list; the classic light of
  the same source lights nothing while the switch is on. Fires, flares and
  motors are kept first when the list is full (`KeepCombatLights`).
- Smoke lit from inside with a transmittance through the column
  (`CombatSmokeTransmittance`), so a wreck's black plume glows only at its
  foot.
- Four radial shadows in a half-res RGBA8 mask traced through the previous
  frame's linear depth (`EngineGL33_PostCombat`), with hysteresis and a
  depth-aware blur; a night-only flash exposure; analytic in-scatter of up
  to 8 lights.
- Switches `combatLights` and `combatShadows` (11/26) with the knobs
  `combatFlash`, `combatFire`, `combatSmoke`, `combatScatter`.

The first build (v7, `runs/v7-combat`) failed review on the wreck plume
(lit at sky level, a 4.4 m ball of lit air over the wreck) and on the fire
colour. The six review fixes (`review-fix/`, build r2) were judged correct
at the second review: the off switch matches the old look, 557 unit cases
pass, the plume stays dark, the flash is within 2 levels of Classic and the
centre seam is gone.

## Why it failed

### 1. Bursts freeze lit under time acceleration (blocker)

`CombatLights.cpp:489`, `CombatLightSet::Tick`:

```cpp
if (!(dt > 0.0f) || dt > kCombatMaxTickStep)
    return;
```

with `kCombatMaxTickStep = 0.25f` (`CombatLights.hpp:220`, commented as "a
longer one is a clock jump (a scripted time pin, a load), not an age
step").

`World::Simulate` multiplies its step by the time acceleration
(`deltaT *= _acceleratedTime;`, World.cpp:258) and passes that same
`deltaT` to `GCombatLights().Tick(deltaT)` (World.cpp:1725). Under
`setAccTime 4` (the Showcase does this: `escalation.sqs:34`) any frame
slower than 1/16 s gives a step over 0.25 s, and Tick returns without
aging anything. Consequences, from the code:

- Every burst stays at age 0, which is the full flash of its envelope
  (muzzle `exp(-age / 0.030)`, explosion flash hold), so shell and muzzle
  flashes stay lit.
- Bursts are only removed in Tick (the finished-burst erase) and in
  `ClearBursts`, which runs only from `Scene::ResetLights` at scene
  teardown. While the steps stay long, every new shot adds a record and
  none leaves: the set grows without bound.
- Fires and flares freeze too (their breathing runs on `age`), and
  `SimTime` stops.

The unit test locks the bug in: `test_combat_lights.cpp` lines 463 to 468
("the set ages by the simulation step") call `set.Tick(5.0f)` and require
that it "ages nothing".

### 2. Commit message known-risks wording

The known risks say the camp fire's pool reads red over blue 1.46 in
`combat_fires` only. The lane's own `review-fix/plume-r2.txt` shows the
same pool crop at 1.43 in `combat_eye_k00` and `combat_eye_k11`, 1.41 in
`combat_eye_k08`, 1.46 in `combat_shell_k16` and 1.54 in `combat_shell_k11`
(its Classic reads 1.28 there); the review also reported 1.43 under the
shell burst of `combat_shell_k08` on the same crop (not in the lane's
files). The message also does not say plainly that with 12 or more fires,
flares or motors in view every burst, including one next to the camera,
lights nothing that frame (`KeepCombatLights` keeps the steady lights first
and the cut bursts' classic lights stay skipped).

## What would fix it

1. `CombatLightSet::Tick`: keep the early return for a step of 0 or less
   (paused) and age by the clamped step instead of returning:

   ```cpp
   void CombatLightSet::Tick(float dt)
   {
       if (!(dt > 0.0f))
           return;
       const float step = std::min(dt, kCombatMaxTickStep);
       _simTime += step;
       for (CombatLightRecord& r : _records)
           r.age += step;
       // erase finished bursts as before
   }
   ```

   A real clock jump then ages the set by at most 0.25 s, which ends a
   muzzle burst (over after 0.18 s) and moves the others on; that is
   wanted.
2. `CombatLights.hpp`: rewrite the `kCombatMaxTickStep` comment and the
   Tick comment: the constant is the most one Tick ages by (a longer step,
   time acceleration at a low frame rate or a clock jump, ages by this
   much), and a step of 0 (paused) ages nothing.
3. `test_combat_lights.cpp`, the "set ages by the simulation step" case:
   drop the `Tick(5.0f)` "ages nothing" check and add a 4x-speed case, a
   muzzle burst ticked by steps of about 0.27 s (a frame under 16 fps at
   4x), that requires the burst to be gone after one or two steps and the
   owned fire's age to have moved. Build it against the shelved code
   first and confirm it fails there, then apply the fix and confirm it
   passes.
4. `COMMIT_MSG.txt` known risks: give the pool's red over blue in all the
   shots that show it (1.43 at eye height in combat_eye_k00, and under a
   shell burst in combat_shell_k08 on the same crop, with 1.46 in
   combat_fires), and add the sentence that with 12 or more fires, flares
   or motors on screen every burst, including one beside the camera,
   lights nothing.
5. Rebase onto the graphics-remaster tip: apply `SHELVED.patch` with
   `--3way` on a fresh scratch copy of the tip, resolve the 23 conflicts
   above, recheck the switch and slot numbering, rebuild, rerun the gate
   tags of plan section 8 item 3 plus [CombatLights], [FrameLights],
   [FeatureSwitches], [FeatureGates] and [PostStages], and redo the
   combat_ab captures (classic, off, on) since the tip's lighting changed
   under them. A short 4x run of the Showcase escalation chapter at a low
   frame rate would show the fix in game.
