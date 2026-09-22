# Local dossier portraits

Implemented on PR 61 head `113e68d98528a7ffe429223b863da69f726abfb0`, in the
isolated `codex/local-dossier-portraits` worktree. No changes to identity rolls,
save schemas, or multiplayer payloads; no generated artwork is distributed.

[The acceptance index](PORTRAIT_ACCEPTANCE.md) maps every supplied plan bullet
to its implementation and verification evidence.

## Three review stages

1. **Isolated renderer** (`61921a0`): shared face resolution, private animated
   geometry and proxies, offscreen capture and native print treatment.
2. **Cache and loading** (`5e9e97e`): selected-faction roster, dependency-addressed
   cache, queue, new/save-load gate and lifecycle cleanup.
3. **Journal and delivery**: opaque portrait handles, texture-backed HTML,
   revision refresh, cache-clear setting, regression coverage and these notes.

## Rendering and identity

`LegendAppearance` is shared with `LegendRegistry::BindRow`. Valid saved face
names survive, including names outside the four identity-roll faces. The roster
uses the selected resistance's companion and civilian classes, and the selected
occupier's normal commander/sniper resolver at every progression threshold.
Recorded appearances, including fallen characters, are added and deduplicated.
If a saved face has disappeared, its exact appearance completes as unavailable;
the fallback for a living character is prepared separately. A fallen dossier
never substitutes another identity's photograph.
Unrelated installed units are not enumerated.

`PortraitRenderer` constructs `Man(type, false, Creation::AppearanceOnly)` directly.
Appearance-only construction skips supply cargo and unused weapon cloudlet shapes;
normal character construction keeps its existing defaults. The mannequin has no
AI brain, group, world registration, equipment, script
initialization, simulation ticks or assigned network identity. It binds the real
face, verifies a personality texture section, and uses `EffectStandStill` plus
the neutral mimic. Unsupported binding fails explicitly.

Normal asset loading can instantiate visual proxies such as flag cloth, whose
constructors draw random phases. Preparation and capture use a fixed private RNG
through a scoped, thread-local override, so those asset operations cannot consume
campaign randomness. Source tests verify nested scopes, exception restoration and
thread isolation; live checks cover every selected Classic and @LoBo appearance.

Prepared model geometry and proxy objects are copied privately, including the
animation originals and skeleton data; unique render buffers are rebuilt.
Visible posed geometry and visible attachments determine head clearance and
chest-up framing. The capture uses LOD 0, a dedicated 512-square colour/depth
framebuffer, fixed camera/light/exposure and a neutral clear colour. Sun, fog,
night vision and player gamma do not affect the capture. Night-eye adaptation,
pixel shader mode and fog colour are fixed too: a time/weather pixel comparison
exposed a rounding dependency even with fog disabled. Scene, camera, light
references, render pass, framebuffer, viewport and pixel-readback state are
restored. The LightList copy fix preserves reference ownership during restoration.

`PortraitRecipe` version `dossier-10` performs the muted colour treatment and
Lanczos-3 reduction in C++, producing an opaque 256-square PNG containing the
236-square photograph and paper border. Players need no external tools.

## Cache and loading

`PortraitService` owns queued/generating/ready/unavailable states on the graphics
thread. Requests deduplicate; an open dossier raises its request's priority.
Each completion changes a revision. Composition carries only an opaque identifier
and status; rendering resolves a texture through `CHTMLContainer::AddImage(Texture*)`.
The existing section is retained when a completed image refreshes the journal.

Before new or restored campaign play, a responsive loading loop prepares the
whole roster and displays `Preparing dossier photographs... X / Y`. Cache hits
count toward completion. Escape, close or quit cancels pending work, retaining
completed disk entries. Individual failures terminate and log once per appearance
in the campaign session. Dedicated and dummy-renderer launches skip preparation.
Unexpected later appearances use the same queue, even after the map is closed.

Entries live under `GamePaths::CacheDir()/portraits/`, including when the cache
directory override is used. Keys hash the resolved body/face, flattened inherited
appearance/animation/material configuration, recipe version, and resolved model,
texture, rendered-proxy and pose file content. Normal archive/loose-file precedence
is re-resolved before accepting hits; file digests are memoized for the campaign
session and cleared on teardown. The manifest records the dependencies hashed.

Publication renames a unique temporary directory containing both complete files.
Readers accept only matching metadata and a successfully decoded, opaque 256-square
PNG; invalid headers and oversized files are rejected before decoding. Corrupt
entries regenerate, abandoned temporary directories are ignored, and concurrent
writers expose a complete entry or a cache miss. Disk failure retains the CPU
pixels and permits a session-only GPU texture.

