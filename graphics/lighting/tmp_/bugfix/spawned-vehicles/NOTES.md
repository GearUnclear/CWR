# Bug fix: script-spawned vehicles skip the remastered lighting (BUGID spawned-vehicles)

Scratch worktree: `D:\Arma_CWA_worktrees\fix-spawned-vehicles` (detached at `claude/youthful-lovelace-8imbdh` = 3c53141, nothing committed). Build: `build/fix-spawned-vehicles-rwdi`, staged to `arma_CWR/dist/fix-spawned-vehicles-rwdi`. Patch: `fix.patch` in this folder (`git diff claude/youthful-lovelace-8imbdh`, the new test file included via `git add -N`; `git apply --check` passes on the branch tip).

## Summary

Shapes first loaded after a mission's `Shapes.OptimizeAll()` never got a vertex buffer, so `Shape::Draw` drew them through the CPU T&L path: classic per-vertex light baked into the vertices, which the remastered shaders pass through. No per-pixel light, no shadow-map receiving. Every `createVehicle` of a class the mission did not place hits this, and Guerrilla Mode spawns nearly every vehicle that way.

The fix queues every shape `ShapeBank::New` loads after `OptimizeAll` and gives it, once per drawn frame before any object draws, the same `LODShape::OptimizeRendering()` buffers a mission-loaded shape gets. Crew views (gunner, driver, commander, cargo interiors) are kept on the CPU path as the brief asked; see "Crew views" below for why, and for an inconsistency with mission-placed vehicles this exposed.

Measured in game: on Abel, a script-spawned Jeep, M113 and tent now render pixel-identical to the same classes placed in `mission.sqm` (mean difference 0.00 display levels, 0.00% of pixels off by more than 4), where before they differed. The CPU-path share of the frame below the horizon (magenta in a debug view) drops from 13.9% / 22.6% / 2.3% to 0% in `lowsun_fp` / `hull_close` / `noon_fields`, and from 17.0% / 21.3% to 0% on Sinai. The night town stays at night. The extra cost is 1.2 to 1.4 ms once, on the frame after a burst of 8 new vehicle classes.

## Root cause (branch tip 3c53141)

- `arma_CWR/engine/Poseidon/Graphics/Rendering/Shape/ShapeDraw.cpp:89`: `Shape::Draw` takes the hardware T&L path only `if (engine->GetTL() && EnableHWTLState && tlAble && _buffer)`. Otherwise it falls back to `_face.Draw(...)` (line 197), the CPU T&L path.
- `_buffer` is only ever built by `Shape::ConvertToVBuffer` (`ShapeDraw.cpp:282`), which for bank shapes is only called from `LODShape::OptimizeRendering` (`ShapeDraw.cpp:317-410`), which runs only from `ShapeBank::OptimizeAll` (`ShapeDraw.cpp:412-422`).
- `Shapes.OptimizeAll()` runs at mission load (`World/WorldInit.cpp:796`, `World/WorldImpl.cpp:2382`) and on a device restore (`World/Terrain/Landscape.cpp:2837`). `ShapeBank::New` (`ShapeDraw.cpp:203-280`) builds no buffer. So any shape first loaded later stays bufferless for the rest of the mission.
- On the CPU path the GL33 backend uses `VSScreen`, whose remastered vertex shader sets `vLitMode = 0.0` ("pre-lit colour: the fragment shader passes it through"), and `psNormalRemasterMain` then outputs `vColor * t0`: the classic light, no `shadeRemastered`, no shadow maps, no sky, no AO share.

## The fix

