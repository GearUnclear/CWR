# graphics-remaster: integration plan for the Remastered lighting research

Written 2026-10-17 from the files on disk. Nothing in this plan has been built or run; every
statement about the code, the prototypes or the fixes comes from reading them (the 15 prototype
patches and shader overrides, every track's NOTES.md, `tmp/research/MENU.md`, the 11 bug-fix
folders under `tmp/bugfix`, `git log 3c53141..HEAD`, and the current sources at 84408f8). Paths
are relative to the worktree root `D:\Arma_CWA_worktrees\lighting` unless they start with a drive.

Short names used below:

- **tip**: `claude/youthful-lovelace-8imbdh` at 84408f8 (the bug fixes landed on top of 3c53141).
- **F1..F11**: the 11 bug fixes (section 2).
- **Feature keys** such as `combatLights`: the runtime switches defined in section 3.1.
- **S mode / P mode**: the two capture modes of section 7.2.

---

## 1. Ground rules and environment

- **Branch.** `graphics-remaster` is cut from the tip only after F10 (object shadows at start-up)
  and F11 (4x MSAA default) have landed. This plan creates no branch and switches nothing.
  Creating the branch needs the owner's OK in chat (CLAUDE.md: no new branches without
  permission). Integration agents work only in the worktree made for that branch (suggested
  `D:\Arma_CWA_worktrees\graphics-remaster`), never in `D:\Arma_CWA`.
- **Parallel work** (pure-math modules, shader drafts) happens in detached scratch worktrees,
  exactly as the research tracks did, and comes back as patches. No side branches.