Graphics reset/remount drops service texture handles; retained CPU images rebuild
textures on demand. Campaign teardown cancels work and drops renderer/CPU/GPU state.
Disk entries persist until **Game settings: Clear dossier portrait cache**.

## Measured Windows acceptance (2026-09-17)

Intel Core i7-14700KF, NVIDIA GeForce RTX 4060 Ti, Windows; Clang 22.1.8,
RelWithDebInfo, GL33; installed player-provided Classic 1.99 and
@LoBo assets. Classic selected GUER/EAST: 16 appearances. @LoBo selected PLO/IDF:
12 appearances, using the tracked third-party faction descriptor and local content
fixup. The source tree and installed script core contained no generated catalogue.

Preparation time is the engine loading-loop measurement. Test wall time includes
startup, scripted dossier interaction, isolation captures of the whole roster,
weather/time comparison, hard graphics-resource recreation, cancellation,
placeholder refresh, screenshot and shutdown. Peak memory is the whole game process's
sampled peak working set, not the portrait service alone. Warm runs reused an
unchanged cache from the corresponding cold run. These are observations on this
machine, not startup guarantees.

| Run | Resolution / gamma | Renders / hits | Preparation ms | Test seconds | Peak MiB | Rewrites |
| --- | --- | --- | --- | --- | --- | --- |
| classic-cold | 800 x 600 / 1.2 | 16 / 0 | 823 | 10.71 | 364.0 | 0 |
| classic-warm | 2560 x 1440 / 1.8 | 0 / 16 | 663 | 10.67 | 447.5 | 0 |
| classic-large-cold | 2560 x 1440 / 1.8 | 16 / 0 | 823 | 11.18 | 439.2 | 0 |
| lobo-cold | 800 x 600 / 1.2 | 12 / 0 | 1033 | 12.22 | 464.2 | 0 |
| lobo-warm | 2560 x 1440 / 1.8 | 0 / 12 | 912 | 12.20 | 540.4 | 0 |
| lobo-large-cold | 2560 x 1440 / 1.8 | 12 / 0 | 1029 | 12.20 | 536.6 | 0 |

Fresh cold captures at 800x600/gamma 1.2 and 2560x1440/gamma 1.8 were byte-identical
for all 16 Classic and all 12 @LoBo PNGs. Actual dossier screenshots were inspected
at both sizes: chest-up framing, covered face, readable photograph, biography and
footer within the page. Generated screenshots remain local and are not committed.

Additional acceptance:

- Texture override via a local authored PNG: 4 affected renders, 12 hits. Changing
  that PNG's bytes at the same mounted path: again 4 renders, 12 hits.
- One damaged PNG plus an abandoned temporary directory: 1 render, 15 hits,
  exactly 1 rewritten image. Bumping recipe 8 to 9 against an existing cache
  produced 16 renders and zero hits, while preserving old entries.
- Two game processes sharing one initially empty cache: both dossier lanes pass.
- Buffered Escape during loading: preparation cancels at one completed entry,
  preserves that entry, then resumes to all 16 using cache hits and zero rewrites.
  The loading loop explicitly processes keyboard edges while simulation is paused.
- Texture-bank reset/remount and full hard graphics-resource reset: photograph
  identity and current section survive; real GPU textures rebuild from retained
  pixels. The test also verifies placeholder-to-ready refresh on the same section.
- Unsupported `Default` face: terminal completion without a render; a repeated
  request deduplicates. Source journal tests cover the unavailable placeholder.
- Dummy-renderer campaign: gameplay ready, zero initial requests/renders/files;
  an explicit late request terminates unavailable without loading a mannequin.
- An unwritable cache root: all 16 portraits ready in memory, dossier and GPU
  texture recreation pass, zero image files written.
- Installer run against an isolated data directory with a populated source
  portrait directory: generated marker excluded, all 16 external cached images
  retained. A Git source archive also contains no portrait directory or temporary
  captures. Helmeted @LoBo commander dossiers inspected at both resolutions.
- Direct synchronous capture of every selected appearance and an initially
  unprepared civilian type preserves copied RNG output, serialized registry,
  AI groups/unit counts/network IDs, world entity counts, next magazine ID,
  network session state, time, lights, weather and camera/main-light state.
- A late appearance completes before any map is opened, then a normally spawned
  recruit uses it in the dossier. The actual Game settings cache-clear action
  empties the service and disk cache; the next request renders with zero hits.
- Source tests exercise cancellation retaining completed work, filesystem write
  failure retaining pixels, prioritization, corrupt reads, concurrent publication,
  exceptions, teardown, saved-face/gender fallback and selected progression rosters.

