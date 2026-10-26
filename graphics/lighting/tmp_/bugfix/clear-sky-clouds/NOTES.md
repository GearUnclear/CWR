# Bug fix: cloud shadows under a clear sky (BUGID clear-sky-clouds)

Status: fixed in the scratch worktree `D:\Arma_CWA_worktrees\fix-clear-sky-clouds` (detached at `claude/youthful-lovelace-8imbdh` tip 3c53141, nothing committed). Build `arma_CWR/build/fix-clear-sky-clouds-rwdi`, exe `arma_CWR/dist/fix-clear-sky-clouds-rwdi`. Patch: `fix.patch` in this folder (4 files, +159 / -15).

The second question of the brief (why overcast 0.5 shows hardly any cloud shadows) is a design question, not a coverage bug. It is documented below with measurements and options; nothing was changed for it.

## Root cause

1. **The clearest weather the engine has is not cloud-free.** `GetWeatherBasics` puts the clear "jasno" keyframe at overcast **-0.08** (`arma_CWR/engine/Poseidon/World/Terrain/Landscape.cpp:426`), while `Weather::SetOvercast` clamps the script value to 0..1 (`Landscape.cpp:474`) and interpolates between keyframes (`Landscape.cpp:505`). So `setOvercast 0` is 0.08 / 0.58 of the way from jasno (skyThrough 1.0) to oblacno (0.7): **skyThrough 0.9586, cover 0.0414**. A skyThrough of 1.0 is unreachable from a mission.
2. **The rig treated any cover above 2% as broken cloud.** `ComputeLightRig` (`arma_CWR/engine/Poseidon/Graphics/Remaster/LightRig.cpp:184-188` at the branch tip): `cover = 1 - skyThrough`, `clouds = ... && cover > 0.02f`, then `rig.cloudCover = cover` (0.041), `cloudTransmit = 0.2`, and the uniform sun dimming `overcastDim` set to 1.
3. The upload (`arma_CWR/engine/PoseidonGL33/EngineGL33_Lighting.cpp:410-411`) turns the shader path on, and `cloudSunVisibility` (`EngineGL33_LightingShaders.cpp:420-428`) thresholds the equalised noise at `1 - cover`: about 4.3% of the ground loses 80% of its sun with no cloud in the sky. A camera under one of those patches sees its whole view dimmed.

## The fix

`LightRig.cpp` only (plus a comment in `LightRig.hpp`, a sentence in the Remaster `README.md`, and the tests):

```cpp
constexpr float kClearSkyCover = 0.05f;   // setOvercast 0 reports cover 0.041
...
const float shadowCover = cover * Smoothstep(kClearSkyCover, 2.0f * kClearSkyCover, cover);
const bool clouds = in.cloudShadowsActive && tuning.cloudShadows > 0.0f && shadowCover > 0.0f;
rig.cloudCover = clouds ? shadowCover : 0.0f;
rig.cloudTransmit = ... unchanged ...;
const float fieldMean = 1.0f - rig.cloudCover * (1.0f - rig.cloudTransmit);
const float overcastDim = std::min(1.0f, (skyThrough * 0.8f + 0.2f) / fieldMean);
```

- **Clear sky (cover up to 0.05, setOvercast 0 to about 0.017):** no cloud shadows. The sun takes the stock dimming uniformly (`skyThrough * 0.8 + 0.2`, 0.967 at overcast 0), exactly as Classic does.
- **Fade-in (cover 0.05 to 0.10, overcast about 0.017 to 0.11):** the shadowed share rises smoothly from 0 to the full cover; the dimming the field does not carry stays uniform.
- **Cover 0.10 and up (overcast about 0.11 and up, so every broken and overcast sky):** `shadowCover == cover`, `fieldMean == classicDim`, `overcastDim == 1`: bit-for-bit the branch tip's behaviour.

Why it is correct:
- Mean preserved everywhere: `overcastDim * fieldMean = classicDim` whenever `overcastDim < 1`. And `overcastDim <= 1` always holds: `shadowCover <= cover` and `1 - T <= 0.8`, so `fieldMean >= 1 - 0.8 cover = classicDim`. The `min` is only a float guard; `fieldMean >= 0.2`, so no division by zero.
- Continuous: no pop as the weather changes. A plain threshold raise (`cover > 0.05`) would switch 5% of the map to 0.2 sun at once at overcast 0.017.
- Side effect, intended: the dev-overlay `cloudShadows` knob below 1 now keeps the mean too (the residual dimming is uniform). Before, any knob value above 0 set `overcastDim = 1`, so the knob jumped from the stock dimming at 0 to an undimmed sun at 0.01, and at 0.5 the mean sun was brighter than stock (unit test below).
- Not touched: the sky fill (`skyPower` still uses the full cover), the shader, the noise, Classic (the rig is used only by the remastered programs).