- **Toolchain** (every build shell):
  ```powershell
  $env:VCPKG_ROOT='C:\dev\vcpkg'
  $env:Path='C:\Program Files\LLVM\bin;C:\Program Files\CMake\bin;C:\Users\Dane\AppData\Local\Microsoft\WinGet\Packages\Ninja-build.Ninja_Microsoft.Winget.Source_8wekyb3d8bbwe;C:\Users\Dane\AppData\Local\Microsoft\WinGet\Packages\Ccache.Ccache_Microsoft.Winget.Source_8wekyb3d8bbwe\ccache-4.13.6-windows-x86_64;'+$env:Path
  cmake --preset win-x64-clang-rwdi -B build/graphics-remaster-rwdi   # once; stages to dist/graphics-remaster-rwdi
  cmake --build build/graphics-remaster-rwdi -j 6
  ```
  Unit tests run from `D:\` (the `/tmp` gotcha in `arma_CWR/CLAUDE.md`):
  `arma_CWR\build\graphics-remaster-rwdi\tests\unit\engine\Poseidon\PoseidonTests.exe "<tags>"`.
  Known pre-existing failures: `DDSConverter: generate DDS fixture ...` (`test_dds_converter.cpp:347`)
  and the localisation cases that need a `packages\Remaster` junction.
- **Games** only through `arma_CWR\tests\rendering\captures\run-captures.ps1 -GameDir
  dist/graphics-remaster-rwdi -DataJunction RemasterUD-graphics-remaster` (each parallel lane its
  own `RemasterUD-gr-<feature>`). Cleanup only through
  `D:\Arma_CWA\arma_CWR\tools\stop-owned-games.ps1 -DataDir <that junction>` (run-captures
  already calls it). No global `Stop-Process` on PoseidonGame.
- **Prose and comments** without em dashes. Commits end with the Co-Authored-By trailer.
  Every landing adds a log line to the lighting item in `roadmap/roadmap.yaml`.

---

## 2. Starting point: the tip and the 11 fixes

`git log 3c53141..HEAD` shows ten graphics commits and the roadmap commit 84408f8. The ten
commits come from nine fix folders (night-bright gave two, F9 and F9b); with the two pending
fixes that makes the eleven folders under `tmp/bugfix`. Every prototype was made against
3c53141, so each must be ported over these.

| Id | Commit | Folder | Files | What a feature port must respect |
|---|---|---|---|---|
| F1 | 874638c | capture-harness | `GameStateExtTestRender.cpp` (`ApplyLightingItem`, `TriSetLighting`, new `triPinObjectDetail`, `triSettleScene`, `triSceneStats`), `Scene.{hpp,cpp}` (`PinObjectDetail`, `_drawnObjects`), `SceneDraw.cpp` (`AdjustComplexity` pin, object count), `TerrainLighting.{hpp,cpp}` (pending query), `WorldSetup.cpp` (setDate lamps), `capture_lib.sqf`, `make_pairs.py`, `test_tri_capture_verbs.cpp` | Every prototype's knob table (`kKnobs` inside `TriSetLighting`) moved into `ApplyLightingItem`; mode words now reset every knob; comma lists are all-or-none. `AllKnobsDefault` in the test lists fields by hand. Perf's `CWR_PERF_PIN_LOD` is superseded by `triPinObjectDetail`. |
| F2 | d57b712 | crown-nan | `LightRig.{hpp,cpp}` (`CrownEllipsoid` finite), `EngineGL33_LightingShaders.cpp` (`s_vsTransformRemasterGLSL` uses `inversesqrt(max(dot(n,n),1e-12))`; `shadeRemastered` computes the crown normal once) | Shade's crown term (V18) and temporal's VS jitter hunk sit on the changed lines. |
| F3 | f102499 | shadow-sampler | `EngineGL33_Lighting.cpp` (`InitLightingResources` parks `_dummyDepthArray` on unit 2), `EngineGL33_ShadowDepth.cpp` (`EnsureArrayTarget` selects unit 2) | Unit 2 must always hold a complete depth array with the right sampler. Any new raw-depth sampler (shadows, temporal) and any post pass that binds the shadow array must keep that invariant. |
| F4 | 5cf85cf | clear-sky-clouds | `LightRig.cpp` (`kClearSkyCover = 0.05`, `shadowCover = cover * Smoothstep(0.05, 0.10, cover)`, mean-preserving `overcastDim`) | Supersedes shade's cloud floor. Clouds' sky-matched shadows must keep the fade-in and the mean. |
| F5 | 463506f | shaft-veil | `EngineGL33_PostFX.cpp` (`shaftInscatter()` in `s_psPostCompositeGLSL`, `depthTex` on unit 5, `depthMap.z`), `LightRig` (`ShaftScatterLength`, `ShaftInscatter`), lab `postfx.py` | Supersedes shade's in-scatter override. The composite hunks of volumetric, combat, shadows, temporal and texsharp no longer apply (observed, section 3.7). |
| F6 | d80f868 | bush-instancing | `World/Scene/InstancedRunRules.hpp` (`RunEntry.ownVertices`, `CanHead`, `CanJoin`), `SceneDraw.cpp` (`InstancedRunEntry`) | Perf flag 64 is landed. Perf's light gate must join `RunEntry`, not the old predicate. |
| F7 | ab61aa5 | streetlamp-crash | `ObjectClasses.{hpp,cpp}` (`USE_CASTING` for `StreetLamp`) | `switchLight` is now safe in capture scripts (lamps on and off in the night videos). |
| F8 | 747ffc8 | spawned-vehicles | `Shape.hpp`, `ShapeDraw.cpp` (`ShapeBank::OptimizeLate`), `World.cpp` | Supersedes shade's lazy VB and materials' `vbfix`. Crew views of late shapes keep the CPU path. |
| F9 | 4515770 | night-bright | `LightRig.{hpp,cpp}` (`kMoonFull` 0.02, chroma (0.82, 0.90, 1.00), `kMoonSky` 0.25, moon in the bounce), `EngineGL33_LightingShaders.cpp` (local-light clamp 1.5 to 1.0; night-eye grey with blue raised) | Night's moon key and lamp model, combat's HDR lights and water variant C (still clamps local light at `vec3(1.5)`) must rebase on this. |
| F9b | 72f32d4 | night-bright | `IO/ParamFileExt.cpp` (`GetColor` reads `{r,g,b}`) | Lamps now carry their authored warm colour (0.9, 0.8, 0.6) and 0.1 ambient. Night's `nightLampWarm` was tuned on the white-lamp bug and is redundant. |
| F10 | b4f14d6 | object-shadows | `UI/Settings/GraphicsApply.{hpp,cpp}` (`ApplyGraphicsShadowsToScene`, `ApplyGraphicsConfigToScene`, `AppliedGraphicsConfig`), `Scene.cpp` (`LoadConfig`), `GameStateExtTestViewer.cpp` (`triEnableShadows` comment), `test_graphicsApply.cpp` | Landed narrowed: `Scene::LoadConfig` applies only the draw-only tiers of the boot config (object and vehicle shadows from the shadow tier, cloudlets from the particles tier); the terrain grid and object LOD bias stay out of the construction path. Needed by shadows, volumetric beams and hfrt's near range. |
| F11 | 0e85455 | msaa-default | `GraphicsConfig.{hpp,cpp}` (`kDefaultMsaaSamples = 4`, `kVersion` 3, `Migrate`), `GameApplication.cpp` (log line), `EngineGL33.cpp` and `EngineGL33_VertexBuffer.cpp` (comments), Remaster `README.md`, tests | Landed narrowed: `Migrate` lifts `msaaSamples` 0 to 4 only below v3 and only with `renderScale < 1.001`; a supersampled profile keeps its Off. TAA keeps MSAA under it. |

Step A0 (section 4) re-checked that F10 and F11 landed in their narrowed form before anything else
(both did; the branch was cut at 026133b, with this plan on top as 0779350; results in 5 A0).

---

## 3. Decisions every feature shares

### 3.1 Switch registry: one table for tri, the overlay and the tests

Ten of the fifteen prototypes added their own rows to the knob table in `TriSetLighting` and
their own floats to `LightingTuning`, and several rode on unrelated knobs (`moonlight=1.01` for shade,
`saturation=1.12+0.001k` for haze, `debug=100+flags` for perf, `terrainShadows=<mode>` for hfrt,
`localLights` as a variant selector, `r0..r7` for clouds and materials). The integrated branch
replaces all of that with one registry:

- New `engine/Poseidon/Graphics/Remaster/FeatureSwitches.{hpp,cpp}` (pure, no GL): a
  `struct FeatureSet` of floats nested in `LightingTuning` (`tuning.features.combatLights`), and
  a constexpr table `kLightingKeys[]` of `{key, float LightingTuning::* or FeatureSet member,
  default, min, max, overlay label, group, isFeatureSwitch}`. Existing knobs move into the same
  table.
- `ApplyLightingItem` (`GameStateExtTestRender.cpp`) iterates the table instead of its local
  `kKnobs`. New mode word **`baseline`**: every feature switch 0 and every knob at its default
  (the look of the branch cut). `remastered` keeps meaning "shipped defaults", which from now
  on is every feature on. `classic` is unchanged.
- `DrawLightingTab` (`Dev/Debug/DebugOverlay.cpp`) gains a "Features" section generated from the
  table: one checkbox per feature switch, its sub-knobs under a collapsing header, and a live
  "active / gated: <reason>" line from the frame gates (3.2) plus that feature's GPU section
  (`GpuTimings`). The copy-back summary line is generated from the table too.
- New verb `triFeatureState` returns the switch values and gate states as one string, logged on
  every capture shot.
- Tests: `AllKnobsDefault` in `test_tri_capture_verbs.cpp` iterates the table; new cases check
  that every feature switch has a key, a label and a default of 1, that `baseline` zeroes all
  of them and that `remastered` restores them.

Feature switch keys (each a float, 0 off, 1 on, default 1):

| Feature | Switch keys | Sub-knobs kept as tuning |
|---|---|---|
| perf | `drawGrouping`, `hillCull` | none (both pixel-exact) |
| temporal | `taa` | `taaSharpen` 0.5, `taaMipBias` -0.5, `taaAlpha` 0.1 |
| shared light list | `lightTiles` | none |
| sky table (atmosphere) | `atmosphere` | `atmoHue` 0.5, `atmoSat` 1.15, `atmoHaze` |
| shadows | `treeShadows`, `softShadows` | `sunSize` 1.0 deg |
| shade | `lowSunRig`, `crownCanopy` | none |
| combat | `combatLights`, `combatShadows` | `combatFlash`, `combatFire`, `combatSmoke`, `combatScatter` |
| night | `nightLights`, `moonShadows`, `stars` | `nightSpot` |
| ssr | `ssr` | `ssrWater` 0.6, `ssrWet` (-1 = weather) |
| materials | `wetWeather`, `lampStreaks` | `wetness` (-1 = weather), `puddles` |
| volumetric | `airBeams` | `volGain` |
| haze | `airHaze`, `valleyMist` | none |
| hfrt | `canopyShadows` | none |
| clouds | `litClouds`, `skyCloudShadows` | none |
| texsharp | `terrainDetail`, `sharpen` | `detailStrength` 1.5 |
| water | `seaWaves` | none |

The existing keys `terrainShadows` (hills) and `cloudShadows` (noise field) keep their meaning.
Debug knobs stay as debug-view values (`debug=<n>`), not as feature switches.

### 3.2 Frame gates and the GPU budget (RTX 3060, 1080p, 60 fps)

The owner's budget only holds if heavy features run only when they can show. New pure module
`engine/Poseidon/Graphics/Remaster/FeatureGates.{hpp,cpp}`:
`FeatureGates ComputeFeatureGates(const GateInputs&, const LightingTuning&)`, evaluated once per
frame in `EngineGL33::UploadLitFrameConstants`. Inputs: sun and moon elevation (the rig),
`NightEffect`, sky-through, rain density, fog range, the wetness state, interior / optics /
night-vision flags (`_worldDepthCleared`, `_worldOverlayDrawn`, `GetNightVision()`), combat
lights alive within 60 m, local lights in view, and last frame's screen coverage of water and
wet surfaces (from the SSR tag readout at 1/8 resolution). Each gate returns on/off plus a
resolution scale. A dev knob `gates=0` forces every enabled feature on for debugging.

| Feature | Runs when (gate) | Resolution | Budget at 1080p on a 3060 (MENU estimates, unmeasured) |
|---|---|---|---|
| perf | always | n/a | saves 7 to 17 ms CPU on long views |
| temporal | world drawn, not interior / optics / NV | full | about 0.3 ms |
| light list | any local or combat light in view | tiles 32 px | binning on CPU; shading cost moves from per-draw to per-tile |
| sky table | sun above -9 deg | 192x64 LUT, rebuilt on GPU when the sun moves 0.02 deg | about 0.5 ms |
| shadows | sun above 4 deg (or moon key) | 2048 map; 4096 only on the Ultra shadow tier | 0.5 to 2.5 ms |
| shade | always (CPU constants) | n/a | about 0 |
| night | `NightEffect` > 0.01 | full (tiled) | 1 to 2 ms |
| combat | combat light within 60 m of the camera | shadow mask half res, 2 to 4 lights | 1 to 3 ms in fights |
| water | sea fragments drawn | full | about 0.2 ms |
| ssr | water or wet share of the screen above 1% | half-res trace | 1 to 3 ms |
| materials (wet) | wetness above 0.02 | full | about 0.8 ms in rain |
| air pass (beams + haze) | beams: sun below 30 deg and within 60 deg of the view, or mist, or headlights at night; haze: always beyond 150 m; valley mist: morning or fog | half res, 24 steps, temporal | 2 to 4 ms at dawn, dusk and in mist |
| hfrt | sun between 2 and 25 deg, sky-through above 0.5, receivers beyond the last cascade | half-res visibility buffer | target 1.5 to 3 ms |
| clouds | sky-through below 0.85, relief and silver lining fade above 35 deg sun | full (sky pixels) | 0.5 to 1.5 ms |
| texsharp | terrain fragments within 300 m | full | 1 to 3 ms |

Worst cases from MENU section 4 (a low sun over water and woods: about 20 ms) do not fit; the
gates above keep the beams, hfrt and SSR from all being at full cost together only if the
timing step of every gate (section 8) is honoured. A later governor (drop hfrt and the air pass
to quarter resolution when the GPU frame stays above 14 ms) is listed as a follow-up.

Measured base (A0, RTX 4060 Ti, 1080p, 4x MSAA, scale 1.0; table in 5 A0): the Remastered frame
before any feature is 2.0 to 11.9 ms of GPU time on seventeen of the twenty views, but 17.4 ms
on `sunrise_ridge`, 23.3 ms in the valley at 05:15 and 24.3 ms on Eden's `broken_high`, the long
views with 10,800 to 12,900 draws. These are not a resolution scale-up of MENU's base frame: its
2.5 to 7.5 ms at 1440x960 ("perhaps 4 to 12 ms at 1080p on a 3060", MENU.md line 483) was
measured after the frame-cost fixes, which are the perf track, step A4, and A0 comes before A4.
On the long views the GPU span is set by draw submission rather than shading: the CPU frame
(26 to 31 ms) is longer than the GPU span, nvidia-smi reads 54 to 73% utilisation at 39 to 46 W,
at P0, in the 3 s before those shots, and Classic costs the same as Remastered on
`sunrise_ridge` (frame 18.10 against 17.41 ms, world 17.95 against 16.26). Remastered takes
more time per draw on the other two (Eden's `broken_high`: 10,784 against 10,787 draws, yet
6.25 ms more world time and 4.4 ms more CPU frame). So the B steps judge their headroom on
these views only after A4 has landed.

### 3.3 LitConstants slot allocation

Today `LitConst::Count = 58` (`EngineGL33_Lighting.hpp`). Six prototypes all start their new
slots at 58 (combat 58-64, materials 58-60, atmosphere 58-64, shadows 58, clouds 58-64,
temporal 58; ssr, volumetric and materials also paste their own copy of the GLSL block into
post shaders). The branch allocates once, in landing order, per-draw slot first so the perf
dirty-range flush (A4) stays small:

| Slot | Name | Owner | Rate |
|---|---|---|---|
| 58 | `MatClass` | materials (class, object kind, blend, hash) | per draw |
| 59 | `Taa` | temporal (frame index, stochastic flags, taps, penumbra per metre) | frame |
| 60 | `Pcss` | shadows (search radius, min texels) | frame |
| 61-63 | `SkyRight`, `SkyUp`, `SkyFwd` | sky pass (atmosphere and clouds share them) | frame |
| 64-67 | `Atmo`, `AtmoCloudSun`, `AtmoCloudAmb`, `AtmoDome` | atmosphere | frame |
| 68-69 | `Sky`, `SkyVp` | clouds | frame |
| 70 | `LightTiles` | light list (tile size, tile counts, light count) | frame |
| 71 | `Night` | night (flags, beam profile, moon key) | frame |
| 72 | `CombatShadow` | combat (mask scale, strength, shadowed count) | frame |
| 73 | `Weather` | materials (wetness, rain, puddles, time) | frame |
| 74-75 | `Water0`, `Water1` | water (wave, swell, ripple, sky weight, sheen, glint, sparkle) | frame |
| 76 | `Canopy` | hfrt | frame |
| 77 | `Detail` | texsharp | frame |
| 78 | `Haze` | haze (analytic fallback for blended surfaces) | frame |
| 79 | `Count` | | |

79 vec4 is 1,264 bytes, far below the GL minimum uniform block size. Combat's reprojection rows
(prototype slots 59-64) are not needed once its shadow becomes a mask pass (B4).

### 3.4 Texture units

Today: 0-1 material, 2 shadow array (compare sampler, F3 invariant), 3 horizon, 4 height,
5 cloud noise, 7 upload unit. Seven prototypes hard-code unit 6 (combat depth, atmosphere LUT,
hfrt canopy, shadows raw depth, temporal raw depth, clouds sky texture, haze sky backdrop) and
hfrt also takes 8. Allocation (GL 3.3 guarantees 16 fragment units), as constants in
`EngineGL33_Lighting.hpp` with a `static_assert` that none is 7 or above 15:

| Unit | Constant | Content | Owner |
|---|---|---|---|
| 6 | `kShadowRawUnit` | shadow array again with a non-compare NEAREST sampler (shadows and temporal had two copies of this) | shadows / temporal |
| 8 | `kSkyLutUnit` | sky-view table | sky table |
| 9 | `kSkyDensityUnit` | cloud density map from the weather sky texture | clouds |
| 10 | `kCanopyUnit` | canopy heightfield (RG16, mips) | hfrt |
| 11 | `kSeaNextFrameUnit` | next sea animation frame (cross-fade) | water |
| 12 | `kLightDataUnit` | light buffer (`samplerBuffer`, RGBA32F) | light list |
| 13 | `kLightTileUnit` | per-tile light indices (RGBA16UI) | light list |
| 14 | `kCombatMaskUnit` | combat shadow mask (half res) | combat |
| 15 | `kCanopyVisUnit` | canopy sun-visibility buffer (half res) | hfrt |

`InitPixelShaders` binds sampler names to units from one table (today `litSamplers[]` with three
rows; every prototype appended a row). Every unit that can be sampled gets a complete stand-in
texture at `InitLightingResources`, the same rule F3 applied to unit 2, and a new
`[ShadowSampler]` audit case covers unit 6.

### 3.5 Shader sources: split by concern, test through the accessors

`EngineGL33_LightingShaders.cpp` (757 lines) and `EngineGL33_PostFX.cpp` (725 lines) are the two
files almost every prototype edits. Before any visible feature lands (step A2):

- `s_psRemasterCommon` keeps the header, the LitConstants block (single copy) and the shared
  helpers; concerns move to their own strings, concatenated in a fixed order by
  `RemasterFragmentCommon()`: shadows (`shadowMapVisibility`, PCSS), terrain and clouds
  (`terrainSunVisibility`, `cloudSunVisibility`), sky (`skyIrradiance`, `fogColorToward`, LUT
  lookups), local lights (`localLights`, tiled lights, night and combat kinds), surface
  (`glossy`, materials, wetness), `shadeRemastered`, `finishFragment`. New files
  `EngineGL33_LightingShadersSky.cpp`, `..._Lights.cpp`, `..._Surface.cpp` hold the pieces that
  grow. Shader overrides keep working on the assembled name (`psRemasterCommon.glsl`).
- The LitConstants GLSL block becomes one string (`s_glslLitConstantsBlock`) pasted into the
  remastered VS that needs it (atmosphere's `s_vsScreenRemasterGLSL`), the common FS and every
  post shader that reads the rig (SSR, air pass). No hand copies (ssr, volumetric, materials and
  atmosphere each pasted one).
- Post passes move into `EngineGL33_PostFX.cpp` (pipeline, AO, bloom, legacy shafts, composite),
  `EngineGL33_PostTemporal.cpp`, `EngineGL33_PostSsr.cpp`, `EngineGL33_PostAir.cpp`,
  `EngineGL33_PostCombat.cpp`. A shared post GLSL string (`s_glslPostCommon`) holds depth
  reconstruction (`viewZ`, `viewPos` with `projInfo`, `depthMap`, `worldRect`), interleaved
  gradient noise, cascade lookup and the terrain and cloud visibility helpers that volumetric,
  ssr and combat each rewrote.
- `test_remaster_lighting.cpp`: the compile test calls `RemasterVertexSource`,
  `RemasterFragmentCommon` and `RemasterFragmentMain` (PoseidonTests links PoseidonGL33) instead
  of regex-splitting one file, keeps the `vertex == 3` and `fragment == 4` counts, checks the
  VS out / FS in varyings for every VS (atmosphere, clouds and haze added varyings to the VS),
  and compiles every `s_ps*` / `s_vs*` symbol of every `EngineGL33_Post*.cpp`. The layout test
  checks every declaration of `uniform LitConstants` in those files against `LitConst`, slot by
  slot, so a stale copy fails.

### 3.6 Post pipeline stages

`EngineGL33::EndWorldScene` is one function today: resolve, AO, bloom, shafts, composite. Eight
prototypes (combat, ssr, volumetric, materials, night, shadows, texsharp, temporal) inserted
passes into it and widened its early return
(`ambientOcclusion <= 0 && bloom <= 0 && sunShafts <= 0`). Step A3 turns it into a fixed stage
list run on a `PostFrame` struct (proj, rect, depth scale and bias, half size, gates):

1. `PostResolve` (colour, depth, stencil) and `PostLinearDepth` (half-res view depth, new; feeds
   AO, SSR, air, combat and hfrt; perf noted the AO's full-res depth reads as the largest fixed
   post cost)
2. `PostTagReadout` (SSR stencil tags, only when `ssr` is gated on)
3. `PostAo`
4. `PostSsr` (before bloom so reflected highlights bloom, as the prototype did)
5. `PostAir` (beams and haze; replaces `PostShafts` when on)
6. `PostBloom`
7. `PostShafts` (legacy, only when `airBeams` is off)
8. `PostCombatScatter`
9. `PostComposite` (F5's `shaftInscatter` kept for the legacy shafts)
10. `PostTemporal` (TAA resolve) and `PostSharpen` (TAA sharpen or, with TAA off, texsharp's
    luma CAS; never both)

Each stage owns its GPU mark (new `Mark` values; `GpuTimings` records marks in a 32-bit mask,
so the count must stay at 32 or fewer) and restores what it binds. The early return asks every
stage's gate. Combat's shadow mask and hfrt's visibility buffer are not post stages: they run at
the start of the next world pass (B4, B9).

### 3.7 Porting method

No prototype is applied blind. Observed with `git apply --check --ignore-whitespace` against the
tip (after stripping the UTF-8 BOM that the ssr, perf, night and atmosphere patches start with):

| Applies cleanly | Fails |
|---|---|
| ssr, materials, hfrt, haze, clouds | combat (`LightingShaders.cpp:564`, `PostFX.cpp:215`), shade (`LightRig.cpp:182`), volumetric (`PostFX.cpp:206`), atmosphere (`Lighting.cpp:107`, `LightingShaders.cpp:606`), shadows (`GameStateExtTestRender.cpp:571`, `PostFX.cpp:214`), night (`PostFX.cpp:668`), texsharp (`GameStateExtTestRender.cpp:32`, `PostFX.cpp:673`), perf (`SceneDraw.cpp:777`), temporal (`LightingShaders.cpp:139`, `PostFX.cpp:673`) |

"Applies cleanly" is textual only: materials re-adds a lazy VB next to F8's `OptimizeLate`, and
every patch collides with every other one on `LightingTuning`, the knob table and slot 58. Each
feature is ported by hand onto the structures of section 3, with the final shader taken from the
track's final override directory, not from the patch:

| Track | Final shader source |
|---|---|
| combat | in the patch (round 9) |
| shade | `shade/ovr_final/psRemasterCommon.glsl` (crown V18) |
| ssr | `ssr/glsl_v12/psPostSsr.glsl` |
| volumetric | `volumetric/glsl/` (probe 8 code) |
| materials | `materials/override_v9/` (5 files) |
| atmosphere | `atmosphere/glsl_final/` |
| hfrt | `hfrt/override_v10/` (mode 20) |
| haze | `haze/shaders_r8/` and `haze/src/haze_block.glsl` (variant 43) |
| shadows | in the patch (`pcssLit`) |
| night | `night/ovr_v9/psRemasterCommon.glsl` |
| clouds | `clouds/glsl_final/` |
| texsharp | `texsharp/glsl7/` (rotated self-detail, `rfinalr`) |
| temporal | in the patch |
| water | `git show 935cb5e:arma_CWR/tests/rendering/water_research/shaders_C/psDetailRemasterMain.glsl` (branch `claude/remaster-water-research`, read without checkout) |
| perf | no shader change kept |

The clouds patch also rewrites the `vpScale` comment line with a mis-encoded character; that hunk
is dropped.

---

## 4. Integration order

Phase A builds the base. It changes no pixel except where a step says so (A7 only behind its
switch). Phase B adds the visible layer, one feature per commit series, in an order that
respects the dependencies and keeps the hot-spot files (section 6) to one editor at a time.

```
A0 cut + verify F10/F11 + timing baseline
 -> A1 switch registry, gates, capture tooling
 -> A2 shader split, shared GLSL, slot and unit tables
 -> A3 post pipeline stages + half-res linear depth
 -> A4 perf ----------------------------------------------> (budget for everything below)
 -> A5 TAA ------> B1 shadows (PCSS), B4 combat mask, B6 ssr, B8 air, B9 hfrt, B11 texsharp sharpen
 -> A6 light list -> B3 night, B4 combat, B7 materials (lamp streaks)
 -> A7 sky table -> B5 water (sky term), B8 air (limit colour), B10 clouds
