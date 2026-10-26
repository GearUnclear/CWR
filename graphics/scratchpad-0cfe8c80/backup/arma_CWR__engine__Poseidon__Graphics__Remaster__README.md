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
| Light rig | `LightRig.cpp` | The sun's colour follows its height (near-neutral by day, red at the horizon; air-mass extinction). Below 16 degrees it takes Classic's own sunset colour (`EveningSunChroma`: 0.65 of LightSun's colorFull hue, taken to linear because Classic applies it in display space), so a sunlit vehicle at golden hour has the original's warm hue instead of a ~5600 K white. The sky is a hemisphere: zenith, ground bounce, and a horizon band split into a warmer, brighter sunward half and a cool anti-sun half, so low sun gives warm light and cool shade. An exposure meter keeps sunlit ground at Classic's on-screen brightness from noon down to about 20 degrees, then lets golden hour darken into a warm, contrasty frame. In twilight the fill is capped by the visible sky, so a wall is never brighter than the sky behind it. Night keeps the classic ambient level. |
| Tonemap | shader `tonemapNeutral` | Khronos PBR Neutral without its toe: identity below 0.8 linear (authored texture colour is kept), a soft shoulder above it. |
| Sun shadows | shader `shadowMapVisibility` | The engine's cascaded shadow maps now dim only direct sun, never the sky fill: shadows have readable detail instead of a flat darkening. 12-tap Poisson over hardware 2x2 PCF. Casters use a slope-scaled normal offset; the terrain never casts into the map and gets no offset, so shadows stay attached to their casters at a low sun. Shadow maps follow the true sun down to 4 degrees (Classic clamps to 11.5), so evening shadows have their full length (`SceneShadowPass.cpp`). |
| Hill shadows and valley sky | `TerrainHorizon.cpp`, `World/Terrain/TerrainLighting.cpp` | A 16-azimuth horizon map built off-thread from the heightfield when an island loads. Ridges shadow the valleys behind them at any range (the shadow maps cover objects near the camera only) and valley floors see less sky. |
| Ground-coloured bounce | `GroundTint`, `TerrainLighting.cpp` `GroundColors` | The light bounced off the ground takes the colour of the landscape texture under each point (its average colour, baked with the horizon map): warm on sand, green over grass. The tint is luminance-neutral, so it changes the colour of the shade, never its brightness. |
| Cloud shadows | `CloudNoise.cpp`, shader `cloudSunVisibility` | Under broken cloud, the classic uniform sun dimming moves into drifting cloud shadows whose mean transmission equals the classic dimming, so the average frame brightness is unchanged. |
| Foliage | shader `shadeRemastered`, `CrownEllipsoid` | Half-lit materials (tree crowns, bushes, nets) are lit with the normal of an ellipsoid fitted to the shape's bounding box, with a wrapped terminator, light through the leaves inside the crown's own shadow, and less sky toward the crown's core. A tree reads as one volume with a lit side and a shaded side, not as noise from scattered leaf cards. Leaf cards authored without normals (zero mesh normals, common in CWA crowns) take the ellipsoid's normal outright. |
| Fog toward the sun | shader `fogColorToward` | Fog takes the sky dome's own sun glow (`SunSkyColor * cos^6`, as `TLVertexTable::DoSkyLighting` shades the dome). Distant ground fades into the colour of the sky behind it, which removes the flat band along the horizon at sunrise and sunset. Away from the sun it is exactly the classic fog. |
| Water | shader `s_psWaterRemasterMain` | Reflects the visible sky (the fog colour with its sun glow, so a low sun lays a path across the sea) with Fresnel. The classic bump glint becomes a tight sparkle that shadows and clouds put out. |
| Local lights, glossy | shader `localLights`, `glossy` | Headlights, lamps and fires per fragment, keeping the classic pool size. Glossy materials get normalised Blinn-Phong and a sky reflection. |
| Post pass | `EngineGL33_PostFX.cpp` | At the world/HUD boundary (`Engine::EndWorldScene`, called from `World.cpp`): half-resolution ambient occlusion that darkens only the sky-light share of each pixel (opaque draws store 1 - that share in the colour alpha; blended draws keep the destination alpha), so a sunlit wall beside a corner keeps its sunlight. Bloom from near-white only (the sun, glints), so the sky does not haze. Screen-space sun shafts toward a low sun. A screen-style composite that is an exact identity where those are zero. |

Pre-lit draws (`VSScreen`: sky, clouds, particles, 2D) pass through unchanged.
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

## Tuning

- Options > Graphics > **Lighting**: Classic or Remastered (the default on this
  branch). Remastered turns the shadow maps on unless the shadow quality is Off.
- Dev overlay, **Lighting** tab: live sliders for exposure, sun, sky, bounce,
  warmth, saturation, specular, local lights, moonlight, terrain shadows and
  occlusion, cloud shadows, shadow softness, AO, bloom and shafts, plus debug
  views (sun visibility, sky visibility, normals, AO) and a copyable summary
  line.
- Trident / SQF: `triSetLighting "classic"`, `"remastered"`, or
  `"key=value,..."` with the same keys, for scripted A/B captures.
- **GPU time**: the Lighting tab's *Measure GPU time* checkbox, or
  `triGpuTimes true`, turns on timestamp queries around the frame, the world
  (with the shadow pass inside it) and each post step
  (`EngineGL33_GpuTimer.cpp`, `Graphics/Core/GpuTimings`). Results trail the
  frame by a few frames and are smoothed; nothing waits on the GPU. Toggle
  Remastered off for the same view's Classic numbers.
- **Shader iteration**: with `CWR_SHADER_OVERRIDE_DIR` set, every shader can be
  replaced by `<dir>/<name>.glsl` (dump them with
  `tests/rendering/lighting_lab/glsl_extract.py --dump`). The remastered
  fragment shaders also take the pieces `psRemasterCommon.glsl` and
  `<ps>RemasterMain.glsl`, the post shaders their own names (`psPostAo`, ...).
  The Lighting tab's *Reload shaders* rebuilds every program at the start of
  the next frame, so edits show without a restart. The shader binary cache is
  keyed on the override files too.

## Files

| File | Role |
|---|---|
| `Graphics/Remaster/LightRig.{hpp,cpp}` | Light rig, exposure meter, tonemap, sun chroma, crown ellipsoid. Pure math. |
| `Graphics/Remaster/TerrainHorizon.{hpp,cpp}` | Horizon map builder and the CPU reference of the shader's sun visibility. Pure math. |
| `Graphics/Remaster/CloudNoise.{hpp,cpp}` | Tileable, histogram-equalised cloud noise. Pure math. |
| `World/Terrain/TerrainLighting.{hpp,cpp}` | Keeps the horizon map in step with the landscape (async build, hand-over on the render thread). |
| `PoseidonGL33/EngineGL33_LightingShaders.cpp` | The remastered GLSL. |
| `PoseidonGL33/EngineGL33_Lighting.{hpp,cpp}` | `LitConstants` layout (binding 3) and uploads, textures on units 2-5, program-set selection. |
| `PoseidonGL33/EngineGL33_PostFX.cpp` | The post pass. |
| `PoseidonGL33/EngineGL33_GpuTimer.cpp`, `Graphics/Core/GpuTimings.{hpp,cpp}` | GPU timestamp queries and their bookkeeping. |
| `tests/unit/.../Graphics/test_remaster_lighting.cpp` | Rig calibration, horizon map, cloud noise, crown ellipsoid, `LitConstants` layout against the GLSL block, glslang compile of every remastered and post shader. |
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
