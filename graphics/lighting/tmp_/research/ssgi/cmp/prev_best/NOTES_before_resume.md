# Track ssgi: screen-space ray-traced diffuse GI with temporal accumulation

Verdict: **failed as an "RTX on" lever for CWA daylight; the pipeline itself works.**
A physically consistent screen-space bounce on top of the Remastered light rig
changes the daytime frames by 0.0 to 0.5 8-bit levels on average (p99 at most
+5 levels, inside the shot-to-shot noise floor of about +-3). Even a deliberately
over-bright upper bound (the full radiance of every ray hit added on top of the
rig, which double counts the rig's own ground bounce) stays at +0.06 to +0.30
levels mean. Night is the only place where the bounce shows (+4 to +7 levels
mean, the tree +9 to +17), and there it brightens a town that is already
over-lit. No daytime image in this folder looks different to the eye; the
`|change| x8` panel of each BEST image shows where the tiny change lands.

## What was built (C++ scratch prototype, `prototype.patch`)

Scratch worktree `D:\Arma_CWA_worktrees\gfx-ssgi` (detached at 3c53141, not a
branch), build `dist/gfx-ssgi-rwdi`. Classic is untouched (the GI only runs in
the remastered post pass and is off at `gi=0`, the default).

Post pass (`EngineGL33_PostFX.cpp`), after the AO blur and before the `AoEnd`
GPU mark (so its cost is in the `ao` column of the timings):

1. **Trace** (`psPostGi`, half resolution, 720x480 for the 1440x960 target).
   View-space position and normal from depth (the AO's own reconstruction).
   N cosine-distributed rays per pixel (default 6), each marched with 20
   quadratically spaced steps out to `clamp(0.25 z, 3, 25) m * giRadius`.
   Hit = the ray passes behind the depth surface within `0.35 + 0.2 t` m.
   Hit radiance = the resolved frame colour, fetched from a mip chain
   (`glGenerateMipmap` on the post colour copy, own trilinear sampler) at a
   cone-footprint LOD, linearised and divided by the rig exposure.
   Per-pixel interleaved-gradient noise, rotated every frame.
2. **Temporal accumulation** (`psPostGiTemporal`): each pixel is reprojected
   into last frame's history with the previous camera axes, position and
   projection (C++ keeps them), accepted where the stored view depth agrees
   (3% + 0.15 m), blend weight 0.1 for the new frame, reset on disocclusion.
3. **Spatial denoise** (`psPostGiBlur`): separable 13-tap depth-aware blur.
4. **Composite** (`psPostComposite`): joint-bilateral 2x2 upsample to full
   resolution; the receiver's albedo is recovered from the sky-light share the
   remastered shaders already write into the colour alpha:
   `share * colour = albedo * ka * skyIrradiance(n) * exposure`, so
   `albedo*ka = share * colour / (skyIrradiance(n) * exposure)`, with
   `skyIrradiance` re-evaluated from the depth normal and the rig hemisphere
   (zenith / horizon / horizon-sun / ground) times the material ambient `ka`.
   The albedo is capped (`giAlbedoCap`, 0.8) so a lamp-lit pixel cannot claim
   a white albedo. Added light goes through an inverse-then-forward tonemap
   shoulder, so pixels with no GI pass through exactly.

Modes (`triSetLighting "giMode=..."`):

| mode | what a ray contributes | intent |
|---|---|---|
| 1 | hit: `max(L - sky(dir), 0)`, miss: 0 | **add only** the light a hit sends back in excess of what the rig's hemisphere already assumes for that direction (the main prototype) |
| 3 | hit: `L`, miss: `sky(dir)`, minus the open hemisphere | the traced result **replaces** the rig hemisphere and the shipped AO (can darken) |
| 2 | disc samples above the tangent plane, excess radiance x cosine x falloff | the cheap SSAO-plus-colour-bleed (SSDO-style) variant |
| 6 | hit: `L` (override_v4 only) | art-directed fill / upper bound: full hit radiance on top of the rig |
| 7, 8 | diagnostics (override_v5): hit fraction / mean LOD / hit radiance | |

New tuning knobs (`LightingTuning`, `triSetLighting key=value`): `gi`,
`giMode`, `giRadius`, `giRays`, `giSteps`, `giTemporal`, `giDebug` (1 = added
light only x4, 2 = raw GI texture, 3 = share/alpha/depth), `giFallbackShare`,
`giAlbedoCap`. GLSL: `glsl/` (first version), `override_v2/` (ratio estimator),
`override_v4/` (adds mode 6), `override_v5/` (diagnostics), `override_v6/`
(crossing-tolerant hit test), `glsl_final/` (the GI and composite shaders as
compiled into the final patch; pass any override dir to `run-captures.ps1
-ShaderOverrideDir` with the scratch build).

## Runs (all on the hidden desktop, junction `RemasterUD-ssgi`, 960x640, 150%, 4x MSAA)

| run | what | note |
|---|---|---|
| `runs/smoke1` | lowsun_fp, pipeline check | test lacked `triEndTest`, stopped by hand |
| `runs/abel1` | first variant set, 4 Abel views | **units bug**: the GI rig was not multiplied by the material ambient `ka` (= eye accommodation = user brightness 1.6), mode 3 not normalised. Daytime GI looked visible (warm bounce into the golden-hour crowns, jeep at noon) and the night blew out to white. Those images are wrong; kept as the record of the bug |
| `runs/v3` | fixed units + ratio estimator + albedo from share, 5 Abel views + Sinai, motion strafe | first run a composite compile error (`pu` declared twice) disabled every post effect; that run was deleted and rerun |
| `runs/v4` | rig bounce off alone (`rem_b0`), traced bounce replacing it (`gi_b0m3`), fill mode 6 at 0.5 and 1, Abel + Sinai | override_v4 |
| `runs/diag1` | lowsun_fp raw GI texture views | override_v5, `diag_lowsun_fp_tiles.png` |
| `runs/v5` | town_night, moonlight 0.3 vs 1, temporal on/off strafe in the debug view | |
| `runs/v6` | crossing-tolerant hit test, 3 Abel day views | override_v6 (GLSL only, not in the patch) |

Per-run comparison images are in `cmp/<run>/` (`<view>__all.png` grid,
`<view>__3way_<variant>.png` Classic | Remastered | variant with GPU ms,
`<view>__crop_<variant>.png` 3x crop of the most-changed 160 px square).
`timings_and_deltas.txt` holds the whole-frame change statistics against the
Remastered shot of the same run, `timings.md` the GPU timings of every shot.

## What the real data showed

Whole-frame change against Remastered (8-bit levels; `>2` = share of pixels
changed by more than 2 levels; the region columns in the txt are rough boxes):

| view | mode 1 x1 (physical) | mode 1 x2 | mode 3 (replaces AO) | mode 2 SSDO x2 | mode 6 x1 (upper bound) |
|---|---|---|---|---|---|
| lowsun_fp | +0.00, p99 +0.7 | +0.01, p99 +1.0 | -0.26, p1 -4.7 | -0.02 | +0.06, p99 +2.3 |
| sinai_lowsun_fp | +0.02, p99 +0.7 | +0.02, p99 +1.3 | -0.41, p1 -6.0 | +0.01 | +0.11, p99 +2.7 |
| noon_fields | +0.02, p99 +1.7 | +0.04, p99 +2.7 | -0.13, p1 -4.7 | +0.04 | +0.11, p99 +4.3 |
| forest_edge_golden | +0.05, p99 +1.3 | +0.12, p99 +2.3 | -0.36, p1 -4.3 | +0.15 | +0.30, p99 +3.3 |
| town_golden (research view) | +0.00, p99 +0.7 | -0.00, p99 +1.3 | -0.80, p1 -6.0 | -0.09 | n/a |
| town_night | +4.07 (tree +10.6) | +7.23 (tree +17.5) | +3.65 | +1.32 | +3.84 |

Turning the rig's own ground bounce off (`rem_b0`, `bounce=0`) changes the
daytime frames by only -0.01 to -0.49 levels, and replacing it with the traced
bounce (`gi_b0m3`) by -0.23 to -0.76: at these sun heights the rig's uniform
bounce is small, and the traced one is about the same size.

Why the daylight bounce is invisible (checked with the diagnostics, not
assumed):

- **The sky dominates the hemisphere.** The rig's sky fill is bright (shade at
  about 0.6 of lit), so a hit only adds light if it is brighter than the sky
  it hides. Sunlit Abel and Sinai ground sits close to the rig's assumed ground
  albedo (0.2 x ka), so ground hits add about nothing over the rig.
- **The acceptance views are backlit.** In lowsun_fp and sinai_lowsun_fp the
  sun is in front of the camera, so every face the camera sees looks down onto
  its own cast shadow. `diag_lowsun_fp_tiles.png` (top row: hit fraction /
  mean hit radiance x4 / mode-6 ratio; values as presented, after the gamma
  pass) shows the hulls' hit fraction at 0.18-0.35 and the mode-6 ratio (full
  hit radiance over the hull's sky irradiance) at only 0.10-0.19: what the
  rays hit is mostly shadowed ground.
- **Not a missed-hit artefact.** A low hit fraction raised the suspicion that
  the fixed thickness test drops grazing ground hits, so `override_v6` also
  accepts a front-to-behind crossing within `max(1.5 m, 0.12 z)` (`runs/v6`).
  That lifted the hull hit fractions to 0.25-0.39, and the result stayed the
  same: mode 1 x1 +0.00 to +0.05 levels, the mode-6 upper bound +0.10 to +0.37
  (lowsun_fp, noon_fields, forest_edge_golden).
- **Vehicles, soldiers, the tent and tree crowns carry no sky share.** Under 4x
  MSAA these draws store alpha 1 (alpha-to-coverage / alpha-tested materials),
  so `share = 0` (bottom-left tile: objects cyan = share 0, alpha 1; sunlit
  ground share ~0.3; shadowed ground magenta = share 1). The GI has to assume a
  share for them (`giFallbackShare` 0.5). **Side finding for the AO owner: the
  shipped Remastered AO therefore never darkens vehicles, soldiers or tree
  crowns either**; its `keep` term is 1 wherever alpha is 1.

Night (town_night): the bounce is visible and the debug view is plausible
(house walls lit from the ground up, the tree trunk lit by the fire and lamp
pools). But the frame gets brighter in an already washed-out town, and the
tree crown gets paler. Two observations on the night baseline, outside this
track but measured here (`runs/v5`, `cmp/v5/town_night__all.png`):
`moonlight=0.3` and `moonlight=1` give the same frame (house wall +2.4 either
way), so the moon is not what lights this town; and the first two shots of
that run (Classic and Remastered, about 2-4 s after `setDate` to 22:00) show a
properly dark night, while every later shot, from about 6 s on, is the bright
version seen in the 20261017-final set, in Classic too. Something time-dependent
(the street lamps switching on with very wide pools is my guess, not verified)
makes the night bright, not the rig's moonlight.

Temporal accumulation works: `motion_town_night_gi_debug_strip.png` /
`.gif` (added light only, camera strafing 0.3 m per frame): with accumulation
the bounce on the ground and walls is smooth and stays attached to the
geometry while moving, with no visible ghosting at this speed; without it the
same 6 rays show blotchy noise in the ground pool.

## GPU cost (RTX 4060 Ti, 1440x960 internal, GI at 720x480)

Measured with `triGpuTimes` while up to four other agents' games shared the GPU,
so the world and frame columns jump by several ms between identical shots;
the `ao` column (AO + GI, both before the `AoEnd` mark) is the usable number:

- Modes 1/3, 6 rays x 20 steps + temporal + blur: **+1.5 to +2.2 ms**
  (`ao` 0.16-0.39 ms without GI, 1.87-2.47 with it, runs v3/v4).
- 4 rays x 16 steps: +0.6 to +1.6 ms (run abel1).
- Mode 2 (SSDO, 16 taps): its time does not show in the `ao` column; the `post`
  total grows by 0.2-1.5 ms, too noisy to pin down.

## What a production version would need (if anyone wanted it)

- A real sky share (or an albedo/normal MRT) for alpha-tested and coverage
  draws; today their alpha is 1 and both AO and GI skip them.
- Hit radiance that excludes particles: blended sprites (smoke, fire) keep the
  background depth, so a ray can fetch smoke colour at a wall's depth.
- The crossing-tolerant hit test of `override_v6` (it finds 10-40% more hits
  on hulls than the fixed thickness of the patch) and a history clamp for
  moving objects.
- Its own GPU mark, and the composite's depth-normal pass shared with the AO.

## Risks against the owner's taste list

- Night: makes the over-bright town brighter and the crowns paler (rejected
  looks: wash, flattened crowns).
- Mode 3 (traced sky visibility replacing AO) is net darker (-0.1 to -0.8
  levels, p1 down to -7): "only darker" is on the rejected list.
- The abel1 images (wrong units) looked like a warm golden-hour bounce into the
  crowns; that look is not real and should not be quoted.

## Recommendation

Do not spend production effort on SSGI for daylight CWA scenes: with this rig
and this content the physically consistent bounce is below one 8-bit level.
The "path traced" jump will have to come from other tracks (large-scale sky
occlusion that actually reaches objects, contact-hardening shadows, reflections,
volumetrics). Two cheap side wins came out of this track: the AO gap on
alpha-1 objects, and the time-dependent night brightness, both worth a look by
their owners. If SSGI is revisited, it belongs at night with local lights,
after the night brightness is fixed.

## Files

- `prototype.patch`: the C++ prototype (git diff of the scratch worktree).
- `BEST_<view>.png`: Classic | Remastered | Remastered + SSGI | |change| x8,
  GPU frame ms per panel. Daytime views use mode 6 x1 (the upper bound), the
  night view mode 1 x1. The town_night change map also contains the smoke
  plume, which animates between shots.
- `scripts/`: `gen_tests.py` (research capture tests), `compare.py`
  (3-way images, grids, timing tables), `make_best.py`, `regions.py` (change
  statistics), `motion.py` (strip + GIF), `apply_postfx.py` (first patch step).
- `tests_copy/`: the generated research tests as run (originals in the scratch
  worktree under `arma_CWR/tests/rendering/research/ssgi/`, git-excluded).