B1 shadows -> B8 air (beams need casters), B9 hfrt (near range)
B5 water -> B6 ssr (sea reflection feeds the water term)
B6 ssr -> B7 materials (puddle reflections)
```

| Step | Content | Depends on | Effort (agent-days) |
|---|---|---|---|
| A0 | Branch cut, F10/F11 checks, quiet-GPU timing baseline at 1080p | owner OK for the branch | 0.5 |
| A1 | `FeatureSwitches`, `FeatureGates`, overlay Features section, `baseline` word, `triFeatureState`, `triSetFixedStep`, feature A/B capture test, video generator skeleton | A0 | 3 |
| A2 | Shader split, `s_glslLitConstantsBlock`, `s_glslPostCommon`, slot and unit tables, test rewrites | A1 | 2 |
| A3 | Post stage list, `PostLinearDepth`, per-stage GPU marks | A2 | 2 |
| A4 | perf | A3 | 4 |
| A5 | temporal (TAA) | A3 | 5 |
| A6 | shared light list | A2, A4 | 5 |
| A7 | sky table and remastered sky pass (atmosphere) | A2 | 5 |
| B1 | shadows | F10, A5 | 3 (+3 leaf transmittance) |
| B2 | shade | B1 | 2 |
| B3 | night | A6 | 5 |
| B4 | combat | A5, A6 | 7 |
| B5 | water | A7 | 3 |
| B6 | ssr | A3, A5, B5 | 5 |
| B7 | materials | A6, B6 | 6 |
| B8 | air pass (volumetric + haze) | A5, A7, B1 | 8 |
| B9 | hfrt | A5, B1 | 6 |
| B10 | clouds | A7 | 4 |
| B11 | texsharp | A5 | 1.5 |
| V | video tooling finish and the shoot | all | 5 |

Total about 89 agent-days. B2, B5, B10 and B11 have no hot-spot overlap with B3/B4 beyond the
shader pieces of 3.5 and can run as parallel scratch lanes once A7 has landed.

---

## 5. Per feature

Field order in every entry: what the prototype touched; keep; rewrite; drop; conflicts with the
fixes; conflicts with other features; switch; critic fixes folded in (cheap) and documented (not
cheap); GPU gating; tests; effort.

### A0. Cut and baseline

- Confirm with `git log` that F10 landed narrowed (shadow tier only, no terrain grid or LOD bias
  at boot) and F11 migrates only profiles with render scale 1. If either is not narrowed, stop.
- Re-shoot `abel_lighting`, `eden_lighting`, `sinai_lobo_lighting` with run-captures as the new
  baseline (`tmp/captures/<stamp>-gr-base`). Every judgement after this is against it, not
  against `20261017-final`, which had no tree shadows and Classic-lit spawned vehicles.
- MENU step 0 timing: one capture run on a quiet machine (no other tri slot holders in
  `tri.log`), 1920x1080 windowed, render scale 1.0, 4x MSAA, `triGpuTimes` sections per view.
  Without a 3060 on hand, record the 4060 Ti numbers and keep MENU's 1.5 to 2x rule.

**Result (2026-10-17, build of 0779350; evidence in `tmp/integration/A0/` of the
graphics-remaster worktree, gitignored).**

- F10 and F11 are in their narrowed form. `Scene::LoadConfig` applies only
  `ApplyGraphicsShadowsToScene` from the boot config (object and vehicle shadows, cloudlets);
  `ApplyGraphicsConfigToScene` (terrain grid, object LOD bias, then the same helper) runs only on
  the live Graphics page path. `GraphicsConfig::Migrate` lifts `msaaSamples` 0 to 4 only when
  `version < 3 && renderScale < 1.001f`. In game, a fresh user dir writes `graphics.cfg` and the
  frame target comes up at 4x MSAA, render scale 1 before the test sets anything, and
  `triShadowSceneDump` at `sunrise_ridge` with no spawned unit answers OK with the trees and
  bushes in the depth map.
- Base capture set: `20261017-233303-gr-base` (3/3, 30 shots, all settled; 960x640 window at
  render scale 1.5, so it is the look baseline, not the timing one). A second boot of the same
  build (`20261017-235653-gr-base-repeat`, identical eye heights) gives the boot-to-boot noise:
  `sunrise_ridge`, `lowsun_fp`, `noon_fields`, `forest_edge_golden`, `coastline_sunset` and the
  Eden overcast views stay within 21 levels with at most 141 pixels over 2 levels (sunrise_ridge
  and lowsun_fp within 8), except `overcast_broken` Remastered (6,830 pixels over 2, mostly in the
  horizon band); the Sinai ground band is not repeatable (its blotch pattern changes between
  boots: in the Remastered shots about 258,000 of the 278,400 ground-band pixels (rows 350-640),
  322,000 of 614,400 in the whole frame, differ by over 2 levels in `sinai_desert_noon`, and
  about 61,000 of 278,400 in `sinai_lowsun_fp`; Classic's counts are within 4,000), nor are the
  crew views (`tank_gunner_*`, edge shifts), the firefights and `town_night` (fire and smoke).
  Differences of the same kinds and sizes separate this set from `20261018-fixes2`, shot with
  the lighting worktree's build of the same fixes.
- KHR_debug ids in the smoke and base runs: 131169 (LOW), 131154 and 131218 (MEDIUM, pixel
  transfer sync from screenshots and a VS recompile on GL state). No 131204 or 131222. These
  three are the known set for gate item 4.
- Timing: `20261017-234549-gr-timing`, RTX 4060 Ti (driver 610.88), no other PoseidonGame on
  the machine for the whole run (sampled every 2 to 3 s), 0 of 4 tri slots held before and after.
  Twenty views: the start view of each feature's first row in 7.3, the Eden and Sinai clips and
  the noon references, with these deviations from 7.3: `forest_edge_golden` has its Jeep parked
  at 25 m (the clip drives it across) and also stands in for texsharp's first row (p 0, no
  props); `town_night` is the capture-set version, without the driven Jeeps;
  `coastline_sunset` takes heading 278 (the water row; the sky row says 250) with the eye 3 m
  above the beach probe (the sky row's convention, not the water row's 3 m ASL);
  `woods_pine2_evening` takes its props from the shadows track (Jeep at 14 m and a soldier at
  9 m, from `gen_r8.py`; 7.3 says a Jeep at 15 m); `lowsun_fp` is shot from the capture-set
  camera, not the orbit start; `road_flat_drive_moonless` has the night track's Jeep standing
  3.4 m behind the eye, not driven; `broken_high` does not step the sky drift; `ffh` is a static
  stand-in. The NOTE field of each view in `gen_timing.py` records its deviation. 3 interleaved
  rounds in one game per island. GPU ms, median of 3 (`triGpuTimes`, 90 frames before each
  shot); Remastered also as median / minimum; the columns after the two frame columns are the
  Remastered shots. GPU MHz is the median graphics clock of the nvidia-smi rows stamped from 3 s
  before each shot up to the shot, median over the rounds, with the P-states seen; the sampler
  stamps whole seconds, so the window edges are good to about a second:

  | view | Classic frame | Remastered frame | world | shadow | post | CPU frame | draws | GPU MHz |
  |---|---|---|---|---|---|---|---|---|
  | sunrise_ridge | 18.10 | 17.41 / 17.34 | 16.26 | 0.09 | 1.01 | 26.57 | 12,910 | 2,790 (P0) |
  | forest_edge_golden | 5.54 | 6.18 / 5.58 | 5.40 | 0.31 | 0.58 | 8.71 | 2,141 | 1,680 (P0/P3) |
  | town_night | 6.56 | 6.53 / 5.42 | 5.64 | | 0.63 | 8.15 | 1,228 | 1,860 (P0/P3) |
  | coastline_sunset | 2.14 | 3.81 / 1.88 | 1.60 | 0.00 | 1.95 | 4.10 | 178 | 480 (P3/P5) |
  | woods_pine2_evening | 6.13 | 6.47 / 6.45 | 5.21 | 0.35 | 1.14 | 9.25 | 2,840 | 1,830 (P0) |
  | lowsun_fp | 4.83 | 5.59 / 4.60 | 4.34 | 0.09 | 1.08 | 8.30 | 1,891 | 1,500 (P0/P3) |
  | road_flat_drive_moonless | 5.22 | 5.27 / 4.52 | 4.46 | | 0.59 | 7.69 | 1,806 | 1,950 (P0/P3) |
  | ffh (static stand-in) | 5.44 | 6.06 / 5.66 | 5.22 | | 0.69 | 7.73 | 796 | 1,905 (P0/P3) |
  | bay_offshore | 6.05 | 7.73 / 7.72 | 7.11 | 0.03 | 0.49 | 12.44 | 4,557 | 2,775 (P0/P3) |
  | town_night_rain | 5.01 | 5.23 / 5.13 | 4.32 | | 0.79 | 7.10 | 809 | 1,575 (P0/P3) |
  | pinewood_morn | 6.03 | 5.84 / 5.46 | 4.67 | 0.57 | 0.95 | 8.58 | 2,844 | 1,852 (P0/P3) |
  | valley 05:15 | 20.30 | 23.30 / 22.47 | 21.99 | 0.04 | 1.16 | 31.02 | 11,404 | 2,790 (P0) |
  | hill_1730 | 2.44 | 1.99 / 1.58 | 1.49 | 0.05 | 0.38 | 3.51 | 1,663 | 1,815 (P0/P5) |
  | coast_oc_up | 3.58 | 3.87 / 1.73 | 1.64 | 0.00 | 1.90 | 3.61 | 186 | 990 (P0/P3/P5) |
  | sea_noon | 3.79 | 4.56 / 1.20 | 3.01 | 0.00 | 1.31 | 5.41 | 180 | 765 (P0/P3/P5) |
  | noon_fields | 8.42 | 9.79 / 9.06 | 9.16 | 0.08 | 0.49 | 14.79 | 6,399 | 2,490 (P0) |
  | overcast_broken (Eden) | 10.89 | 11.85 / 11.77 | 11.10 | 0.18 | 0.65 | 18.51 | 8,066 | 2,790 (P0) |
  | broken_high (Eden) | 17.27 | 24.31 / 24.12 | 23.42 | 0.03 | 0.79 | 27.35 | 10,784 | 2,790 (P0) |
  | sinai_lowsun_fp | 4.52 | 5.06 / 4.24 | 3.75 | 0.12 | 1.21 | 7.46 | 764 | 818 (P0/P3/P5) |
  | sinai_desert_noon | 8.81 | 9.70 / 9.31 | 8.36 | 0.07 | 1.08 | 14.42 | 2,672 | 1,245 (P3/P5) |

  What it says: the long views (sunrise_ridge, valley 05:15, Eden broken_high) take 17 to 24 ms
  of GPU time before any feature, with 10,800 to 12,900 draws and a 26 to 31 ms CPU frame.
  There the GPU span is set by draw submission rather than shading: the CPU frame is longer than
  the GPU span on all three, nvidia-smi reads 54 to 73% utilisation at 39 to 46 W, at P0, in the
  same window as the GPU MHz column, and Classic costs the same as Remastered on sunrise_ridge
  (frame 18.10 against 17.41 ms, world 17.95 against 16.26). Remastered takes more time per draw
  on the other two: on Eden from 250 m the draw counts match (10,784 against 10,787) and
  Remastered still adds 6.25 ms of world time (23.42 against 17.17 ms) and 4.4 ms of CPU frame
  (27.35 against 22.94); the valley adds 1.8 ms of world time (21.99 against 20.17) and 5.5 ms
  of CPU frame (31.02 against 25.49) with fewer draws (11,404 against 11,551). These numbers are
  not a 1080p scale-up of MENU's 2.5 to 7.5 ms base frame, which was measured after the
  frame-cost fixes (MENU.md line 483; the perf track, step A4), and A0 comes before A4, so the B
  steps judge headroom on these views only after A4 has landed. Sky and sea views run the GPU
  mostly at P3 or P5 (Remastered medians of 0.5 to 1.0 GHz; coast_oc_up and sea_noon also read
  P0 in one round each), so their milliseconds overstate the cost at full clock and swing
  between rounds by up to about 7x in Remastered (round to round: sea_noon 1.20 to 8.60 ms,
  coast_oc_up 1.73 to 8.58, hill_1730 1.58 to 8.36; round 3 of every Remastered sky and sea
  view and of hill_1730 read 7.5 to 8.6 ms with the post pass at 3.0 to 3.9 ms) and about 5x
  in Classic's coast_oc_up (3.58 / 0.73, not in the table). The post pass medians are 0.4 to
  2.0 ms (single rounds reach 3.9 ms at low clocks), the sun shadow pass 0.00 to
  0.57 ms, and the night views report no shadow section. `ffh` is a static stand-in (posed men,
  burning Ural, Fire; no volleys or shells) and never reports settled within 300 frames. MENU's
  1.5 to 2x rule combined the 1440x960 to 1080p pixel ratio (1.5x) with the slower card; these
  numbers are already at 1080p, so applying the whole rule to them is conservative.
- The rig is local, not in the repo: `tmp/integration/A0/timing/` (`gen_timing.py` writes the
  three tests from one view table, `timing_lib.sqf` adds `capRain` and `capEyeAbs` to the repo
  `capture_lib.sqf`, `gpu_sampler.ps1` logs clocks and foreign games, `summarize_timing.py`
  reduces the rounds and joins the clocks). Gate item 7 reruns it until A1 brings it into the
  repo and its capture tooling carries the same views.

### A1. Switches, gates, tooling

- New files: `Graphics/Remaster/FeatureSwitches.{hpp,cpp}`, `Graphics/Remaster/FeatureGates.{hpp,cpp}`.
- Edits: `LightRig.hpp` (`LightingTuning` gets `FeatureSet features`), `GameStateExtTestRender.cpp`
  (`ApplyLightingItem` table-driven, `baseline`, `triFeatureState`), `DebugOverlay.cpp`
  (`DrawLightingTab`), `GameStateExtTest*.cpp` (`triSetFixedStep <seconds>`: a fixed
  `World::Simulate` delta while set, 0 = real time).
- Capture tooling in `arma_CWR/tests/rendering/captures/`: `features_ab.test.sqf` (one hero view
  per feature, shot `remastered,<key>=0` then `remastered`, labels `<view>__off` / `__on`),
  `make_pairs.py` learns the off/on labels, and `video/gen_video_tests.py`, `video/capture_motion.sqf`,
  `video/make_video.py` (section 7). A0's timing rig moves in with them (`gpu_sampler.ps1`,
  `summarize_timing.py`, `timing_lib.sqf`'s `capRain` and `capEyeAbs`, and the twenty A0 views),
  so gate item 7 no longer depends on the gitignored `tmp/integration/A0/timing/`.
- Tests: registry cases (3.1), `FeatureGates` cases (one per gate row of 3.2, plus `gates=0`),
  `triSetFixedStep` round trip.
- Effort 3 days.

### A2. Shader split and shared tables

As 3.3 to 3.5. Pixel-exact: the assembled GLSL strings must be byte-identical to the tip's
(compare `glsl_extract.py --dump` output before and after). Effort 2 days.

### A3. Post stages

As 3.6. Pixel-exact against A2 (capture A/B, `banddiff.py` from `tmp/research/perf/scripts`).
`PostLinearDepth` is new work but feeds nothing yet. Effort 2 days.

### A4. perf (draw grouping and hill culling)

- **Prototype touched**: `Graphics/Core/Engine.{hpp,cpp}` (research globals), `GpuTimings.{hpp,cpp}`
  (Ground, Pass1, Rest sections and a running minimum), `WorldFrameObserver.cpp` (flag 32),
  `SceneDraw.cpp` (`TerrainHorizonCull`, `AdjustComplexity` env pin, Pass1 gates),
  `LandscapeRender.cpp` (`GpuMarkPhase` in `Landscape::DrawRect`), `EngineGL33.hpp`,
  `EngineGL33_GpuTimer.cpp` (counters), `EngineGL33_Lighting.cpp` (`FlushLitConstants` dirty
  range, mirror), `EngineGL33_Shaders.cpp` (`FlushVSConstants` skip-unchanged, mirror updates in
  `UploadVSCrown`, `UploadVSWorldMatrix`).
- **Keep**: the per-object light gate (flag 4), terrain horizon occlusion (flag 256), both UBO
  mirrors (flags 1 and 2), the Ground / Pass1 / Rest GPU sections and the minimum column.
- **Rewrite**: the light gate becomes `RunEntry.localLit` in `InstancedRunRules.hpp`, filled in
  `InstancedRunEntry` from `SelectLights` only when `NLights() > 0`; `CanHead` and `CanJoin` read
  it. The horizon table moves out of `SceneDraw.cpp` into a pure `World/Scene/TerrainOcclusion.{hpp,cpp}`
  (`BuildHorizonTable(heightFn, eye, range, params)`, `IsHidden(table, sphere)`), built with
  steps that grow with distance (25 m near, 100 m far), 512 rays plus a wider margin, on a
  worker thread for an eye raised by the expected motion until the next rebuild (stays
  conservative), and it runs before LOD selection and sorting. The UBO mirrors go through one
  helper (`UploadLitSlots(first, count)`) that every partial upload uses (`SetLitOpaque` today,
  materials' `UploadLitSurface` later), and the mirror resets in `InitLightingResources` and
  `InitVertexShaders`. Counters become one `PerfCounters` struct shown by `triPerfStats`.
- **Drop**: `CWR_PERF_PIN_LOD` (F1 has `triPinObjectDetail`), flag 32, flags 8 and 16 (change far
  pixels), flag 128 (near-exact only), flag 64 (landed as F6), the `debug=100+flags` channel,
  the shader-side cutout and tap experiments (no gain).
- **Conflicts with fixes**: F6 owns the grouping predicate (port onto `RunEntry`); F1 changed
  `AdjustComplexity` (the failing hunk at `SceneDraw.cpp:777`) and counts `_drawnObjects` before
  occlusion, so the hill cull must not reduce that count (add `culled=` to `triSceneStats`
  instead, or capture tables stop being comparable).
- **Conflicts with features**: A6 makes the per-object light gate unnecessary in Remastered
  (tiled lights do not depend on the head's list), so after A6 the gate applies to Classic only
  and Remastered nights batch fully. Materials' per-draw `MatClass` upload must use the mirror
  helper. Night's moon shadow pass and the hill cull: perf's notes say the shadow-map pass selects its
  casters separately and that this was not verified; verify it before `hillCull` ships.
- **Switch**: `drawGrouping`, `hillCull`.
- **Critic fixes folded**: prove the cull safe: a unit test of `IsHidden` against brute-force
  ray marching on synthetic ridges and notches narrower than one ray; an in-game check that long
  sun shadows of culled objects still fall on visible ground (sunrise_ridge at 05:08 with
  `debug=1`); make the cull adaptive (skip when fewer than 200 objects were tested or less than
  10% were culled in the last 30 frames), which answers "on noon_fields the full set was slower".
  **Documented**: the "34 to 2 ms world pass" headline was mostly CPU stalls; report the CPU
  frame.
- **GPU gating**: none needed (it removes work).
- **Tests**: `test_instanced_run_rules.cpp` (lit members never join an unlit head),
  new `test_terrain_occlusion.cpp` `[scene][occlusion]`, a UBO mirror test (the mirror equals the
  buffer after every partial upload, using the lab's GL-free path or a recorded-call fake),
  `test_gpu_timings.cpp` for the new sections.
- **Effort**: 4 days.

### A5. temporal (TAA)

- **Prototype touched**: `GameStateExtTestRender.cpp`, `LightRig.hpp` (14 knobs),
  `EngineGL33.hpp` (`TaaState`, history targets), `EngineGL33_Lighting.{hpp,cpp}` (raw shadow
  sampler on unit 6, `Taa` slot), `EngineGL33_LightingShaders.cpp` (jitter in all three
  remastered VS, `shadowStochastic`, stochastic alpha in `finishFragment`),
  `EngineGL33_PostFX.cpp` (`s_psPostTaaGLSL`, `s_psPostTaaOutGLSL`, `aoStoch`),
  `EngineGL33_Queue.cpp` (`BeginPass` applies jitter), `EngineGL33_Shaders.cpp` (jitter in the
  classic VS, `UploadVSScreenConstants`).
- **Keep**: Halton(2,3) jitter in `vpScale.zw`, set only for world 3D passes and zeroed in
  `EndWorldScene`; the resolve (3x3 reconstruction, nearest depth, camera reprojection built in
  double precision, 9-tap Catmull-Rom history, depth rejection with the stored view depth,
  YCoCg variance clip, alpha 0.1); the bounded sharpen; mip bias -0.5 while TAA runs; the
  stochastic PCSS and stochastic AO as consumers.
- **Rewrite**: move into `EngineGL33_PostTemporal.cpp` as `PostTemporal` and `PostSharpen`;
  the jitter goes only into the remastered VS sources (the prototype also edited the classic
  VS strings in `EngineGL33_Shaders.cpp`; in Remastered every world program is built on a
  remastered VS, and the pre-world sky draws move to the remastered set in A7); history drop on interiors (`_worldDepthCleared`), optics, NV, camera cuts and
  render-target resizes; the shadow-map depth pass is never jittered.
- **Drop**: `taaStochAlpha` (only for no-MSAA; MSAA stays on), `taaGamma` and `taaJitter` knobs.
- **Conflicts with fixes**: F2 changed `s_vsTransformRemasterGLSL` (failing hunk at line 139);
  F5 changed the composite (failing hunk at `PostFX.cpp:673`); F11: TAA must sit on 4x MSAA at
  render scale 1.0 (the prototype showed sub-pixel antennas break without MSAA); F3: the unit-6
  raw sampler gets `_dummyDepthArray` whenever no map is active (the prototype already did) and
  must be re-bound after `EnsureArrayTarget` recreates the array.
- **Conflicts with features**: shadows also built a raw sampler on unit 6 and a PCSS; one
  implementation (B1) with a TAA path (4 rotated taps) and a no-TAA path (12-tap PCF or
  deterministic PCSS). Volumetric had its own temporal accumulation (`volTemporal`); it moves
  onto the shared history and reprojection helpers. texsharp's CAS and TAA's sharpen share
  `PostSharpen`.
- **Switch**: `taa`.
- **Critic fixes folded**: frame 6 to 18% softer: MSAA under TAA, sharpen 0.5, mip bias -0.5,
  checked on the sharpen sweep view (`sinai_lowsun_fp`); soft-shadow demo used a 2.5 deg sun:
  ship 1 deg; ghosting behind fast vehicles untested: the video's TAA path includes a Jeep at
  0.12 m per frame and at 0.5 m per frame. **Documented**: per-object motion vectors and a
  reactive mask for particles, muzzle flashes and glass (needs a velocity attachment or moving
  particles after the resolve), and an HDR resolve before the tonemap (the frame target is RGBA8
  display space today). Both become a TAA phase 2 only if the video shows ghosting or smeared
  smoke.
- **GPU gating**: world drawn, post on, not interior, optics or NV.
- **Tests**: Halton sequence and jitter range; reprojection matrix (C++ mirror of the shader
  math, previous view to current) against hand values; `Taa` slot in the layout test; glslang
  for both new post shaders; a GL state audit that the jitter is zero for the HUD.
- **Effort**: 5 days.

### A6. Shared light list

Not a research track of its own; MENU phase 2 asks for it, and combat, night and materials each
built a different list (combat: HDR flag in `LightDescription::hdrRange` through the per-draw 8
slots and `Scene::SelectLights` letting combat lights through by day; night: a camera-centric
list of the 8 strongest lights handed to every draw; materials: `localSpec` over the same 8).

- **New**: `Graphics/Rendering/Lighting/FrameLights.{hpp,cpp}` (pure binning: per light kind
  `Lamp | Spot | Fire | Combat | Classic`, camera-relative position, linear radiance, core
  radius, range, spot direction and cut-off profile, owner object id for self-exclusion, shadow
  slot; `BinLightsToTiles(lights, view, proj, tileSize, out)` from each light's bounding sphere);
  GL side in `EngineGL33_Lighting.cpp`: a texture buffer (`kLightDataUnit`, up to 64 lights, 4
  texels each) and a tile index texture (`kLightTileUnit`, 32 px tiles, up to 8 indices per
  tile); GLSL `localLightsTiled()` in the lights piece.
- **Keep the 8-slot path** (`UploadLitLights`) for Classic and as the `lightTiles=0` fallback.
  The F9 clamp of 1.0 applies to the `Classic` and `Lamp` kinds (display-space sum, as today);
  `Combat` and `Fire` kinds are linear HDR and skip it.
- **Conflicts with fixes**: F9 (clamp, night eye), F7 (lamps switch cleanly).
- **Conflicts with features**: A4 (per-object gate becomes Classic-only), B3, B4, B7 consume it.
- **Switch**: `lightTiles`.
- **GPU gating**: binning only when a local or combat light is in view; tiles with no light cost
  one integer fetch.
- **Tests**: `test_frame_lights.cpp` (`BinLightsToTiles` against brute force; ranking keeps the
  strongest at the camera; owner exclusion); layout (`LightTiles`); glslang (integer and buffer
  samplers compile under 3.30).
- **Effort**: 5 days.

### A7. Sky table and the remastered sky pass (atmosphere)

- **Prototype touched**: new `Graphics/Remaster/Atmosphere.{hpp,cpp}` (Hillaire / Bruneton
  tables, `BuildSkyView`, `Sample`), `LightRig.hpp`, `TransLight.cpp` (`DoSkyLighting` marks dome
  vertices with `t1 = 4096`), `EngineGL33_Lighting.{hpp,cpp}` (CPU LUT rebuild and upload in
  `UploadLitFrameConstants`, unit 6, `atmoLight` rig tint), `EngineGL33_LightingShaders.cpp`
  (LitConstants copy in `s_vsScreenRemasterGLSL`, view ray from NDC, `vLitMode` 2 and -1, sky
  shading and aerial fog in the common FS and the mains), `EngineGL33_Mesh.cpp` (`BeginMesh`
  opens the remastered programs before the world pass), `EngineGL33_Shaders.cpp` (sampler row).
- **Keep**: the atmosphere model and its constants; the per-row classic brightness reference;
  the aureole from Mie per pixel; the night fade between -3 and -9 deg; the fog colour from the
  table (aerial perspective) for lit and pre-lit draws including the horizon polygon.
- **Rewrite**: the sky-view table is built on the GPU (one fragment pass into a 192x64 RGBA16F
  target), with the transmittance and multiple-scattering tables built once on a worker thread
  at load (MENU: 8 to 12 ms CPU per rebuild would hitch). The pre-world sky draws go through one
  explicit hook, `Engine::BeginRemasterSky(mode)` / `EndRemasterSky()`, called from
  `Landscape::DrawSky` and `Landscape::DrawClouds` (the clouds prototype's `SetRemasterSkyMode`
  shape), replacing the `BeginMesh` hack; the dome is identified by that mode, not by the t1
  marker. The NDC reconstruction takes the pillarbox rectangle (`_aspectSettings`). Portrait,
  preview and NV paths stay classic.
- **Drop**: `atmoLight` (blue ground, rejected), `atmoSun` (never captured), `atmoExposure`, the
  `ATMO lut` and `ATMO skymesh` logs.
- **Conflicts with fixes**: F3 (`InitLightingResources`, failing hunk at `Lighting.cpp:107`);
  F9 (`finishFragment` night eye, failing hunk at `LightingShaders.cpp:606`).
- **Conflicts with features**: clouds forces the same programs for the same draws and declares
  the same three camera-basis slots: one hook and one set of slots (3.3). Haze blitted the sky
  into a backdrop texture on unit 6 to get "the sky behind": the table replaces that blit. Water
  and SSR take the sky in a reflected direction from the table.
- **Switch**: `atmosphere` (dome and fog colour; the table itself always builds by day because
  water, air and clouds read it).
- **Critic fixes folded**: ship `atmoHue` 0.5 and `atmoSat` 1.15 (critic); inspect the olive band
  away from the sun at sunset (relit-hue mix) on `coastline_sunset` and `firefight_dusk`; keep
  noon cirrus by taking the cloud mask from clouds' density map (B10) once it lands, and until
  then lower the 0.015 dead zone and check `noon_fields`; "water reflection not visible" is fixed
  by B5 reading the table. **Documented**: the physical sky light for the rig (needs the phase 4
  bounce experiment).
- **GPU gating**: sun above -9 deg; LUT pass only when the sun moved 0.02 deg or haze changed.
- **Tests**: `test_atmosphere.cpp` (`Atmosphere::Sample` against reference values at noon,
  sunset and -4 deg; transmittance monotone with air mass); a CPU mirror of one LUT texel
  against the GPU pass output in the lab; layout for slots 61-67; glslang for the LUT pass.
- **Effort**: 5 days.

### B1. shadows (tree shadows, soft sun shadows)

- **Prototype touched**: `GameStateExtTestRender.cpp` (12 knobs and `objShadows`, `casterLod`),
  `LightRig.hpp`, `EngineGL33.hpp`, `EngineGL33_Lighting.{hpp,cpp}` (raw sampler `shadowDepth`
  on unit 6, `Pcss` slot), `EngineGL33_LightingShaders.cpp` (`pcssLit`), `EngineGL33_PostFX.cpp`
  (`contactOcclusion` in the composite), `EngineGL33_Shaders.cpp` (sampler row).
- **Keep**: PCSS at a 1 deg sun on the stock 2048 map with the tight near tiers 0.04 / 0.15
  (the `prod` variant, which matched `best` at a fraction of the shadow-pass cost).
- **Rewrite**: one PCSS in the shadows piece with two paths (TAA on: 4 rotated taps per frame
  from A5; TAA off: the deterministic 16-tap search and 12/24/32 taps of `pcssLit`); blocker
  search on a coarser cascade for tiers 2-3; larger normal offset for foliage receivers against
  leaf-card acne.
- **Drop**: contact shadows (near-invisible, and the composite version darkens local lights and
  fog), the far-field contact variant, `bestlong` (range x2: no gain, +4 to 8 ms), 4096 except
  on the Ultra tier, `objShadows` and `casterLod` knobs (F10 drives casters from the Graphics
  tier).
- **Conflicts with fixes**: F10 is the visible part of this track; F3 (unit 6 sampler);
  F1 knob table (failing hunk at `GameStateExtTestRender.cpp:571`); F5 composite (failing hunk at
  `PostFX.cpp:214`, irrelevant once contact shadows are dropped).
- **Conflicts with features**: A5 (one PCSS), B8 and B9 need casters, B2 crown term and the crown
  self-shadow.
- **Switch**: `treeShadows` (static casters in Remastered; Classic follows the Graphics tier as
  F10 made it), `softShadows`.
- **Critic fixes folded**: the wrong large shadow on the Sinai foreground with the tight 4096
  setup: reproduce on `sinai_lowsun_fp` with `debug=1` before shipping tight tiers there, and
  gate the tight tiers off if not understood; crown self-shadow flicker: foliage receivers fade
  their own crown's shadow with distance and TAA smooths the rest. **Cheap or not**: woods 14 to
  21% darker and greyer. Cheap part: B2's vertical fill and crown term go in right after.
  Not cheap: a leaf transmittance layer (an RG8 colour attachment in the shadow pass storing the
  minimum leaf transmittance and a tint, so 15 to 25% of the sun passes crowns; switch
  `leafLight`, 3 extra days); do it only if the B1 look check still reads woods as "only darker".
- **GPU gating**: sun above 4 deg (or the moon key from B3); PCSS taps follow `taa`.
- **Tests**: PCSS penumbra width math (C++ mirror of the blocker-to-metres conversion);
  `[ShadowSampler]` for unit 6; layout for `Pcss`.
- **Effort**: 3 days (+3 for `leafLight`).

### B2. shade (low-sun rig retune)

- **Prototype touched**: `LightRig.cpp` (evening hue share, vertical-face fill, cloud floor,
  all behind `moonlight=1.01`), `Shape.hpp`, `ShapeDraw.cpp` (lazy VB); shader half in
  `shade/ovr_final/` (crown V18 in `psRemasterCommon.glsl`, in-scatter in `psPostComposite.glsl`).
- **Keep**: in `ComputeLightRig`, the extra sunset-hue share
  `0.4 (1 - S(6,25,e)) + 0.3 (1 - S(4,9,e))` and the vertical-face fill
  `(1 - S(8,40,e)) S(-2,3,e)` (horizon x(1 + 2.6 fill), sunward horizon x(1 + 0.5 fill), ground
  x(1 + 1.3 fill)); in `shadeRemastered`, crown V18 (canopy saturation `pow(a,1.3)` renormalised,
  crown optical depth with `T = mix(1, exp(-t), 0.75)`, crown core sky 0.45, low-sun crown sun
  x0.65 and sky x0.75 below 12 deg, none above 35).
- **Rewrite**: the `moonlight` sentinel becomes `lowSunRig`; V18 becomes plain code under
  `crownCanopy` (no `g_variant`).
- **Drop**: lazy VB (F8), cloud floor (F4), in-scatter composite (F5), the debug-3 albedo hack
  (or keep it as `debug=5 albedo`, which made the calibration possible).
- **Conflicts with fixes**: F4 (failing hunk at `LightRig.cpp:182`), F2 (crown normal now
  computed once; V18 goes on top of that), F8, F5.
- **Conflicts with features**: B1 (crowns now self-shadow), atmosphere (no rig tint from the sky).
- **Switch**: `lowSunRig`, `crownCanopy`.
- **Critic fixes folded**: re-shoot the forest-edge noon and coastline pictures under F4 so the
  comparison does not mix cloud positions. **Documented** (owner calls): spawned vehicle sides
  at noon go from 0.97 (accidental Classic light) to 0.60 (F8 made that real; `bounce=2.5` is
  the lever); golden crowns darker than the tip; the fill is flat, not bounce light.
- **GPU gating**: none (constants once per frame).
- **Tests**: re-pin `test_remaster_lighting.cpp`: the 7 deg "evening sun keeps the original's
  sunset hue" case moves; add pins for the anti-sun wall at about 0.59 of the sun wall from 7 to
  14 deg (shade's `unitcheck.py` numbers), noon unchanged, and `lowSunRig=0` equal to the tip
  rig. Update the README calibration table.
- **Effort**: 2 days.

### B3. night (moon shadows, low beams, stars, physical lamps)

- **Prototype touched**: `GameStateExtTestRender.cpp` (12 knobs), `LightRig.hpp`, `Lights.cpp`
  (`g_nightPhysicalLights`, `LightPoint::Apply` inverse square), `Object.cpp`
  (`Object::DrawDecal` linear sum plus moon fill), `SceneDraw.cpp`
  (`DrawObjectsAndShadowsPass2` renders the shadow pass for the moon), `SceneShadowPass.cpp`
  (`RenderShadowMapDepthPass` takes the moon direction), `EngineGL33_DrawShared.cpp`
  (`DrawPoints` star splats), `EngineGL33_Lighting.cpp` (moon key in the sun slots, camera
  list, headlight self-levelling aim, lamp warming, logs), `EngineGL33_PostFX.cpp` (haze);
  shader `night/ovr_v9/psRemasterCommon.glsl`.
- **Keep**: moon as key light routed through the sun slots so the shadow map, terrain horizon,
  cloud field and glossy follow it; the low-beam pattern (wide horizontally, cut-off 0.9 deg
  above the axis, hot band 1.7 deg below) with the self-levelling aim at the ground 25 m ahead;
  inverse-square lamps from the bulb; Oren-Nayar rough diffuse for local lights; star splats;
  linear-light smoke lighting.
- **Rewrite**: the moon key selection moves into the rig as a pure function
  (`KeyLight(rig, in, tuning)` in `LightRig.cpp`) that keeps F9's calibration (`kMoonFull`
  0.02, chroma, `kMoonSky`) instead of the prototype's 0.05 and its own chroma: `moonShadows`
  only moves the existing moon from the shadowless `litMoon` term into the key slots and turns
  on the shadow pass for it. The camera list is replaced by A6. Lamp and spot shading become
  kinds in `localLightsTiled()`. Particle lighting (`Object::DrawDecal`) becomes one shared
  function `ParticleLight(pos)` in `FrameLights`, used by night and combat (both prototypes
  rewrote the same loop). The `w` lanes the prototype borrowed (`litHorizonSun.w`,
  `litFogCap.w`, `litMoon.w`, `litCamPos.w`, `litLightCount.yzw`) move to the `Night` slot.
- **Drop**: `nightHaze` and the post haze pass (brown sky, rejected), `nightBounce` (no effect),
  `nightLampWarm` (F9b gives the authored colour; re-judge, default 0), `nightCamLights`
  (replaced by A6), `nightDebug` views, `NIGHT:` logs, the Purkinje model (F9 already ships a
  tested night-eye grey; keep one).
- **Conflicts with fixes**: F9 and F9b throughout (`Lights.cpp:404` failing hunk shows the
  overlap); F7 (lamp switching in tests); F1 knob table; F10 (moon shadows need casters).
- **Conflicts with features**: A6, B4 (`DrawDecal`, light kinds), B8 (headlight cones read the
  same spot profile), B7 (lamp streaks read the same lamps), A4 (moon shadow pass and the hill
  cull).
- **Switch**: `nightLights` (physical lamps and low beams), `moonShadows`, `stars`.
- **Critic fixes folded**: mustard verges under headlights (spot colour from config, no warming,
  then measure verge hue on `road_flat_drive_moonless`); black tree crowns next to lamps (local
  lights on foliage use the crown normal with the same wrapped terminator as the sun);
  jeep-rear leak (owner exclusion in `FrameLights` plus a back-plane test for spots); hill roads
  (softer cut-off, aim from the body pitch plus a 1% down-tilt, test on `road_head`); headline
  pictures must be shot with bounce off (the recommended setting). **Documented**: moonless
  country may be too dark to play (needs an owner night-exposure policy; the exposure track's
  eye adaptation is the candidate); lamp light through walls (needs lamp shadow maps);
  "moving the moon makes no visible difference in town".
- **GPU gating**: `NightEffect` above 0.01; moon shadow pass only with the moon above 3 deg and
  phase above 0.25; tiled lights as A6.
- **Tests**: `KeyLight` (moon key only when the sun is 3 deg under and the moon up; day rig
  bit-identical to the tip, as F9's test pins); low-beam profile (cut-off angle, hot band) as
  a C++ mirror; self-levelling aim on flat and 14 deg slopes; star splat geometry; layout
  (`Night`).
- **Effort**: 5 days.

### B4. combat (emissive combat light with shadows)

- **Prototype touched**: new `Graphics/Rendering/Lighting/CombatLights.{hpp,cpp}`;
  `Smokes.{hpp,cpp}` (`WeaponLightSource::Start/Simulate`, `WeaponFireSource`,
  `SmokeSourceVehicle::Simulate`, `Explosion::Simulate`), `Lights.hpp` (`hdrRange`,
  `IsCombat`, `IsVisualOnly`), `SoldierOldAI.cpp` and `TankWithAI.cpp` (`FireWeaponEffects` pass
  `hit`), `Shots.{hpp,cpp}` (`ShotBullet::Simulate` tracers, `IlluminatingShell::Simulate`
  flares), `Fireplace.{hpp,cpp}`, `Object.cpp` (`DrawDecal`), `Scene.{hpp,cpp}`
  (`SelectLights` by day, `SelectActiveLights` ticks), `EngineGL33.hpp`,
  `EngineGL33_Lighting.{hpp,cpp}` (slots 58-64, unit 6, `UploadLitLights` HDR path),
  `EngineGL33_LightingShaders.cpp` (`combatLights()`, `combatShadow`, glossy flash, night-eye
  lift), `EngineGL33_PostFX.cpp` (in-scatter), `EngineGL33_Shaders.cpp` (`UploadVSLights` skips
  HDR), `GameStateExtTestRender.cpp` (7 knobs).
- **Keep**: `EvaluateCombatLight` envelopes (muzzle, cannon, explosion, fire breathing, tracer,
  flare from `lightColor`), the ball falloff `I / (1 + d^2 / r0^2)` with the window, the day
  scaling, all gameplay hooks, the analytic in-scatter, smoke lit from inside.
- **Rewrite**: lights become `Combat` and `Fire` kinds in A6. The per-fragment 24-step trace
  through last frame's depth becomes a mask pass in `EngineGL33_PostCombat.cpp`: at the start of
  each world pass, for up to 4 shadowed lights picked with hysteresis, a half-resolution trace
  through the previous frame's half-res linear depth (A3), bilateral-blurred and accumulated
  over frames, written one light per channel of an RGBA8 target on `kCombatMaskUnit`; the
  forward shader reprojects its fragment into last frame's screen and takes one bilinear tap per
  shadowed light. Combat lights are always simulated in Remastered and only the upload and
  shading follow `combatLights`, so the switch never changes the simulation (needed for the
  paired video mode); the classic lights' "visual only" state is decided at light selection, not
  at spawn. `CombatLights::Tick` ages lights by simulation time so paused frames do not age them.
  Per-ammo strength comes from config (`hit`, `indirectHit`, explosion size) instead of the
  table.
- **Drop**: slots 59-64 (reprojection rows move into the mask pass uniforms), the 2x2 ordered
  jitter (failed), `combatWreck` knob (always on with `combatLights`).
- **Conflicts with fixes**: F9: the 1.0 clamp must not touch combat kinds (HDR); the failing hunk
  at `LightingShaders.cpp:564` is `shadeRemastered` after F2/F9; F5 composite (`PostFX.cpp:215`)
  for the in-scatter.
- **Conflicts with features**: A6 (kinds, 8-slot path), B3 (`DrawDecal`, night eye), A5 (mask
  accumulation shares the reprojection helpers), A3 (half-res depth), B7 (flashes give wet
  streaks for free via the light list).
- **Switch**: `combatLights`, `combatShadows`.
- **Critic fixes folded**: shadow flipping between two fires (2 to 4 shadowed lights plus
  hysteresis: keep a shadowed light unless another exceeds it by 1.5x for 0.3 s); the seam down
  the exact screen centre (find it in the mask pass reprojection with a unit test of the UV
  mapping at NDC 0, and check a frame with a flat ground in debug); stipple and cross-hatch
  (mask blur and accumulation); flash frame darker than Classic's whiteout (37 against 63 at eye
  height): a brief flash exposure response proportional to the flash irradiance at the camera,
  tuned until the `ffn` flash frame mean reaches Classic's within 10 levels; beige ground under
  fire light (lower the fire chroma saturation, check R/B on `ffn` lit ground against 1.4); the
  warm glow strip on the horizon behind fires (cut combat light on receivers beyond 2x the
  light's range from the camera-to-light line, or fade the wrap term at grazing). **Documented**:
  one-frame lag on moving men (mask from last frame), the muzzle light visible in one captured
  frame (decide if intended), tank cannon, interiors, MP, rain not tested.
- **GPU gating**: combat light within 60 m; mask pass only for shadowed lights in view; day
  scaling already makes daytime lights near invisible, so by day the shadow mask is skipped
  unless `NightEffect` is above 0.3.
- **Tests**: `test_combat_lights.cpp` (`EvaluateCombatLight` envelopes, finish times, day
  scaling, hysteresis selection); mask reprojection UV at the centre and edges; layout
  (`CombatShadow`); glslang for the mask pass.
- **Effort**: 7 days.

### B5. water (sea waves, variant C)

- **Prototype**: shader-only override of `psDetailRemasterMain` (`isSea()` by material
  specular), on branch `claude/remaster-water-research` commit 935cb5e
  (`tests/rendering/water_research/shaders_C/`).
- **Keep**: the wave normal from the sea texture's luminance with the per-pixel UV-to-world
  Jacobian; Fresnel sky reflection with the low mean weight and full wave-driven variation;
  the broad and tight sun lobes; C's swell (0.27x, rotated 37 deg) and ripple
  (`specular_dx` alpha) layers, the 400 m to 3 km fade, Toksvig-style sparkle widening.
- **Rewrite**: route the sea explicitly: landscape water faces get an `IsWater`-style bit in
  Remastered only (`LandscapeRender.cpp`, the `WaterFlags` lines the water notes cite), mapped
  to `s_psWaterRemasterMain`, which becomes the variant C shader (its current constant "bump
  normal" from the detail texture is dead code and goes). Classic keeps the PSDetail path.
  Constants move to `Water0`/`Water1` and tuning. The next animation frame is bound on
  `kSeaNextFrameUnit` and cross-faded by `texPhase` (computed in `LandscapeRender.cpp`) so the
  normal does not pop every 0.14 s. The sky term reads the A7 table in the reflected direction
  instead of `fogColorToward(r)`.
- **Drop**: `isSea()` heuristic, variants A and B, the dead `misc\waveBumpMap.paa` entry and
  `GetWaterBumpMap()` (and fix the README Water row).
- **Conflicts with fixes**: F9: the shader copies the pre-F9 `vec3(1.5)` local clamp; use the
  shared lights piece.
- **Conflicts with features**: texsharp's self-detail and haze also edited the terrain detail
  main the sea used; routing the sea out of PSDetail removes that overlap. B6 feeds it. A7.
- **Switch**: `seaWaves`.
- **Critic-equivalent fixes** (no critic reviewed it; the owner named the noon colour): noon
  water cooler and bluer than Classic's yellow-green: at high sun lower the mean sky weight and
  tint the reflected sky toward the classic fog colour, then measure the water region of
  `sea_noon` against Classic with `src/compare.py` until the hue is within 10 deg and the mean
  within 10 levels; far sunset sea lighter and greyer (fade the mean sky weight with distance
  beyond 1 km); popping and crawl checked in the video (S mode with the sea clock running).
- **GPU gating**: per sea fragment only.
- **Tests**: glslang (water main), layout (`Water0/1`), a render-flag test that landscape water
  faces route to `ShaderFamily::Water` only in Remastered.
- **Effort**: 3 days.

### B6. ssr (screen-space reflections)

- **Prototype touched**: `GLDepthStencilState.hpp` (`StencilEqualZeroIncr` compare mask 0xFF to
  0x1F), `LightRig.hpp`, `TransportCore.cpp` (`g_ssrDrawClass`, `FlushQueues` around
  `Transport::Draw`), `EngineGL33.hpp` (tags, targets), `EngineGL33_Lighting.cpp`
  (`UploadLitMaterial` gloss tag), `EngineGL33_PostFX.cpp` (tag readout, mip chain,
  `RunSsrPass`), `EngineGL33_Queue.cpp` and `EngineGL33_VertexBuffer.cpp` (class tag on
  `FlushQueue` / `EmitDraw`), `EngineGL33_Shaders.cpp` (`DoSelectPixelShader` tag),
  `EngineGL33_State.cpp` (`ApplyDepthMode` keeps the tag, `ApplySsrStencilMask`,
  `UpdateSsrFamilyTag`, `ApplySsrStencilTag`, `ApplyPipeline` on-surface flag); shader
  `ssr/glsl_v12/psPostSsr.glsl`.
- **Keep**: stencil tags in the top three bits (water 0x80, terrain 0x40, road 0x60, vehicle
  0xA0, glossy 0x20) with the zero write mask for depthless untagged draws; the readout; the mip
  cone; the march with its miss handling (sky continuation, side-exit edge hits only above the
  ray start, behind-near dark estimate); water as one mirror trace plus ripple re-projection;
  puddles on flat roads and bare ground, none on grass.
- **Rewrite**: the vehicle tag comes from the draw's object kind (materials' `ClassifyDrawObject`
  moved onto the draw descriptor) instead of `g_ssrDrawClass` and the extra `FlushQueues`;
  half-resolution trace with temporal accumulation (A5) and a bilateral upsample; edge hits fade
  with distance from the edge; on water the post pass replaces only the forward sky share,
  `frame += F * (hit - skyFromTable)`, so the reflection feeds B5's sky term instead of being
  laid over it (the post pass recomputes the same table lookup); the sea ripples use B5's wave
  normal function where cheap, the procedural octaves otherwise. Wetness reads B7's state.
- **Drop**: `ssrGlossy` paint reflections (null result), debug logs in `TransportCore.cpp`,
  `EngineGL33_Queue.cpp`, `EngineGL33_VertexBuffer.cpp`, `ssrDebug` (keep as `debug` views).
- **Conflicts with fixes**: F1 knob table; the stencil mask change touches the projected-shadow
  path that Classic uses (Classic never writes the top bits; verify with a Classic A/B).
- **Conflicts with features**: B7 had its own `s_psPostSsrGLSL` for puddles (same symbol name):
  one SSR pass; B5 sky term; A5 history.
- **Switch**: `ssr`.
- **Critic fixes folded**: sunset sea pale lilac-grey (ship `ssrWater` 0.6 with the masking
  floor, and replace the sky share instead of adding); edge seams (edge fade); pale boxes under
  bushes and vehicles (treat hits on alpha-tested clutter whose coverage is low as misses; the
  bush quad writes depth over its whole card); the missing settings-registration file (the
  registry covers it); the dusk picture from an older shader (re-shoot on v12). **Documented**:
  nights get nothing from SSR (lamp streaks are B7's job).
- **GPU gating**: water or wet tag share above 1% of the screen (1/8-res readout of the last
  frame), not interior, optics or NV.
- **Tests**: tag code table (bits do not overlap the 0x1F projected-shadow range); march hit
  logic as a CPU mirror on a synthetic depth row; glslang (`psPostSsr`, tag shader).
- **Effort**: 5 days.

### B7. materials (wet weather, puddles, lamp streaks, material classes)

- **Prototype touched**: `GameStateExtTestRender.cpp`, `Engine.hpp` (`GDrawObjectKind`),
  `LightRig.hpp`, `ShapeDraw.cpp` (`ClassifyDrawObject`, `KindScope`, `vbfix`, diagnostics),
  `EngineGL33.hpp`, `EngineGL33_Lighting.{hpp,cpp}` (`ClassifySurface` token lists,
  `UploadLitSurface` per section, `MatClass`, `Weather`, `Research` slots),
  `EngineGL33_PostFX.cpp` (puddle SSR), `EngineGL33_Queue.cpp` (`PrepareTriangleTL` calls
  `UploadLitSurface`); shaders `materials/override_v9/` (common, three mains, `psPostSsr`).
- **Keep**: 14 surface classes and the per-class response table; wetness darkening and
  saturation by porosity, water film, run-down streaks, canopy-sheltered ground drier; puddles in
  flat hollows (noise plus heightfield concavity) with rain rings; the v9 light specular split
  (dry base plus water-film lobe) for lamp and fire streaks; glass reflecting the sky.
- **Rewrite**: classes are baked at texture load from a config table that mods extend
  (`CfgSurfaceMaterials`-style name patterns, Czech and English tokens moved out of C++), stored
  on `TextureGL33`, plus an object-kind field on the draw descriptor (no `GDrawObjectKind`
  global); terrain classes per terrain texture name, not by colour; wetness becomes world state
  (new pure `Graphics/Remaster/Wetness.{hpp,cpp}`: wet up in rain, dry over minutes with sun and
  wind, puddles slower than surfaces); `MatClass` uploads through A4's mirror helper; puddle
  reflections use B6's pass; streaks loop over A6's tiles.
- **Drop**: `vbfix` (F8), the dry satin rim (almost invisible; keep glass), albedo-derived bump
  on dry ground unless the B7 look check shows it, `r1`/`r2` research knobs, `MATRESEARCH` logs.
- **Conflicts with fixes**: F8 (`ShapeDraw.cpp` overlap: patch applies textually but duplicates
  the late-buffer logic); F9 (night is now darker, so streaks read stronger; re-tune against it).
- **Conflicts with features**: B6 (one SSR), A6 (streaks), A4 (partial uploads), B3 (lamps).
- **Switch**: `wetWeather`, `lampStreaks`.
- **Critic fixes folded**: wet paint bluish at noon (cap the sky reflection weight on paint and
  tint it toward the fog colour; check saturation of the noon jeep side against the dry value);
  vertical stripes on the wet M113 (lower streak frequency and contrast, fade beyond 20 m); hard
  triangle puddle edges following the terrain grid (bicubic height and concavity, soft 0.3 m
  edge); wet film on the grass-textured town square (class table per terrain texture); thin
  white dashes on far wet ground (fade the film lobe with distance and mip roughness, TAA).
  **Documented**: the soldier's puddle reflection is not visible; dry reconstruction near
  invisible.
- **GPU gating**: wet path only with wetness above 0.02; classes are free (one cached slot).
- **Tests**: class table parser and token matching on the `classification_log_r6.txt` names;
  `Wetness` integrator (wets in rain, dries, puddles lag); layout (`MatClass`, `Weather`);
  glslang.
- **Effort**: 6 days.

### B8. air pass (volumetric beams and headlight cones + haze and valley mist)

- **Volumetric prototype touched**: `GameStateExtTestRender.cpp` (16 knobs), `LightRig.hpp`,
  `EngineGL33.hpp` (vol targets, history), `EngineGL33_PostFX.cpp` (`s_psPostVolumetricGLSL`,
  `s_psPostVolBlurGLSL`, `s_psPostVolTemporalGLSL`, composite changes, the shadow array and
  cascade uniforms bound in post); shaders `volumetric/glsl/`.
- **Haze prototype touched**: `Engine.hpp` (`CaptureSkyBackdrop`), `LandscapeRender.cpp`
  (`Landscape::Draw` captures after sky and clouds), `EngineGL33.hpp`, `EngineGL33_Lighting.cpp`
  (blit into a mipmapped backdrop on unit 6), `EngineGL33_Shaders.cpp` (sampler row); shaders
  `haze/shaders_r8/` (five files including all three VS) and `haze/src/haze_block.glsl`.
- **Keep**: the half-res ray march of single scattering with sun visibility from the shadow
  map, the terrain horizon (lowered 0.07 deg per metre of height) and the cloud field; the
  `0.85 HG(g) + 0.15 HG(-0.3)` phase; spotlight cones over the shading reach (`volSpot`); the
  depth-aware mean with `volGain` and `volEdge`; haze's closed-form height extinction (scale
  height 350 m, chroma (0.85, 1, 1.2) with grey extinction as the option), the sky-light and
  sun-light split, the valley mist pooled on the lowest ground within about 850 m with the
  morning weight, and the 12-sample sun march for the mist.
- **Rewrite**: one `PostAir` stage in `EngineGL33_PostAir.cpp` computing inscatter and
  transmittance once per half-res pixel (`C' = C * T + S`), 24 steps plus temporal accumulation
  (probe 7 matched 48 steps), full-res depth-aware upsample near edges. The medium is driven by
  weather and time: mist 0.3 with gain 4 in clear weather, rising toward 1.5 with gain 2.5 at
  dawn, dusk, in `setFog` and in rain; haze sigma from the fog range. The limit colour is the
  A7 table, not a backdrop blit. The mist floor is precomputed once per island beside the
  horizon map (`TerrainLighting.cpp`, a texture of the lowest ground with the noise banks baked
  in), so 16 taps become 1. The forward classic fog keeps only the view-distance edge (55% of
  the fog end in clear weather) when `airHaze` is on; blended surfaces get the cheap analytic
  version from the `Haze` slot. A pure `AirMedium` function (`Graphics/Remaster/AirMedium.{hpp,cpp}`)
  holds the medium so AI sight can later read the same optical depth.
