# Bugs found by the Remastered lighting research

The research tracks (see `README.md`) found engine bugs on the way. On 2026-10-17 the owner asked to fix them before anything else: "Fix bugs with opus agents first". Each bug got one agent working in a scratch worktree detached at the branch tip 3c53141, an independent reviewer, and then one integrator who committed the accepted fixes to `claude/youthful-lovelace-8imbdh`.

- Commits: 874638c to 72f32d4, plus the roadmap log line 84408f8 (all on 2026-10-17, local time).
- Fix notes, before and after captures and unit-test logs: `D:\Arma_CWA_worktrees\lighting\tmp\bugfix\<bug>\NOTES.md` and `fix.patch` (dev machine only).
- Every fix has a unit test that fails without the fix and passes with it, as the per-bug notes record.
- The one remaining failure in the `[graphics]` tag in every run is the hidden fixture test `DDSConverter: generate DDS fixture from synthetic_dxt5 DXT5` (`test_dds_converter.cpp:347`, expects width 64, the fixture is 32x32). It fails the same way without these fixes.

## Summary

| Bug | Who found it | Fix | Status |
|---|---|---|---|
| Capture rig gave misleading before pictures | perf, aa, shadows | 874638c | landed |
| NaN lighting normals on tree crowns | wildcard | d57b712 | landed |
| Shadow sampler bound to texture 0 (GL undefined behaviour) | shadows, clouds | f102499 | landed |
| Cloud shadows under a clear sky | shade, clouds | 5cf85cf | landed |
| Sun-shaft grey veil on near vehicles | shade | 463506f | landed |
| Grouped bushes take the first bush's terrain fit | perf | d80f868 | landed |
| `switchLight` on a non-lamp crashes the game | ssgi | ab61aa5 | landed |
| Script-spawned vehicles skip the new lighting | shade, gbuffer, materials, normals | 747ffc8 | landed |
| The Remastered night is over-bright and yellow-green | exposure, materials, night | 4515770 and 72f32d4 | landed |
| Tree and building shadows off at start-up | shadows, volumetric, foliage, hfrt | none | **not landed**, second attempt in progress |
| Remastered ships with no anti-aliasing | aa | none | **not landed**, second attempt in progress |

## 1. Capture rig (874638c)

