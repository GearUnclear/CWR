# Track "combat": emissive combat light (flashes, explosions, fires, tracers, flares)

Verdict: **promising**. Final prototype = round 9 (`prototype.patch`, run 9 captures). Look at `BEST_ffh.png`, `BEST_ffn_fires.png`, `BEST_flr.png` first.

## What the engine does today

Found in the code (paths under `arma_CWR/engine/Poseidon`):

| Source | Classic light | Where |
|---|---|---|
| Rifle / MG / cannon flash | `WeaponLightSource`: a `LightPoint(FireColor, FireAmbient)` while `_timeToLight` runs; brightness 0.004 for a soldier (`Soldier::FireWeaponEffects`, effectively nothing), 0.4 for a vehicle MG, 1 for a tank main gun (`Tank::FireWeaponEffects`); MGs get a random 0.8 +- 0.2 per frame | `Graphics/Rendering/Effects/Smokes.cpp` |
| Explosion | `Explosion::Simulate`: a `LightPoint` while the fire is active (`_minLightTime`) | same |
| Burning wreck | `SmokeSourceVehicle`: a light during the fuel explosion only, nothing while it burns | same |
| Campfire | `Fireplace`: `LightPointOnVehicle`, a new random brightness every frame | `World/Scene/Fireplace.cpp` |
| Flare | `IlluminatingShell`: `LightPointOnVehicle`, the ammo's `lightColor`, brightness 2, random 0.8 +- 0.2 per frame; ignites 2 s after launch, lives 17 s | `World/Entities/Weapons/Shots.cpp` |
| Missile motor | `LIGHT_MISSILE` glow | same |
| Tracer | none | `ShotBullet` |

How they shade: Classic sums local lights per vertex in display space and clamps. `Scene::SelectLights` drops every local light by day (`NightEffect() < 0.01`), so by day no flash lights anything. Remastered `localLights()` (`PoseidonGL33/EngineGL33_LightingShaders.cpp`) keeps the classic falloff and pool size, night only, display-space sum taken to linear, capped at 1.5. The classic explosion and flare lights reach far with almost no falloff: for the flash frame the whole visible ground whitens evenly, with no direction, no colour and no shadow (Classic and Remastered panels of `BEST_ffh.png`, `BEST_ffn.png`, `BEST_flr.png`).

## The prototype (C++, scratch worktree)

Worktree `D:\Arma_CWA_worktrees\gfx-combat` (detached at the graphics branch tip 3c53141, no branch), built into `arma_CWR\dist\gfx-combat-rwdi` from `build\gfx-combat-rwdi`. Diff: `prototype.patch` (round 9, 20 files plus the new `CombatLights.{hpp,cpp}`). `prototype_r3/r4/r5.patch` are the states runs 3, 4 and 5 were shot with (rounds 6-8 were not saved as separate patches; their deltas are listed under "Rounds"). Everything sits behind `triSetLighting "combatLights=1"`; at 0 every classic light path runs as before. Classic mode is not touched by the prototype in principle (combat lights exist only when the remastered lighting is active and the knob is on; the only Classic-side line is `UploadVSLights` skipping HDR lights, which then never exist). Not verified bit-identical by capture.

1. **`Graphics/Rendering/Lighting/CombatLights.{hpp,cpp}`**: `LightCombat`, a `LightPoint` carrying linear HDR radiance with falloff `E(d) = I / (1 + d^2 / r0^2)` (a ball of radius r0, not a point), windowed smoothly to 0 where E would fall to 0.0025 (about the night ambient), max 160 m (600 m for flares). Kinds:
   - Muzzle: orange, `exp(-t/30 ms)`, power from the fired ammo's `hit`, range <= 45 m.
   - Cannon: `0.8 exp(-t/45 ms) + 0.2 exp(-t/160 ms)`.
   - Explosion: white-hot flash `0.65 * combatFlash * exp(-t/60 ms)` cooling to an orange fireball `0.35 exp(-t/160 ms)` with value-noise flicker; size from the engine's explosion size; placed at the fireball centre (1 + 2.5 * size m up).
   - Fire: smooth breathing (1.3 and 3.7 Hz sines, 9 and 23 Hz value noise) instead of a new random brightness each frame; power x `combatFire`. Wrecks: embers under the smoke while it burns, 1.5 m up.
   - Tracer: a small light carried with the round.
   - Flare: 2500 at 1 m, tinted by the ammo's `lightColor`, 0.35 s ignition, sputtering flicker.
   - Day scaling (round 9): the remastered exposure does not adapt between day and night, so a light of fixed power reads as strong under the noon sun as at night. Intensity x `0.12 + 0.88 * NightEffect` for continuous lights, `0.5 + 0.5 * NightEffect` for flashes. (`NightEffect` logged 1 at the 19:35 dusk views and 0 at noon.)
