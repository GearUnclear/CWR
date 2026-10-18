# Remastered lighting

A second lighting model for the GL 3.3 backend, chosen by the **Lighting** row
of the Graphics options (Classic / Remastered). It touches the lighting and
rendering code only; it needs no new art, models or textures. Classic is the
original lighting and is unchanged: the classic shader strings, uploads and
program set are untouched, and the remastered programs are a separate set.

## Why Classic looks flat

The classic vertex shader sums a yellow sun `(0.85, 0.75, 0.4)` and a blue
ambient `(0.55, 0.6, 0.8)`, both multiplied by the default brightness of 1.6
(`GraphicsConfig` brightness folded into `_accomodateEye`, then into the
material colours). It then clamps the sum to 1 before texturing
(`s_vsTransformGLSL`, `vColor = clamp(litColor, 0, 1)`). At most sun heights
both the sunlit side and the shaded side of a surface clamp to white, so the
frame shows bare texture colour with almost no shading. Making shade readable
inside that model can only darken the whole frame, which reviewers rejected.

## What Remastered does

Geometry drawn through `VSTransform` (terrain, objects, vehicles, soldiers,
trees, water) is lit per fragment in linear light and tonemapped per fragment
(`EngineGL33_LightingShaders.cpp`):

| Piece | Where | What it changes on screen |
|---|---|---|
| Light rig | `LightRig.cpp` | The sun's colour follows its height (near-neutral by day, red at the horizon; air-mass extinction). Below 16 degrees it takes Classic's own sunset colour (`EveningSunChroma`: 0.65 of LightSun's colorFull hue, taken to linear because Classic applies it in display space), so a sunlit vehicle at golden hour has the original's warm hue instead of a ~5600 K white. The sky is a hemisphere: zenith, ground bounce, and a horizon band split into a warmer, brighter sunward half and a cool anti-sun half, so low sun gives warm light and cool shade. An exposure meter keeps sunlit ground at Classic's on-screen brightness from noon down to about 20 degrees, then lets golden hour darken into a warm, contrasty frame. In twilight the fill is capped by the visible sky, so the twilight fill never makes a wall brighter than the sky behind it (the moonlit sky fill is added after that cap). At night the moon is the key light (cool and pale, dimmed by overcast) and the moonlit sky fills its shade; a moonless night keeps the classic ambient level. |
| Tonemap | shader `tonemapNeutral` | Khronos PBR Neutral without its toe: identity below 0.8 linear (authored texture colour is kept), a soft shoulder above it. |
| Sun shadows | shader `shadowMapVisibility` | The engine's cascaded shadow maps now dim only direct sun, never the sky fill: shadows have readable detail instead of a flat darkening. 12-tap Poisson over hardware 2x2 PCF. Casters use a slope-scaled normal offset; the terrain never casts into the map and gets no offset, so shadows stay attached to their casters at a low sun. Shadow maps follow the true sun down to 4 degrees (Classic clamps to 11.5), so evening shadows have their full length (`SceneShadowPass.cpp`). |
| Hill shadows and valley sky | `TerrainHorizon.cpp`, `World/Terrain/TerrainLighting.cpp` | A 16-azimuth horizon map built off-thread from the heightfield when an island loads. Ridges shadow the valleys behind them at any range (the shadow maps cover objects near the camera only) and valley floors see less sky. |
| Ground-coloured bounce | `GroundTint`, `TerrainLighting.cpp` `GroundColors` | The light bounced off the ground takes the colour of the landscape texture under each point (its average colour, baked with the horizon map): warm on sand, green over grass. The tint is luminance-neutral, so it changes the colour of the shade, never its brightness. |
| Cloud shadows | `CloudNoise.cpp`, shader `cloudSunVisibility` | Under broken cloud, the classic uniform sun dimming moves into drifting cloud shadows whose mean transmission equals the classic dimming, so the average frame brightness is unchanged. A clear sky casts none: `setOvercast 0` still reports 4% cover (the clear weather keyframe sits at overcast -0.08), so the shadows fade in from 5% cover and take the full cover from 10% (about overcast 0.1); the dimming they do not carry stays uniform. |
| Foliage | shader `shadeRemastered`, `CrownEllipsoid` | Half-lit materials (tree crowns, bushes, nets) are lit with the normal of an ellipsoid fitted to the shape's bounding box, with a wrapped terminator, light through the leaves inside the crown's own shadow, and less sky toward the crown's core. A tree reads as one volume with a lit side and a shaded side, not as noise from scattered leaf cards. Leaf cards authored without normals (zero mesh normals, common in CWA crowns) take the ellipsoid's normal outright. |
| Fog toward the sun | shader `fogColorToward` | Fog takes the sky dome's own sun glow (`SunSkyColor * cos^6`, as `TLVertexTable::DoSkyLighting` shades the dome). Distant ground fades into the colour of the sky behind it, which removes the flat band along the horizon at sunrise and sunset. Away from the sun it is exactly the classic fog. |
| Water | shader `s_psWaterRemasterMain` | Reflects the visible sky (the fog colour with its sun glow, so a low sun lays a path across the sea) with Fresnel. The classic bump glint becomes a tight sparkle that shadows and clouds put out. |
| Local lights, glossy | shader `localLights`, `glossy` | Headlights, lamps and fires per fragment, keeping the classic pool size. The sum is clamped where classic clamps it, so where pools overlap a lamp-lit wall is never brighter than the original's. Glossy materials get normalised Blinn-Phong and a sky reflection. |
| Post pass | `EngineGL33_PostFX.cpp` | At the world/HUD boundary (`Engine::EndWorldScene`, called from `World.cpp`): half-resolution ambient occlusion that darkens only the sky-light share of each pixel (opaque draws store 1 - that share in the colour alpha; blended draws keep the destination alpha), so a sunlit wall beside a corner keeps its sunlight. Bloom from near-white only (the sun, glints), so the sky does not haze. Screen-space sun shafts toward a low sun, kept in proportion to the air in front of each pixel (`ShaftInscatter`: 1 - exp(-distance / 300 m), a shorter length in thick fog), so the sky and distant ground carry them and a vehicle 10 m away is not veiled grey. A screen-style composite that is an exact identity where those are zero. |

Pre-lit draws (`VSScreen`: sky, clouds, particles, 2D) pass through unchanged.
Only shapes with a vertex buffer take the per-fragment path. A shape first
loaded after the mission starts (`createVehicle` of a class the mission does not
place, as Guerrilla Mode does for nearly every vehicle) gets its buffers from
`ShapeBank::OptimizeLate` before the next frame draws it, the same ones
`Shapes.OptimizeAll` gives the mission's own shapes; before that fix such
shapes drew pre-lit with the classic light and received no shadow maps. Their
crew views (gunner, driver, commander, cargo interiors) stay pre-lit: inside a
hull this lighting has no interior light, so a tank interior on the per-pixel
path is far darker (mission-placed vehicles still draw theirs per pixel).
The remastered programs are used only inside the world's 3D passes
(`EngineGL33::UpdateRemasterPrograms`). Portrait captures, 3D previews in
dialogs and night vision draw through the classic programs, because their own
light setups and the green intensifier are calibrated for them; night vision
keeps its sun shadows through the classic shadow test. If the remastered
programs fail to build, the engine runs Classic, projected shadows included.

Edge cases the post pass handles: a vehicle interior view clears the world's
depth, so that frame has no AO or shafts; a screen overlay drawn inside the
world pass (gunner optics, pillarbox bars) turns bloom and shafts off for the
frame so its black surround stays black; camera effects (cinema borders) draw
after the post pass. Shadow-map casters beyond a cascade's light near plane
are depth-clamped rather than clipped, so the long shadows of a 4-degree sun
are not cut off.

## Calibration

On-screen fraction of the texture brightness, after the default display gamma
of 1.2, clear sky, default tuning (before the tonemap shoulder):

| Sun | Exposure | Sunlit ground | Cast shadow | Wall facing sun | Wall facing away |
|---|---|---|---|---|---|
| 60 deg | 1.00 | 1.00 | 0.58 | 0.86 | 0.57 |
| 30 deg | 1.56 | 0.97 | 0.61 | 1.00 | 0.57 |
| 14 deg | 1.31 | 0.70 | 0.48 | 1.00 | 0.42 |
| 7 deg | 1.03 | 0.49 | 0.38 | 0.88 (warm) | 0.32 (cool) |

Classic shows sunlit ground and most shade at 1.0. Shade near 0.6 is bold but
well above the ~0.42 of the shadow-map darkening that reviewers called crushed.
`test_remaster_lighting.cpp` pins these ranges.

At night (same units, before the classic night-eye pass, which fades dim
pixels toward a grey): a full moon 60 degrees up shows moonlit ground at 0.29
and its shade at 0.17; a moonless night keeps the classic ambient, 0.08. The
night-eye grey of lit surfaces is slightly cool, so moonlit ground reads pale
and cool rather than neutral; lamp-lit surfaces keep their colour.

## Tuning

- Options > Graphics > **Lighting**: Classic or Remastered (the default on this
  branch). Remastered turns the shadow maps on unless the shadow quality is Off.
- Options > Graphics > **Anti-aliasing**: 4x MSAA by default. Opaque cutouts
  (foliage, fences) use alpha-to-coverage with alpha-to-one; blended draws
  (camouflage nets, glass) keep plain blending. A graphics.cfg older than v3
  holding the old default Off is lifted to 4x once, but only when it has no
  supersampling (render scale 1.0): a supersampled profile already has
  anti-aliasing and keeps its Off. Off chosen after that stays.
- **Feature switches and gates** (`FeatureSwitches.{hpp,cpp}`,
  `FeatureGates.{hpp,cpp}`): every lighting feature of the integration plan
  (`research/INTEGRATION-PLAN.md` section 3.1) has a switch, a float in
  `LightingTuning::features` (0 off, 1 on, all on by default) that is also a
  `triSetLighting` key, and a frame gate computed once per frame at the world
  pass (`EngineGL33::UpdateFeatureGates`) that runs an enabled feature only
  where it can show (a low sun for beams, wet weather for wet shading, and so
  on; the thresholds are named in `FeatureGates.hpp`). `gates=0` forces every
  enabled feature on, for debugging. Every switch carries the step that makes
  it do something; until then it is inert and its on and off look the same.
- Dev overlay, **Lighting** tab, generated from the same registry: sliders for
  exposure, sun, sky, bounce, warmth, saturation, specular, local lights,
  moonlight, terrain shadows and occlusion, cloud shadows, shadow softness, AO,
  bloom and shafts, debug views (sun visibility, sky visibility, normals, AO),
  a **Features** section (per area, one checkbox per switch with its live gate,
  "active" or "gated: <reason>", and its GPU section once it has one) and a
  summary line that is a `triSetLighting` setting, to copy back.
- Trident / SQF: `triSetLighting "classic"`, `"remastered"` (every switch on)
  or `"baseline"` (every switch off: the look of the branch cut); each puts
  every knob back to its default. `"key=value"` with any registry key, or a
  comma list applied in order, all or none (`"remastered,taa=0"`), for
  scripted A/B captures. `triFeatureState` answers every switch with its gate;
  `triPinCloudDrift` holds the cloud shadows and sky clouds still;
  `triSetFixedStep` fixes the simulation step. `triPinObjectDetail 1` holds the
  object detail at its finest so both shots draw the same objects, and
  `triSettleScene` waits until a view has finished loading
  (`tests/rendering/captures/README.md`).
- **GPU time**: the Lighting tab's *Measure GPU time* checkbox, or
  `triGpuTimes true`, turns on timestamp queries around the frame, the world
  (with the shadow pass inside it) and each post step
  (`EngineGL33_GpuTimer.cpp`, `Graphics/Core/GpuTimings`). Results trail the
  frame by a few frames and are smoothed; nothing waits on the GPU. Toggle
  Remastered off for the same view's Classic numbers.
- **Shader iteration**: with `CWR_SHADER_OVERRIDE_DIR` set, every shader can be
  replaced by `<dir>/<name>.glsl` (dump them with
  `tests/rendering/lighting_lab/glsl_extract.py --dump`, which writes the
  sources the engine assembles). The remastered fragment shaders also take the
  pieces `psRemasterCommon.glsl` (the whole prelude, all its concern pieces
  joined) and `<ps>RemasterMain.glsl`, the post shaders their own names
  (`psPostAo`, ...).
  The Lighting tab's *Reload shaders* rebuilds every program at the start of
  the next frame, so edits show without a restart. The shader binary cache is
  keyed on the override files too.

## Files

| File | Role |
|---|---|
| `Graphics/Remaster/LightRig.{hpp,cpp}` | Light rig, exposure meter, tonemap, sun chroma, crown ellipsoid. Pure math. |
| `Graphics/Remaster/TerrainHorizon.{hpp,cpp}` | Horizon map builder and the CPU reference of the shader's sun visibility. Pure math. |
| `Graphics/Remaster/CloudNoise.{hpp,cpp}` | Tileable, histogram-equalised cloud noise. Pure math. |
| `Graphics/Remaster/FeatureSwitches.{hpp,cpp}` | The registry of every `triSetLighting` key: knobs and feature switches, labels, groups, mode words, the summary line. Pure. |
| `Graphics/Remaster/FeatureGates.{hpp,cpp}` | Per-frame gates of the features (whether an enabled feature runs, at what resolution, why) and the `triFeatureState` line. Pure. |
| `World/Terrain/TerrainLighting.{hpp,cpp}` | Keeps the horizon map in step with the landscape (async build, hand-over on the render thread). |
| `PoseidonGL33/EngineGL33_LightingShaders{,Sky,Lights,Surface}.cpp` | The remastered GLSL: the vertex shaders, the fragment mains and the fragment prelude split by concern (`FragmentPiece`: header, `LitConstants`, inputs, shadows, terrain and clouds, sky, local lights, surface, shade, finish), joined in that order. The `LitConstants` block is one string that every shader reading the rig pastes. |
| `PoseidonGL33/EngineGL33_Lighting.{hpp,cpp}` | `LitConstants` layout (binding 3) and uploads, the slots planned for the features still to land, the texture unit table (units 2-5 in use, 6 and 8-15 allocated) and the sampler table the programs are bound from, program-set selection. |
| `PoseidonGL33/EngineGL33_PostFX.cpp`, `EngineGL33_PostShaders.cpp` | The post pass, and its GLSL (no GL code there, so the unit tests compile the sources the engine builds), assembled by the `kPostShaders` table; `s_glslPostCommon` holds the shared depth reconstruction. |
| `PoseidonGL33/EngineGL33_GpuTimer.cpp`, `Graphics/Core/GpuTimings.{hpp,cpp}` | GPU timestamp queries and their bookkeeping. |
| `tests/unit/.../Graphics/test_remaster_lighting.cpp` | Rig calibration, horizon map, cloud noise, crown ellipsoid, `LitConstants` layout against the GLSL block (one declaration, slot by slot), glslang compile of every remastered and post program through the accessors, every raw GLSL string reaching a program, the assembly tables the lab parses, the texture unit table. |
| `tests/unit/.../Graphics/test_feature_switches.cpp`, `.../Game/test_tri_capture_verbs.cpp` | The registry (keys, labels, defaults of 1, `baseline` and `remastered`, the summary round trip), one case per gate row of plan section 3.2, and the capture verbs (`triSetLighting`, `triFeatureState`, `triSetFixedStep`, `triPinCloudDrift`). |
| `tests/rendering/lighting_lab/` | Offline lab: renders a synthetic scene through the real engine GLSL and C++ rig (`--renderer remaster`). |

## What has and has not been validated

- Validated: every remastered and post shader compiles (glslang) and links and
  renders on Mesa llvmpipe in the lab, using the engine's own GLSL, light rig,
  horizon map and cloud noise; the unit tests above; the GL-state, debug-group,
  buffer, IBO, shutdown and UBO source audits.
- Not validated here: an in-game run on real data and a real GPU, the GPU
  cost (the timers exist but have not run on the target hardware), MSAA with
  alpha-to-coverage, cockpits, night vision, particles and the 3D UI previews. The lab's scene and textures are
  procedural stand-ins, so absolute colours are not those of CWA.
