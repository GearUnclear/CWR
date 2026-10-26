# Volumetric light (track `volumetric`)

This is ray-marched single scattering of the sun and of local lights. The light
passes through a thin haze plus a ground mist, and every sample is occluded by
the engine's own cascaded shadow map, the terrain horizon map and the cloud
field. It replaces the screen-space radial-blur sun shafts. It is a C++
prototype in a scratch worktree (`D:\Arma_CWA_worktrees\gfx-volumetric`,
detached at 3c53141, built into `dist/gfx-volumetric-rwdi`). The patch is
`prototype.patch` and the post shaders are in `glsl/` (they need the patched
C++ for their uniforms and textures; once it is built they work as
`CWR_SHADER_OVERRIDE_DIR` overrides).

The work ran in two sessions. Probes 1 to 5 (14:10 to 15:21) built the march,
found the missing object shadows and fought the haze. The run was then paused.
Probes 6 to 9 (18:28 onward) added beam contrast, headlight cones, an edge fix,
a camera pan and a clear-weather medium test. The probe 5 versions of the
`BEST_` images are kept in `cmp/best_probe5/`; the `BEST_` files in this folder
are the current ones, except `BEST_pine_mist.png`, which is from probe 5 (that
spot was not shot again).

## Verdict: promising (daytime beams and headlights); street lamps and fires fail

- **God rays through a wood** (`BEST_pinewood_morn.png`). With the probe 6
  beam-contrast composite (`volGain`), sun shafts slant through the pine
  crowns, and the air around the sun glows behind them. The scattering field
  on its own is in `BEST_pinewood_morn_debug.png`.
- **Misty morning** (`BEST_pinewood_mist.png`, fog 0.35). The current
  Remastered shows almost no mist at this fog setting. The prototype gives
  shafts and a warm glow in the crowns.
- **Evening beams through a cypress row** (`BEST_pine_eve.png`,
  `BEST_pine_eve_crop.png`). A warm beam falls through the gap onto the lit
  ground stripe, with rays fanning from the sun between the trunks.
- **Beams between buildings** (`BEST_town_eve.png`). Light spills over a roof
  and between the trees of a village toward a low sun; tree and house shadow
  volumes run across the foreground grass.
- **Sunrise** (`BEST_sunrise_ridge.png`). A cypress's shadow volume cuts
  through glowing warm mist. The current Remastered instead smears an orange
  screen-space streak over the ground.
- **Headlights at night** (`BEST_hl_front.png`, `BEST_hl_front_crop.png`). A
  Jeep's headlight cones glow in the night mist above the lit road. This is the
  biggest "different game" change of the track. It needed a fix to the
  spotlight reach and a phase-function sign bug (probe 6).
- **No change where none is wanted.** With the sun behind the camera
  (`cmp/probe7/forest_edge_golden_7.png`) or overhead
  (`cmp/probe3/noon_fields_3way.png`), nothing visible changes.

**Weak spots.**

- With the misty default (`volMist=1.5`), open ground toward a low sun gets a
  warm-white band of glowing ground mist along the horizon
  (`cmp/probe8/lowsun_glow_check.png`). This is a mild wash. A clear-weather
  mist of 0.3 removes it (`BEST_lowsun_fp.png`, probe 9) and keeps the
  narrow beam through the cypress gap (`BEST_pine_eve_clear.png`), but it also
  loses most of the wide god-ray fan in the wood. The medium has to follow the
  weather and the time of day.
- Backlit crowns near the sun take a pale grey in-scatter veil (top right of
  `BEST_pinewood_morn.png`, the bushes in `BEST_town_eve.png`). This is
  physically right in mist, but it is close to the owner's "flattened tree
  crowns" and "wash" rejections.
- Street lamps and the campfire do not work. The fire's halo tints the whole
  night sky brown (`cmp/probe8/town_night_fog_8.png`), and the lamp halos are
  invisible. Do not ship the point-light part.

## Big side finding: the capture rig had no tree or building shadows