| File | Change |
|---|---|
| `Shape.hpp` (`ShapeBank`) | `OptimizeLate()`; private `_optimizedAll` (set by `OptimizeAll`) and `LinkArray<LODShapeWithShadow> _late` (weak links). `LODShape::OptimizeRendering(bool keepInsideViews = false)`. |
| `ShapeDraw.cpp:278` (`ShapeBank::New`) | After the shape is cached: if `_optimizedAll` and hardware T&L (or the SSE path) is on, `_late.Add(shape)`. No buffer is built here. |
| `ShapeDraw.cpp:428-441` (`OptimizeAll`) | Clears `_late` (every cached shape was just optimised) and sets `_optimizedAll`. |
| `ShapeDraw.cpp:443-482` (`OptimizeLate`) | For each queued shape still alive and without any buffer: `OptimizeRendering(true)`. Clears the queue. Logs `PERF: ShapeBank::OptimizeLate` at debug level when it takes 1 ms or more. |
| `ShapeDraw.cpp:341` (`OptimizeRendering`) | With `keepInsideViews`, levels at `VIEW_GUNNER`, `VIEW_PILOT`, `VIEW_CARGO`, `VIEW_COMMANDER` get no hardware buffer. The default (`false`) leaves `OptimizeAll` and `PortraitRenderer` exactly as before. |
| `ShapeDraw.cpp:488` (`ShapeBank::Clear`) | Also clears `_late`. |
| `World/World.cpp:1395` (`World::Simulate`) | `Shapes.OptimizeLate();` inside `if (enableDraw)`, right before `_scene.BeginObjects()`. Scripts run earlier in the same function (`SimulateScripts`, line ~901), so a vehicle created this frame gets its buffers before this frame draws it. Plus an explicit `#include` of `Shape.hpp`. |
| `Graphics/Remaster/README.md` | One paragraph after "Pre-lit draws": only buffered shapes take the per-fragment path, how late shapes get theirs, and that their crew views stay pre-lit. |
| `tests/unit/.../Graphics/Rendering/test_shape_late_vertex_buffers.cpp` (+ CMakeLists) | New Catch2 tests, below. |

## Why it is correct

- **Same decision as for a mission-placed shape.** The exclusions that matter (sky, stars, clouds and line-lit shapes by their `ClipLight` hints, `IsLight` and `OnSurface` levels, the special geometry levels, `VBDynamic` for animated shapes and tents) live in `OptimizeRendering`. The fix calls that same function, so a spawned shape gets the same levels and buffer types. The unit test checks the per-level layout against a fresh `OptimizeAll` (`{1, -1, 1}` both ways for `multi_lod_vehicle.p3d`), and the captures show pixel identity with placed vehicles on Abel.
- **After the loader has finished, not inside `New`.** Loaders set the flags `OptimizeRendering` reads after `New` returns: `Landscape.cpp:86-104` sets `ClipLightSky` and `SetSpecial` on the sky dome, `Smokes.cpp:911` sets cloudlet specials, vehicle types set animation and cockpit state in `InitShape`. The gbuffer track optimised inside `New` and its night town lit up like day; this fix defers to the next drawn frame, when those calls are done. The unit test sets `IsLight`, or sky-lit vertex hints the way `Landscape` does, after `New`, and checks that no buffer is built.
- **Per shape, not per drawn level.** The shade prototype built a buffer for any bufferless level at its first draw. That also catches shapes that are bufferless on purpose and never in the bank: the merged sun and moon halo copies (`Landscape.cpp:187`, `216`), shadow-cache splits, the tyre-track ribbon "rebuilt every frame" (`ShapeDraw.cpp:189`), and it needs pointer memory. Here only bank shapes are queued, and in `Link`s: a shape released before its first frame drops out, and a new shape at a reused address is a new entry (the normals track hit that reuse bug with a pointer set).
- **No double work.** `OptimizeAll` clears the queue; `OptimizeLate` skips a shape that already has any buffer (`ConvertToVBuffer` asserts on a second one).
- **Sky, sun and clouds.** The sky dome, stars and clouds are excluded by their light hints, decided after `Landscape` has set them. The sun and moon objects are private merged copies (`new LODShapeWithShadow(*haloShape)`), never in the bank, never queued. In the AFTER captures the sun disc and halo are present and the sky matches BEFORE.
- **UI.** 3D UI objects pass `ClipUser0`, which keeps them on the CPU path whatever their buffers (`UIControlsBase.cpp:454-457`). README: dialogs and portraits draw through the classic programs.
- **Classic mode.** A spawned vehicle now takes the GPU classic path a placed one already takes. Classic pixels barely move (lowsun_fp 0.06 mean, 0.26% of pixels off by more than 4; Jeep box luma 43.8 to 45.4), and spawned now equals placed in Classic too.

## Crew views (the gunner interior): a decision and a finding

The brief says to skip the gunner interior view. I measured what that means:

- In the shipped build a **mission-placed** M60's gunner interior is already on the per-pixel path (debug view: albedo, not magenta; `interior_m60_gunner.png`, column 3). The research notes assumed `OptimizeRendering` skips it; it does not.
- On that path the interior is much darker: frame luma 59.4 against 95.6 on the CPU path (centre box 62.4 against 103.3), at 10:00 with the sun at 59 degrees (`research/interior_luma_*.txt`). The hull shadows the sun and there is no interior light.
- I first built full parity (`runs/after_v1_full_parity`): the spawned interior then matched the placed one exactly (0.29 mean difference), but became 38% darker than today. That is the "only darker" look the owner rejects, and it would hit nearly every Guerrilla tank. So the final fix keeps the crew views of late shapes on the CPU path (`OptimizeRendering(true)`): the spawned interior is unchanged from today (debug: 88.69% magenta, 88.71% before; luma 95.6 both).
- Consequence: spawned and placed vehicles still differ inside (placed ones keep the darker per-pixel interior, as shipped). The proper fix is an interior light model (or a uniform crew-view rule) for both, which needs care because a soldier's own first-person view is also a `VIEW_PILOT` level. To switch to full parity instead, change `OptimizeRendering(true)` to `OptimizeRendering()` in `OptimizeLate`.

## The Zelda slat armour: grey to tan

Measured on `sinai_hull_close` (`research/zelda_slats_light.txt`, `crop_zelda_slats.png`), light = shot / albedo, albedo from debug view 3 on the per-pixel path:

| Slat face (in shade) | shot RGB | light b/r | light g/r |
|---|---|---|---|
| albedo (the texture) | 174 / 151 / 123 (tan) | | |
| BEFORE spawned, Remastered (CPU path) | 64.7 / 60.8 / 62.0 (grey) | 1.36 | 1.08 |
| BEFORE spawned, Classic | 60.6 / 56.7 / 58.6 (grey) | 1.37 | 1.08 |
| AFTER spawned = AFTER placed, Remastered | 74.6 / 64.1 / 53.5 (tan) | 1.02 | 0.99 |

The slat texture is tan. The slat side faces away from the sun, so on the CPU path it gets only the classic ambient, which is blue (blue 36% above red); tan times blue reads grey. The rear door, which faces the sun, gets the warm classic sun and stays tan on both paths. Per pixel, the shaded face gets near-neutral light (blue sky balanced by the warm sand bounce, `GroundTint`), so the paint shows its own colour. Nothing is dropped on the per-pixel path; the CPU path's blue ambient is what turned it grey, and a mission-placed Zelda in the shipped build is already tan.

## Night (town_night)

The campfire (`Fire`, `ohniste.p3d`) is the late shape there. AFTER, its pit draws per pixel like a placed one (pit debug RGB 238.2 / 167.4 / 234.2 against placed 238.2 / 168.1 / 234.2; BEFORE 244.6 / 129.5 / 240.2, the magenta CPU path). The town stays dark: frame luma 24.4 BEFORE, 25.0 AFTER, 25.0 placed (`research/town_night_measure.txt`, `before_after_town_night.png`, `crop_town_night_fire.png`). Whole-frame differences at night are dominated by the fire's random flicker and smoke: two runs of the same placed scene differ by 3.0 mean levels. I did not reproduce or isolate the gbuffer track's day-lit failure; my explanation above (optimising inside `New`) is reasoning, not a measured cause.

## Tests

New file `tests/unit/engine/Poseidon/Graphics/Rendering/test_shape_late_vertex_buffers.cpp`, tags `[Graphics][remaster][Shape][VertexBuffer]`. A recording `EngineDummy` returns a fake `VertexBuffer` from `CreateVertexBuffer`, so no GPU is needed; real fixtures `tests/fixtures/p3d/multi_lod_vehicle.p3d` and `flat_quad.p3d` load through `Shapes.New`.

1. A shape loaded after `OptimizeAll` has no buffer until `OptimizeLate`, then has the same per-level layout and buffer types as a fresh `OptimizeAll` gives it; a second `OptimizeLate` builds nothing.
2. Flags set after `New` decide: `IsLight` gives no buffer; sky-lit vertex hints (as `Landscape` sets them) give no buffer; a plain prop gets one; a `VIEW_GUNNER` level stays bufferless while level 0 gets one.
3. A shape released before its first frame is skipped; a shape loaded before `OptimizeAll` is optimised once, by `OptimizeAll`.

Before/after: the tests need the new `OptimizeLate` entry point, so they do not compile on the branch tip. To show they catch the bug, I disabled only the queueing line (`_late.Add(shape)`, i.e. the stock behaviour) and rebuilt: 2 of 3 test cases failed, 3 assertions (`BufferedLevels(*late) > 0` got `0 > 0` twice; layout `{1,-1,1}` against `{-1,-1,-1}`), log `research/unit_test_without_queue.txt`. That check ran on the first version of the test file, before I added the crew-view section. With the line restored: all passed (23 assertions then; final build with the crew-view section: 29 assertions in 3 test cases).

