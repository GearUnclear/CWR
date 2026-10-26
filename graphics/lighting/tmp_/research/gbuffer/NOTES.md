# Track gbuffer: G-buffer spike (normals and material targets)

Research only. Scratch worktree `D:\Arma_CWA_worktrees\gfx-gbuffer` (detached at 3c53141, the
graphics branch tip), build `dist/gfx-gbuffer-rwdi`. Nothing committed, no branch made. This run
resumed an earlier agent's work: its build, its run01 to run05 captures and its scripts were reused;
runs 06 to 13, the GI v2 shader, the late-VBO checks and every note below are from this session.

## Verdict in five lines

1. **The plumbing works on real data.** Three extra MSAA colour targets are written by the
   Remastered lit programs during the world pass and resolved for the post pass: world normal +
   roughness, non-sky radiance (sun, moon, local lights, emissive) + fog, ambient response
   (albedo x material ambient) + sky share. Debug views on six real scenes show clean normals for
   terrain, buildings, soldiers, vehicles, and the ellipsoid crown normals of trees and the LoBo camo
   net (`BEST_forest_edge_golden.png`, `run05_main/dbg_*.png`).
2. **The consumer I built on it (screen-space one-bounce GI + ray sky visibility) is physically
   right and visually subtle.** At strength 1 only about 1 to 4.5% of pixels move by more than 4 display
   levels; even the strongest variant (x3, whole-frame source, 15 m rays, bounce only) only lifts
   shaded vehicle and soldier faces by roughly 5 to 10 levels (`BEST_town_day_crop.png`). It does not
   make CWA "look like a different game". Honest result: on open outdoor CWA scenes the rig's
   analytic sky + ground bounce already carries most of the indirect light, and dark military albedo
   (about 0.16 linear) eats most of one more bounce.
