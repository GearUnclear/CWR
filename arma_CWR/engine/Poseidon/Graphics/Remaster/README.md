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
| Light rig | `LightRig.cpp` | The sun's colour follows its height (near-neutral by day, red at the horizon; air-mass extinction). Below 16 degrees it takes Classic's own sunset colour (`EveningSunChroma`: 0.65 of LightSun's colorFull hue, taken to linear because Classic applies it in display space), so a sunlit vehicle at golden hour has the original's warm hue instead of a ~5600 K white. The sky is a hemisphere: zenith, ground bounce, and a horizon band split into a warmer, brighter sunward half and a cool anti-sun half, so low sun gives warm light and cool shade. The low-sun rig (switch `lowSunRig`) starts the evening hue at 25 degrees, where Classic's own sunset ramp starts (`LowSunHueShare`: a further 0.4 of Classic's hue from 25 degrees down and 0.3 more below 9), and gives vertical faces sideways fill below 40 degrees (`LowSunFill`: the sky part of the anti-sun horizon x(1 + 2.6 fill), the sunward one x(1 + 0.5 fill), the ground bounce x(1 + 1.3 fill)), so the side of a hull turned away from a 5 to 13 degree sun shows at about 0.6 of its sunlit side instead of 0.3 to 0.45; up-facing ground and its cast shadows keep their light, and the fill lifts a wall at most to the visible sky (wherever the fill runs, not only in twilight; a clear sky does not bind it). The sun shafts take the rig's sun colour, so the low-sun hue tints them too. An exposure meter keeps sunlit ground at Classic's on-screen brightness from noon down to about 20 degrees, then lets golden hour darken into a warm, contrasty frame. In twilight the fill is capped by the visible sky, so the twilight fill never makes a wall brighter than the sky behind it (the moonlit sky fill is added after that cap). At night the moon is the key light (cool and pale, dimmed by overcast) and the moonlit sky fills its shade; a moonless night keeps the classic ambient level. |
| Tonemap | shader `tonemapNeutral` | Khronos PBR Neutral without its toe: identity below 0.8 linear (authored texture colour is kept), a soft shoulder above it. |
| Sun shadows | shader `shadowMapVisibility` | The engine's cascaded shadow maps now dim only direct sun, never the sky fill: shadows have readable detail instead of a flat darkening. 12-tap Poisson over hardware 2x2 PCF. Casters use a slope-scaled normal offset; the terrain never casts into the map and gets no offset, so shadows stay attached to their casters at a low sun. Shadow maps follow the true sun down to 4 degrees (Classic clamps to 11.5), so evening shadows have their full length (`SceneShadowPass.cpp`). |
| Hill shadows and valley sky | `TerrainHorizon.cpp`, `World/Terrain/TerrainLighting.cpp` | A 16-azimuth horizon map built off-thread from the heightfield when an island loads. Ridges shadow the valleys behind them at any range (the shadow maps cover objects near the camera only) and valley floors see less sky. |
| Long tree shadows (switch `canopyShadows`) | `Canopy.cpp`, `World/Terrain/TerrainLighting.cpp` `GatherCanopy`, `PoseidonGL33/EngineGL33_Canopy.cpp`, shader `canopySunVisibility` | At a low sun (2 to 25 degrees, fading over both ends, sky more than half clear) every tree, bush, forest block and building casts its long shadow beyond the shadow maps' reach too, so whole hillsides of trees stand on their shadows at golden hour. The landscape objects are drawn top-down into a canopy heightfield at 6.25 m when an island loads (on the horizon map's worker; a destroyed tree or building takes its footprint out). A GPU pass cone-traces that canopy toward the sun into a world-space map around the camera (2048 texels a side, 1.6 to 3.1 m a texel by the view distance), only when the sun has moved 0.05 degrees, the camera has left the middle of the map or the canopy changed, so it is still in motion and costs nothing on frames that do not rebuild. The world reads it once per fragment beyond the shadow maps, cross-faded over their far fade: the ground, and raised receivers (crowns, walls, roofs) that never shade themselves. It only takes sun away; the shade is the sky fill, cooler than the lit ground. |
| Ground-coloured bounce | `GroundTint`, `TerrainLighting.cpp` `GroundColors` | The light bounced off the ground takes the colour of the landscape texture under each point (its average colour, baked with the horizon map): warm on sand, green over grass. The tint is luminance-neutral, so it changes the colour of the shade, never its brightness. |
| Cloud shadows | `CloudNoise.cpp`, shader `cloudSunVisibility` | Under broken cloud, the classic uniform sun dimming moves into drifting cloud shadows whose mean transmission equals the classic dimming, so the average frame brightness is unchanged. A clear sky casts none: `setOvercast 0` still reports 4% cover (the clear weather keyframe sits at overcast -0.08), so the shadows fade in from 5% cover and take the full cover from 10% (about overcast 0.1); the dimming they do not carry stays uniform. |
| Foliage | shader `shadeRemastered`, `CrownEllipsoid` | Half-lit materials (tree crowns, bushes, nets) are lit with the normal of an ellipsoid fitted to the shape's bounding box, with a wrapped terminator, light through the leaves inside the crown's own shadow, and less sky toward the crown's core. A tree reads as one volume with a lit side and a shaded side, not as noise from scattered leaf cards. Leaf cards authored without normals (zero mesh normals, common in CWA crowns) take the ellipsoid's normal outright. With the crown canopy (switch `crownCanopy`) a crown is lit as a canopy: the sun reaching a leaf crosses the crown in front of it (optical depth 1 per crown radius, a quarter of the sun always passes), the core sees 0.45 of the sky instead of 0.6, the leaf albedo deepens its saturation at equal luminance (light bounced between leaves), and below 35 degrees of sun, fully below 12, the crown takes 0.65 of the sun and 0.75 of the sky, so golden-hour crowns read green instead of pale grey-green. Grass shades as terrain and keeps its look. |
| Fog toward the sun | shader `fogColorToward` | Fog takes the sky dome's own sun glow (`SunSkyColor * cos^6`, as `TLVertexTable::DoSkyLighting` shades the dome). Distant ground fades into the colour of the sky behind it, which removes the flat band along the horizon at sunrise and sunset. Away from the sun it is exactly the classic fog. |
| Sky table (switch `atmosphere`) | `Atmosphere.cpp`, `PoseidonGL33/EngineGL33_SkyTable.cpp`, shaders `shadeSkyDome`, `atmoFogColor` | A physically based clear sky (Rayleigh, Mie, ozone and multiple scattering, after Hillaire 2020) shades the sky dome under the original sky texture's clouds and gives the fog of every world draw its colour (aerial perspective, the far horizon polygon included). A low sun gets a golden horizon band and an aureole, dusk an orange band under a mauve sky, sunrise keeps its pink with a coral glow at the sun, noon a graded blue with its cirrus. Each sky row keeps the original dome's brightness on average (its sun glow included); the table supplies how a direction differs from its row, and the hue is half way between the original's and the physical one (`atmoHue` 0.5, `atmoSat` 1.15). The clouds are the texture's own, masked against each row's clearest texel and lit by the sun seen from 2.5 km. The transmittance and multiple-scattering tables are built on a worker thread when the haze changes; the 192x64 sky-view table on the GPU, in two fullscreen passes, whenever the sun moves 0.02 degrees. It fades out between -3 and -9 degrees of sun (night keeps the classic sky); an overcast deck keeps the classic fog colour. |
| Sky clouds (switches `litClouds`, `skyCloudShadows`) | `SkyDensity.cpp`, `PoseidonGL33/EngineGL33_SkyClouds.cpp`, shaders `litCloudTermsDome`, `litCloudTermsSheet`, `domeCloudCover` | The sky's own painted clouds become a density map (built from the weather sky texture on a worker thread when the weather sky changes: whiteness above the clearest sky of the same row, a grey row as an overcast deck, darkness as thickness; no new art). Lit clouds: a short march from each sky pixel toward the sun through the dome's own texture mapping gives mean-free relief, so at a low sun the sun-facing edges glow peach and the far sides go slate, with a silver lining on thin rims near the sun, darker thick cloud against the sun and deeper bellies under a deck; every one of these terms fades out between 25 and 45 degrees of sun, so a midday sky is the sky before (the relief embossed it, and the darkening against the sun on its own made it only darker; above 45 degrees the shaders skip the march), and they work on luminance tinted only by the sun's colour (no colour fringes). The sky's mean brightness is kept within about 1.5 %. Sky-matched shadows: a lit point is projected along the sun to a deck at 1800 m and that point is looked up in the map in the direction an anchor near the camera sees it, so the cloud in the sky is the cloud whose shadow lies on the ground (and the sun disk dims behind it); the anchor stays put until the camera is 1.5 km away, then steps and cross-fades, so shadows do not slide under a moving player. They keep the classic overcast dimming on average over the sky's own covered share (`LightRigInputs::skyShadowCoverage`). The dome turns about the zenith with the sky clouds' drift (`Landscape::CloudsPosition`, about 6 degrees a minute at overcast 0.5), its clouds, their light and their shadows together. |
| Sea waves (switch `seaWaves`) | `SeaWaves.cpp`, `PoseidonGL33/EngineGL33_Sea.cpp`, shader `s_psWaterRemasterMain`, `skyReflection` | The sea is the landscape's own faces, which carry the specular layer alone and used to draw through the detail shader like the terrain (the engine's `IsWater` family was never used for it). `Landscape::DrawWater` now opens `Engine::BeginSea` around them, and with the switch on the backend draws them with the water program. The sea texture's brightness is the wave height: the texture, a swell copy at 0.27x frequency and the ripple pattern of `specular_dx` give a wave normal (the texture-to-world mapping solved from screen derivatives), fading from 400 m to 3 km. The sea is its lit texture under the sky reflected by Fresnel on that normal, with a low mean share (no pale mirror at grazing range) and the waves' share at full weight, so rows of waves show to the horizon; the sky is the one the player sees (the sky table's when it is on, else the fog colour with its sun glow). The sun lays a broad path and a tight sparkle on the waves, and the material's own broad lobe keeps the original's sheen in the original sun's hue, which is what made the original's noon sea green-yellow: at a high sun it grows and moves to the flat normal, a smooth wash as the original's per-vertex sheen, and the mean sky share halves (noon hue within 8 degrees of the original's). The landscape's 7-frame animation is cross-faded (the neighbouring frame on texture unit 11), so the waves do not pop every 0.14 s. Sub-knobs `seaRelief`, `seaSky`, `seaSheen`. |
| Local lights, glossy | shader `localLights`, `glossy` | Headlights, lamps and fires per fragment, keeping the classic pool size. The sum is clamped where classic clamps it, so where pools overlap a lamp-lit wall is never brighter than the original's. Glossy materials get normalised Blinn-Phong and a sky reflection. |
| Post pass | `EngineGL33_PostFX.cpp` | At the world/HUD boundary (`Engine::EndWorldScene`, called from `World.cpp`): half-resolution ambient occlusion that darkens only the sky-light share of each pixel (opaque draws store 1 - that share in the colour alpha; blended draws keep the destination alpha), so a sunlit wall beside a corner keeps its sunlight. Bloom from near-white only (the sun, glints), so the sky does not haze. Screen-space sun shafts toward a low sun, kept in proportion to the air in front of each pixel (`ShaftInscatter`: 1 - exp(-distance / 300 m), a shorter length in thick fog), so the sky and distant ground carry them and a vehicle 10 m away is not veiled grey. A screen-style composite that is an exact identity where those are zero. |
| Temporal anti-aliasing | `TemporalAA.cpp`, `EngineGL33_PostTemporal.cpp` | The world is drawn with a sub-pixel jitter (Halton 2, 3 over 16 frames) and each frame is blended into a history of the frames before it (switch `taa`): the history is moved with the camera motion and the depth, clipped to the colours around each pixel and mixed in at 0.1, then sharpened back into the frame target (bounded, no halos). Alpha-tested leaves and wires stop crawling, sub-pixel wire stays continuous, and the AO and shaft sample patterns turn every frame so the history averages them. Smoke, fire, muzzle flashes, glass and rain take most of their own frame (a particle mask from a copy of the frame made between the opaque and the translucent objects), so they do not smear. MSAA stays under it; a texture LOD bias of -0.5 wins back texture detail. |
| Terrain detail | `TerrainDetail.cpp`, the terrain main `s_psDetailRemasterMain` | Every terrain square is one 128 px texture over 50 m, so slopes at 10 to 150 m were a bilinear smear (switch `terrainDetail`). Each land fragment takes the luminance high-pass of its own square's texture, zoomed six times and read within a few metres (grass grain from grass, rock grain from rock), in 78 cm cells, each cell's patch turned by its own random angle and the four cells blended so the grain keeps its contrast, through a soft limiter. The authored detail texture keeps its grain as its mips average it out (at most 2x) with a rotated second sample against tiling. Brightness only, relative to the local mean: hue and average brightness stay as authored. It fades in past the near ground (the detail texture's pebbles stay as authored within about 10 m) and out by 300 m, and the sea, drawn by the same shader, is left alone (the backend flags the land's draws). With TAA off, a contrast-adaptive sharpen of brightness only (switch `sharpen`, strength 0.3) takes the place of TAA's sharpen. |
| Air (switches `airBeams`, `airHaze`, `valleyMist`) | `AirMedium.cpp`, `PoseidonGL33/EngineGL33_PostAir.cpp`, `EngineGL33_PostAirShaders.cpp` | One post stage after the occlusion. Sun beams: the sun's single scattering in a ground mist, marched at half resolution over the first 250 m with its visibility from the shadow map, the hills and the cloud field, so crowns, roofs and ridges cut shafts out of it; at night the same for headlight cones. They replace the screen-space shafts while they run. The mist follows the weather: thin in clear air (only narrow beams through gaps, no glow over open ground), thick in setFog and rain and with the sun at the horizon (fans of god rays). Aerial haze: beyond 150 m a surface fades toward the sky behind it (the sky table's fog colour) through air that thins with height, so low ground is hazier than ridges and the shadow of a ridge in the haze stays grey; the classic fog then keeps only the view-distance edge (its start moves out to 55% of its end in clear weather). Valley mist: at dawn (an eastern sun up to 10 degrees) and in setFog, a flat-topped layer about 40 m above the lowest ground within 850 m, only where that ground lies in a valley, so it pools on valley floors and along shores and leaves ridges, plains and the open sea clear. The near field is untouched; noon in clear weather keeps only the far haze. |

Pre-lit draws (`VSScreen`: sky, clouds, particles, 2D) pass through unchanged
(inside the world pass they take the TAA jitter with the rest of the world),
except under the sky table: the dome is then shaded from it and pre-lit world
draws fade into its horizon colour with their fog. The dome and the sky clouds
come before the world's first 3D pass; `Landscape::DrawSky` and `DrawClouds`
open the remastered programs for them through one hook
(`Engine::BeginRemasterSky` / `EndRemasterSky`), which does nothing while the
sky table is off.
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
are not cut off. TAA runs only in the world's own passes: the HUD, dialogs,
portraits, 3D previews and the shadow-map depth pass are never jittered; a
vehicle interior, a screen overlay (optics) or night vision gates it off, and
any frame without a resolve, a camera cut (over 50 m or 60 degrees in a frame)
or a new frame size starts the history again.

## Calibration

On-screen fraction of the texture brightness, after the default display gamma
of 1.2, clear sky, default tuning (before the tonemap shoulder):

| Sun | Exposure | Sunlit ground | Cast shadow | Wall facing sun | Wall facing away |
|---|---|---|---|---|---|
| 60 deg | 1.00 | 1.00 | 0.58 | 0.86 | 0.57 |
| 30 deg | 1.56 | 0.97 | 0.61 | 1.00 | 0.66 (0.57) |
| 14 deg | 1.31 | 0.70 | 0.48 | 1.00 | 0.64 (0.42) |
| 7 deg | 1.03 | 0.49 | 0.38 | 0.90 (warm) | 0.49 (0.32) (cool) |

Classic shows sunlit ground and most shade at 1.0. Shade near 0.6 is bold but
well above the ~0.42 of the shadow-map darkening that reviewers called crushed.
The walls are with the low-sun rig (`lowSunRig`); in brackets without it, where
a wall turned from a low sun fell to 0.32 to 0.42. The ground and its cast
shadows are the same either way, and so is everything from 40 degrees up.
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
  A feature's sub-knobs sit under its switch: TAA's `taaSharpen` (0.5),
  `taaMipBias` (-0.5) and `taaAlpha` (0.1), the terrain detail's
  `detailStrength` (1.5), the luma sharpen's `sharpenStrength` (0.3) and the beams'
  `volGain` (-1: from the weather, 4 in clear air, 2.5 in mist).
- Dev overlay, **Lighting** tab, generated from the same registry: sliders for
  exposure, sun, sky, bounce, warmth, saturation, specular, local lights,
  moonlight, terrain shadows and occlusion, cloud shadows, shadow softness, AO,
  bloom and shafts, debug views (sun visibility, sky visibility, normals, AO,
  the post pass's half-resolution linear depth, the lights per tile, the surface albedo: with
  `post=0` a shot of it divides a lit shot into the light on every surface, the air pass's
  in-scatter alone),
  a **Features** section (per area, one checkbox per switch with its live gate,
  "active" or "gated: <reason>", and its GPU section once it has one) and a
  summary line that is a `triSetLighting` setting, to copy back.
- Trident / SQF: `triSetLighting "classic"`, `"remastered"` (every switch on)
  or `"baseline"` (every switch off: the look of the branch cut); each puts
  every knob back to its default. `"key=value"` with any registry key, or a
  comma list applied in order, all or none (`"remastered,taa=0"`), for
  scripted A/B captures. `triFeatureState` answers every switch with its gate;
  `triPinCloudDrift` holds the cloud shadows and sky clouds still;
  `triSetFixedStep` fixes the simulation step; `triSkyTableCheck <texels>`
  reads the GPU sky-view table back and compares it with its CPU mirror
  (`SkyViewTexel`, `SkyViewRowMean`, `SkyViewRowGlow`), and `triCanopyCheck
  <texels>` does the same for the canopy's sun-visibility map (`CanopySunAt`). The sky table's
  sub-knobs are `atmoHue`, `atmoSat` and `atmoHaze`. `triPinObjectDetail 1` holds the
  object detail at its finest so both shots draw the same objects, and
  `triSettleScene` waits until a view has finished loading
  (`tests/rendering/captures/README.md`).
- **GPU time**: the Lighting tab's *Measure GPU time* checkbox, or
  `triGpuTimes true`, turns on timestamp queries around the frame, the world
  (with the shadow pass inside it), the remastered sky dome (`sky`, with the
  sky-view table's build, `skytable`, on the frames that rebuild it) and each
  post stage that ran (resolve, linear depth, AO, air, bloom, shafts, composite,
  taa, sharpen, in the order of the stage list; TAA's copy for its particle
  mask lands in the world section)
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
| `Graphics/Remaster/Atmosphere.{hpp,cpp}` | The sky table's atmosphere: its constants, the transmittance and multiple-scattering tables, the sky-view parametrisation and the CPU mirrors of the two GPU passes (`SkyViewTexel`, `SkyViewRowMean`, `SkyViewRowGlow`), the lookup the shaders make, and the frame rules (rebuild, night fade, overcast floor). Pure math. |
| `Graphics/Remaster/SkyDensity.{hpp,cpp}` | The sky's own clouds: the dome's texture mapping, the cloud-density map of a weather sky and its shadow coverage, the constants the shaders mirror (deck, threshold, relief, silver lining, the low-sun fade), the sky's turn and the shadow anchor. Pure math. |
| `PoseidonGL33/EngineGL33_SkyClouds.cpp` | The density map on unit 9 (read once per weather sky file, decoded, mixed and built on a worker thread, a stand-in until the first), the shadow anchor and the `Sky` and `SkyShadow` slots of `LitConstants`. |
| `PoseidonGL33/EngineGL33_SkyTable.cpp`, `EngineGL33_PostSkyTableShaders.cpp` | The sky table on the GPU: the worker build of the atmosphere tables, the two fullscreen passes that build the sky-view table (their GLSL in a GL-free file, rows of `kPostShaders`), the sky pass hook and its `LitConstants` slots, and `triSkyTableCheck`. |
| `Graphics/Remaster/PostStages.{hpp,cpp}` | The post pass as a fixed list of stages (plan section 3.6), each with its GPU mark, and the plan of a frame: which stages run and at what strength (the early return asks every stage). Pure. |
| `Graphics/Remaster/TemporalAA.{hpp,cpp}` | TAA's math: the Halton jitter and the rule that gives a pass its jitter, the camera reprojection built in double precision (the C++ mirror of the resolve's), the cut and history-chain rules, the particle mask thresholds. Pure. |
| `Graphics/Remaster/TerrainDetail.{hpp,cpp}` | The terrain detail's constants and the C++ mirror of the terrain main's math: the cell hash, rotation and patch position, the variance-preserving blend, the soft limiter, the near and far fades, the detail-kept boost and the anti-tiling weight, and the frame constants (zero when the switch or its gate is off). Pure. |
| `Graphics/Remaster/AirMedium.{hpp,cpp}` | The air pass's medium: the haze's, the valley mist's and the beams' constants, the weather and time rules (`ComputeAirMedium`), the closed forms of the haze and the mist the shaders mirror, the valley floor and its depth, the classic fog's moved edge, and `AirOpticalDepth` for a line of sight (for AI sight). Pure math. |
| `PoseidonGL33/EngineGL33_PostAir.cpp`, `EngineGL33_PostAirShaders.cpp` | The `Air` post stage: the medium of the frame and the classic fog's edge (`UpdateAirMedium`, `litHaze`), the half-resolution march, blur and mean, the full-resolution apply; the GLSL in a GL-free file, assembled by `kPostShaders` with the prelude's `LitConstants`, terrain and clouds and sky pieces. |
| `World/Terrain/TerrainLighting.{hpp,cpp}` | Keeps the horizon map and the canopy in step with the landscape (async build, hand-over on the render thread); gathers the canopy's footprints from the landscape objects and takes a destroyed object's out (`TerrainCanopyObjectDestroyed`, from `Object::SetDestroyed`). |
| `Graphics/Remaster/Canopy.{hpp,cpp}` | The long tree shadows: the canopy raster and its mip rules, the removal of a destroyed object, the C++ mirrors of the GPU trace (`CanopySunAt`) and of the world's read (`CanopyReceiverVisibility`), the sun-visibility map's placement and rebuild rule, the strength over the gate's edges, the frame constants. Pure. |
| `PoseidonGL33/EngineGL33_Canopy.cpp`, `EngineGL33_PostCanopyShaders.cpp` | The canopy on the GPU: its upload (whole, or the rectangles a removal changed), the fullscreen pass that builds the sun-visibility map (its GLSL in a GL-free file, a row of `kPostShaders`), the `Canopy` slot of `LitConstants`, the stand-ins on units 10 and 15, and `triCanopyCheck`. |
| `PoseidonGL33/EngineGL33_LightingShaders{,Sky,Lights,Surface}.cpp` | The remastered GLSL: the vertex shaders, the fragment mains and the fragment prelude split by concern (`FragmentPiece`: header, `LitConstants`, inputs, shadows, terrain and clouds, sky, local lights, surface, shade, finish), joined in that order. The `LitConstants` block is one string that every shader reading the rig pastes. |
| `PoseidonGL33/EngineGL33_Lighting.{hpp,cpp}` | `LitConstants` layout (binding 3) and uploads, the slots planned for the features still to land, the texture unit table (units 2-6, 8 to 11, 13 and 15 in use, 12 and 14 allocated) and the sampler table the programs are bound from, program-set selection. |
| `PoseidonGL33/EngineGL33_PostFX.cpp`, `EngineGL33_PostShaders.cpp` | The post pass: its stages run in list order on one `PostFrame` (`EndWorldScene`), each followed by its GPU mark; the half-resolution linear depth (R32F, view depth in metres) for the stages that trace or upsample at half resolution. Its GLSL (no GL code there, so the unit tests compile the sources the engine builds) is assembled by the `kPostShaders` table; `s_glslPostCommon` holds the shared depth reconstruction. |
| `PoseidonGL33/EngineGL33_PostTemporal.cpp`, `EngineGL33_PostPass.hpp` | TAA in the backend: the jitter and LOD bias of the world passes (`ApplyTaaJitter`, taken off in `EndWorldScene` and `InitDraw`), the opaque copy for the particle mask (`BeginTranslucentObjects`, called by the scene before its translucent objects), the Temporal and Sharpen stages (with TAA off the Sharpen stage is the luma sharpen); the GL helpers the post stages share. |
| `PoseidonGL33/EngineGL33_GpuTimer.cpp`, `Graphics/Core/GpuTimings.{hpp,cpp}` | GPU timestamp queries and their bookkeeping. |
| `tests/unit/.../Graphics/test_remaster_lighting.cpp` | Rig calibration, horizon map, cloud noise, crown ellipsoid, `LitConstants` layout against the GLSL block (one declaration, slot by slot), glslang compile of every remastered and post program through the accessors, every raw GLSL string reaching a program, the assembly tables the lab parses, the texture unit table. |
| `tests/unit/.../Graphics/test_post_stages.cpp`, `test_gpu_timings.cpp` | The stage list (order, marks, sections, what has landed), the plan of a frame against the pass as it ran before the list, the linear depth running only for a reader, the TAA stages running together and only on a jittered frame, and the backend running and marking every landed stage. |
| `tests/unit/.../Graphics/test_temporal_aa.cpp` | The jitter sequence and range, the jitter of a pass, the reprojection against hand values and camera pairs, cuts and the history chain, the jitter in the remastered vertex shaders only, the resolve and sharpen contracts, and source audits that the jitter and LOD bias never reach the HUD and that the opaque copy sits between the opaque and the translucent objects. |
| `tests/unit/.../Graphics/test_atmosphere.cpp` | The atmosphere against an independent brute-force integration (the vertical optical depth in closed form, transmittance and sky-view texels at noon, sunset and 4 degrees under), the colours of a clear day, a sunset and a twilight, the table's lookups and row means, and the frame rules, switch, gate and sub-knobs of the sky table. |
| `tests/unit/.../Graphics/test_canopy.cpp` | The canopy raster on synthetic crowns, boxes, hulls and slopes, its mip rules, a removal equal to a build without the object at every level, the trace's shadow length and its sqrt crossing density, the raised receivers' own-crown skip and smooth edge, the map's placement and rebuild rule, the strength over the gate's edges, the frame constants, the canopy's GPU section, the shader constants against the mirror's, and source audits of units 10 and 15. |
| `tests/unit/.../Graphics/test_terrain_detail.cpp` | The terrain detail's mirror (limiter, blend variance, rotation spread, locality and zoom of a cell's patch, the near fade on flat ground at 1080p, the 300 m fade, the 2x cap), its frame constants and gate, the shader's constants against the mirror's, and source audits that only the land's draws and only the terrain main take it. |
| `tests/unit/.../Graphics/test_sky_density.cpp` | The dome mapping, the density map on a clear, a broken and an overcast sky made like the shipped ones, the blur, shadow cover and coverage, the low-sun fade (the lit-cloud terms identity from 45 degrees of sun up, lighter as well as darker below), the sky's turn, the shadow anchor, and the rig keeping the overcast dimming over the sky's covered share; `test_remaster_lighting.cpp` pins the shaders' constants and mapping to it and the clouds' off contract. |
| `tests/unit/.../Graphics/test_air_medium.cpp` | The air's closed forms against numeric integration, the valley floor on a synthetic valley, the morning and weather rules, the classic fog's moved edge, the shaders' constants and pieces against the medium's, "off" leaving every draw and stage as it was, unit 2 around the march. |
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