**Symptom.** The first original-look shot of a view was sometimes taken at a coarser detail level than the Remastered shot right after it (`sunrise_ridge`: 1,465 against 2,202 draws in the perf track's replay), so before and after pictures mixed lighting with model detail.

**Root cause.** Four things:
- Not view-distance streaming (the perf track's guess). `Scene::AdjustComplexity` (`SceneDraw.cpp:778`) picks the object detail from the frame time and refuses any step toward finer detail for 5 s after a step toward coarser detail (`SceneDraw.cpp:907`). A camera jump makes a slow frame, so the detail stays coarse for up to 5 s. The rig waited a fixed 150 + 90 frames (2 to 6 s), so it depended on machine load. Measured: `sunrise_ridge` drew 1,063 objects during the hold and 7,680 after it.
- `triSetLighting "remastered"` kept every knob from an earlier call. The comma form read only the first key, and bad values answered OK.
- No per-shot draw or object count was logged.
- `World::SetDate` did not tell the landscape the time changed (unlike `skipTime`), so auto street lamps switched on up to 12.1 s later, at a random moment relative to the shots. Any light slot turns instancing off for the whole scene, so draws jumped from 2,797 to 11,172 mid-run.

**Fix.**
- New tri verbs: `triPinObjectDetail` (holds the object detail; 1 = finest), `triSettleScene` (runs frames until draws and detail are steady and no horizon-map build is pending, lifting the texture upload budget; decision logic in `Dev::SceneSettle`), `triSceneStats` (draws, objects, lights, LOD, pin, horizon state).
- `triSetLighting "classic"` and `"remastered"` reset the whole tuning to its defaults; comma lists apply in order, all or none; bad values answer `FAIL`.
- `capture_lib.sqf` pins detail, settles and logs stats on every shot; `make_pairs.py` adds them to the tables and flags pairs whose objects differ by more than 5%.
- `World::SetDate` now calls `GLandscape->OnTimeSkipped()`. **Gameplay-visible:** `setDate` in any mission now switches auto street lamps at once, as `skipTime` does.

**Tests.** `test_tri_capture_verbs.cpp` (7 cases, 61 assertions; 3 cases and 10 assertions fail at the old tip). In game: every matched view's original-look and Remastered shots now draw the same objects.

**Risks.** Captures now measure the finest detail, so long-view timings are higher than in older runs and not comparable with them. A script that sets knobs and then calls `triSetLighting "remastered"` now loses them.

## 2. NaN crown normals (d57b712)

**Symptom.** Some tree crowns (the Saint Louis birch, several pines) lit with NaN normals and fell back to driver-defined lighting; their sun shadow lookup always returned "lit".

**Root cause.** Measured with diagnostic shader overrides on four views: only one cause, leaf cards whose mesh normal is (0,0,0). `normalize()` of a zero vector is undefined in GLSL and NaN on this NVIDIA driver (`EngineGL33_LightingShaders.cpp:139` at the tip). The NaN interpolates across the whole triangle (2,767 zero-normal pixels became 62,115 NaN pixels in one view), and `shadeRemastered` guarded a zero length but not NaN. Degenerate crown ellipsoids were suspected too, but none occurred.

**Fix.** The vertex shader normalises with `n *= inversesqrt(max(dot(n, n), 1e-12))`, so a zero normal stays zero. A foliage fragment with no normal of its own takes the crown normal for sun, sky, moon, local lights and the shadow offset. Healthy leaves compute exactly what they did. Hardening: `CrownEllipsoid` replaces an axis with a non-finite bound by a 5 cm half extent.

**Tests.** Two new cases in `test_remaster_lighting.cpp` (21 of 96 assertions fail without the fix). In game: 0 NaN crown pixels left; affected crowns change by about 2 levels on average (they gain a lit and a shaded side); whole-frame means move by at most 0.15 levels.

**Risk.** Leaves that were NaN now receive tree shadows once object shadows are on.

## 3. Shadow sampler on texture 0 (f102499)

**Symptom.** NVIDIA's debug output reported GL 131204 (texture 0 on unit 2 has no base level) at every start-up, and 131222 (a comparison sampler used with texture 0, undefined behaviour) after a shadow-map resolution or cascade change in Remastered. The first in-game run (`20261017-final`) logged 131204 once per island.

**Root cause.** Every lit program samples the shadow map on unit 2, and the driver validates it at every draw. (1) `InitLightingResources` created the 1x1 depth stand-in on upload unit 7 and never put it on unit 2. (2) `EnsureArrayTarget` deleted the cascade array bound on unit 2 and bound the new one on whatever unit was active.

**Fix.** `InitLightingResources` binds the depth stand-in on unit 2; `EnsureArrayTarget` selects unit 2 before creating the new array. No per-draw work, no shader change, no look change.

**Tests.** Three `[ShadowSampler]` source audits in `test_gl_state_cache_audit.cpp` (two fail without the fix). In game: neither warning fires; images unchanged (mean difference 0.008 to 0.014). After all fixes, the full capture set `20261018-bugfix` logged neither id.

## 4. Cloud shadows under a clear sky (5cf85cf)

**Symptom.** With `setOvercast 0`, about 4% of the map lost 80% of its sun with no cloud in the sky. With the camera under one patch the whole view was dim (`forest_edge_noon`: 74.5% of the frame shadowed, brightness -14.5%); at sunset a patch cut the sun path on the sea.

**Root cause.** The clearest weather keyframe sits at overcast -0.08 (`Landscape.cpp:426`) and `SetOvercast` clamps to 0..1 and interpolates, so `setOvercast 0` gives skyThrough 0.9586, cover 0.0414. `ComputeLightRig` treated any cover above 0.02 as broken cloud at transmission 0.2.

**Fix** (`LightRig.cpp` only). Shadow cover = `cover * Smoothstep(0.05, 0.10, cover)`: none for a clear sky, a smooth fade-in, full cover from 0.10 (about overcast 0.11). The uniform dimming is `min(1, classicDim / fieldMean)`, so the mean sun equals the stock dimming at every overcast. From overcast about 0.11 up the rig is bit-identical to before.

**Tests.** Three new `[remaster]` cases and one extended (3 cases, 18 assertions fail without the fix). In game: `forest_edge_noon` cloud-shadowed share 74.5% to 0%; overcast 0.1, 0.5 and 0.9 frames unchanged to three decimals.

**Open question for the owner.** Why overcast 0.5 shows few cloud shadows at eye level is a scale question, not a bug: the noise field's patches are kilometre-sized and repeat every 6 km. The notes list options (smaller patches, a 3 km tile, or the clouds track's sky-matched shadows). Not changed.

## 5. Sun-shaft grey veil (463506f)

**Symptom.** Against a low sun, nearby vehicles read as flat grey-violet cut-outs. The Jeep side in `lowsun_fp` read 63.7 with shafts on, 27.8 with them off.

**Root cause.** The shaft pass sums sky light along a ray toward the sun (the light an endless view ray would scatter), and the composite added that at full weight to every pixel, whatever its depth (`EngineGL33_PostFX.cpp`).

**Fix.** The composite scales the shaft term by the share of light the air in front of each pixel scatters, `1 - exp(-distance / L)`, read from full-resolution depth. `L` is 300 m in clear air, shorter in fog (`ShaftInscatter`, `ShaftScatterLength` in `LightRig`). The lighting lab mirror was updated (not run: `moderngl` is not installed).

**Tests.** One new case, 149 assertions (the 4 shader-contract checks fail without the fix). In game: near-vehicle veil +36 to +4 levels; the sky near the sun unchanged.

**Trade-off.** The warm beam over the near ground at a low sun is mostly gone too (`sunrise_ridge` beam region 50.3 to 44.1); it is the same term at the same distance. The research critic had flagged the same loss.

## 6. Grouped bushes take the first bush's terrain fit (d80f868)

**Symptom.** In an instanced draw (many copies of one shape in one call), every bush took the first bush's terrain fit, so members floated, sank or were sheared on their own slope. A stock bug.

**Root cause.** `Object::Animate` rewrites land-following vertices per object, but the instancing predicate never asked `Object::IsAnimated`, so members drew from the head's rewritten buffer. It only shows where instancing runs: on Abel today the street lamps keep it off, but any mission on a map without lamps hits it, and so would the perf track's light gate.

**Fix.** New header `InstancedRunRules.hpp` with the grouping rules over a plain struct: objects that rewrite their own vertices draw alone. Static shapes without per-object vertices, including skewed forest blocks, still batch.

**Tests.** `test_instanced_run_rules.cpp` (4 cases, 17 assertions; 2 cases fail with the old rule). In game, with a research build that lets instancing run: the fixed instanced frame equals the non-instanced frame on the bush slope (max 1 level), at +430 draws there (still about half the non-instanced count).

## 7. `switchLight` crash (ab61aa5)

**Symptom.** `switchLight` over map objects (for example over `nearestObjects` in a town) crashed the game in `StreetLamp::SimulateSwitch` calling `EntityHit::GetHit`, or corrupted a soldier so it crashed one draw later.

**Root cause.** Stock (unchanged since the public source and in upstream 3.05): `StreetLamp` declared no casting id, so `dyn_cast<StreetLamp>` accepted every `Entity` (houses, soldiers, vehicles), and the command wrote lamp state into the wrong object.

**Fix.** `USE_CASTING(base)` and `DEFINE_CASTING(StreetLamp)`. `switchLight` on anything else is now a no-op and `lightIsOn` returns "ERROR".

**Tests.** `test_streetlamp_casting.cpp` (2 cases; 3 of 4 assertions fail without the fix). In game: switching 666 objects near Saint Louis switches exactly the 6 real lamps; before, the same script crashed.

**Follow-up (not fixed).** Eight more classes have the same defect: `Airplane` (worst: `setDammage 0` calls `Airplane::RepairGear` on any Transport, houses included), `Car`, `Motorcycle`, `Tank`, `HelicopterAuto`, `Explosion`, `ObjectDestructed`, `SeaGullAuto`. A compile-time check in `dyn_cast` would catch them all.

## 8. Script-spawned vehicles skip the new lighting (747ffc8)

**Symptom.** Every vehicle created with `createVehicle` of a class the mission did not place drew with the original look's per-vertex light inside a Remastered frame: no per-pixel light, no shadows received. Guerrilla Mode spawns nearly every vehicle this way, and every vehicle in the capture set was spawned, so every hull judgement before this fix was made on the original look's lighting.

**Root cause.** `Shape::Draw` takes the GPU path only when the shape has a vertex buffer, and buffers are built only by `ShapeBank::OptimizeAll` at mission load, save load and device restore.

**Fix.** `ShapeBank::New` queues shapes loaded after the first `OptimizeAll` (weak links). `ShapeBank::OptimizeLate`, once per drawn frame right before the objects draw, runs the same `OptimizeRendering` on each queued shape. Building inside `New` is avoided on purpose: loaders set the flags it reads (sky hints, light, animation) after `New` returns; the gbuffer track built inside `New` and its night town lit up like day. Crew views (gunner, driver, commander, cargo) of late shapes stay on the CPU path, because on the per-pixel path a tank interior is 38% darker (no interior light).

**Tests.** `test_shape_late_vertex_buffers.cpp` (3 cases, 29 assertions; 2 cases fail without the queue). In game: spawned Jeep, M113 and tent are pixel-identical to mission-placed ones on Abel; the CPU-path share below the horizon goes to 0% in every view; the night town stays dark; about 1.2 to 1.4 ms once, on the frame after a burst of 8 new vehicle classes.

**Findings.** The Zelda slat armour turns from grey to tan: the texture is tan, and on the CPU path the shaded face got only the blue ambient. At noon, spawned vehicle sides get darker than before (Jeep 76.6 to 58.5, the same as a placed Jeep): the old brighter sides were the old lighting by accident.

**Follow-up.** Crew-view lighting now depends on load time (spawned interiors keep the old look, placed interiors are per pixel). An interior light model is needed.

## 9. The over-bright night (4515770 and 72f32d4)

**Symptom.** The moonlit town metered brighter than every low-sun daytime view and its olive grass turned yellow-green. The moonless country 260 m from town was a flat grey.

**Root cause.** Measured with knob A/B: almost all night light came from the street lamps, not from the rig.
1. **Lamp colours read as white** (stock data bug, both looks): `GetColor()` in `ParamFileExt.cpp` returned white for any colour that is not exactly 4 entries long. The street lamp config gives `colorDiffuse[]={0.9,0.8,0.6}` and `colorAmbient[]={0.1,0.1,0.1}` (3 entries), so every lamp lit white with ten times its authored ambient. That ambient has no direction and reaches 500 m.
2. **The Remastered local-light sum was capped at 1.5** where the original look clamps at 1.0, so lamp-lit ground got up to 2.44 times the original look's linear light and escaped the night-eye greying.
3. **The moon added nothing visible** (`kMoonFull = 0.0035`).

**Fix.**
- 4515770: the local-light clamp is 1.0; the moon is a key light (`kMoonFull` 0.02, a cool pale colour, dimmed by overcast) with a moonlit sky fill; dim lit surfaces fade to a slightly cool grey. Day rigs are bit-identical. A hue-keeping clamp was tried and rejected (mustard lawn at dusk).
- 72f32d4 (kept separate so it can be reverted alone): `GetColor` reads `{r,g,b}` with alpha 1.

**Tests.** "A full moon lights a cool, pale, readable night" (4 of 18 assertions fail without the fix); "GetColor reads three-entry colours with alpha 1" (5 of 10 fail). In game: town ground 164/159/123 (saturation 0.25) to 124/122/111 (0.10), the original look's level; moonless country 51 to 15; dusk matches the original look. In the full capture set after all fixes, `town_night` Remastered lower-frame brightness went from 157.5 to 121.4 (original look 120.4).

**Side effects.** The data fix also changes the original look and night vision near towns (open country 57 to 20; night-vision ground green 106 to 79), and @LoBo's two 3-entry lamp panels drop from 1.0 to their authored 0.7 ambient. All night local lights, fires, headlights and muzzle flashes included, now peak at 1.0. Untested: with a waxing moon up at sunset, the moon lifts twilight by up to 1.3 EV. Lamp-lit town ground still meters like sunset ground, because the original look's 50 m flat lamp discs overlap; that needs the night track's falloff rework.

## 10. Tree and building shadows off at start-up (not landed)

**Symptom.** Trees, bushes, walls and buildings cast no sun shadows in any fresh profile or capture run. The foliage track's "shadow maps vanish when facing the sun" is the same bug (with no static casters, a frame without spawned units has no casters at all).

**Root cause.** `Scene::_objectShadows` starts false. `LoadAndApplyGraphicsConfig()` runs before `InitializeWorld()` (`GameApplication.cpp` 681, 683), and the scene half of the apply sits behind `if (GScene)`, so at boot it does nothing. Each session that never opened the Graphics page then saved `objectShadows=0` to the profile. The same bug skipped the terrain grid, object LOD and particle tiers.

**Fix built** (scratch worktree `fix-object-shadows`): the scene half moved into `ApplyGraphicsConfigToScene`, applied from `Scene::LoadConfig`. Tests: `test_graphicsApply.cpp` (3 cases fail without, pass with). In game: long tree shadows from the first frame; the shadow pass 0.15 to 0.8 ms.

**Why it was not landed.** The roadmap log (84408f8): the patch also applied the terrain grid tier at boot, which changes physical terrain (on a 16 GB+ machine the autodetected Ultra preset sets a 6.25 m grid and LOD bias 2.0). A second attempt was in progress in `D:\Arma_CWA_worktrees\fix2-object-shadows` (uncommitted) when this page was written.

**Risk.** Frames full of trees or houses at low sun get darker (forest edge -4%, town at 17:00 -5%, into-the-sun grove -9%). See `shadows.md`.

## 11. No anti-aliasing in the Remastered default (not landed)

**Symptom.** A fresh profile runs with no MSAA (`GraphicsConfig.hpp:89`, `msaaSamples = 0`), so alpha-to-coverage is off and polygon edges are not anti-aliased. Every existing profile also holds `msaaSamples=0`, because `Save` always writes it.

**Fix built** (scratch worktree `fix-msaa-default`): default 4x, config version 2 to 3, a migration that turns an untouched 0 into 4 once (and keeps 2x, 4x, 8x), and a guard so a later version bump cannot overwrite a user's Unlimited frame cap. Tests: 6 cases fail without, pass with. In game: `frame target 960x640 (4x MSAA, render scale 1)` on first boot; light unchanged.

**Why it was not landed.** The roadmap log (84408f8): its migration also added MSAA on top of a supersampling scale the user had chosen. A second attempt was in progress in `D:\Arma_CWA_worktrees\fix2-msaa-default` (uncommitted) when this page was written.

## Other findings, not fixed

- **Dead water shader path:** `misc\waveBumpMap.paa` (named in `remaster.cpp`) does not exist, `GetWaterBumpMap()` has no callers, and the README's Water row describes a shader the sea does not use. See `water.md`.
- **Mist is visual only:** AI spotting ignores the haze track's valley mist. See `haze.md`.
- **The shipped Remastered AO never darkens vehicles, soldiers or tree crowns:** under 4x MSAA those draws store alpha 1 (the "sky share" the AO reads). See `ssgi.md`.
- **The capture harness's per-frame pixel readback** under `--autotest` serialises CPU and GPU (3 to 16 ms of CPU frame). See `perf.md`.

## After all fixes: the full capture set

`D:\Arma_CWA_worktrees\lighting\tmp\captures\20261018-bugfix\` (run on the integrated build, data junction `RemasterUD-integ`; comparison against `20261017-final` in `cmp_vs_20261017-final.txt` and `vs_20261017-final\<view>__stack.png`):

- Every mode settled (`OK:30` to `OK:279` in the load column); no 131204 or 131222 in any game log (the remaining GL messages, 131169, 131154 and 131218, are performance and info notes that were there before too).
- `town_night` Remastered lower-frame brightness 157.5 to 121.4 (original look 120.4).
- `lowsun_fp` and `sinai_lowsun_fp`: vehicles solid instead of veiled (Remastered frame mean 130.3 to 127.3 and 106.3 to 103.5).
- `coastline_sunset`: the shore cloud shadow is gone (roadmap log, 84408f8).
- Original-look firefight frames are darker near the Abel lamps (the lamp wash is gone, mean -4.0 to -4.7). The firefight views are not matched by design (AI fire and smoke differ; `firefight_2` Remastered reads +16.5).
- Unit tests on the integrated build: PoseidonTests 3,081 of 3,091 cases pass, 6 fail (localisation tests that read `packages\Remaster` data), 4 skipped; PoseidonCoreTests 823 of 827 pass, 4 skipped; PoseidonFoundationTests 475 of 475 pass.