- **Drop**: point-light halos for lamps and fires (brown sky), `volCarve`, the un-floored
  high-pass, `CaptureSkyBackdrop`, the per-fragment haze in every remastered pixel shader, the
  `saturation=1.12+0.001k` variant channel, haze variants other than 43 (and 38 as the grey
  option).
- **Conflicts with fixes**: F5: the air pass replaces the legacy shafts when on; `shaftInscatter`
  stays for the legacy path (failing hunk at `PostFX.cpp:206`). F10 (no beams without casters).
  F3 (the post pass binds the shadow array on unit 2 with the compare sampler and must restore
  it).
- **Conflicts with features**: B1, A5, A7, B3 (spot profile), B9 (canopy visibility could also
  occlude far beams), haze and volumetric were each touching every VS (haze added `vSkyClip`).
- **Switch**: `airBeams`, `airHaze`, `valleyMist`.
- **Critic fixes folded**: the veil over backlit crowns and the horizon band (weather-driven
  mist); clear weather making backlit vehicles 15 to 19% darker than the tip: re-measure first,
  because F5 already removed the old shaft glow on near objects that the critic compared
  against; blocky glow patches beside trees (full-res upsample at depth edges); haze's 06:00 to
  06:30 mid-field cool lift and lost contrast (cap the mist forward lobe and sky term inside
  600 m); far hills bluer (ship the gentler chroma, offer grey extinction); the crosshatch (TAA);
  the hard line where the horizon band meets the mist (blend the horizon polygon's fog with the
  mist top). **Documented**: mist hides the lake glint; AI sight ignores the mist (gameplay
  follow-up issue on the fork, `gh -R GearUnclear/CWR`); one volumetric headline picture from
  older code.
