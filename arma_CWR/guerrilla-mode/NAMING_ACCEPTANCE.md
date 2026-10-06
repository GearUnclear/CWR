# Naming feature: issue #57 acceptance

Audit date: 2026-09-24. Source requirement: [Naming System: Friendly and Enemy Legends](https://github.com/GearUnclear/CWR/issues/57).

**Status: source tests verified; local runtime acceptance failed on 2026-10-05.**
[PR #61](https://github.com/GearUnclear/CWR/pull/61) merged the original registry, dossier and enemy commander implementation.
The audit fixes are in [draft PR #64](https://github.com/GearUnclear/CWR/pull/64).
This audit checks that implementation against the original issue rather than
treating the merge or the older design notes as proof of completion.

## Requirements and evidence

| Original requirement | Current implementation and verification | Remaining acceptance |
| --- | --- | --- |
| Prefix, regional first name, describer, regional surname, title in that order; optional nickname slots | `LegendNames` assembles all five slots and preserves the issue's quoted describer and title capitalization. Its unit cases verify formatting, all bank entries and deterministic selection. | None for string assembly. |
| Use the attached name bank | The downloaded `test.json` matches `tests/fixtures/legend-names/issue57-names.json` as parsed JSON. `gen_legend_names.py --check` verifies compiled tables against it. | None for the bank transcription. |
| Regional personal names | New companion rows draw both personal names from the resistance region. The all-pools registry regression checks membership for both slots across all 33 pools. Script roster keys remain stable; loaded resolved identities stay unchanged. | Classic naming lane passed; exercise additional resistance choices and LoBo in game. |
| Earn one nickname at a level threshold; max level adds a different slot and Legend status | SERGEANT / 250 XP earns the first slot; COLONEL / 1900 XP earns the second. Tests cover both thresholds, a jump across both, repeat observations, and save/load award latches. | Classic live and save/load naming lanes passed on 2026-10-05. |
| Earning a name feels like an event | The native poll sends the resolved award text to the normal hint UI and writes an attributed journal entry. Tests verify both announcements, ordinary promotion announcements, and silence on repeated observations. The script no longer overwrites promotion/award hints. | Observe/capture both award hints at normal gameplay speed. |
| Names recur in game and persist after death in records/memorials | `BindRow` sets the live body's name; journal gathering and the persisted companion status line read the same registry display name. Registry tests cover awards, death, retained deeds, reload and correction of a stale startup roster without another character event; journal compose tests cover rosters, memorials and records keyed by character id. The naming integration lane now checks the real memorial and fallen dossier. | Classic content assertions passed; visual acceptance is blocked by the rendering defects recorded below. |
| Three dangerous enemy Legends each playthrough | Three distinct identities; sniper, elite commander and tank commander where faction assets permit. Skill is 0.9, with guards/crew. Placement now recovers a feasible triple when preferred stands block the other two. Failed creations with no surviving actors retry after 30 ticks at their original stand. Tests cover roles, feasible placement, pending spawn persistence and no duplicate defeat. | Run `scripting/guerrilla_legends_place` for supported Classic/LoBo campaigns; exercise temporary group exhaustion. |
| Enemy personal names come from the supplied modern Western male bank; hostile nicknames | Enemy rows use the attachment's `western_evil` regions: western, british or israeli. Other faction regions fall back to western. All-pools tests verify enemy personal-name and hostile-word membership. | None for selection; observe final labels in game. |
| Fixed positions, separate from reinforcement/QRF, identifiable and killable | Named bosses and tank crew have movement disabled while retaining targeting/fire. Their own groups are not registered with garrison or QRF state. Existing integration lanes check displacement under fire, group isolation, combat and defeat; source tests cover placement, persistent positions and defeat latches. | Classic placement and save/load passed; defeat lane failed the commander displacement check. LoBo remains pending. |
| Names and roles remain visible on the campaign map | Live labels contain name and role; defeated labels now retain both plus `(defeated)`. The marker regression exercises the actual defeat repaint and persisted identity/role. Integration expectations were updated. | Inspect live and defeated map markers before and after full game reload. |

## Source verification: Linux audit, 2026-09-24

- Linux `PoseidonGame` and `PoseidonTests` build successfully with Clang.
- `PoseidonTests '[legends],[journal],[guerrilla]' --reporter compact`: **412 cases, 3,954,128 assertions passed**.
- The new placement regression failed before the fix: one commander placed where three legal stands existed. It passes with the fallback search.
- `python3 -m unittest discover -s arma_CWR/tests/contracts -v`: **9 cases passed**.
- `python3 arma_CWR/tools/legend-names/gen_legend_names.py --check`: passed.
- Formatting checks on changed C++ files and `git diff --check`: passed.
- Roadmap validation passes with two existing, unrelated dependency warnings.

Several older integration lanes put assertions inside loops whose return values
were discarded by Trident. The affected naming/save lanes now collect failures
and assert their aggregate at the top level. Prior green runs of those lines
are not treated as acceptance evidence.

The 2026-10-05 review also added sequence-level `.seq.toml` metadata for both
Legend save/load sequences. Phase tags alone do not participate in discovery.
The `legend_sequences_have_graphics_tags` Trident regression verifies that
`headful`, `full_cwa` and `save-load` include both sequences and that skipping
`headful` excludes them; the installed CLI exclusion check also passed.
Rendered execution still requires `--render gl33`.

## Local runtime verification: Windows, 2026-10-05

The local checkout combined current main, PR #63, PR #64 and the civilian
assailant completion fixes. It used a freshly built `PoseidonGame`, installed
shared `gmcore`, Classic 1.99 data and the GL33 renderer on an NVIDIA RTX 4060 Ti.
Trident used separate temporary profiles and no automatic retries. These
results are from Classic; they do not establish LoBo naming acceptance.

**Seven of eight naming/journal selectors passed.** A sequence counts as one
selector and runs its save and reload phases in separate game processes.

| Selector under `tests/integration/` | Result | Elapsed |
| --- | --- | --- |
| `scripting/guerrilla_legend_names.test.sqf` | PASS: regional identity, two awards, live name, attributed records and fallen dossier | 25.0 s |
| `scripting/guerrilla_legend_save.seq` | PASS: names, award latches, history and portrait rows after save/load | 23.6 s |
| `scripting/guerrilla_legends_place.test.sqf` | PASS: three commanders placed and spawned | 20.1 s |
| `scripting/guerrilla_legends_defeat.test.sqf` | FAIL: pinned commander's displacement was 3.60759 m; required less than 3 m | 308.0 s |
| `scripting/guerrilla_legends_save_reload.seq` | PASS: enemy identities, positions and defeat state after save/load | 79.1 s |
| `scripting/guerrilla_journal_pages.test.sqf` | PASS: journal content and navigation assertions | 7.2 s |
| `ui/guerrilla_journal_capture_800.test.sqf` | PASS: 800x600 layout assertions and ten captures | 8.6 s |
| `ui/guerrilla_journal_capture_1440.test.sqf` | PASS: 1440x1080 layout assertions and ten captures | 9.1 s |

The defeat failure was the first stationarity assertion after the 60-second
combat window, `triAssertLt [glPinMoved, 3]`. That run used campaign seed
`42205185`; its foot commander stood near Outpost at `[6777,5825]` with four
guards. The subsequent tank-destruction and defeat-latch checks in that
selector were not reached. No retry or threshold change was used to turn this
result green.

A separate 60-second diagnostic kept the commander's movement-disabled flag
(`DAMove`) set while recording abrupt displacement at more than 19 km/h.
Combat collision impulses can still move a stationary, invulnerable body:
the collision and soldier-force code applies those impulses independently of
damage and the AI movement flag. This narrows the failure to physical
stationarity under combat; it does not establish that the AI flag was lost or
that the commander deliberately walked away. The diagnostic ended after its
measurement window and is not a replacement passing run. Its log is
`tmp/acceptance-20261005-pin-probe/legend_pin_probe/game_stdout.log`.

All twenty journal captures were also inspected visually. Their passing
layout assertions do **not** establish a usable rendered journal:

- At both resolutions, the notebook's opaque page background is absent.
  Map contours, grid lines and marker labels remain visible through the
  prose, making the pages difficult to read.
- Both dossier captures contain a pale, untextured character figure rather
  than a textured face and outfit. A ready portrait row alone does not verify
  the rendered appearance.
- The separate `ui/guerrilla_human_visual` lane passed its UI assertions, but
  all four inspected captures also show pale, translucent or untextured scene
  geometry and weapon models. The visible problem therefore extends beyond
  the journal portrait. This combined-checkout run does not isolate which
  change introduced it.

Local evidence, relative to `arma_CWR/` (generated artifacts, not tracked):

- `tmp/acceptance-20261005-naming-rendered/guerrilla_legend_names/runner.log`
- `tmp/acceptance-20261005-naming-rendered-rest/results.json` and each selector's
  `runner.log`, `game_stdout.log` and `game_stderr.log`
- `tmp/acceptance-20261005-naming-rendered-rest/guerrilla_journal_capture_800/000_c800_contents.png`
- `tmp/acceptance-20261005-naming-rendered-rest/guerrilla_journal_capture_1440/007_c1440_dossier.png`
- `tmp/acceptance-20261005-acceptance-rendered/guerrilla_human_visual/002_03_human_step.png`

The first naming test passed before sandbox permissions prevented the runner's
process-inspection cleanup. The remaining seven selectors ran with that
permission under the separate `naming-rendered-rest` output directory; the
successful first test was not rerun.

## Rendering build repair — 2026-10-05

The rendering defect above was isolated to the local incremental build. Four
August 21 engine objects had no recorded header dependencies and survived the
September 17 change to the engine virtual interface. The stale drawing code
called `EndInstancedRun` instead of `PrepareTriangleTL`, skipping texture and
blending setup. A preserved September 17 executable rendered the same current
Classic assets correctly, including the notebook and portrait.

An isolated C/C++ reproduction confirmed that ccache 4.13.6 loses CMake 4.3's
forwarded clang-cl depfiles on cache hits. The build now uses `/showIncludes`
with Ninja's MSVC dependency parser. The standalone regression at
`tests/ci/test_clang_cl_dependencies.py` verifies real cache hits retain headers
and header edits rebuild C, skipped-PCH C++, and PCH C++ objects.

The current source, including the existing uncommitted campaign work, was
rebuilt with caching enabled. All seven formerly empty dependency records
(four engine objects plus three tool/test objects) now contain headers.
The repaired staged `PoseidonGame.exe` SHA-256 is
`996628636756FC504ED65E9C3D5DC1061AA4699A4FEF4B3487E858B38FF6D9AB`.

Visual inspection confirms restored world and weapon textures, foliage cutouts,
notebook paper and instruments, and textured dossier portraits. The exact
world/journal probes, `guerrilla_portrait_dossier`, both journal capture
resolutions (800x600 and 1440x1080), and `guerrilla_human_visual` passed.
The ordinary rendering/graphics/journal/Legends unit selection passed all
510 cases / 3,958,235 assertions. An initial broader invocation hit sandbox
restrictions on the documented `D:\tmp` outputs and included a hidden DDS
fixture-generation case with a mismatched dimension expectation; the ordinary
selection excludes hidden fixture generators and ran with temporary-file access.

Evidence is under `tmp/render-diagnosis-20261005/`: `repair/full-build.log`,
`repair/ninja-deps-final.txt`, `repair/unit-tests-ordinary.log`,
`post-fix-regressions/`, and the separately logged `final-world/`,
`final-journal/`, `final-journal-1440/`, and `final-human-visual/` captures.
The pre-repair patch, file hashes, executable and stale objects are retained
under `repair/`. Normal user saves, settings and caches were untouched.

## Remaining runtime gate

The original Linux audit had no Classic 1.99 or LoBo game-data installation;
the Windows results above now supply partial runtime evidence. Acceptance
still requires resolving and rerunning the commander displacement failure
and exercising the naming lanes on supported LoBo campaigns. Rendering is
repaired and visually verified above. Both nickname notices still need captures
at normal gameplay speed. Recheck roster, memorial and all three enemy labels
at 800x600 and 1440x1080 with the repaired build.

Keep issue #57 and the roadmap item open until those results are recorded.
Do not infer gameplay acceptance from the source-test totals.

## Compatibility and limits

Existing saves keep resolved names, awards and history; they are not renamed
to match the new selection policy. Saves predating the Legend registry retain
the existing derived-record behavior. A custom island with fewer than three
legal terrain positions still reports the placement shortage; the fallback
search does not put commanders underwater or inside the player's camp.
Unsupported/missing faction assets still use the existing role fallback.
These limits must be distinguished from passing the three-commander runtime
gate on supported campaign setups.