`Scene::_objectShadows` defaults to false and only the Options page turns it on
(see the `triEnableShadows` comment in `GameStateExtTestViewer.cpp`). Tests run
in a fresh user dir, so in every capture run so far, including the
`20261017-final` baseline, **terrain objects (trees, buildings, walls) cast
nothing into the shadow map**. Only spawned props (vehicles, soldiers) did. This
explains three things:

- the baseline timings show `world (shadow 0.00..0.09)`
- probe 1 below shows no beams at all
- the baseline Remastered forests have no tree shadows

My research tests call `triEnableShadows` after setup. From probe 2 on, the log
shows `shadow 1 ... casc 4 splits 30.72 61.44 168 417`, tree shadows appear on
the ground (compare the `forest_edge_golden` remastered panels in
`cmp/probe3/forest_edge_golden_4.png` with the baseline), and the shadow pass
costs a real 0.1 to 1.9 ms. The committed capture tests probably need the same
call. Without it, every track is judging Remastered without tree shadows and
with an unrealistically cheap shadow pass.

## Technique

The pass runs at the world/HUD boundary (`EngineGL33::EndWorldScene`), after AO
and bloom and before the composite. All of it is in `EngineGL33_PostFX.cpp`;
the tuning knobs are in `LightRig.hpp` and `triSetLighting`.

1. **March, at half resolution (720x480 for the 1440x960 target).** For each
   pixel the view ray is rebuilt from the resolved depth, using
   `inverse(mat3(view))` and the same projection terms as the AO pass. The
   march takes 48 samples (`volSteps`) on a quadratic distribution that is
   dense near the camera. It is capped at 250 m (`volDistance`), about the
   reach of the cascades, and jittered per pixel with interleaved gradient
   noise.
2. **Sun visibility per sample.** Three terms are multiplied:
   - one hardware 2x2 PCF compare tap of the cascade that holds the point. This
     uses the shading's own tier selection, re-anchored by the camera's move
     since the depth pass.
   - the terrain horizon map, for ridges. The stored horizon angle is lowered
     0.07 deg per metre of height above the ground, so air above a valley comes
     out of the ridge shadow.
   - the cloud shadow field, using the same function as the shading.

   The shadow map, the compare sampler and the cascade matrices are bound and
   uploaded as plain uniforms for the post program. The `LitConstants` UBO
   (binding 3) is attached to it for the rig, terrain and cloud terms.
3. **Medium.** Two terms are added:
   - haze: `sigma = volHaze * 0.6 / fogMaxRange`
   - ground mist: `volMist * 0.004 * (1 + 6 * fogness) * exp(-h / volMistHeight)`

   Here `h` is the height above the terrain, read from the height texture that
   comes with the horizon map, and `fogness = 1 - fogMaxRange / viewDistance`,
   so `setFog` thickens the mist. Scattering is single, with transmittance
   along the ray. The transmittance is not applied to the scene, because the
   engine fog already does that.
4. **Sun light.** The radiance is the rig's own sun times exposure
   (`litSun * litZenith.w`), so the beams take the same warm evening chroma the
   hulls get. The phase function is `0.85 HG(g) + 0.15 HG(-0.3)` with
   `g = volAniso` (default 0.75). The term is gated off below the horizon.