3. **The most consequential finding is a stock bug the G-buffer exposed:** every shape first
   loaded after mission load (anything `createVehicle`/`createUnit` brings in that the mission did
   not already contain: all capture props, Guerrilla traffic, garrisons, spawns) has no vertex
   buffer, falls back to software T&L and draws pre-lit through `VSScreen`. It skips the Remastered
   shader entirely: no per-pixel light, no shadow-map receive, no AO share, no G-buffer.
   Confirmed on the shared stock build `dist/lighting-rwdi` (`BEST_lowsun_shadow.png`, top left).
   Every vehicle in the 13-view baseline captures is on this path (the capture missions contain only the
   player's `SoldierWB`; the committed tests spawn 15 vehicles, and LoBo infantry on Sinai too), so vehicle judgements made from
   those captures (crushed shaded faces, hull warmth) describe the fallback, not what a
   mission-placed vehicle looks like.
4. **GPU cost:** the GI pass is 1.6 to 3.1 ms at half resolution (the stock AO it replaces is 0.17 to
   0.6 ms); the G-buffer resolve is a few tenths of a ms to about 1 ms; the MRT writes themselves
   could not be resolved on the shared GPU (bandwidth estimate about 1 to 2 ms). Total roughly +3 to
   +5 ms at 1440x960 4x MSAA. See `timings.md`.
5. **Self-verdict: mixed.** Infrastructure: worth building (SSR, SSGI, contact shadows and
   volumetrics all want it, and the late-VBO fix is needed regardless). The GI consumer as a look:
   failed to deliver a visible RTX-style change at sane strengths.

## What exists and how it works

### MRT layout (prototype)

| attachment | format | rgb | a |
|---|---|---|---|
| 0 | RGBA8 (existing) | lit colour | 1 minus sky share (stock AO contract) |
| 1 | RGBA16F | world-space shading normal (crown normal for foliage) | roughness from specular power (1 = matte) |
| 2 | RGBA16F | non-sky radiance, linear, exposed: `a * (direct + moon + local + emissive) * exposure` | fog transmittance |
| 3 | RGBA8 | ambient response `a * ka * 0.5`, display encoded | 0.5 + 0.5 x (1 minus sky share x fog), 0 = nothing lit drawn |

Water writes a flat up normal with roughness 0.05. Pre-lit draws (sky, clouds, particles, 2D) never
write. "sky/ambient radiance" is not stored: the debug view derives it as frame minus target 2.

### Where it plugs in (all in `prototype.patch`, 12 files, about 720 lines)

- **FBO** (`EngineGL33_VertexBuffer.cpp`, `ApplyPendingRenderScale`): three more multisampled
  renderbuffers attached to the SSAA frame target `_ssaaFbo` as `COLOR_ATTACHMENT1..3`. The FBO's
  draw buffers stay `{COLOR_ATTACHMENT0}` (the GL default) except inside the world pass, so nothing
  else (HUD, dialogs, portraits, Classic) ever writes them. Destroyed with the SSAA target.
- **Begin/end** (`EngineGL33_PostFX.cpp` `BeginGbuffer`/`EndGbuffer`, `EngineGL33_Queue.cpp`
  `BeginPass`, `EngineGL33_Lifecycle.cpp` `InitDraw`): when the world pass of a Remastered frame
  opens and the `gbuffer` knob is on, switch to four draw buffers and clear 1..3; `EndWorldScene`
  flushes the queues and switches back; `InitDraw` ends it defensively for a world pass that never
  reached `EndWorldScene`.
- **Write gating per draw** (`UpdateGbufMask`, called from `DoSelectPixelShader`,
  `ApplyBlendMode` and `BeginPass`): `glColorMaski` on attachments 1..3. A draw may write when the
  bound program is a Remastered lit program (`PSNormal`, `PSDetail`, `PSGrass`, `PSWater` with a
  Remastered variant) and it is opaque, or alpha-blended through `VSTransform` (vehicle and window
  textures carry alpha and draw blended). Blending is switched off on 1..3 with `glDisablei`, so the
  G-buffer always holds the last writer's values. `BeginPass` calls `glColorMask`, which resets every
  draw buffer's mask, so the gate is re-applied there.
- **Shader outputs** (`EngineGL33_LightingShaders.cpp`): `layout(location = 1..3)` outputs in the
  Remastered fragment shaders only; `shadeRemastered` fills three globals and `finishFragment` writes
  them (zeros for pre-lit fragments). Classic programs are untouched.
- **Resolve** (`EndWorldScene`): one `glBlitFramebuffer` per attachment from the multisampled frame
  target into single-sample textures of the same format (sample average). Depth comes from the
  existing depth resolve.
- **Consumers** (`EndWorldScene`): GI pass, temporal accumulation, two-pass bilateral blur, debug
  views 5 to 11, and composite changes. Post shaders are overridable by name (`psPostGi.glsl`).
- **Knobs** (`triSetLighting`): `gbuffer`, `gi` (bounce strength), `giRadius`, `giSteps`, `giDirs`,
  `giAo` (use the GI rays' occlusion instead of the stock AO), `giTemporal`, `giSource` (0 = target 2,
  1 = the whole lit frame, a cheap multi-bounce), `gbufMaskDebug`, `lateVbo`, `debug=5..11`.
- **Late vertex buffers** (`ShapeDraw.cpp`, `Shape.hpp`): `ShapeBank::New` calls
  `OptimizeRendering` for shapes created after `Shapes.OptimizeAll`, flags them, and `Shape::Draw`
  uses their buffer only when `lateVbo=1`, so captures can A/B it.

### The GI consumer

Half resolution. Per pixel: 8 cosine-weighted directions around the G-buffer normal, 14 (or 20)
quadratic steps through the resolved depth, radius 6 (or 15) m. A ray that hits a surface facing it
(the hit's own G-buffer normal) gathers the hit's non-sky radiance (target 2) and counts as sky
occlusion; a ray that leaves the screen or the radius counts as open sky, so the far field keeps the
rig's analytic hemisphere and ground bounce. Output rgb = bounce irradiance, a = sky visibility.
Then an exponential temporal average while the camera holds still (reset on any camera move, no
reprojection), then a depth- and normal-aware 13-tap blur in x and y. The composite adds
`albedo x ka x bounce x strength` (target 3 gives the receiver's response) and, with `giAo`, takes
the occluded part of the sky share from the GI rays instead of the stock AO.

**v1 to v2** (`override_v1_ref/psPostGi.glsl` is the built-in v1, `override_v2/psPostGi.glsl` the
fix, used for runs 06, 07, 10, 11, 12): v1 painted a hard-edged black wedge of false occlusion on
near terrain (`run06_v2/wedge_forest_edge_golden.png`, left). Cause: CWA terrain is a coarse grid
whose interpolated shading normal leans off the triangle, so rays around the shading normal dived
into the facet. v2 rebuilds the geometric normal from depth, offsets the ray origin along it and
bends any direction below it back above it. The wedge is gone in both the visibility and the bounce
targets.

## Variants and what the real data showed

Change against the Remastered + late VBO frame of the same run (display levels, mean of RGB):

| variant | lowsun_fp | town_day | forest_edge_golden | sinai_lowsun_fp |
|---|---|---|---|---|
| v2 GI x1, r 6, GI occlusion on (run06) | mean -0.34, 1.6% px beyond 4 | -0.46, 3.2% | -0.04, 0.7% | -0.60, 3.6% |
| v2 x2 (run06) | -0.28, 1.7% | -0.39, 2.9% | +0.08, 1.6% | -0.53, 3.1% |
| v2 x3 whole-frame source (run06) | -0.04, 2.8%, p99 +5.7 | -0.14, 2.7%, p99 +4.3 | +0.29, 3.1%, p99 +9.0 | -0.07, 3.4%, p99 +5.7 |
| r 15, x2, bounce only (run07) | +0.02, 3.4% | +0.24, 0.6% | not shot | +0.14, 0.8% |
| r 15, x3 whole-frame, bounce only (run07) | +0.30, 5.9%, p99 +9.7 | +0.68, 4.9%, p99 +6.7 | not shot | +0.65, 5.6%, p99 +11.0 |

- Variants that keep the GI occlusion (`giAo=1`) come out slightly **darker** on average, which is
  on the owner's rejected list ("only darker"). The bounce-only variants are the ones to judge.
- Why so small, measured on the town_day Jeep's shaded rear (`v2dbg_bounce`, `dbg_albedo`,
  `dbg_direct`): sunlit grass carries about 0.11 linear of non-sky radiance; the bounce irradiance
  reaching the Jeep's rear is about 0.01 (the ground right under and behind the Jeep is in its own
  shadow, near-horizontal rays leave the 6 m radius, rays toward the camera leave the screen); the
  Jeep's response `a x ka` is about 0.16, so one bounce adds about 0.0016 linear to a surface at
  about 0.05. With r 15 m the irradiance roughly doubles; the result is still a few levels.
- **Single frame vs accumulated** (`run06_v2/noise_*.png`): with the blur, one frame of 8 rays
  already looks clean in the final image, so a moving camera would not show crawling noise at
  this strength. At much higher strengths it would.
- **Night** (`run10_night`, `run12_fire`): no visible GI gain at night. Two traps found: (a) the
  G-buffer radiance is written before the classic night-eye darkening and fog in `finishFragment`,
  so at night target 2 is far brighter than the frame it belongs to (`town_night__night_novbo_dbg_direct`
  display luma 99 vs 25 for the frame, about 13x in linear); a production G-buffer must store radiance in the frame's own units. (b) The
  late-VBO hack broke night: with `lateVbo=1` the whole town renders as if lit by day
  (`run10_night/cmp_town_night.png`, panels 3 and 4). The same fire placed by the mission (normal
  vertex buffer) does not do this (`run12_fire/fire_sheet.png`), so it is a bug of the hack (the
  fire pit `ohniste.p3d` is the late shape in view), not of the stock engine. The night images with
  `lateVbo=1` are invalid as look evidence.

## The late-vertex-buffer finding in detail

- Code: `LODShape::OptimizeRendering` (the only place `_buffer` is built) runs only from
  `Shapes.OptimizeAll()` at world/mission load (`WorldInit.cpp`, `WorldImpl.cpp`, `Landscape.cpp`).
  `Shape::Draw` takes the hardware T&L path only `if (... && _buffer)`. The Remastered vertex shader
  for `VSScreen` sets `vLitMode = 0.0`, so those fragments pass the CPU's vertex colour through.
- Evidence: game log lines `late shape vertex buffers: data3d\jeep.p3d (13 LODs)`,
  `data3d\m113.p3d (14 LODs)` and others when the prototype builds them; the first G-buffer
  captures had the Jeep, tent and M113 missing from every target even with every draw forced to
  write (`run04_diag/diag_grid.png`); with debug view 1 (sun visibility) the stock build shows the
  spawned M113 and Jeeps textured, i.e. never reaching the lit shader (`run09_stock`, the shared
  `dist/lighting-rwdi` exe, no prototype code). The same classes preloaded by a mission
  (`tests_snapshot/missions/udcap_abelpre.Abel`) and the late-VBO prototype render the same
  (`lowsun_shadow`: 0.42% of pixels differ by more than 4 levels, max 6; `lowsun_fp`: 0.08%).
- Visual size: on spawned vehicles the change is moderate (0.1 to 1.5% of the frame moves by more
  than 10 levels, the vehicles): the pre-lit vehicles are lighter and flatter, the per-pixel ones darker on faces away
  from the sun, with real shadow-map shadows on them (`crops/scratch_lateVbo_town.png`,
  `crops/scratch_lateVbo_sinai.png`).
- Side check that turned out fine: the G-buffer direct target is nearly black on vehicle faces at low
  sun. Toggling `sun=0` and `sky=0` on preloaded vehicles (`run13_sun`) shows they still lose most of
  their light without the sun, so most of a vehicle's sun-driven light arrives through the rig's
  ground-bounce term (part of the ambient), not the direct term. That makes the rig's `bounce` the
  main lever on shaded hull brightness and warmth (a note for the shade and exposure tracks).

## What a production version needs

1. **Late vertex buffers, independent of the G-buffer.** Build buffers for shapes loaded after
   `OptimizeAll`, after the shape is fully initialised (the prototype calls `OptimizeRendering`
   inside `ShapeBank::New`, which broke the lit fire pit at night). Gate the draw-time switch on the
   Remastered programs so Classic keeps its software path and stays bit-identical. Check the spawn
   hitch: `OptimizeRendering` on a 13-LOD vehicle runs at `createVehicle` time. Then re-shoot every
   vehicle judgement in the capture set on this path (or preload the prop classes in the capture
   missions).
2. **Targets.** Pack them: octahedral normal + roughness in RG16 + a byte, radiance in R11G11B10F,
   response in RGBA8 (or fold response into the radiance target's spare bits), so 8 to 12 bytes per
   sample instead of 20. Use `GL_TEXTURE_2D_MULTISAMPLE` attachments instead of renderbuffers so
   consumers can `texelFetch` sample 0 (or the sample that matches the resolved depth) and the
   three resolve blits go away; averaged normals at silhouettes are wrong for SSR.
3. **Units.** Store radiance after night-eye and fog (or store the factors), so a bounce or reflection
   pass re-emits what the player sees.
4. **Blended draws.** Unblended G-buffer writes from alpha-blended vehicle and window draws let a
   transparent texel overwrite the surface behind it. Enable per-attachment blending
   (`glEnablei` + `glBlendFunci(i, SRC_ALPHA, ONE_MINUS_SRC_ALPHA)`) on 1..3 for those draws so the
   G-buffer follows coverage, or write only for alpha above 0.5 through a separate opaque pass.
   Alpha-to-coverage cutouts (foliage under MSAA) already write only their covered samples, which is
   what a multisampled G-buffer wants. Some birch leaf cards in town_day leave holes in the normal
   target (not investigated: probably drawn on a path the mask excludes).
5. **Roughness is almost empty.** Authored specular power exists on glass and little else: terrain
   comes out mid grey and nearly every model 1.0 (matte). SSR on paint, wet ground or windows will
   need roughness reconstructed per material class or from albedo, not this channel alone.
6. **GI quality.** Reprojected temporal accumulation with history clamping instead of reset on any
   camera move; a far-field term for rays that leave the screen toward sunlit ground (today the
   rig's analytic bounce covers it); interleaved ray directions across 2x2 pixels to cut the pass to
   about 1 ms.
7. **Classic stays bit-identical** as long as the draw buffers are switched only for Remastered world
   passes and the late-VBO draw switch is Remastered-gated (both hold in the prototype when the knobs
   are 0, which is the default).

## Risks against the owner's taste list

- "Only darker": GI with its own occlusion nets slightly darker frames; only bounce-only variants
  avoid it.
- "Crushed shade": the late-VBO fix makes spawned vehicles' away-from-sun faces darker than in the
  baseline captures. That is the look mission-placed vehicles already have in the Remastered mode, so
  the shade judgement has to be made on this path, and the fix for crushed hulls belongs in the rig
  (ground bounce), not in keeping vehicles on the pre-lit fallback.
- "Olive / mustard": x3 bounce from grass tints shaded faces green-warm; mild at x3 in these views,
  stronger multipliers would push toward olive.
- No colour grading, haze or new art involved.

## Files

- `prototype.patch`: current diff of the scratch worktree (all C++ and built-in GLSL);
  `prototype_early_1424.patch` is the earlier agent's first snapshot (superseded).
- `override_v2/psPostGi.glsl` (GI v2, the wedge fix), `override_v1_ref/psPostGi.glsl` (built-in v1
  for reference).
- `BEST_town_day.png`, `BEST_town_day_crop.png`, `BEST_lowsun_fp.png`: Classic | Remastered |
  Remastered + late VBO + strongest GI variant, GPU ms per panel. `BEST_lowsun_shadow.png`: the
  late-VBO evidence. `BEST_forest_edge_golden.png`: G-buffer targets on real scenes.
- `timings.md`: cost summary; per-run `timings_gb.md`, `timing_rounds.md`.
- Runs (each has `tri/` with game logs and raw shots, plus collected PNGs and sheets):
  `run01_plumb` (first plumbing), `run02_main` (v1, no late VBO), `run03_diag`/`run04_diag`
  (missing vehicles diagnosis), `run05_main` (v1 + late VBO, 6 views, debug sheets),
  `run06_v2` (GI v2 strengths, noise, wedge, timing rounds), `run07_v3` (r 15 m, bounce only, 8
  timing rounds), `run08_shadow` (spawned vehicles in an M113's shadow), `run09_stock` (the same on
  the stock exe), `run10_night` (night, invalid with late VBO), `run11_pre` (vehicles preloaded by the
  mission), `run12_fire` (mission-placed fire vs spawned, night GI without late VBO), `run13_sun`
  (sun/sky toggles on preloaded vehicles).
- Scripts: `gen_tests.py` (earlier agent), `gen_tests_v2.py` (sets v2, timing, v3, shadow, stock,
  night, pre, fire, sun), `collect_gb.py`, `make_v2_sheets.py`, `make_v3.py`, `make_best.py`,
  `crop_cmp.py`, `crop_multi.py`. `tests_snapshot/` holds the research tests and the two research
  missions as run (they live git-excluded in the scratch worktree).
- `crops/`: working crops referenced above.
