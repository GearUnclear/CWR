# Lighting captures: Classic | Remastered

A repeatable, scripted in-game photo run for the Remastered lighting work
(`engine/Poseidon/Graphics/Remaster/README.md`). Every view is shot twice with
the same camera, sim clock, date, weather and props, once with
`triSetLighting "classic"` and once with `triSetLighting "remastered"`, and the
GPU and CPU frame timings of each mode are logged next to the image.

These are photo generators, not regression tests: they assert nothing and live
outside `tests/integration/`, so a `tri test tests/integration` sweep never
picks them up. Run them explicitly.

## Files

| File | What it is |
|---|---|
| `abel_lighting.test.sqf` / `.toml` | Abel (Malden): the acceptance stand-in and most of the hero set, 22 shots |
| `eden_lighting.test.sqf` / `.toml` | Eden (Everon): broken and heavy overcast, 4 shots |
| `sinai_lobo_lighting.test.sqf` / `.toml` | Sinai with `@LoBo` mounted: low sun and desert noon, 4 shots |
| `capture_lib.sqf` | The shared rig, `#include`d by the three tests (helpers in globals `cap*`) |
| `missions/udcap_*.<World>/mission.sqm` | Minimal one-player missions the tests boot (`mission =` in each toml) |
| `run-captures.ps1` | Runs the set on the hidden desktop, cleans up its games, collects the output |
| `make_pairs.py` | Copies the captures out of tri's tree, builds the pairs, contact sheet and timing tables |
| `sunplan.py` | Port of `LightSun::Recalculate`: sun and moon position for an engine date and latitude |

## Preconditions

- A built game in `dist/lighting-rwdi` (or pass `-GameDir`).
- `packages/RemasterUD`: the gitignored junction to the setup-prepared full
  Remaster shadow. In this worktree:
  `cmd /c mklink /J "D:\Arma_CWA_worktrees\lighting\arma_CWR\packages\RemasterUD" "D:\Arma_CWA\tmp\remaster-shadow-67"`.
  The Sinai test mounts the shadow's own `@LoBo` copy (`--mods-dir . --mod @LoBo`,
  the same as the `remaster_ud` Sinai tests), which carries the mod doctor repairs.
- `tri.exe`: the main checkout's `D:\Arma_CWA\arma_CWR\target\debug\tri.exe`
  (the runner is unchanged on this branch).
- Python 3 with Pillow on `PATH` as `python` (or pass `-Python`).

## Running

From `arma_CWR` (PowerShell):

```powershell
powershell -NoProfile -File tests\rendering\captures\run-captures.ps1
# one island only:
powershell -NoProfile -File tests\rendering\captures\run-captures.ps1 -Tests tests/rendering/captures/eden_lighting.test.sqf
# rebuild pairs and tables from an earlier run, no game launched:
powershell -NoProfile -File tests\rendering\captures\run-captures.ps1 -CollectOnly -OutDir ..\tmp\captures\20261017-140000
```

The script runs `tri test --retries 1 -j1 --game-dir dist/lighting-rwdi
--output-dir <out>\tri <tests>` through `D:\Arma_CWA\tmp\tri-hidden-desktop.ps1`
(a private desktop, so no game window takes the owner's focus; `-Visible` runs
tri directly instead). Afterwards it always calls
`D:\Arma_CWA\arma_CWR\tools\stop-owned-games.ps1 -DataDir <arma_CWR>\packages\RemasterUD`:
tri starts every game with `-C <that junction path>`, so only this worktree's
games match (never a global `Stop-Process`). Paths in the tri command line must
not contain spaces (the wrapper passes it through `cmd /c`).

## Output