- **GPU gating**: as 3.2; nothing at noon in clear weather except the cheap far haze.
- **Tests**: `AirMedium` optical depth against numeric integration; mist floor builder on a
  synthetic valley; glslang for the air shaders; F3 audit for unit 2 after the stage.
- **Effort**: 8 days.

### B9. hfrt (long tree shadows from the canopy heightfield)

- **Prototype touched**: `TerrainHorizon.{hpp,cpp}` (canopy fields, options),
  `TerrainLighting.cpp` (`GatherCanopy`, `RasterizeCanopy`, `RasterizeCanopyFine`, mips,
  `CWR_HFRT_CANOPY_RES`, `CWR_HFRT_CANOPY_HORIZON`), `EngineGL33_Lighting.{hpp,cpp}` (upload,
  units 6 and 8), `EngineGL33_Shaders.cpp`; shaders `hfrt/override_v10/` (common and
  `psNormalRemasterMain`), mode picked by `terrainShadows=<mode>`.
- **Keep**: mode 20 at k = 2 (6.25 m texels, sqrt crossing density, baked horizon map for the
  terrain, no canopy sky occlusion); the gather (map types and opacities), the worker-thread
  rasteriser and the max/mean mip rules; raised receivers skipping their own crown.
- **Rewrite**: a half-res canopy sun-visibility buffer built at the start of the world pass
  from last frame's half-res depth (same mechanism as combat's mask), temporally accumulated,
  read with one reprojected tap on `kCanopyVisUnit`; the canopy only for receivers beyond the
  shadow map's reach (last split minus the fade), cross-faded with it, now that F10 gives the
  near trees real shadows; terrain height packed into the canopy texture so each step is one
  fetch; `Canopy` slot and `canopyShadows` switch; the canopy build counts as "horizon build
  pending" for `triSettleScene`; destroyed trees re-rasterise their texels.