5. **Local lights.** The pass takes the 16 nearest switched-on scene lights
   within 250 m. It has to read the scene's light list, because the active list
   is already cleared by `EndWorldScene`.
   - Point lights: a closed-form integral of a softened `s^2/d^2` falloff along
     the ray, clipped at the light's reach. The halo radius is clamped to
     1.5 m, because street lamps report a 50 m start distance.
   - Spotlights (headlights): a march through the cone with the shading's cone
     test and a forward HG(0.5) lobe. With `volSpot=1` (probe 6) the beam
     reaches as far as its pool on the ground does (the shading's 10x start
     distance, capped at 60 m; a Jeep's headlights report a 110 m start), with
     the shading's `start^2/d^2` falloff and 32 steps. Probe 6 also fixed the
     phase sign: the scattering angle is between the light's travel and the
     way to the eye, so looking into a headlight is the bright direction. The
     probe 1 to 5 code had it backwards, which no shot exposed because no
     spotlight was ever in view.
6. **Filter.** A depth-aware 11-tap separable blur, then a joint-bilateral 2x2
   upsample in the composite (full-resolution depth against the view depth the
   march stores). There is also a temporal path (`volTemporal=1`): per-frame
   golden-ratio jitter, reprojection with depth rejection and 3x3 variance
   clipping, and a 10 % blend.
7. **Composite.** The result goes through the existing screen-style glow add,
   which never clips. Anti-haze and beam controls:
   - `volCarve` (probes 2 to 3): shadowed air below the unshadowed mean
     darkens. Darkens, so not used in the final variants.
   - `volHighpass` (probes 4 and 5): subtracts a share of the scattering's broad
     local mean (a disc 0.12 of the frame height wide), floored at `(1 - k)`
     of the original value, so it never darkens anything.
   - **`volGain` (probe 6):** the local mean takes only samples at a similar
     view depth (sky with sky, a tree line with the tree line), so a silhouette
     against bright glow gets no rim. Scattering that stands above that mean (a
     lit shaft between shadowed ones) is then boosted by `gain - 1`. The term
     only adds light, and smooth glow (scattering equal to its mean) gets no
     boost. This is what made the beams readable: the field in the debug view
     was always rich, but before probe 6 most of it was lost in the composite.
   - **`volEdge` (probe 8):** thin foliage against the glow got a speckled
     bright rim, because the 2x2 upsample borrowed the sky's scattering when no
     half-res tap matched a leaf's depth. The fix measures the best depth match,
     and where it is poor it clamps the scattering to the depth-matched mean and
     gives it no boost. It also centres the mean on the full-resolution depth.
     `cmp/probe8/town_eve_edge_zoom.png` (left without, right with) shows the
     rim mostly gone; a few speckles remain.

Tuning keys (`triSetLighting "key=value"`):

| Key | What it sets |
|---|---|
| `vol` | strength (0 = the old shafts) |
| `volHaze`, `volMist`, `volMistHeight` | medium density and mist height |
| `volAniso` | phase anisotropy `g` |
| `volDistance`, `volSteps` | march length and step count |
| `volLocal`, `volSpot` | local-light strength; 1 = spotlights scatter over their full reach |
| `volCarve`, `volHighpass`, `volGain`, `volEdge` | anti-haze and beam-contrast controls |
| `volTemporal`, `volBlur` | filtering |
| `volDebug` | 1 = scattering alone; red = carved |

## Views

| View | Where | Why |
|---|---|---|
| `pine_eve` | Abel 4355, 3275, heading 288, 18:15 on 5 Jul (sun 12 deg, az 290), clear | Cypress row (`str_pinie`) 30-75 m away with the sun behind it. Found with `scripts/find_spot.py` over the `PoseidonTools terrain objects` dump (`abel_objects.txt`) |
| `pine_low` | same spot, heading 292, 18:32 (sun ~9 deg) | lower sun, more of the row in front of it (probe 2 only) |
| `pine_mist` | Abel 4220, 3260, heading 70, 05:45 (sun 11.7 deg), fog 0.25 | the same grove from the east: misty morning (probes 2 to 5) |
| `pinewood_morn` | Abel 6830, 9905, heading 72, 06:00 (sun 14.4 deg), fog 0.1 | edge of a Scots pine wood (`str borovice`), sun behind the crowns |
| `pinewood_mist` | same, fog 0.35 | misty morning in the wood (probes 6 and 8) |
| `pinewood_pan` | same, a 24-frame pan at 0.3 deg per frame | spatial against temporal filtering in motion (probe 6) |
| `town_eve` | Abel 7390, 7940, heading 288, 18:15, fog 0.1 | village street with houses and trees toward the sun. Found with `scripts/find_spot2.py` (`spots_house_az288.txt`) |
| `olive_morn` | Abel 1620, 1990, heading 72, 06:00, fog 0.15 | olive grove (`spots_oliva_az72.txt`). A failed spot: the grove sits on a ridge above the camera, so the frame is mostly hillside (`cmp/probe7/olive_morn_7.png`) |
| `hl_front`, `hl_side` | Abel forest road at 7259, 8343 (road heading 10 deg), 22:00 on 27 Feb, fog 0.2 | a Jeep with a SAFE AI driver, headlights on; the nearest street lamp is 400 m away (`scripts/find_road.py`). Front: 14 m up the road, looking back at it. Side: 16 m to the side |
| `sunrise_ridge` | baseline view | 5 deg sun over the central massif |
| `forest_edge_golden`, `lowsun_fp`, `noon_fields` | baseline views | sun behind, to the side and overhead: the "does it wash the frame" checks |
| `town_night`, `town_night_fog` | baseline view with 900 settle frames; the second with fog 0.15 | lamps and the campfire |
| `grove_sun`, `grove_mist` | Abel 5105, 3380 | probe 1 only; low hazel bushes with the sun above them |

## What the real data showed, run by run

All comparison images are in `cmp/probe<N>/`, with GPU ms on every panel. The
raw shots and `game_stdout.log` are under `runs/probe<N>/tri/`.

**Probe 1 (no object shadows, strong haze).** A uniform white-grey glow covers
the lower frame and the area around the sun, and there are no beams at all
(`grove_sun_3way.png`, `sunrise_ridge_3way.png`; `shadow 0` in the log). This is
the rejected "wash over the frame". The night lights were missing because the
active light list is empty at `EndWorldScene`. The probe 1 test lacked
`triEndTest`, so I stopped its game (and tri's retry) with
`stop-owned-games.ps1`.

**Probe 2 (object shadows on).** The first real beams: through the cypress gap
(`pine_eve_3way.png`), through the grove in mist (`pine_mist_3way.png`), and a
cypress's shadow volume in the sunrise mist (`sunrise_ridge_3way.png`). The
first `volCarve` painted a cyan streak above the sun (rejected; made
colour-neutral). The night shot was white fog over the frame.

**Probe 3.** God rays through the pine crowns (`pinewood_morn_3way.png`). No
change with the sun behind or overhead (`forest_edge_golden_4.png`,
`noon_fields_3way.png`). Open ground toward a low sun gets a glow
(`lowsun_fp_3way.png`).

**Probe 4.** The plain high-pass removed the open-ground wash but darkened
objects in front of bright glow (a darker jeep, dark smears under the
cypresses). Rejected.

**Probe 5.** The floored high-pass: the wash drops to a faint glow with no
darkening (`lowsun_fp_4.png`), and the beams survive (`pine_eve_5.png`). Night
is still a brown wash or invisible (`town_night_5.png`).

**Probe 6 (beam contrast, headlights, pan).**

- `pinewood_morn_crop.png`, `pinewood_morn_g4_pair.png`: with `volGain` 2.5 to
  4 the shafts read clearly: a diagonal shaft left of the birch and a fan
  through the crowns on the right. The probe 5 best (AH) shows them only
  faintly in the same run. The cost is a pale veil over the backlit crowns at
  the top right.
- `pine_eve_crop.png`: gain 2.5 removes a blocky glow patch that the old
  (depth-blind) mean left right of the right-hand cypress, and the beam is
  crisper.
- `pinewood_mist_6.png`, `pinewood_mist_crop.png`: with fog 0.35 all variants
  give shafts and a glowing crown line. Very atmospheric.
- `lowsun_fp_6.png`, `sunrise_ridge_6.png`: no new wash from the gain. At full
  size the open-ground horizon band is still there (probe 8 check below).
- Headlights: `hl_front_crop.png` shows the cones glowing in the mist with
  `volSpot=1, volLocal=2`. From the side (`hl_side_crop.png`) the beam is only
  a faint glow above the lit stripe.
- `pinewood_pan_strip.png`, `pinewood_pan_strip_sp.gif`,
  `pinewood_pan_strip_tt.gif`: in a 7.2 degree pan, spatial and temporal
  filtering look the same at this scale. The temporal path shows no visible
  ghosting or lag at 0.3 deg per frame. A faster pan, strafing and moving
  objects were not tested.

**Probe 7.**

- `hl_front_crop7.png`: `volLocal=4` gives a strong headlight cone. The tree
  line shows as silhouettes inside the beam (`hl_front_edge_zoom.png`; the
  debug field has them too). That is physically right: rays that hit a tree
  stop before the far part of the beam. The edges are half-resolution steps.
  With `volMist=3` the beam is softer and slightly dimmer near the lamps.
- `hl_side_crop7.png`: the side view stays a soft glow. There is a lighter
  block beside the big trunk, an upsample artefact.
- `town_eve_crop.png`: beams between buildings and trees, but also a speckled
  bright rim around the crowns (fixed in probe 8) and a warm veil over the
  bushes.
- `pinewood_cost_7.png`: 24 steps alone loses part of the left shaft; 24 steps
  plus temporal matches 48 steps on this static view.
- `forest_edge_golden_7.png`: gain 2.5 with the sun behind: no visible change.
- `olive_morn_7.png`: a failed spot (see Views).

**Probe 8 (edge fix).**

- `town_eve_edge_zoom.png`, `town_eve_edge_crop.png`: the speckled rim around
  the crowns is mostly gone.
- `pinewood_morn_8.png`, `pinewood_morn_crop8.png`: the shafts stay. Gain 4
  is the most readable.
- `lowsun_fp_8.png`, `lowsun_glow_check.png`: a warm-white band of glowing
  ground mist along the horizon behind the jeep (current Remastered, probe 6
  gain 2.5 and probe 8 gain 2.5 with the edge fix, side by side). I first
  judged the probe 6 version as clean from the small panel, but at full size it
  has the band too; the edge-fix version is slightly weaker.
- `town_night_fog_8.png`: `volLocal=4` on the point lights in fog: the campfire
  tints the whole sky brown, and the street lamp halo is barely visible.
  Rejected.

**Probe 9 (clear-weather medium: `volMist=0.3` against the default 1.5).**

- `lowsun_fp_9.png`, `lowsun_fp_crop9.png`: with mist 0.3 the glowing horizon
  band and the haze around the jeep are gone; the frame looks like the current
  Remastered. This is the open-ground fix (`BEST_lowsun_fp.png`).
- `pine_eve_9.png`: with mist 0.3 and gain 4 the beam through the cypress gap
  stays clear and crisp, and the glow around it mostly goes
  (`BEST_pine_eve_clear.png`). Gain 2.5 at mist 0.3 leaves the beam faint.
- `pinewood_morn_9.png`, `pinewood_morn_crop9.png`: in the wood, mist 0.3
  loses most of the shafts (gain 4 keeps a faint one). The fan of god rays
  needs the dense mist.
- `town_eve_9.png`: mist 0.3 removes the veil over the bushes; only a faint
  ray over the roof remains with gain 4.

So the mist density is the one control that trades beams against wash, and it
does so in the physically expected way: narrow beams through gaps survive thin
air, wide fans need mist. A production default could be mist 0.3 with gain 4
in clear weather, rising toward mist 1.5 with gain 2.5 at dawn, at dusk, in
`setFog` and in rain.

## Variants

| id | knobs | look |
|---|---|---|
| A (`va`) | `vol=1 volHaze=0.3 volMist=1.5 volDistance=250` | The reference. Clear beams through casters and a warm glow toward the sun; hazes open ground toward a low sun |
| AH (`vah`) | A + `volHighpass=0.6` | The probe 5 best: beams stay and the open-ground glow drops to about 40 %, but the beams are faint |
| **G25E (`vg25e`)** | AH + `volGain=2.5 volEdge=1` | **Best all-rounder.** Readable beams, no rim on silhouettes, no extra glow. Still has the horizon mist band on open ground |
| **G4E (`vg4e`)** | AH + `volGain=4 volEdge=1` | **Best for woods.** The strongest shafts; more veil on backlit crowns |
| A3G (`va3g`) | `vol=1.4 volAniso=0.82` + hp 0.6 + gain 3 | The most dramatic sun glow; the haziest |
| **G4EM3 (`vg4em3`)** | G4E + `volMist=0.3` | **Clear weather.** No horizon band or crown veil; beams through narrow gaps stay, wide fans in woods mostly go |
| G25EM3 (`vg25em3`) | G25E + `volMist=0.3` | Clear weather, gain 2.5: beams too faint |
| **VL4 (`vl4`)** | A + `volSpot=1 volLocal=4` | **Headlights.** Strong cones in night mist |
| VNSM (`vnsm`) | A + `volSpot=1 volLocal=2` | Headlights, softer |
| AK, carve, un-floored high-pass | probes 2 to 4 | Rejected: they darken (cyan sky, darker objects) |
| night point lights | `volLocal` 1 to 4 | Rejected: brown sky from the fire, invisible lamp halos |
| T (`vt`, `tt`) | temporal | Same as spatial on static shots and in the probe 6 pan |

## GPU cost

The full table is in `timings.md` (265 shots, generated by `scripts/timings.py`).
The volumetric pass column covers the march, the optional temporal resolve and
the two blur passes, at 1440x960 render size (half-res march 720x480, 48 steps).

| Run | Daytime vol pass (min / median / max ms) | Night (headlights) |
|---|---|---|
| probe 6 | 1.19 / 1.98 / 2.38 (17 shots) | 0.07 to 1.18 |
| probe 7 | 1.10 / 1.45 / 1.94 (10 shots) | 0.12 to 0.33 |
| probe 8 | 0.51 / 1.13 / 1.65 (10 shots) | 0.05 (point lights only) |
| probe 9 | 0.41 / 0.72 / 1.36 (12 shots) | none |

The old screen-space shafts pass costs 0.15 ms. The composite goes from about
0.04 ms to about 0.12-0.14 ms with the upsample and the 24-tap depth-aware mean.

**Caveat:** every run shared the GPU with up to three other agents' games (tri
allows four at once), so frame and world times swing by 2-5x between shots of
the same view, and even the pass column is inflated. The lowest readings
(0.41 ms in probe 9 for `lowsun_fp`, 0.51-0.54 ms in probe 8 for
`pinewood_morn` and `town_eve`) are the best estimate of the uncontended
cost. Halving the steps (probe 7) did not halve the
measured cost, which also points at contention, not the march, dominating the
numbers. Honest range: 0.4 to 2 ms on this machine.

Turning on object shadows (needed for any beams) also makes the shadow pass
real: 0.1 to 1.9 ms instead of the baseline's ~0.05 ms.

## What a production version needs

- **Weather and time-driven medium.** This is the main decision between
  "beams" and "wash". Clear weather should get thin mist; dawn, valleys,
  `setFog` and rain should be misty. The prototype infers fog from the fog
  range (`Landscape::_weather` is private; the range also shrinks at night) and
  keeps a ground mist even at fog 0. Probe 9 shows what to aim for: mist 0.3
  with gain 4 in clear weather (no wash, beams through gaps), rising toward
  mist 1.5 with gain 2.5 at dawn and dusk, in fog and in rain (god-ray fans in
  woods).
- **The `objectShadows` default.** Volumetrics need tree and building casters.
  Make sure the Remastered default has them on, and fix the capture rig.
- **Headlights** are ready to build on: `volSpot` plus a strength around 2 to
  4, scaled by the mist. Check the light colour (the Jeep's lights are a pink
  white, `0.79 0.61 0.61` after the night factor) and a half-res artefact
  beside thin occluders.
- **Night point lights need a different design.** The halo must be sized by
  the light head (a flame, a lamp globe), not by the shading's start distance,
  and fires need a flicker-free, small-radius halo. Do not ship the current
  point-light halo.
- **Cost reduction**, in rough order of payoff:
  1. Always-on temporal accumulation with 24 steps (probe 7 matches 48
     steps on a static view; probe 6's pan showed no ghosting).
  2. Quarter resolution with the edge-aware upsample.
  3. Read the terrain height and horizon every 4th step only; skip the cloud
     tap when there is no cloud field.
  4. Or a froxel grid (for example 160x90x64) that shares the shadow lookups
     and gives local lights cheaply.
- **Edges.** The half-res upsample still leaves a few speckles at thin foliage
  and a lighter block beside a trunk in front of a headlight beam. Full-res
  depth-aware upsampling (or a full-res march near depth edges) would finish
  it.
- **Classic stays bit-identical.** The pass only runs with the Remastered
  post effects and `vol > 0`, and the Classic programs are untouched. The
  prototype adds knobs to `LightingTuning` and changes the composite shader
  string, which is a Remastered-only program. The composite's non-volumetric
  path computes the same glow as before; it only declares extra uniforms.
- **Unvalidated:** vehicle interiors, gunner optics and night vision (the pass
  is skipped where the old shafts were skipped), fast motion and moving
  occluders, MSAA edges at the bilateral upsample, other islands (only Abel was
  captured), and the real-GPU cost without contention.

## Risks against the owner's taste list

- **"Blue haze or a wash over the frame."** The main risk. The scattering is
  warm, never blue, and with the sun behind the camera or overhead nothing
  changes. But toward a low sun the default ground mist makes a glowing band
  along the horizon on open ground, and a pale veil over backlit crowns. The
  probe 1 settings (strong haze, no shadows) were a textbook rejected wash.
- **"Any result that is only darker."** The carve variants and the un-floored
  high-pass darkened and were dropped. The final variants (G25E, G4E, VL4)
  only add light.
- **Evenings must be at least as warm as Classic.** The beams use the rig's own
  low-sun chroma and read warm (`BEST_pine_eve.png`, `BEST_town_eve.png`,
  `BEST_sunrise_ridge.png`). The haze lowers contrast toward the sun.
- **"Flattened tree crowns."** In-scatter in front of backlit crowns flattens
  them somewhat (top right of `BEST_pinewood_morn.png`, the bushes in
  `BEST_town_eve.png`). The clear-weather mist of probe 9 removes the veil
  (`cmp/probe9/town_eve_9.png`), along with most of the fan.
- **Colour grading:** none, by design.
- **Night:** the moonlit-town brightness problem from the baseline is
  untouched, and the point-light halos add a brown cast. Headlights are the
  one night case that works.

## Files

- `prototype.patch`: the C++ and GLSL change against 3c53141
  (`EngineGL33_PostFX.cpp`, `EngineGL33.hpp`, `LightRig.hpp`,
  `GameStateExtTestRender.cpp`). `prototype_probe5.patch` is the probe 5
  state.
- `glsl/`: `psPostVolumetric`, `psPostVolBlur`, `psPostVolTemporal` and
  `psPostComposite`, as built (probe 8 code). `glsl_probe5/` is the probe 5
  state.
- `BEST_*.png`: Classic, Remastered (current) and Remastered with the
  prototype, side by side, same view and time, with GPU ms on each panel
  (`BEST_pinewood_morn_debug.png` shows the scattering field instead of
  Classic). `cmp/best_probe5/` holds the probe 5 versions.
- `cmp/probe1..9/`: every comparison made along the way, including the pan
  strip and its two GIFs in `cmp/probe6/`.
- `runs/probe1..9/`: run-captures output (tri tree, `game_stdout.log`, the
  `UDCAP` lines, contact sheets) and `runs/probe<N>.runner.log`.
- `tests/`: the generated research tests (`vol_probe*.test.sqf/.toml`) and the
  copied `capture_lib.sqf`. The same files are in the scratch worktree under
  `arma_CWR/tests/rendering/research/volumetric/`, which git excludes.
- `scripts/`: `gen_test.py` (test generator: views, variants, pans, the
  headlight driver), `compare.py` (labelled N-way panels), `pan_strip.py`
  (pan strip and GIFs), `timings.py`, `find_spot.py`, `find_spot2.py` (olive
  and house spots), `find_road.py` (dark forest road), `patch_postfx.py`,
  `patch_temporal.py`, `patch_gen_probe6.py` and `patch_edge_probe8.py` (how
  the code and generator were changed), and `extract_glsl.py`.
- `timings.md`, `abel_objects.txt` (Abel object dump), `spots_*.txt` (spot
  search results), and `build*.log`.