Everything lands in `D:\Arma_CWA_worktrees\lighting\tmp\captures\<yyyyMMdd-HHmmss>\`
(the git root ignores `/tmp/`):

| File | Content |
|---|---|
| `<view>__classic.png`, `<view>__remastered.png` | The two captures, 960x640 |
| `<view>__pair.png` | Classic left, Remastered right, labelled, GPU frame time in the corner |
| `contact_sheet.png` | Every pair on one page |
| `timings.csv`, `timings.json` | One row per shot: GPU frame / world / shadow / post / ao / bloom / shafts / composite (ms), CPU fps and frame ms, `triSetLighting` answer, active camera |
| `timings.md` | The same as a table, with a Remastered minus Classic row per view |
| `udcap.log` | Every raw `UDCAP;...` log line and the horizon-map build line |
| `tri.log`, `tri\` | tri's console output and its own output tree (game logs, raw captures; the BMP twins are deleted) |

## How a pair is shot

Per view (`capBegin`) the test sets the camera, date, weather and view
distance, spawns the props, and lets 150 frames pass so terrain and textures
stream in. Then, for Classic and again for Remastered:

1. `capMode`: `triSetLighting "<mode>"` (Remastered also turns the shadow maps
   on, Classic turns them off, as the Options row does), then `triGpuTimes
   false` / `true` and `triPerfReset`, which restart the GPU and CPU averages.
2. `triSimFrames 90`: shaders, shadow maps and the timers settle. GPU results
   trail the frame by a few frames and are an exponential average with weight
   0.1, so 90 frames leave well under 1% of anything earlier in them.
3. `capReadTimers`: `triGpuTimes true` (the summary string, read while timing
   is still on; `false` would answer "gpu: timing off") and `triPerfStats 0`.
4. `capPin`: the same `setDate`, `triCheatWeather` + `setOvercast` / `setFog`
   / `setRain 0`, view distance, prop positions, `triSetView` and
   `triSetSimTime` as the other mode. Re-pinning the sim clock puts water,
   sky and the remastered cloud-shadow drift (keyed on `Glob.time`) back where
   they were. It moves `time` back a few seconds for the second shot; nothing
   in these static scenes depends on that.
5. `triSimFrames 3`, then `triScreenshot "<view>__<mode>"` and `capRecord`.

Settings shared by every shot: 960x640 windowed (`--display-mode windowed
--width 960 --height 640`), `triSetMsaa 4`, `triSetRenderScale 1.5`, and
`triCaptureAtPresent true`, so the image is the presented frame after the gamma
pass (the default `triScreenshot` reads the frame before gamma, darker than what
a player sees). The remastered exposure meter is a pure function of the sun
height (`LightRig.cpp`) and the classic eye accommodation is instant, so there
is no adaptation to wait for between the two modes.

Framing uses the `tools/screenshots` rig: a cutscene camera exists only to hide
the HUD and give `camSetFov` a target, and `triSetView [east, up, north, dir]`
sets the exact eye point; the eye height comes from a ground probe (an invisible
`HeliHEmpty` at `[x, z, 0]`, read back with `getPosASL`), so no terrain height
is hard-coded. Props are placed relative to the view origin and heading
(`capSpawn`, `[forward, right]` in metres). A class that does not exist in the
merged config is skipped and logged (`event=skip_spawn`), a spawn that fails is
logged (`event=spawn_failed`), and the view is still shot. Every tri verb that
can answer `FAIL:` is called inside a helper and its answer logged, because a
top-level `FAIL:` result aborts the whole test.

## Views

Sun and moon numbers come from `sunplan.py` (`python sunplan.py 1985 7 5 18 15`),
a port of the engine's own sun model; they have not been checked in game yet.
Headings: 0 = north, clockwise. Origins are `[east, north]` in map metres.

| View | World | Origin | Eye m / heading / pitch | Date, time | Sun elev / az | Weather | In frame |
|---|---|---|---|---|---|---|---|
| `lowsun_fp` | Abel | 7300, 10900 | 1.75 / 152.5 / -3 | 1985-03-01 08:00 | 12.7 / 117.5 (35 deg left) | clear | **Acceptance stand-in** for `LightingLowSun.Demo`: Jeep 10 m, M113 16 m, standing SoldierWB 7 m, camouflage-net stand-in 22 m, each turned to show a sunlit and a shaded face; open flat ground on north Malden |
| `sunrise_ridge` | Abel | 4400, 3300 | 1.75 / 60 / +2 | 1985-07-05 05:08 | 5.1 / 64 | clear, VD 4000 | Shoulder of the central massif 2 km away (4.2 deg); the sun has just cleared it, 4 deg right of centre |
| `noon_fields` | Abel | 6250, 10800 | 1.75 / 150 / -2 | 1985-07-05 12:30 | 71.9 / 202 | clear, VD 2500 | Open upland fields, tree lines at 150-800 m; Jeep 13 m and soldier 8 m for noon shadows |
| `forest_edge_golden` | Abel | 4674, 10156 | 1.75 / 75 / +3 | 1985-07-05 18:15 | 12.0 / 290 (behind left) | clear | Wooded slope rising 40-500 m east, lit across open sea; soldier facing the sun at 12 m |
| `coastline_sunset` | Abel | 1550, 5150 | 3 / 278 / -4 | 1985-07-05 18:35 | 8.4 / 293 (15 deg right) | clear, VD 4000 | West-coast shore, surf, 4 km of open sea and the sun path on the water |
| `town_night` | Abel | 7152, 8915 | 1.75 / 345 / +3 | 1985-02-27 22:00 | sun -50; moon 68.7 / 183, intensity 1 | clear, VD 800 | Saint Louis: the six street lamps (`lampazel`) of the town centre 30-90 m ahead, an inflamed campfire (`Fire`) at 9 m, moonlight from behind |
| `firefight_1..3` | Abel | 7600, 10850 | 2 / 0 / 0 | 1985-07-05 19:35 | -1.7 / 302 (dusk) | clear | **Not matched.** 4 SoldierWB and 3 SoldierEB 22-28 m apart across the frame, captive and undamageable, each ordered to `doFire` a man of the other line; a burning Ural behind the east line; a 120 mm shell (`createShell "Shell120"`) dropped 30+ m behind the lines before shots 1 and 2. The set is staged afresh for each mode; AI fire and the shell timing differ between the modes |
| `tank_gunner_interior` | Abel | 7300, 10900 | player in the M60 gunner seat, INTERNAL camera | 1985-07-05 10:00 | 59.3 / 114.6 | clear | The vehicle interior (the remastered post pass gets no world depth here: no AO or shafts) |
| `tank_gunner_optics` | Abel | same | same seat, GUNNER camera | same | same | clear | The gunner optic over an M113 at 70 m, a Jeep at 55 m and a soldier at 50 m (the overlay turns bloom and shafts off) |
| `overcast_broken` | Eden | 9200, 3700 | 1.75 / 288 / -4 | 1985-07-05 14:30 | 54.3 / 253 | overcast 0.5, VD 3500 | SE Everon upland (271 m): a broad grassy valley with wooded hills at about eye level 1-3 km away; Jeep 12 m. Drifting cloud shadows expected in Remastered |
| `overcast_heavy` | Eden | same | same | same | same | overcast 0.9, rain 0 | Same frame, flat light |
| `sinai_lowsun_fp` | Sinai + @LoBo | 8350, 2950 | 1.75 / 338.8 / -3 | 2005-07-21 16:10 | 12.2 / 303.8 (35 deg left) | clear, VD 2000 | The acceptance set on @LoBo content at the Guerrilla.Sinai camp ground: HMMWV (`LoBoHMWV_Mag_Patrol`), M113 Zelda, Golani rifleman, `LoBo_camnet` |
| `sinai_desert_noon` | Sinai + @LoBo | 7300, 10300 | 1.75 / 210 / +1 | 2005-07-21 12:15 | 38.8 / 357 (behind) | clear, VD 4000 | Desert plateau (132 m), mountains 2.4 km away; HMMWV 14 m, rifleman 8 m |

Notes on the choices:

- **Acceptance stand-in.** The owner's `LightingLowSun.Demo` was on the `demo`
  world, which the Remaster shadow does not have. `lowsun_fp` rebuilds it on
  Abel. The Remaster's vanilla data ships no camouflage-net class (CONFIG.BIN,
  AddOns and DTA searched), so the net's candidate list is `LoBo_camnet`
  (@LoBo, `LoBofightpos.pbo`), then `CampEast`, then `Camp`: on Abel without
  mods it is the Soviet tent, on Sinai it is the real net.
- **Sun on Sinai.** @LoBo's `CfgWorlds >> sinai >> latitude = 29.39`, but the
  engine's convention is the opposite sign (`DefaultWorld` -40 is 40 deg north,
  `WorldInit.cpp`, `Lights.cpp`), so the engine puts Sinai at 29.4 deg south and
  the noon sun in the north. The Sinai times and headings follow what the
  engine does.
- **How the spots were found.** `PoseidonTools terrain objects <wrp>` (columns
  `[east, elevation, north]`) for trees, lamps and towns, and the heightmap
  from `PoseidonTools terrain render`, whose colour ramp
  (`Asset/Probes/AssetPreview.cpp` `PreviewTerrain`) inverts back to metres
  (checked against road objects: median 1.4 m above the decoded ground on Abel,
  0.9 m on Sinai). Each spot was then scored for slope, nearby objects, the
  horizon angle along the sun's azimuth and what lies along the view heading.
  `sunplan.py --scan <y> <m> <d> [lat]` lists a day's sun path for re-planning.

## Log lines

Every helper writes `UDCAP;view=...;event=...` at INFO through `logInfo`, which
tri saves in the test's `game_stdout.log`. Events: `setup`, `begin` / `view`
(camera, date, weather, ground-probe and `triSetView` answers), `spawn`,
`skip_spawn`, `spawn_failed`, `mode` (the `triSetLighting` answer), `shot`
(one per image: label, mode, lighting answer, `triCamView`, `triFps`,
`triPerfStats`, and last the `triGpuTimes` summary), `squads`, `tank`, `done`.