Tagged suites on the final build (`[remaster],[graphics],[gpu],[GL33],[DebugGroup],[ShutdownOrder],[UBORestoreGuard],[StateOwnership],[StateCacheAudit],[BufferUsageAudit],[IboBindAudit],[Conventions],[Shape],[VertexBuffer]`): 374 test cases, 373 passed, 1 failed: `DDSConverter: generate DDS fixture from synthetic_dxt5 DXT5` (`test_dds_converter.cpp:347`, `32 == 64`). It fails the same way on the branch tip's own test exe (`lighting\arma_CWR\build\lighting-rwdi`; note that test writes an exe-relative tmp file in that build folder). Log `research/unit_tests_after.txt`.

Full `PoseidonTests` run: 3067 test cases, 3057 passed, 6 failed, 4 skipped. The 6 (map legend, equipment and MP-version localization tests reading stringtables and `equipment.utf8.html`) also fail on the branch tip exe. Log `research/unit_tests_full_after.txt`.

clang-format `--dry-run --Werror`: clean on `ShapeDraw.cpp`, `World.cpp` and the new test. `Shape.hpp` is a legacy tab-formatted header that is not in the Format target's list (`format_sources.txt`); it reports 1853 warnings at the branch tip and 1868 with my lines, which follow the file's own tab style.

## In-game evidence

All runs through the scratch copy's `tests/rendering/captures/run-captures.ps1`, data junction `RemasterUD-fix-spawned-vehicles`, hidden desktop, cleanup by `stop-owned-games.ps1`. BEFORE = the branch tip build `D:\Arma_CWA_worktrees\lighting\arma_CWR\dist\lighting-rwdi` (not rebuilt). AFTER = `dist/fix-spawned-vehicles-rwdi`. Research tests (git-excluded) in the scratch copy's `tests/rendering/research/spawned-vehicles/`, copied to `research/tests/`: `sv_abel` and `sv_sinai` spawn the props as the committed capture set does; `sv_abel_pre` and `sv_sinai_pre` boot missions that place the same classes far from every view, so the same `createVehicle` reuses a shape loaded before `OptimizeAll`: the mission-placed reference at identical positions. Each view is shot Classic, Remastered and Remastered debug view 3 with `post=0` and a shader override (`research/ovr_debug3/`): lit fragments show their albedo, CPU-path (pre-lit) object fragments are painted magenta. Debug view 3 normally shows normals; the override exists only for these captures.

Difference statistics (`research/diff_stats_final.txt`; mean absolute difference in display levels / % of pixels with a channel off by more than 4 / by more than 10):

| View (Remastered) | BEFORE vs AFTER spawned | AFTER spawned vs placed | run-to-run floor (placed BEFORE vs placed AFTER) | CPU-path share below the horizon, BEFORE to AFTER |
|---|---|---|---|---|
| lowsun_fp | 0.20 / 1.48% / 0.13% | 0.00 / 0.00% / 0.00% | 0.00 / 0.00% | 13.93% to 0% |
| hull_close | 0.46 / 2.62% / 0.52% | 0.00 / 0.00% / 0.00% | 0.00 / 0.00% | 22.64% to 0% |
| noon_fields | 0.39 / 1.71% / 1.46% | 0.00 / 0.00% / 0.00% | 0.00 / 0.00% | 2.34% to 0% |
| sinai_lowsun_fp | 1.46 / 16.35% / 4.87% | 0.85 / 8.16% / 3.05% | 0.92 / 8.60% | 16.96% to 0% |
| sinai_hull_close | 2.09 / 24.15% / 10.35% | 0.76 / 7.33% / 2.57% | 0.81 / 7.79% | 21.31% to 0% |
| town_night | 5.58 / 21.69% | 4.90 / 20.41% | 3.02 / 15.26% (fire flicker, smoke) | pit: magenta to albedo (above) |
| tank_gunner_interior | 2.19 / 14.97% | 35.84 / 94.72% (crew view kept, see above) | 0.84 / 4.30% | 88.71% to 88.69% (unchanged on purpose) |

On Sinai the spawned-to-placed difference equals the run-to-run floor of the scene itself; I did not track down what moves there.