Note on the shade track's prototype: it renormalised linearly (`(cover - 0.05) / 0.95`), which also changes every broken sky (overcast 0.5: cover 0.30 to 0.263, lit gaps dimmed to 0.963). The smooth fade here leaves everything above cover 0.10 exactly as it was.

## Tests

`arma_CWR/tests/unit/engine/Poseidon/Graphics/test_remaster_lighting.cpp` (helper `SkyThroughAtOvercast` replays the engine's weather keyframes):

| Test case | What it pins | Branch-tip rig | Fix |
|---|---|---|---|
| `Remaster light rig: a clear sky casts no cloud shadows` (new) | setOvercast 0 and 0.01: cloudCover 0, transmit 1, sun equals the uniform stock dimming | **FAILED** 6 assertions (cloudCover 0.04138 and 0.04655, transmit 0.2, sun 0.53877 vs 0.52093) | passed |
| `Remaster light rig: cloud shadows carry the classic overcast dimming on average` (extended) | mean sun = stock dimming at overcast 0 to 1 and with the knob at 0.5; cover > 0 from overcast 0.1; cover = 1 - skyThrough from overcast 0.25 | **FAILED** 10 assertions (knob 0.5: mean 0.530 vs 0.521 at overcast 0 up to 0.323 vs 0.108 at overcast 1) | passed |
| `Remaster light rig: cloud shadows fade in with the overcast` (new) | sweep skyThrough 1 to 0 in 0.001 steps: cover monotonic, steps < 0.005, lit-gap sun steps < 0.5% | **FAILED** 2 assertions (cover jumps 0.021 at skyThrough 0.979, sun jump) | passed |
| `Remaster cloud shadows cover the share of the ground the rig asks for` (new) | the coverage mapping: the engine's noise (256, seed 1) read bilinearly under the shader's soft threshold covers the rig's cover within 0.01 from overcast 0.02 to 0.8, and the mean sun stays within 1% of stock | passed (the mapping was never broken) | passed |

Before = the scratch test binary built against the branch tip `LightRig.cpp` (`unit_before_remaster.txt`: 16 cases, 13 passed, 3 failed; 3273 assertions, 18 failed). After = the fix (`unit_after.txt`).

After, run from `D:\` with `build/fix-clear-sky-clouds-rwdi/.../PoseidonTests.exe`: `[remaster]` 16/16 cases (3273 assertions), `[gpu]`, `[GL33]` (104 cases), `[DebugGroup]`, `[ShutdownOrder]`, `[UBORestoreGuard]`, `[StateOwnership]`, `[StateCacheAudit]`, `[BufferUsageAudit]`, `[IboBindAudit]`, `[Conventions]`, `[Shaders]` all pass. `[graphics]`: 354/355; the one failure is `DDSConverter: generate DDS fixture from synthetic_dxt5 DXT5` (`test_dds_converter.cpp:347`, `dds2.width == 64` gives 32). It fails identically with the branch-tip rig and alone; the file is untouched by this patch and by the lighting branch. Pre-existing, unrelated.

clang-format 22.1.8 `--dry-run --Werror`: exit 0 on `LightRig.cpp`, `LightRig.hpp`, `test_remaster_lighting.cpp`.

## In-game evidence

Runs through `run-captures.ps1` on a hidden desktop, junction `RemasterUD-fix-clear-sky-clouds`, 960x640, 4x MSAA, 150% scale. Research tests (git-excluded) in the scratch worktree at `arma_CWR/tests/rendering/research/clear-sky-clouds/` (copies in `tests/`, generated by `scripts/gen_csc_tests.py`). BEFORE = `D:\Arma_CWA_worktrees\lighting\arma_CWR\dist\lighting-rwdi` (branch tip, not rebuilt), folder `before/`. AFTER = `dist/fix-clear-sky-clouds-rwdi`, folder `after/`. Same test script for both; both runs passed (tri 2/2). Every view was shot as `remastered`, `dbg1` (debug view 1, sun visibility, post off) and `dbg1cs0` (the same with `cloudShadows=0`); "cloud-shadowed" below is the share of the frame at least 0.1 darker in `dbg1` than in `dbg1cs0` (`scripts/measure_csc.py`, `measure_before.json`, `measure_after.json`).

| View | Weather | Cloud-shadowed share of frame, before | after | Remastered frame luma, before / after |
|---|---|---|---|---|
| `forest_edge_noon` (Abel, eye level, noon) | overcast 0 | **74.5%** (the whole slope) | **0.0%** | 0.395 / 0.463 (cloudShadows=0: 0.463 / 0.463) |
| `clear_high_s` (Abel, 700 m up, noon) | overcast 0 | **4.0%** (four patches) | **0.0%** | 0.471 / 0.470 |
| `coastline_sunset` (Abel, sun 8 deg) | overcast 0 | **1.7%** (cuts the sun path on the sea) | **0.0%** | 0.528 / 0.527 |
| `clear_high_n` (Abel, 700 m up, noon) | overcast 0 | 0.2% (no patch on the ground in this view) | 0.0% | 0.505 / 0.502 |
| `light_high_n` (Abel, 700 m up) | overcast 0.1 | 1.3% (distant patches) | 1.1% (still there) | 0.510 / 0.510 |
| `broken_high` (Eden, 250 m up) | overcast 0.5 | 27.7% | 26.4% (drift moved a little) | 0.459 / 0.459 |
| `overcast_broken` (Eden, eye level) | overcast 0.5 | dark share of ground 6.5% | 6.5% | 0.486 / 0.486 |
| `overcast_heavy` (Eden, eye level) | overcast 0.9 | dark share of ground 79.4% | 79.5% | 0.442 / 0.442 |

What the images show (looked at each one):
- `cmp_forest_edge_noon.png`: before, the sun-visibility view is uniformly 0.2 grey over the slope and trees (the camera sits under an invisible cloud, the same 74% and -14.5% frame luma the shade track measured in its run12); the remastered frame is dim and grey-green. After, the debug view is white and the remastered frame matches `cloudShadows=0` to 0.0001 luma.
- `cmp_coastline_sunset.png`: before, a cloud-shadow wedge cuts the right half of the sun path on the sea under a cloudless pink sky. After, the full sun path.
- `cmp_clear_high_s.png`: before, four soft dark patches on the far hills under a clear sky. After, none. (The darker green bands in the middle of both remastered frames are terrain texture: they are white in both debug views.)
- `cmp_clear_high_n.png`: no patch fell in this view in either run; after is a uniform 3.3% lower sun (the stock clear-sky dimming), frame luma -0.6%.
- `cmp_light_high_n.png`, `cmp_broken_high.png`, `cmp_overcast_broken.png`, `cmp_overcast_heavy.png`: cloud shadows at overcast 0.1, 0.5 and 0.9 are still there and the same size; frame luma equal to 3 decimals. The cloud field drifts with sim time, so patch positions move slightly between the two runs.
- `field_maps_cpu.png`: CPU reconstruction (not a capture) of the field over 12 x 12 km at the covers before and after (overcast 0: 4.3% shadowed before, 0 after; overcast 0.1: 9%; overcast 0.5: 30%).

A first BEFORE attempt (`before_run1_aborted/`) had no `triEndTest` at the end of the tests, so I stopped it (my own tri PID, then `stop-owned-games.ps1 -DataDir` on my junction) after its Abel shots. Those shots show the same bug at another drift position: `clear_high_s` 5.1% and `forest_edge_noon` 24% of the frame under cloud (`measure_before_run1_aborted.json`). The tests were then regenerated with `triEndTest` and both final runs completed.

## Overcast 0.5: why the cloud shadows are hard to see (design question, not changed)

Coverage is exact, so this is not a mapping bug: at overcast 0.5 the rig asks for cover 0.30, the engine's noise under the shader's soft threshold covers 30.1% of the ground (`noise_analysis.txt`, cover_check), the mean sun equals stock, and the new unit test pins it. The shadows exist in game (`broken_high`: 26 to 28% of the frame, 47 to 49% of the visible ground). What hides them is their scale and the eye-level composition. Numbers from a numpy port of `BuildCloudNoise` that matches the engine's C++ output on 65534 of 65536 texels (the other two differ by one level; `scripts/dumpnoise.cpp`):

- One 6 km tile, base lattice 4 cells, so 1.5 km cells. At cover 0.3 a point stays in shadow for 760 m and in sun for 1740 m on average (chord lengths); connected shadow regions are km-sized (area-weighted diameter about 4.9 km, because neighbouring blobs merge into bands; `field_maps_cpu.png`, right panel). Soft edges are about 120 m wide.
- An eye-level frame is dominated by the near ground. Weighting the visible ground by screen area, only 3% of random eye-level views have between 10% and 90% of their ground shadowed: the rest are almost entirely lit or almost entirely shadowed. A cloud edge within 400 m of the camera falls in the view cone in 42% of views. So a still usually shows either a uniformly lit frame with a few darker distant hills (which read as slope shading, `cmp_overcast_broken.png`) or a uniformly darker frame.
- The field repeats every 6 km: twice across a 12.8 km island, plainly visible from the air (`field_maps_cpu.png`).
- The field has no relation to the clouds painted on the sky dome (the clouds track's finding).

Options for the owner (measured on the same port, `noise_analysis.txt`):

| Option | Shadow / sun chord at cover 0.3 | Views with an edge within 400 m | Cost / risk |
|---|---|---|---|
| A. leave as is | 760 m / 1740 m | 42% | none |
| B. `BuildCloudNoise` base period 16, 4 octaves, same 6 km tile | 250 m / 570 m | 81% | no GPU cost, no extra repetition; smaller, busier patches; a look decision |
| C. `kCloudScale` 1/3000 (3 km tile) | 380 m / 870 m | 67% | doubles the visible repetition (4 tiles across an island) |
| D. the clouds track's sky-consistent shadows (MENU item 12) | follows the painted sky | | M effort, +0.2 to 0.9 ms; already sits on `rig.cloudCover`, so it inherits this fix (no shadow under a clear sky) |

Even B moves the eye-level "mixed frame" share only from 3% to 7%; at eye level, cloud shadows mostly show as change over time (drift) and on distant slopes, which is also how they look in reality. A separate lever is the cover itself: with transmission 0.2 the mean-preserving cover is 1 - skyThrough, so overcast 0.5 (the "oblacno" sky, which looks about half cloudy) shadows only 30% of the ground; a higher transmission under a cloud would allow a larger cover at the same mean. I left all of these for the owner.

## Risks

- **Clear-sky frames that were not under the invisible cloud get 3.3% less sun** than on the branch tip (frame luma -0.1% to -0.6% in the clear views). That is the stock Classic dimming at setOvercast 0 and keeps the frame mean equal to stock (the brief's rule); the frames that were under the cloud get up to +17% luma back. If the owner prefers a clear sky with an undimmed sun, set `overcastDim = 1` below `kClearSkyCover`; that would break mean parity with stock by +3.4% at overcast 0.
- **Fewer shadows between overcast 0.017 and 0.11** than on the tip (overcast 0.05: shadowed share 2.0% instead of 6.7%; lit gaps then get 0.96 of the sun instead of 1.0). Above overcast 0.11 nothing changes.
- **8-bit onset:** the noise is 8-bit with a plateau of texels at the top value, so as soon as the cover passes 0.05 about 0.3% of the ground (patches of about 100 m) is covered within a small overcast step, a fraction smaller than the requested cover would give. A minor pop during a slow weather change; the tip had the same kind of step, only at a cover (0.02) that `setOvercast` never reaches.
- **The `cloudShadows` knob** below 1 now dims the lit gaps to keep the mean; the dev overlay slider therefore reads as "shadow contrast at the same mean" instead of "less shadow, brighter frame".
- `kClearSkyCover` is tied to the hard-coded weather table in `Landscape.cpp`; if a future change moves the jasno keyframe to 0, the 0.05 floor would hide light cover up to overcast about 0.08. The comment and the unit test (`SkyThroughAtOvercast`) name the dependency.
- Not checked: night (the sun is off there anyway), multiplayer weather sync (the rig reads `SkyThrough()` as before).

## Files in this folder

- `fix.patch`: `git -C D:\Arma_CWA_worktrees\fix-clear-sky-clouds diff claude/youthful-lovelace-8imbdh` (no new tracked files).
- `cmp_*.png`: before | after sheets (remastered on top, sun visibility below) for the 8 views.
- `field_maps_cpu.png`: CPU maps of the cloud-shadow field (not captures).
- `before/`, `after/`: the capture runs (`shots/` holds every shot of the run, `tri/` tri's tree, `contact_sheet.png` and `*__pair.png` Classic | Remastered from `make_pairs.py`, `timings.*`, `udcap.log`, `run.txt`); `before_run1_aborted/`: the stopped first attempt.
- `measure_before.json`, `measure_after.json`, `measure_before_run1_aborted.json`: the shadow-share and luma numbers above.
- `unit_before_remaster.txt`, `unit_after.txt`: unit test output.
- `noise_analysis.txt`: the overcast-0.5 field statistics.
- `scripts/`: the analysis and comparison scripts; `tests/`: copies of the research capture tests.
- `configure.log`, `build.log`, `build2.log`, `before.run.log`, `after.run.log`, `before_run1_aborted.run.log`: logs.
