# Reference sets: vanilla upstream and landed graphics-remaster

Shot once (2026-10-19) so every later comparison reuses them. Nothing committed, no branch.

| side | engine | how it was shot |
|---|---|---|
| `vanilla/` | Bohemia upstream/main 3.05 (`ffc6183`), `D:\Arma_CWA_worktrees\vanilla-upstream`, build `build/vanilla-upstream-rwdi`, game `dist/vanilla-upstream-rwdi`; engine code untouched | vanilla-safe copies of the capture tests (below), junction `packages\RemasterUD-vanilla` |
| `landed/` | graphics-remaster `fec1075` as landed, game `D:\Arma_CWA_worktrees\graphics-remaster\arma_CWR\dist\graphics-remaster-rwdi`, `triSetLighting "remastered"` | the look-director test files (fec1075 + `fix/base.patch`) in the detached copy `D:\Arma_CWA_worktrees\look-ref`, junction `packages\RemasterUD-landed` |

Both sides ran on the same data: `D:\Arma_CWA\tmp\remaster-shadow-67` (for vanilla Sinai, see "LoBo" below).

## Files

- `vanilla/<view>.png`, `landed/<view>.png`: one file per view of `director/final` (50 names), each at its
  test's native size (960x640 for the run-captures views, 1920x1080 for look_final and optics). Views shot
  only at 1080p appear twice, `<view>.png` and `<view>__1080.png`, the same shot, as in `director/final`.
- `index.json`: per view the island, the source test in look-director, the reference test that shot it,
  the resolution, the vanilla and landed paths, the raw tri file each came from, and the overhaul
  "before" (`director/final/<view>__overhaul.png`). `meta` names the builds and the vanilla gamma.
- `contact.png`: vanilla | landed per view, 300 px a panel. `contact_framing.png` adds the look-director
  overhaul as a third column; framing matches in every row (checked by eye, all 50).
- `extra/vanilla_optics_mist_hud_onfoot.png`: vanilla's own on-foot HUD shot (see "Optics").
- `raw/`: tri output trees and logs of every run (`landed`, `vanilla_smoke` = Eden 960, `vanilla_main`,
  `vanilla_optics`, `vanilla_sinai`; `probe_lobo` = the failed @LoBo probe). The PNGs in `vanilla/` are
  the raw vanilla shots with the gamma pass applied; `landed/` PNGs are the raw files unchanged.
- `tools/make_ref_tests.py` (writes both test sets), `tools/collect_ref.py` (fills `vanilla/`, `landed/`,
  `index.json`, the sheets).

## Views and tests

| final names | look-director test | reference tests |
|---|---|---|
| lowsun_fp, sunrise_ridge, noon_fields, forest_edge_golden, coastline_sunset, town_night, firefight_1..3, tank_gunner_interior, tank_gunner_optics | `abel_lighting` (960x640, scale 1.5, 4x MSAA) | `ref_<side>_960_abel_a/b/c` |
| overcast_broken, overcast_heavy | `eden_lighting` | `ref_<side>_960_eden_a` |
| sinai_lowsun_fp, sinai_desert_noon | `sinai_lobo_lighting` | `ref_<side>_960_sinai_a` |
| the 15 Abel `*__1080` / 1080-only views | `look_final_abel` (1920x1080, scale 1.0, 4x MSAA) | `ref_<side>_1080_abel_a/b/c` |
| overcast_broken/heavy, rain_day, noon_vista (1080) | `look_final_eden` | `ref_<side>_1080_eden_a` |
| sinai_lowsun_fp, sinai_desert_noon (1080) | `look_final_sinai` | `ref_<side>_1080_sinai_a` |
| optics_mist_hud, optics_mist_sight, optics_mist_sight_f1 | `look_director_optics` (Eden) | `ref_<side>_optics` |