2. **Hooks**: `WeaponLightSource::Start/Simulate` (one fire-and-forget burst per shot; `Soldier`/`Tank::FireWeaponEffects` pass `mode->_ammo->hit`), `Explosion::Simulate`, `SmokeSourceVehicle`, `Fireplace`, `ShotBullet`, `IlluminatingShell`. The classic fire and flare lights stay alive "visual only" so their glow sprites still draw. `Scene::SelectLights` lets combat lights through by day.
3. **Lit shader** (`combatLights()` in `shadeRemastered`): linear radiance, wrap from the ball's angular size (a fireball low over flat ground still lights it; a muzzle flash lights the shooter's arm), a Blinn-Phong flash highlight on glossy materials, and the classic night-eye desaturation lifted where combat light dominates (a fire-lit face keeps its colour).
4. **Screen-space shadow** (rounds 6-9, the big one): each frame the combat light adding most to the view (`I * r0^2 / max(d_cam^2, 100)`) traces a shadow per fragment: 24 steps from the fragment toward the light (jittered with interleaved gradient noise, quadratic spacing so they crowd near the receiver, at most 25 m) through the **previous frame's resolved depth** (`_post.depthTex`, which the post pass already resolves; bound on unit 6), reprojected with the previous frame's camera (rows in new `LitConstants` slots 58-64). A sample behind the depth buffer by more than a bias (0.05 m + 0.6 % of depth) and less than 1.5 m is blocked; how deep it sits relative to the cone toward the light ball (`r0 * t`) gives a contact-hardening penumbra. Off-screen occluders count as open.
5. **Smoke lit from inside**: `Object::DrawDecal` adds the combat lights' linear irradiance to each smoke/dust particle (CPU, isotropic), soft-saturated (round 8), x `combatSmoke`.
6. **In-scatter** (post composite): analytic single scattering of the 8 strongest combat lights through a ball of lit air around each, clipped by the depth buffer; fires spread thinner (round 7: a dense ball read as a flat disc behind the wreck).