## Regression commands and results

Windows game and unit targets build with `win-x64-clang-rwdi`. The full source-only
unit run passes **2,920 cases / 4,005,489 assertions**. Existing tests use absolute
`/tmp` paths on Windows; allow that test scratch directory as well as `TMPDIR`.

```text
cmake --build build/win-x64-clang-rwdi --target PoseidonGame PoseidonTests
PoseidonTests "~[GameData] ~[gamescan] ~[external-data]" --reporter compact
python -m unittest discover -s tests/contracts -p "test_*.py"
```

All 8 Python source contracts pass. Integration lanes with installed Classic
assets: portrait dossier, portrait headless, late appearance/recruit, Game settings
cache clear, legend names, journal pages, legend
placement, legend defeat, legend save, legends save/reload, and outfit save/reload.
The defeat lane initially failed its existing NPC movement bound (7.43 m versus
3 m); both the unchanged PR executable and an isolated feature rerun passed.
No movement assertion was weakened. The final seven-lane audit passes in 134.9 s
(`build/portrait-audit-regressions-final.log`). Its three cross-process save/outfit
sequences compare every saved body, face and portrait ID, including fallen rows,
then check the actual loaded dossier texture and complete page bounds.

Linux **game and unit executables build successfully**, and the affected
`[portrait],[journal],[guerrilla]` suite passes **407 cases / 3,953,367 assertions**
on Ubuntu 24.04 under WSL with Clang 18.1.3. These tests use no game assets and
exercise the native Linux cache/filesystem implementation. One existing build
blocker was corrected: `V3RewriteEntry` cannot be `constexpr` when `UserAction`
is the non-literal Linux enum wrapper; its immutable table is now `const`.

The Linux run uses `linux-x64-clang-rwdi` with compiler launchers disabled,
the pinned vcpkg dependencies, a native build directory at
`/var/tmp/codex-dossier-engine-build`, and Linux-only `PATH`. Native tool/build
scratch directories avoid WSL's slow Windows-path discovery. The initially bare
Ubuntu installation needed the repository's build prerequisites, including
`python3-venv` and `clang-format`. Final log: `build/portrait-audit-linux.log`.

```text
cmake --preset linux-x64-clang-rwdi -DCMAKE_C_COMPILER_LAUNCHER= -DCMAKE_CXX_COMPILER_LAUNCHER=
cmake --build build/linux-x64-clang-rwdi --target PoseidonGame PoseidonTests -j8
build/linux-x64-clang-rwdi/tests/unit/engine/Poseidon/PoseidonTests "[portrait],[journal],[guerrilla]" --reporter compact
```

Local evidence is under `build/portrait-*.log`, `build/portrait-*-metrics.json`,
and `tmp/portrait-*`. To repeat the graphical lane with your own installed assets:

```text
tri test --game-dir dist/x64-win-rwdi --data-dir <installed-data> --retries 0 tests/integration/scripting/guerrilla_portrait_dossier.test.sqf
tri test --game-dir dist/x64-win-rwdi --data-dir <installed-data> --retries 0 tests/integration/scripting/guerrilla_portrait_headless.test.sqf
```

Use an isolated `POSEIDON_USER_DIR` with Trident (its cache override follows that
profile), and two consecutive launches for cold/warm counts. For direct game
launches, `POSEIDON_CACHE_DIR` controls the cache root. Use `--width 2560 --height
1440` for the large-window check; changing display.cfg alone is insufficient when
the harness supplies its own resolution arguments.

## Distribution

Photographs are generated locally from player-provided assets. Generation does
not change the source assets' licenses or grant redistribution rights. No asset
download, AI artwork, portrait sharing, or bundled photograph is introduced.
Developer shoot missions, catalogue metadata and Python processing remain optional
comparison tools; they are not runtime requirements.

The installer excludes the developer `core/portraits` directory even when it is
populated. The portable `Sync-InstallTree` helper receives the source-relative
`portraits` exclusion for the core install only. Excluded destination subtrees
are neither overwritten nor pruned, including when a source archive omits them;
other scripts and mission templates retain normal mirror behavior. The synthetic
installer suite covers these cases on the Windows and Linux CI lanes.
Runtime caches are outside installation mirrors and saves. Generated
portrait extensions are ignored by Git; source archives exclude the portrait
catalogue directory and temporary captures. Release binary staging copies named
binary dependencies, and current CI publishes dependency/compiler caches only;
no generated portrait artifact upload is added. Do not add runtime captures to
future automated artifact uploads.