Landed tests are in `look-ref\arma_CWR\tests\rendering\captures\`, vanilla tests in
`vanilla-upstream\tests\rendering\captures\` (with the vanilla `capture_lib.sqf`, the three `missions\`
and a copy of `run-captures.ps1` that skips `make_pairs.py`).

How the tests were made (`make_ref_tests.py`):
- 1080p: `gen_look_director.py --modes "landed=remastered" --tag landed` (and `vanilla=vanilla` for the
  vanilla side), run over all views at once so each view keeps the sim clock of `look_final_*`
  (2000 + 600 s per view index), then cut into chunks of at most five views. No game ran longer than
  about a minute (landed: 11 games in 355 s).
- 960x640 and optics: the look-director test files with the Classic block of every view removed, so each
  view is shot once in the one mode (`"remastered" call capMode`, or the vanilla label).

## How the vanilla variant was made

The vanilla engine lacks these fork verbs (an unknown verb fails the whole script): `triPinCloudDrift`,
`triGpuTimes`, `triSettleScene`, `triSetLighting`, `triSceneStats`, `triPinObjectDetail`,
`triFeatureState`, `triCaptureAtPresent`, `triCamView` (`triSetFixedStep` is not used by these tests).
The vanilla `capture_lib.sqf` keeps every helper, global, camera rig, prop spawner and pin of the fork's,
with these changes:
- `capMode`: no detail pin, no lighting switch; waits `capSettleFrames` = 450 frames (`triSimFrames`)
  in place of `triSettleScene`, then `triPerfReset`. With the test's own 150 frames before and 90
  after, every vanilla shot waits about 690 frames on its view.
- `capReadTimers` / `capRecord`: log `triPerfStats` and `triFps` only.
- `capPinEnv`: `setDate; skipTime 0.01; setDate`. Vanilla `setDate` does not switch the auto street
  lamps (they wait for their 12 s tick); `skipTime` over 20 s calls `Landscape::OnTimeSkipped`, which
  switches them at once, as the fork's `setDate` does; the second `setDate` pins the date back. The
  night views have their lamps lit.
- Generated tests: the `triPinCloudDrift` lines and `triCaptureAtPresent true` are removed. Every other
  line (camera, date, weather, fog, rain, view distance, props, FOV, sim clock, window size, MSAA via
  `triSetMsaa`, render scale via `triSetRenderScale`) is unchanged; vanilla has all those verbs.
- `make_ref_tests.py` asserts that no fork-only verb is left in any vanilla file.

## Every difference from our captures

1. **Lighting and shadows.** Vanilla has one lighting model. It draws the default projected object and
   vehicle shadows (shadow tier High); shadow maps stay off, as vanilla offers no player setting for them
   (only the test verb `triEnableShadowMaps`, not called). Landed: `triSetLighting "remastered"` (every
   landed feature, shadow maps on).
2. **Capture point and gamma.** Vanilla `triScreenshot` reads the last rendered frame at once, before the
   gamma pass (`ApplyGammaPass`, `pow(c, 1/gamma)`). The collector applies that pass offline with the
   vanilla default gamma 1.2 (each tri test runs with a fresh config dir, so the default holds; it is
   also our build's default). Check: on the Eden 960 views the corrected vanilla means are within 1.5
   levels of our build's features-off presented shots (overcast_broken 112/127/124 against 112/126/124).
   The offline pass can differ from the GL pass by 1 level from rounding. Ours (`triCaptureAtPresent`)
   take the next presented frame, vanilla the frame before the statement: one frame apart, which shows
   only in moving things (firefight flashes, smoke, rain streaks).
3. **Optics frame mapping.** `look_director_optics` takes its shots inside a `forEach` block, where an
   at-present shot lands one frame late: the final set's `optics_mist_hud` is really the first GUNNER
   frame after the switch (the iron sight, sky not yet filled in), and `optics_mist_sight_f1` the second
   frame. Vanilla captures at once, so its `hud` shot is the true on-foot view. To show the same frame as
   the final set, vanilla `optics_mist_hud` is its `gun_f1` shot, `optics_mist_sight_f1` its `gun_f2`,
   `optics_mist_sight` its `gun_f30` (frame 32 against 33). The on-foot HUD shot is kept in `extra/`.
   Landed uses the fork's own labels.
4. **Object detail.** Vanilla detail stays adaptive (no `triPinObjectDetail`), and nothing proves the
   view finished loading (vanilla has no scene stats); the long wait stands in for it. By eye, tree lines,
   houses and far props are present in every vanilla view.
5. **Clouds.** No cloud pin in vanilla (the 1080p views pin the fork's dome clouds at their load
   position; vanilla has no cloud-shadow field). The sim clock is re-pinned before each shot on both
   sides; the vanilla cloud layout matches the landed one closely in the overcast and rain views.
6. **LoBo (Sinai).** Vanilla 3.05 cannot boot any mission with the shadow's `@LoBo` mounted, even the Eden
   one: a mod's `Bin\config.cpp` replaces the whole base config in vanilla, and that file in the shadow is
   UD's generated Guerrilla faction table (`install-lobo-factions.ps1`), written for the fork's deferred
   mod-config merge (log: `No class BulletBurstW`, `StartAutoTest could not boot`). The vanilla Sinai
   tests mount `D:\Arma_CWA_worktrees\vanilla-upstream\tmp\lobo-mods\@LoBo` instead: junctions to the
   shadow's own `@LoBo\addons` and `@LoBo\dta`, no `Bin` (so also no `Bin\RESOURCE.bin`, a UI resource).
   The faction table draws nothing. All four LoBo props spawn with the same classes on both sides
   (LoBoHMWV_Mag_Patrol, LoBo_Zelda, LoBoGolaniWB, LoBo_camnet), and Sinai loads its objects: vanilla
   3.05 already has the 4WVR `QIStream::seekg` fix.
   Remove those junctions with `cmd /c rmdir`, never `Remove-Item -Recurse` (it would empty the
   shadow's folders).
7. **Firefight.** AI fire, the shell bursts and the smoke differ run to run, as between the director's
   two modes.
8. **Timings.** No GPU timings, draw counts or feature states in the vanilla logs.
9. **Landed against the director's captures.** Each view is shot once, so the landed shot comes after a
   shorter stay on the view than the director's overhaul shot (which followed a Classic shot). Landed
   `town_night`, `firefight` and `sinai_lowsun_fp` (960) logged `UNSETTLED:300` (fire flicker, AI), which
   only means the draw count kept moving. The landed images are fec1075 with every landed feature; the
   base.patch engine changes are not in that game (only its test files were used).

Things vanilla could match after all: MSAA (`triSetMsaa`), render scale, window size, LoBo buildings.
Things it cannot: the presented-frame capture (done offline), the pinned object detail, the settle check,
the cloud pin, `setDate` switching lamps (done with `skipTime`), mounting the shadow's `@LoBo` as is.

## Rerunning

```powershell
python D:\Arma_CWA_worktrees\graphics-remaster\tmp\look\ref\tools\make_ref_tests.py   # rewrite both test sets
# landed (from look-ref):
powershell -NoProfile -File D:\Arma_CWA_worktrees\look-ref\arma_CWR\tests\rendering\captures\run-captures.ps1 -GameDir D:\Arma_CWA_worktrees\graphics-remaster\arma_CWR\dist\graphics-remaster-rwdi -DataJunction RemasterUD-landed -Tests <ref_landed_* tests> -OutDir <dir>
# vanilla:
powershell -NoProfile -File D:\Arma_CWA_worktrees\vanilla-upstream\tests\rendering\captures\run-captures.ps1 -GameDir D:\Arma_CWA_worktrees\vanilla-upstream\dist\vanilla-upstream-rwdi -DataJunction RemasterUD-vanilla -Tests <ref_vanilla_* tests> -OutDir <dir>
python D:\Arma_CWA_worktrees\graphics-remaster\tmp\look\ref\tools\collect_ref.py         # reads raw\landed and raw\vanilla*
```