Knobs (`triSetLighting "key=value"`, ONE key per call: the README's "key=value,..." form only sets the first key): `combatLights`, `combatSmoke`, `combatScatter`, `combatWreck`, `combatShadow`, `combatFlash`, `combatFire`. Round-9 look: `combatLights=1, combatSmoke=0.35, combatScatter=1, combatWreck=1, combatShadow=1, combatFlash=6, combatFire=2.5`.

## Rounds

| Round / run | Build | What changed | Outcome |
|---|---|---|---|
| 1-5 (earlier agent) | build0-4 | Combat lights, smoke lighting, in-scatter, wreck embers, night-eye lift | Run 5: warm local pools, but the flash frame much darker than Classic's whiteout (ffn b2 ground 33 vs 98 sRGB) and no shadows: flat, pasted-on look |
| 6 / run6 | build5 | Screen-space shadow (16 steps), `combatFlash` x6, `combatFire` x2.5 | The breakthrough: long radial shadows from the flash (`run6/ffh_b02_5way.png`) |
| 7 / run7 | build6-7 | Flares as combat lights, march capped at 25 m, softer fire in-scatter, `combatSmoke` 0.6 | Flare shadows work (`run7/flr_2_4way.png`); campfire view matched (`BEST_camp.png`) |
| 8 / run8 | build8 | 2x2 ordered jitter, 24 steps, fireball tau 0.28 -> 0.16 s, soft smoke saturation | Ordered jitter **failed**: a regular grid on fire-lit ground (see `run8/ffh_x_0.png` right of the Ural); dust cloud still too bright |
| 9 / run9 | build9 | Back to IGN jitter (24 steps), fire lights raised (wreck 1.5 m, campfire 0.6 m), day scaling, `combatSmoke` 0.35 | Final. The grid is much reduced but a fainter cross-hatch remains on the ground beside the wreck (`zoom_r8_grid_vs_r9.png`); dust cloud grey with a warm base; day fire pools subtle |

## What real data showed (run 9 unless noted)

Every view is restaged per mode; AI fire, the shell's flight and the blast differ between modes, so shot N of one mode is close to, not identical with, shot N of another. Shell sequences were shot frame by frame (`b01..b12` = 6, 7, 8, 9, 10, 11, 12, 14, 17, 22, 30, 45 sim frames after `createShell`); the BEST sheets use b02, the first frame in which the shell has burst in every mode (checked in each `*_seq.png`). Numbers are mean sRGB of the presented frame / its lower third, and red/blue of the lower third.

- **ffh** (moonless night, camera 5 m up, shell behind two lines of men), `BEST_ffh.png`, `run9/ffh_boom_crx.gif`: Classic and Remastered whiten the ground evenly (frame 72 / 75). Combat: the men throw long radial shadows toward the camera, the crater glows orange, the Ural is backlit; frame 63, lower third 68 (Classic 90), R/B 1.41. Before the shell (`ffh_*_0`) combat is the brightest mode (27 vs 15 / 18) because the burning Ural now lights its surroundings and casts shadows.
- **ffn** (eye height, `BEST_ffn.png`, `BEST_ffn_fires.png`, `run9/ffn_boom_crx.gif`): in the steady state the campfire and the burning Ural light the battlefield warmly and the Ural throws long shadows of the men toward the camera (frame 25 vs Classic 24 / Remastered 19). **Flash frame: combat is darker than Classic (37 vs 63)**: at eye height the ground near the camera is lit at grazing incidence, so physically it gets little of a blast 40 m away, while Classic floods the whole ground. The men read as backlit silhouettes with shadows; the flash itself does not read as a whole-screen whiteout any more.
- **ffd** (dusk 19:35, firefight_1..3 layout), `BEST_ffd.png`: the flash lights the ground around the burst warm with shadow streaks; at the flash frame the frame is darker than Classic (70 vs 80); after it (b04) combat is a little brighter than Classic (61 vs 52) and level with Remastered.
- **flr** (white flare, CfgAmmo `Flare`, 40 m left of the men), `BEST_flr.png`, `run9/flr_crx.gif`: Classic and Remastered light the field flat grey. Combat: a falloff from the flare's side of the field, every man and the Ural casting a long shadow away from it, lengthening as the flare sinks; frame 77 vs 53 / 57. The most "RTX" result of the set.
- **muz** (rifle + MG at night), `BEST_muz.png`: Classic shows the flash sprite only. Combat lights the shooters' arms and chests and an orange pool on the ground with a soft shadow behind the MG gunner. In these captures the muzzle light appears in exactly one captured frame per shot (f1c, f2c); the classic sprite does the same. Why the 30 ms envelope does not survive into the next captured frame was not investigated.
- **camp** (town_night's campfire, three men; run 7, all modes matched), `BEST_camp.png`: warm pool and fire-lit fronts of the men; under this view's bright moonlight the fire is a modest change. Run 9's `campn` sheets are **not matched**: after the noon view, Classic and Remastered rendered dark and the combat mode bright (the environment had changed between modes, not the combat lights); ignore `run9/campn_*`.
- **ffday** (noon), `BEST_ffday.png`: with the round-9 day scaling the combat lights are close to invisible by day (lower third 166 vs 163); the flash frame is unchanged in practice. Round 8 (no day scaling) painted visible orange pools around the fires at noon (`run8/ffday_x_seq.png`).

## GPU cost (honest version)

The machine ran 3-4 other games at once (tri's slot ledger), so GPU timestamps include other contexts' work: the same Remastered view varied from 5 to 20 ms between cycles. The per-panel GPU number on the sheets is the timer average of the settle window **before** each sequence (no explosion in it), the same for every shot of a mode; it is not the flash-frame cost.

Dedicated alternation under sustained AI fire (ffn scene: campfire + burning Ural + muzzle flashes and tracers), GPU frame ms, r = Remastered, x = round 9 with shadow, w = without:

| Run | Cycles | r median | x median | w median | median of x - r per cycle | median of x - w |
|---|---|---|---|---|---|---|
| run6 (round 6) | 3 | 16.99 | 19.76 | 17.38 | +2.8 | -0.2 |
| run7 (round 7) | 5 | 16.58 | 14.90 | 13.15 | +5.7 | +0.5 |
| run8 (round 8) | 8 | 9.27 | 7.73 | 6.29 | +1.41 | +1.83 |
| run9 (round 9) | 8 | 13.35 | 11.19 | 12.60 | -0.32 | -0.05 |

(Run 6/7 medians from `run6/timings.json`, `run7/timings.json`; run 8/9 computed per cycle.) Conclusion: the cost is below what this contended machine can resolve; a cautious upper bound is about +1 to +2 ms at 960x640, 1.5x scale, 4x MSAA with a fire tracing shadows over most of the frame. The shadow trace is 24 dependent depth fetches per fragment inside the shadowed light's range, so its worst case is the flash frame of a big explosion near the camera; that spike is not captured by the averaged timers. CPU side: one `LightCombat` per shot and per explosion (the diagnostics line saw up to 9 muzzle lights and 6 tracers alive at once), plus a per-particle light loop for smoke.

## What a production version needs

- Shadows as a separate screen-space pass after the depth prepass or at the post boundary of the *current* frame (half resolution, bilateral blur, temporal accumulation) instead of a per-fragment trace through last frame's depth inside the forward shader: removes the stipple, the one-frame lag on moving men and the disocclusion errors, and makes the cost flat. With a light-index buffer it can shadow 2-4 lights instead of one.
- Light budget and selection: cap simultaneous combat lights (merge muzzle flashes of one squad into one light), sort by contribution; the 8 local-light slots per draw are shared with lamps and headlights.
- Particles: light smoke per vertex or in a small froxel grid with a phase function and self-shadowing instead of isotropic CPU irradiance; the explosion dust still reads a little lighter than it should for half a second.
- Calibrate per ammo/explosion class from config (`hit`, `indirectHit`, explosion size) rather than the hard-coded table; flares from `lightColor` already.
- An exposure response to the flash (a brief eye adaptation) if the owner wants the flash frame to read as bright as Classic's whiteout; the physical falloff alone makes the eye-height flash frame darker.
- Investigate the one-captured-frame muzzle light; decide whether one frame is the intended look.
- Classic must stay bit-identical: the hooks already branch on `CombatLights::Enabled()`; production should move them behind the lighting-model switch explicitly and add a Classic capture regression.

## Risks against the owner's taste list

- **"Any result that is only darker"**: the flash frame at eye height and at dusk is darker overall than Classic's whiteout (ffn 37 vs 63, ffd 70 vs 80), even though it is far more directional and the frames before and after are brighter. This is the main risk; the knob `combatFlash` (x6 now) can push it, at the cost of overexposing near surfaces.
- **Warmth**: the light is warm (R/B 1.4-1.6 on lit ground), never neutral; flares are tinted by their config colour (white flare R/B 1.07).
- **Haze/wash**: the in-scatter is limited to a few metres of lit air around each light; no frame-wide wash. The horizon line behind fires shows a bright warm band (grazing light on distant ground), which some may read as a glow strip.
- **Crushed shade**: shadows from combat lights are full occlusion of that light only; ambient and moon remain, so shaded ground keeps its night level, not black.
- **Stipple / cross-hatch**: penumbrae show an interleaved-gradient-noise stipple at 1:1 (`zoom_shadow_stipple.png`), and ground lit at grazing angles by a low fire keeps a faint cross-hatch (`zoom_r8_grid_vs_r9.png`, right panel). Production needs the temporal or bilateral filter above.
- Not tested: tank cannon flashes (no tank in the views), vehicle interiors, MP, rain, mods' explosions.

## Files

- `NOTES.md` (this), `prototype.patch` (round 9), `prototype_r3.patch`, `prototype_r4.patch`, `prototype_r5.patch`.
- `BEST_*.png`: final comparisons (Classic | Remastered | Remastered + combat lights, GPU ms on each panel). `zoom_shadow_stipple.png`, `zoom_r8_grid_vs_r9.png`: 1:1 crops of the shadow artefacts.
- `run1`..`run5`: the earlier agent's runs (runs 1-4 have sheets; run 5's sheets were built this session from its shots). `run6`..`run9`: this session. Each has `shots/` (one PNG per label), `timings.json`, `udcap.log`, `tri/` (game logs), `<view>_<shot>_3way.png` / `_4way` / `_5way` sheets, `<view>_<mode>_seq.png` strips, GIFs in run6, run7, run9.
- `scripts/`: `gen_test.py` (runs 1-5), `gen_test6.py` .. `gen_test9.py` (test generators), `collect.py` (shots + timings), `make_sheets.py`, `make_best.py`, `make_gif.py`.
- Research tests (git-excluded) in `D:\Arma_CWA_worktrees\gfx-combat\arma_CWR\tests\rendering\research\combat\combat_r1..r9.test.sqf` with a copy of `capture_lib.sqf`.
- Build logs `build0..build9.log`, `configure.log`; capture logs `run1..run9.log`.