Vehicle boxes (`research/vehicle_boxes.txt`): at noon (`noon_fields`, sun 72 degrees) the spawned Jeep box goes from luma 76.6 to 58.5, equal to the placed Jeep (58.5). In `lowsun_fp` the Jeep box is 73.9 to 74.5 and the shaded M113 side 43.5 to 44.6, with the shade colour going from neutral grey to olive (44.5 / 43.3 / 42.8 to 46.2 / 44.6 / 40.3). Both lowsun boxes sit under the sun-shaft composite's grey veil (a separate bug in MENU section 5), which lifts BEFORE and AFTER alike.

Images (each BEFORE spawned | AFTER spawned | AFTER placed, Remastered on top and debug view below):

- `before_after_lowsun_fp.png`, `before_after_sinai_lowsun_fp.png`, `before_after_noon_fields.png`, `before_after_town_night.png` (the four views the brief asked for), plus `before_after_hull_close.png`, `before_after_sinai_hull_close.png` (close-ups).
- `crop_lowsun_fp_vehicles.png` (2x), `crop_zelda_slats.png` (with the albedo), `crop_town_night_fire.png` (2x), `interior_m60_gunner.png` (BEFORE | AFTER | placed | the rejected full-parity variant), `classic_lowsun_fp.png` (Classic: spawned keeps the classic look).
- Raw frames, contact sheets and per-shot timings: `runs/before`, `runs/after`, `runs/after_v1_full_parity` (the first build, full parity including crew views); per-view comparison sheets `runs/cmp`, `runs/cmp_v1_full_parity`.

What the images show: BEFORE, the spawned hulls carry the classic light inside a Remastered frame (the M113's star is tinted blue by the classic ambient, the Zelda slats are grey); the debug row paints them, the tent, the camnet and the spawned Golani rifleman solid magenta, while the player-class soldier (already loaded with the mission) shows albedo. AFTER, the same props show albedo in the debug row and look like the placed ones in the Remastered row: olive and tan paint in the shade, the Zelda slats tan.

## Cost

`research/hitch_perf_lines.txt`, test `research/tests/sv_hitch.test.sqf`, AFTER build, `--log-level debug`, one run on the shared machine: 8 `createVehicle` calls in one frame loaded 7 new vehicle shapes (3.5 to 6.5 ms each in `ShapeBank::New`, about 34 ms together); on the next frame `OptimizeLate` built buffers for 26 shapes (vehicles plus their proxies) in 1.40 ms. A second burst: 13 shapes in 1.21 ms. Per frame with nothing queued it is one size check. Afterwards the spawned vehicles cost what placed ones cost: less CPU (no CPU T&L), per-pixel shading and shadow-map lookups on the GPU. No clean GPU timing was taken.

## Risks

- **Noon vehicle sides are darker than today** (Jeep box 76.6 to 58.5 at a 72-degree sun), and darker than Classic (77.8). That is what the per-pixel rig already gives every placed vehicle; today's brighter spawned sides were the classic light by accident. If the owner wants them lighter, it is a rig calibration (the shade track's `bounce` lever), not this fix.
- **Crew views differ between spawned and placed vehicles** (see above): the spawned interior keeps the classic look, the placed one keeps the shipped, darker per-pixel look. The crew-view rule also keeps a script-created soldier's own first-person LOD (`VIEW_PILOT`) on the CPU path, as today; it only shows if the player controls a soldier class that the mission did not load.
- **One frame on the CPU path** for a shape first loaded during the draw itself (after `OptimizeLate` ran that frame). I saw no such case.
- **A shape a loader changes again after its first frame** would keep buffers built from the earlier state. `OptimizeAll` has the same exposure for mission shapes; none was seen here.
- **GPU memory**: every LOD of every spawned class gets buffers, as placed classes do.
- **Not tested**: multiplayer clients, savegame load (that path calls `OptimizeAll` at `WorldImpl.cpp:2382`, which clears the queue), a device restore (`RestoreVBuffers` calls `OptimizeAll`), the 3D UI previews (a weapon proxy first loaded by a preview could gain buffers, as it would if a mission had loaded it), a Guerrilla Mode session, mods other than @LoBo, GPU cost on an RTX 3060.

## Files in this folder

See the directory listing in the final report. `fix.patch`, `NOTES.md`, the evidence PNGs, `runs/` (raw captures), `research/` (tests, scripts, shader override, measurement text files, unit-test logs).