- **Drop**: terrain tracing, terrain GI and per-pixel sky visibility (negative), per-vertex mode,
  the env vars, canopy sky occlusion, the static interleaved-gradient jitter.
- **Conflicts with fixes**: F1 (`TerrainLighting.cpp` pending query), F10.
- **Conflicts with features**: B1 (near range), A5 (accumulation), B8 (could occlude far beams).
- **Switch**: `canopyShadows`.
- **Critic fixes folded**: cost too high as built (half-res buffer, far receivers only, low-sun
  gate; time it on `slope_tele_1730`, the +13 to 16 ms worst case); near shadows as soft slabs
  (the shadow maps own the near range now); fixed dither crawl (accumulation); the mixed-run
  timing table (re-time on a quiet GPU). **Documented**: it only removes light (no bounce);
  cool shade on pink rock (the shade is the sky fill; B2's fill helps); crown base heights for
  broadleaf trees.
- **GPU gating**: sun 2 to 25 deg, sky-through above 0.5, receivers beyond the cascades.
- **Tests**: rasteriser and mip rules on synthetic crowns; sqrt crossing density; layout
  (`Canopy`); glslang.
- **Effort**: 6 days.

### B10. clouds (lit sky clouds, sky-matched cloud shadows)

- **Prototype touched**: `GameStateExtTestRender.cpp` (`skyClouds`, `cloudShadowSource`,
  `r0..r7`), `Engine.hpp` (`SetRemasterSkyMode`), `LightRig.hpp`, `Landscape.hpp` and
  `LandscapeRender.cpp` (`DrawSky` sky modes, dome and sheet dumps, `DrawCloudsClassic`,
  `CollectCloudSheets`), `EngineGL33.hpp`, `EngineGL33_Lighting.{hpp,cpp}` (`UpdateSkyDensityMap`,
  `UploadSkyCamera`, unit 6), `EngineGL33_LightingShaders.cpp` (`skyCloudShade`,
  `domeCloudCover`), `EngineGL33_Shaders.cpp`, `TextureGL33.hpp` and `TextureGL33_Init.cpp`
  (`GetPixels`); shaders `clouds/glsl_final/`.
- **Keep**: the density map from the weather sky texture (row-relative whiteness, overcast-deck
  flag, darkness), blurred; the march toward the sun in direction space through the dome UV
  mapping; mean-free relief; silver lining; dome-matched cloud shadows; the sun disk dimming
  under a dome cloud.
- **Rewrite**: the density-map builder moves to a pure `Graphics/Remaster/SkyDensity.{hpp,cpp}`
  built on weather change off the render thread; the sky hook is A7's; `r0..r7` become real
  constants in `Sky` (deck altitude 1800 m, threshold 0.55, relief 1.8, silver 1.0); the shadow
  projection anchors to a slowly re-centred origin with a cross-fade instead of the camera;
  the drift comes from the existing wind integrator; atmosphere takes its cloud mask from this
  map.
- **Drop**: the `CLOUDSDOME`/`CLOUDSSHEET` dumps, the unused `CollectCloudSheets` scaffolding
  (keep as a follow-up for sheet shadows), variant B, the mis-encoded comment hunk.
- **Conflicts with fixes**: F4: sky-matched shadows must use `rig.cloudCover` after the clear-sky
  fade and keep the mean-preserving dimming.
- **Conflicts with features**: A7 (hook and camera slots), B8 (cloud field in the march), B5
  (sea glitter under clouds changed in the prototype and was not mentioned).
- **Switch**: `litClouds`, `skyCloudShadows`.
- **Critic fixes folded**: midday embossing and rainbow fringes (relief on luminance tinted by
  the sun chroma only, relief and silver lining fade between 25 and 45 deg sun); the overcast
  headline was a different view (re-shoot matched). **Documented**: a still under a cloud reads
  darker; overcast barely changes; some shots read +5.5 ms (re-time).
- **GPU gating**: sky-through below 0.85 for lit clouds; shadows from the dome only above the
  clear-sky cover.
- **Tests**: `SkyDensity` on the three weather skies (jasno has no cloud, oblacno streaks,
  zatazeno deck); dome UV mapping; layout (`Sky`, `SkyVp`); glslang.
- **Effort**: 4 days.

### B11. texsharp (terrain detail)

- **Prototype touched**: `GameStateExtTestRender.cpp` (tex knobs), `DrawPoly.cpp`
  (`PrepareTexture`, `StreamFix`, `MipBias`), `EngineGL33_Lighting.cpp`, `EngineGL33_PostFX.cpp`
  (CAS), `EngineGL33_State.cpp` (`CreateSamplerStates` aniso and bias knobs),
  `TextureBankGL33_Cache.cpp`, `TextureBankGL33_Core.cpp`, `TextureGL33.hpp`,
  `TextureGL33_Loading.cpp` (remip, stats); shaders `texsharp/glsl7/`.
- **Keep**: rotated, limited self-detail (zoom 6, strength 1.5, 4 cells, variance-preserving
  blend, `textureGrad` so it fades by itself), detail kept (`K2` 2) with the anti-tiling second
  sample (`K1`), luma-only CAS 0.3, the residency estimate scaled by the render scale
  (`StreamFix`).
- **Rewrite**: into the remastered terrain main with real `Detail` constants (not the borrowed
  `litLightCount.yzw` / `litCamPos.w`); skip beyond 300 m; optional 3-cell hex grid; CAS lives in
  `PostSharpen` (A5) and is skipped when TAA sharpens.
- **Drop**: remip, aniso and LOD-bias knobs, texture stats, the release hook, `UD_TEX_*` env vars.
- **Conflicts with fixes**: F1 (failing hunk at `GameStateExtTestRender.cpp:32`), F5 (failing
  hunk at `PostFX.cpp:673`).
- **Conflicts with features**: B5 (the sea leaves PSDetail), A5 (sharpen), materials' terrain
  classes (skip roads and paths).
- **Switch**: `terrainDetail`, `sharpen`.
- **Critic fixes folded**: dark blotches on paths (cap `K2` at 2x, skip road and path classes);
  olive mottling (rotation already, lower strength to 1.2 if the look check still shows it);
  check at native 1080p and in motion (the video path). **Documented**: a golden slope reads
  drier (owner taste).
- **GPU gating**: terrain within 300 m.
- **Tests**: glslang; a C++ mirror of the cell rotation and limiter.
- **Effort**: 1.5 days.

---

## 6. Conflict hot spots and how to sequence them

| File / unit | Touched by | Rule |
|---|---|---|
| `EngineGL33_LightingShaders.cpp` | F2, F9, temporal, combat, atmosphere, shadows, clouds, night (override), materials (override), shade (override), hfrt (override), haze (override), texsharp (override), water | A2 splits it by concern first. After that one feature edits one piece; `shadeRemastered` (crown and ambient) and `finishFragment` stay single-owner per step: B2 then B7 then B9. The VS sources change only in A5 (jitter) and A7 (sky ray); haze's `vSkyClip` is not ported (A7 replaces it). |
| `EngineGL33_PostFX.cpp` | F5, combat, ssr, volumetric, materials, night, shadows, texsharp, temporal | A3 turns it into stages; every feature adds its own `EngineGL33_Post*.cpp`. Only A3 and A5 touch the stage list itself; later steps register a stage. |
| `EngineGL33_Lighting.{hpp,cpp}` | F3, perf, combat, ssr, materials, atmosphere, hfrt, haze, shadows, night, clouds, texsharp, temporal | `LitConst` and the unit table are fixed in A2 (3.3, 3.4). `UploadLitFrameConstants` is split into per-feature `UploadLit<Feature>()` calls made from it; `BindLightingTextures` binds from the unit table; all partial uploads go through A4's mirror helper. |
| `LightRig.{hpp,cpp}` | F2, F4, F5, F9, every prototype's `LightingTuning` fields, shade's rig terms, night's moon key | Feature switches leave `LightingTuning` for `FeatureSet` (A1). Rig math changes only in B2 (`lowSunRig`) and B3 (`KeyLight`), in that order, each with re-pinned tests. |
| `SceneDraw.cpp` | F1, F6, perf, night | A4 first (cull before LOD selection and sort; light gate via `RunEntry`). B3 then adds the moon shadow-pass condition in `DrawObjectsAndShadowsPass2`. |
| `GameStateExtTestRender.cpp` | F1 and every prototype's knobs | A1 makes the knob table data; no later step edits `ApplyLightingItem`. |
| `Object.cpp` (`DrawDecal`), `Lights.cpp` | combat, night | One `ParticleLight` in A6's module, written in B3 or B4 (whichever lands first) and reused. |
| `ShapeDraw.cpp` | F8, shade, materials | F8 owns late buffers; B7 adds only the object-kind field on the draw descriptor. |
| `LandscapeRender.cpp` | perf (marks), haze, clouds, water (`WaterFlags`) | A4 marks; A7 sky hook in `DrawSky`/`DrawClouds`; B5 water routing; haze's backdrop capture is not ported. |
| `TerrainLighting.cpp` | F1, hfrt, haze (mist floor) | B8 and B9 both add builds to the async horizon job; B8 lands its mist floor first, B9 the canopy; both report pending to `triSettleScene`. |
| `EngineGL33_State.cpp`, `GLDepthStencilState.hpp`, `EngineGL33_Queue.cpp`, `EngineGL33_VertexBuffer.cpp` | ssr, materials, texsharp | B6 owns stencil tagging; B7 adds the class field; B11 does not port its sampler changes. |
| Texture unit 6 | combat, atmosphere, hfrt, shadows, temporal, clouds, haze | Fixed by 3.4. |
| LitConstants slot 58 | combat, materials, atmosphere, shadows, clouds, temporal | Fixed by 3.3. |
| `test_remaster_lighting.cpp`, `test_tri_capture_verbs.cpp` | every step | Rewritten once in A1/A2 to read the registry and the accessors; later steps add cases only. |

---

## 7. Capture plan for the comparison video

### 7.1 Settings for every pass

- 1920x1080 windowed (`extra_args = ["--no-strict", "--display-mode", "windowed", "--width",
  "1920", "--height", "1080"]`); the first statement logs `triGetWindowSize` and the generator
  falls back to 1600x900 if the hidden desktop cannot hold 1080p (A0 observed 1920x1080 on the
  hidden desktop, so the fallback was not needed there).
- `triCaptureAtPresent true`, `triSetMsaa 4`, `triSetRenderScale 1.0` (the 3060 target),
  `triPinObjectDetail 1`, `triSetViewDistance <vd>`, `triSettleScene [300, 30, 2]` before each
  pass, `showCinemaBorder false`, `enableRadio false`, the capture camera from `capture_lib.sqf`
  (cutscene camera for the HUD, `triSetView` for the exact eye).
- Off and on are `triSetLighting "remastered,<keys>=0"` and `triSetLighting "remastered"`; the
  log line per frame carries `triFeatureState` and every 10th frame `triGpuTimes true`.
- Output 30 fps. One pass of 8 s is 240 frames.
- Run through `run-captures.ps1 -GameDir dist/graphics-remaster-rwdi -DataJunction
  RemasterUD-gr-video -Tests <generated test>` (toml `timeout = 3600`, `assert_timeout = 180`,
  tags `lighting_capture`, `headful`, `exclusive`; the Sinai test mounts `@LoBo` as
  `sinai_lobo_lighting.test.toml` does).

### 7.2 Driving the camera and the clock

`triScreenshot` must be a top-level statement with a string-literal label (`capture_lib.sqf`
header), so `video/gen_video_tests.py` writes every frame out, as `shadows/gen_motion.py` did.
New `video/capture_motion.sqf` (included after `capture_lib.sqf`):

- `[k, n] call capPathAt`: sets `capOX`, `capOZ`, `capEye`, `capH`, `capP` from the path's
  start and end keys with a smoothstep ease (or an orbit: centre, radius, start and end angle),
  then `capPinView`. A path flag `capEyeAbs` keeps the eye at a fixed ASL height (sea views)
  instead of the ground probe.
- `[k] call capPropsAt`: teleports moving props (`setPos`, `setDir`) along their own key path.
- `[k] call capClockAt`: `triSetSimTime (capSimT + k / 30)` and, for time-lapse paths,
  `setDate` with the minutes for frame k.

**S mode (sequential passes)** for deterministic scenes and every temporal feature: run the
whole path with the feature off, then again with it on. Per frame: `capClockAt`, `capPathAt`,
`capPropsAt`, `triSimFrames 2`, `triScreenshot "<feature>__off__f0001"`. AI props stay captive
with `disableAI "MOVE"`; vehicles move only by teleport. Water, sky drift and cloud shadows
follow the pinned clock, so both passes match; smoke and fire particles do not, which the video
accepts.

**P mode (paired, paused)** for non-deterministic scenes (combat, rain): `triSetFixedStep
0.0333` and per frame: `triPauseGame`, `capPathAt`, `triSetLighting "remastered,<keys>=0"`,
`triSimFrames 6` (warm-up for mask accumulation), `triScreenshot "<feature>__off__f0001"`,
`triSetLighting "remastered"`, `triSimFrames 6`, `triScreenshot "<feature>__on__f0001"`,
`triUnpauseGame`, `triSimFrames 1` (one fixed step). Both images of a pair share the exact
simulation state. Requires two A1/B4 properties: `triSimFrames` renders the world while
`triPauseGame` holds the simulation (verify in A1 with two paused renders of a burning fire
being pixel-identical; if not, add a render-only `triRenderFrames` verb), and combat lights
age by simulation time and exist whatever the switch says (B4). P mode is not used for TAA
(static warm-up hides its motion behaviour).

### 7.3 Paths

Coordinates are `[east, north]` map metres, eye in metres above ground, heading 0 = north
clockwise. All views below come from the capture README, the research test generators or the
tracks' notes.

| Feature (keys off) | World | Start view | End view | Frames | Date, time | Weather, VD | Props and staging | Mode |
|---|---|---|---|---|---|---|---|---|
| perf (`drawGrouping`, `hillCull`) | Abel | 4400, 3300, eye 1.75, h 60, p +2 (`sunrise_ridge`) | 4426, 3315, h 75 | 240 | 1985-07-05 05:08 | clear, VD 4000 | none; panels show `draws`, `objects`, CPU frame ms and a difference heat strip (pixel-exact) | S |
| temporal (`taa`) | Abel | 4674, 10156, eye 1.75, h 75, p +3 (`forest_edge_golden`) | strafe 5.4 m right, h 80 | 180 | 1985-07-05 18:15 | clear, VD 1500 | SoldierWB at 12 m facing the sun; a Jeep teleported 0.12 m per frame across at 25 m, then 0.5 m per frame for 60 frames | S |
| temporal, second clip | Sinai + @LoBo | 8350, 2950, eye 1.75, h 338.8, p -3 (`sinai_lowsun_fp`) | yaw pan 0.1 deg per frame | 180 | 2005-07-21 16:10 | clear, VD 2000 | `LoBoHMWV_Mag_Patrol`, M113 Zelda, Golani rifleman, `LoBo_camnet` (antennas, net) | S |
| light list (`lightTiles`) | Abel | 7152, 8915, eye 1.75, h 345, p +3 (`town_night`) | 7144, 8944 (30 m forward) | 240 | 1985-02-27 22:00 | clear, VD 800 | 6 street lamps, `Fire` at 9 m, two Jeeps with SAFE AI drivers and headlights teleported 0.3 m per frame along the street | S |
| sky table (`atmosphere`) | Abel | 1550, 5150, eye 3, h 250, p -4 (`coastline_sunset`) | h 310, p +6 | 240 | 1985-07-05 18:35 | clear, VD 4000 | none | S |
| sky table, time-lapse | Abel | 4400, 3300, eye 1.75, h 60, p +4 | same | 300 | 1985-07-05 04:40 to 05:40 (12 s game time per frame by `setDate`) | clear, VD 4000 | none | S |
| shadows (`treeShadows`, `softShadows`) | Abel | 6940, 9964, eye 1.75, h 240, p -2 (`woods_pine2_evening`) | 20 m along 240, h 225 | 240 | 1985-07-05 17:30 | clear, VD 1500 | Jeep at 15 m | S |
| shadows, time-lapse | Abel | 4674, 10156, eye 1.75, h 75, p +3 | same | 240 | 1985-07-05 17:30 to 18:45 | clear, VD 1500 | SoldierWB at 12 m | S |
| shade (`lowSunRig`, `crownCanopy`) | Abel | orbit at 12 m radius around the M113 placed 16 m ahead of 7300, 10900 (`lowsun_fp`), angle 0 | angle 120 | 240 | 1985-03-01 08:00 | clear | Jeep 10 m, M113 16 m, SoldierWB 7 m, tent 22 m (the `lowsun_fp` set) | S |
| shade, second clip | Sinai + @LoBo | orbit around the Zelda at the `sinai_lowsun_fp` camp, angle 0 | angle 90 | 180 | 2005-07-21 16:10 | clear, VD 2000 | the `sinai_lowsun_fp` set | S |
| night (`nightLights`, `moonShadows`, `stars`) | Abel | 6895.2, 8836.6, eye 1.5, h 69.6, p -2 (`road_flat_drive_moonless`) | 112 m along 69.6, camera 4 m behind the Jeep | 280 | 1985-03-17 23:00 (moonless) | clear, VD 800 | Jeep with a SAFE AI driver, headlights on, teleported 0.4 m per frame on the probed flat road | S |
| night, moon clip | Abel | 6891.9, 8835.4, eye 3, h 69.6, p -6 (`road_flat_head`) | crane to eye 8, then tilt to p +25 for the stars | 240 | 1985-03-01 20:00 (full moon at 36 deg) | clear, VD 800 | parked Jeep with headlights | S |
| combat (`combatLights`, `combatShadows`) | Abel | 7600, 10850, eye 5, h 0, p -12 (`ffh`) | 7600, 10860, eye 2, p -3 | 300 | 1985-07-12 23:00 (moonless) | clear, VD 1000 | `capFFStageN` (3 SoldierWB, 3 SoldierEB as captive fighters with `doFire`, burning Ural at [36, 13], `Fire` at [21, -12]); `Flare` at [30, -35] at frame 0; `Shell120` at [27, 1] at frame 60 and at [40, -6] at frame 180 (`capBoom`) | P |
| water (`seaWaves`) | Abel | 1550, 5150, eye 3 ASL, h 278, p -4 (`coastline_sunset`) | 1550, 5170, h 290 | 240 | 1985-07-05 18:35 | clear, VD 4000 | none; the sea clock runs 1/30 s per frame so animation-frame popping shows if present | S |
| water, noon clip | Abel | 1550, 5150, eye 12, h 278, p -12 (`sea_noon`) | strafe 15 m, p -8 | 180 | 1985-07-05 12:30 | clear | none (the noon colour check) | S |
| ssr (`ssr`) | Abel | 9240, 6950, eye 3 ASL, h 272, p -3 (`bay_offshore`) | 9210, 6951, h 262 | 240 | 1985-07-05 06:30 | clear, VD 3000 | `BoatW` (or `BoatE`, `Boat`) at [45, 10] facing 70 | S |
| ssr, wet street | Abel | 7162, 8931, eye 1.75, h 320 (`town_rain`) | 15 m forward | 180 | 1985-07-05 19:30 | overcast 0.9, rain 1, VD 800 | Jeep parked at 12 m | P |
| materials (`wetWeather`, `lampStreaks`) | Abel | 7152, 8915, eye 1.75, h 345, p +3 (`town_night_rain`) | 20 m forward | 240 | 1985-02-27 22:00 | overcast 0.9, rain 1, VD 800 | lamps on, `Fire` at [9, 3] inflamed | P |
| materials, after rain | Abel | orbit the `lowsun_fp` set as in shade | angle 120 | 180 | 1985-03-01 08:00 | clear, `wetness=1` in both passes, `wetWeather` toggled | `lowsun_fp` set | S |
| air (`airBeams`) | Abel | 6830, 9905, eye 1.75, h 72, p +6 (`pinewood_morn`) | 15 m along 72, h 80 | 240 | 1985-07-05 06:00 | fog 0.1, VD 1500 | none | S |
| air, gap beam | Abel | 4355, 3275, eye 1.75, h 288, p +5 (`pine_eve`) | strafe 10 m left | 180 | 1985-07-05 18:15 | clear, VD 1500 | none | S |
| air, headlights | Abel | 7266.4, 8355.9, eye 1.6, h 200, p -1 (`hl_front`) | static camera | 180 | 1985-02-27 22:00 | fog 0.2, VD 800 | Jeep with a SAFE AI driver teleported toward the camera along the road at 0.25 m per frame | S |
| air (`airHaze`, `valleyMist`) | Abel | 6100, 5000, eye 1.75, h 40, p -6 (valley) | h 60 | 300 | 1985-07-05 05:15 to 07:30 (27 s game time per frame) | clear, VD 4000 | none | S |
| hfrt (`canopyShadows`) | Abel | 5500, 6250, eye 15, h 185, p -14 (`hill_1730`) | 5440, 6200, eye 2.5, h 180, p -8 (`ground_b_1730`) | 300 | 1985-09-20 17:30 | clear, VD 2000 | none | S |
| hfrt, time-lapse | Abel | 5500, 6250, eye 15, h 200, p -12 (`slope_tele_1730`) | same | 240 | 1985-09-20 16:30 to 18:00 | clear, VD 2000 | none | S |
| clouds (`litClouds`, `skyCloudShadows`) | Abel | 1550, 5150, eye 3, h 260, p +14 (`coast_oc_up`) | h 300, p -4 | 240 | 1985-07-05 18:35 | overcast 0.5, VD 4000 | none | S |
| clouds, shadows clip | Eden | 9200, 3700, eye 250, h 288, p -12 (`broken_high`) | h 300 | 240 | 1985-07-05 14:30, sky drift stepped 0 to 15 deg over the clip | overcast 0.5, VD 5000 | none | S |
| texsharp (`terrainDetail`, `sharpen`) | Abel | 4674, 10156, eye 1.75, h 75, p 0 | 60 m along 75 (0.25 m per frame) | 240 | 1985-07-05 18:15 | clear, VD 1500 | none | S |
| texsharp, meadow | Eden | 9200, 3700, eye 1.75, h 288, p -4 (`overcast_broken`) | 30 m along 288 | 180 | 1985-07-05 14:30 | overcast 0.5, VD 3500 | Jeep at 12 m | S |

Feature A/B hero stills for the gate (section 8) use the start view of the first row of each
feature.

### 7.4 Assembly

`video/make_video.py <run folder>`: collects `<seq>_<feature>__off__f####.png` and `__on__`
from the tri tree (deleting the BMP twins as `make_pairs.py` does), checks both passes have every
frame, writes `tmp/video/<feature>/<feature>__sbs.mp4` (two 1920x1080 panels side by side,
3840x1080, labels "<feature> off" and "on", optional GPU ms from the frame log) and
`<feature>__wipe.mp4` (1920x1080, left half off, right half on, divider line) with
`C:\ffmpeg\bin\ffmpeg.exe` and Pillow (both present on this machine: ffmpeg on PATH, Pillow
12.1.0). A final `montage.mp4` concatenates the features in section 4 order with a title card
per feature naming the switch.

---

## 8. Verification gate per feature

Every step from A1 on passes all of these before its commit:

1. **Build**: the toolchain block of section 1, `cmake --build build/graphics-remaster-rwdi -j 6`,
   exit 0, no new warnings in touched files.
2. **Format**: `clang-format --dry-run --Werror` on every touched C++ file (whole-file targets
   fail on pre-existing tab-style headers such as `GraphicsConfig.hpp`; check touched lines
   there with `clang-format-diff.py`).
3. **Unit tests** from `D:\`: `PoseidonTests.exe "[remaster],[Shaders],[tri],[triSetLighting],[GL33],[gpu],[ShadowSampler],[DebugGroup],[ShutdownOrder],[UBORestoreGuard],[StateOwnership],[StateCacheAudit],[BufferUsageAudit],[IboBindAudit],[Conventions],[Settings],[GraphicsConfig],[GraphicsApply],[scene],[Shape]"`
   plus the step's new tags; the new cases must fail with the feature's code reverted where they
   test a contract (registry, layout, gates, math).
4. **Smoke**: run-captures with `smoke_abel`, `smoke_eden`, `smoke_sinai_lobo`; the game logs show
   no shader compile or link error, no KHR_debug warning (131204, 131222 or new ids) and
   `triGetGLErrorCount` 0.
5. **Off equals baseline**: `features_ab.test.sqf` with `<keys>=0` on the new build against
   `remastered` on the previous step's build at the hero view: within the same-config repeat
   noise (`banddiff.py`, per band). This is what makes the video's "off" honest. A0 measured that
   noise between two boots of one build (5 A0): the Sinai ground band, the crew views and the
   fire and smoke views are not repeatable between boots, so a pixel-exact claim (A2, A3, A4) is
   judged on the repeatable views or on off and on shot in the same game.
6. **Capture pair**: the hero view off and on, plus the full three-island capture set with
   everything on, through run-captures.
7. **Timing**: on a quiet GPU (no other slot holders in `tri.log`), 1080p, scale 1.0, 4x MSAA:
   the step's GPU section (median and minimum over 3 interleaved rounds) against its row in 3.2,
   and the gate states logged where the gate should be off (noon for low-sun features, dry for
   wet ones) showing zero cost there. Log the GPU clock with every shot (A0's `gpu_sampler.ps1`,
   in the repo from A1 on): the 4060 Ti picks its clock from the load (A0 saw P0 at 2.79 GHz on
   the long views and mostly P3 or P5, with medians of 0.5 to 1.0 GHz, on sky and sea views, in
   the 3 s up to each shot), so a light view's milliseconds are only comparable at a matching
   clock. Judge a feature's cost as on minus off in the same view and round, and compare with
   the A0 table only where both runs read P0.
8. **Look check** (open the images): against the track's `BEST_*` image and the step's critic
   list in section 5, and against the owner's standing list (no blue haze or frame-wide wash,
   not only darker, evenings at least as warm as Classic, no flattened crowns, no mustard
   ground). Any item that fails goes into the commit message as a known risk or blocks the commit.
9. **Commit**: one commit per step (more only if a step has a separately revertable part, as F9
   and F9b were), message in the branch style, ending with the trailer, plus the roadmap line.

Commit subjects:

| Step | Subject |
|---|---|
| A1 | `graphics: every lighting feature has a switch, a gate and an A/B capture` |
| A2 | `graphics: remastered shaders split by concern, one LitConstants block` |
| A3 | `graphics: the post pass runs as a list of stages` |
| A4 | `graphics: long views draw the same frame from a fraction of the draw calls` |
| A5 | `graphics: temporal anti-aliasing steadies foliage and cleans cheap soft effects` |
| A6 | `graphics: one tiled light list feeds every local light` |
| A7 | `graphics: a physically based sky table lights the dome and the fog colour` |
| B1 | `graphics: trees and buildings cast soft, contact-hardening sun shadows` |
| B2 | `graphics: low-sun vehicles keep Classic's evening hue and readable shaded sides` |
| B3 | `graphics: low-beam headlights, moon shadows and stars at night` |
| B4 | `graphics: fires, flares and shell bursts light the men and cast shadows` |
| B5 | `graphics: the sea gets wave relief and a broken, glittering sun path` |
| B6 | `graphics: calm water and wet streets reflect what is on screen` |
| B7 | `graphics: rain wets cloth and stone, fills puddles and streaks lamp light` |
| B8 | `graphics: sun beams, headlight cones, aerial haze and morning valley mist` |
| B9 | `graphics: every tree casts a long golden-hour shadow at any distance` |
| B10 | `graphics: sky clouds glow on their sun side and their shadows match the sky` |
| B11 | `graphics: terrain slopes show grass and rock grain instead of a smear` |

Body, in the style of the F-commits: what the player sees; how it works; the switch and its
gates; what was kept, rewritten and dropped from the prototype; measured cost (quiet GPU, which
views); tests (which fail without the change); in game (capture folders); known risks; trailer
`Co-Authored-By: Claude Opus 5.5 <noreply@anthropic.com>`.

Per-step extras for the look check:

| Step | Hero view | Must see | Must not see |
|---|---|---|---|
| A4 | `sunrise_ridge` walking | same image, draws about 1,700 instead of 11,000 | any far pixel over 2 levels outside the sky band |
| A5 | `forest_edge_golden` strafe | steadier crowns, soft-ended soldier shadow | blur over 10% sharpness loss, ghost trail behind the Jeep |
| A7 | `coastline_sunset`, `firefight_dusk` | aureole, golden horizon band, orange twilight | olive band, grey dawn, erased noon cirrus |
| B1 | `woods_pine2_evening` | long tree shadows, dappled floor | woods more than 10% darker than the baseline after B2, Sinai foreground slab |
| B2 | `lowsun_fp`, `sinai_lowsun_fp` | hull shade/lit 0.58 to 0.62 | greener or greyer lit faces than Classic |
| B3 | `road_flat_drive_moonless` | dark starry night, warm low beam, hard top edge | mustard verges, black crowns by lamps, floodlit Jeep rear |
| B4 | `ffh` flash, `flr` | long radial shadows, lit men | centre seam, shadow flipping between fires, flash frame darker than Classic by more than 10 levels |
| B5 | `coastline_sunset`, `sea_noon` | wave rows, broken sun path | popping every 0.14 s, noon hue more than 10 deg off Classic |
| B6 | `bay_offshore` | boat and hill mirrored with ripples | edge seams, boxes under bushes, pale lilac sunset sea |
| B7 | `town_night_rain` | lamp streaks, puddles mirroring houses | white sheets, triangle puddle edges, stripes on hulls |
| B8 | `pinewood_morn`, `hl_front`, valley 05:15 | beams through crowns, headlight cones, pink valley mist | horizon glow band in clear weather, brown night sky, crosshatch |
| B9 | `hill_1730`, `slope_tele_1730` | every tree's long shadow | soft slabs near the camera, crawling dither, cost over 3 ms |
| B10 | `coast_oc_up`, `broken_high` | peach-lit cloud undersides, shadows matching the sky | embossed midday clouds, colour fringes |
| B11 | `forest_edge_golden` walk | grass and rock grain at 10 to 150 m | blotches on paths, shimmer in motion |

---

## 9. Owner decisions and open risks

- **Branch**: settled. `graphics-remaster` exists and was cut at 026133b, after F10 and F11
  landed in narrowed form (5 A0).
- **Defaults**: every feature switch defaults to on, gated (the new lighting replaces the
  original look). Confirm `atmoHue` 0.5 / `atmoSat` 1.15, the noon spawned-vehicle side level
  (0.60, or `bounce=2.5`), whether golden crowns may be darker than the tip, the moonless night
  brightness, warm lamp colour (now the authored 0.9, 0.8, 0.6), and texsharp's sharpening.
- **Gameplay**: valley mist and haze are visual only; AI sight ignoring them is unfair to the
  player until `AirMedium` feeds AI visibility (follow-up issue, `gh -R GearUnclear/CWR`).
- **Not measured anywhere yet**: cost at 1080p on an RTX 3060 (no 3060 on hand; A0 measured the
  RTX 4060 Ti at 1080p on a quiet machine, 5 A0), motion behaviour of most features (the video
  is the first real test), TAA ghosting on fast vehicles and smoke, water popping, the hill
  cull's edge cases, crew-view interiors (F8 left late-loaded interiors on the CPU path).
- **Out of scope here** (MENU tracks not in the worked list): gbuffer, foliage, tiling, normals,
  aa beyond F11, exposure (its eye adaptation is a candidate for the night policy), ssgi,
  wildcard, and MENU phase 4 (the bounce-light bet).
