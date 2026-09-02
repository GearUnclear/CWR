# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## What this is

This directory holds the engine and overhaul for **Uslu dur!** (**UD**) — a **total game overhaul** of *Arma: Cold War Assault* (originally *Operation Flashpoint*, 2001), not just an engine port. "Uslu dur!" is the public brand; the rename is branding-only, so the `cwr`/`CWR`/`Poseidon` codenames throughout the source are unchanged and stay as-is.

> **Repo layout note:** the git root is one level up at `D:\Arma_CWA`, not here. `arma_CWR/` is now a subdirectory alongside `site/` (the web portal) and `@LoBo/` (a mod test bed). See `D:\Arma_CWA\CLAUDE.md` for the repo-wide map; this file covers the engine + overhaul that live under `arma_CWR/`.

Two layers live here:

- **The upstream engine** (`engine/`, `apps/`, etc.), codename **Poseidon**. Modernized to C++20, built with CMake + Clang, cross-platform Windows x64 / Linux x64. This part is a **locked** Bohemia Interactive source release: PRs are not accepted upstream and there is no CI workflow checked into the repo (the sub-READMEs describe what CI *did* run). The code is GPL-3.0-or-later; game data (models, textures, sounds, missions) is **not** in this repo and ships separately under APL-SA.
- **The overhaul itself** ([`mod-plans/`](mod-plans/README.md), [`guerrilla-mode/`](guerrilla-mode/README.md)) — a from-the-ground-up rework of AI tactics/perception plus **Guerrilla Mode**, a new persistent open-world insurgency game mode with swappable islands and factions chosen at new-game. This is where active development happens. **Direction (2026-07-01): this is a total overhaul of the engine source, not a mod on a fixed binary — C++ engine changes are a first-class tool here, not a last resort, alongside SQF mission scripting.** Treat the mod-plans as living, AI-drafted design docs subject to correction, not settled hard specs.

## Working map (where to look / what to skip)

Most of this engine is 1,300+ C++ files, 160 of them over 1,000 lines. To avoid reading extraneous code:

**Active overhaul surface — start here:**
- `engine/Poseidon/Game/Guerrilla/` — `ZoneRegistry`, `GarrisonCache`, `AlertMachine`, `TownFlags` (+ their `*Commands.cpp` SQF bindings). The Phase-1.5 native subsystems.
- `engine/Poseidon/UI/Guerrilla/` — `GuerrillaModule`, `GuerrillaNewGame` (new-game island/faction selection).
- `engine/Poseidon/AI/` — the AI tactics/perception rework. See [`mod-plans/`](mod-plans/README.md) for which behavior maps to which files; treat those docs as intent, not a precise index.
- `voice-lines/` — TTS pipeline for custom in-game speech (WAVs via OpenRouter, played through `CfgSounds` + `say`/`sideRadio`; no engine changes). Key comes from the gitignored repo-root `.env`.

**Frozen upstream — read as reference, don't expect to edit:** `Network/`, `Graphics/`, and most of `World/` and `UI/`. This is a locked Bohemia source release (no upstream PRs). You'll read here to understand mechanisms, but the overhaul rarely changes them.

**Do NOT read or grep these — generated or non-source, pure context waste:**
- `thirdparty/glad/` — ~22k lines of generated OpenGL bindings.
- `@LoBo/` (mod test data, 2.2G binary), `build/`, and `ARMA Cold War Assault [Classic]/AddOns/` (base game data). Not reading it is about context cost, not permission: **@LoBo is ours to modify as of 2026-08-08** (see the repo-root `CLAUDE.md`) — repair a content defect in its pbos rather than shimming around it, and land the repair as a rerunnable script under `tools/lobo/` because the directory itself is gitignored.
- A repo-root `.ignore` already excludes these from `Grep`/`Glob`; the note here is for `Read`.

**Navigating big files efficiently:** grep for the symbol first, then `Read` a ±80-line window (`offset`/`limit`) rather than the whole 2,500–3,300-line file. For open-ended "where/how does X work" questions, delegate to an `Explore` subagent so the file dumps stay in *its* context and only the conclusion returns to yours.

## Toolchain prerequisites

Builds require, on PATH / in env:
- **Clang** — the only supported compiler (`clang-cl` frontend on Windows, GNU-driver clang on Linux). No MSVC, no GCC.
- **Ninja** — the generator used by every preset.
- **vcpkg** — manifest mode (`vcpkg.json`). `VCPKG_ROOT` env var must point at the vcpkg checkout; the base preset chainloads its toolchain.
- **ccache** — set as the compiler launcher by the base preset.

## Local environment (this Windows machine)

**Engine location:** `D:\Arma_CWA\arma_CWR` (moved here from `C:\dev\arma_CWR` on 2026-06-28 to sit next to the game data). The **git root is the parent** `D:\Arma_CWA` — run git and path-relative commands accordingly. A reminder shortcut `C:\dev\arma_CWR.lnk` points back here.

**Game data (two packages — both are game data, not tracked source; treat read-only except where a task documents an exception):**
- `D:\Arma_CWA\ARMA Cold War Assault [Classic]` — the full v1.99 install (AddOns, Campaigns, Worlds, `ColdWarAssault.exe`). Trident's `OFPR_DATA_DIR` in `.trident.env`; the default for the `full_cwa`/`lobo` lanes.
- `D:\Arma_CWA\Arma Cold War Assault Demo [Remaster]` — the official remaster Demo (Steam app 4819000), read-only reference. **Asset-targeting policy (2026-07-14): the Demo's asset set is NOT a gameplay target** — Guerrilla content targets Classic 1.99 + `@LoBo` assets only; the Remaster side of the bargain is *future compatibility with the full Remaster's assets*, kept by the plan-15 descriptor-resolution pass (missing classes degrade with a logged substitution, never fatal/sterile) and by never hard-requiring an asset the full Remaster is expected to lack. Surviving roles: (1) contract source — its `BIN\remaster.cpp` inventories the `Remaster >>` config keys the engine reads, the backfill checklist for the Classic shim (#11); (2) reference binary `PoseidonGameDemo.exe` for upstream-behaviour diffs (#13); (3) occasional `demo`-lane data source for engine-level (non-content) tests. The three Demo-world Guerrilla tests stay unrunnable locally (Classic lacks the `demo` world, Demo lacks the GUER roster) and making them run on Demo data is **explicitly no longer a goal** — see Tests below.

Both bracket dirs are wildcard-hostile: quote the path and use `-LiteralPath` in PowerShell (#12). Per-run `--data-dir` usage lives in the Tests section.

The toolchain is installed and verified as of 2026-06-28. Versions and locations on this machine:

- **Clang/LLVM 22.1.8** — installed via `winget` (`LLVM.LLVM`) to `C:\Program Files\LLVM\bin`. The LLVM winget package does **not** add itself to PATH; `C:\Program Files\LLVM\bin` was added to the **User** PATH manually.
- **CMake 4.3.3** — `winget` (`Kitware.CMake`), `C:\Program Files\CMake\bin` (on PATH).
- **Ninja 1.13.2** — `winget` (`Ninja-build.Ninja`), in the WinGet Packages dir (on PATH).
- **ccache 4.13.6** — `winget` (`Ccache.Ccache`), in the WinGet Packages dir (on PATH).
- **vcpkg** (tool 2026-05-27) — **git-cloned** to `C:\dev\vcpkg` and bootstrapped (not the packaged version). `VCPKG_ROOT=C:\dev\vcpkg` is set in **User** env vars. vcpkg is intentionally **not** on PATH — the build finds it through `VCPKG_ROOT`.
- **git** — pre-existing at `C:\Program Files\Git`.

Gotchas: PATH/env changes only apply to **newly opened** terminals. The first `cmake --preset` is slow (vcpkg compiles all deps from source once, then caches). Target platform/build for this machine: `win-x64-clang-rwdi`.

## Build

Configure with a preset, then build its binary dir (presets are *configure* presets only — there are no build presets):

```sh
cmake --preset win-x64-clang-rwdi          # Linux: linux-x64-clang-rwdi
cmake --build build/win-x64-clang-rwdi
```

Configure-preset families (suffix = build type): `*-clang-dbg` (Debug), `*-clang-rwdi` (RelWithDebInfo), `*-clang-rel` (Release), for `win-x64` and `linux-x64`. Plus `linux-x64-steamrt4` (Steam Runtime), sanitizer presets `*-clang-san` / `linux-x64-clang-tsan`, and fuzzer presets `*-clang-fuzz` (turn on `POSEIDON_BUILD_FUZZERS`). The binary dir always mirrors the preset name under `build/`. Compiled apps are staged into `dist/<arch>-<platform>-<suffix>/`.

## Run

`run-game.ps1` (repo root) is the fast path to eyeball a change: it handles the
toolchain-on-PATH gotcha, does an incremental `PoseidonGame`-target build, and launches
the exe windowed against the game data dir.

```powershell
.\run-game.ps1                                      # build + launch to the main menu
.\run-game.ps1 -Mission 'Missions\Guerrilla.Demo'   # build + jump straight into a mission
.\run-game.ps1 -SkipBuild                           # launch the existing dist/ build as-is
```

Default `-DataDir` is `D:\Arma_CWA\ARMA Cold War Assault [Classic]`. `-Mission` is resolved relative
to `-DataDir` and passed as `--test-mission` (omit it to boot to the main menu, where
GUERRILLA lets you pick island/faction interactively).

### Console-output capture (default on — read the files, don't run the exe in a terminal)

`run-game.ps1` launches the game **detached** and captures all console output into
timestamped files under `logs\` (gitignored), so a play session never dumps output into
the terminal/AI-session transcript. Per run it writes:

- `logs\game-<ts>.log` — the engine log (`--log-file` spdlog file sink; everything the
  engine logger prints to the console also lands here)
- `logs\game-<ts>.stdout.log` — raw stdout
- `logs\game-<ts>.stderr.log` — raw stderr; **crash-handler stack traces, minidump paths
  and CLI errors write directly to stderr**, bypassing the engine logger — this file is
  where to look after a crash

To follow a live session, `Get-Content <log> -Wait -Tail 50` in a background shell, or
just Read/Grep the newest `logs\game-*` files after the run. Diagnostics knobs when you
need more detail: `--log-level`, `--log-categories`, `--log-format` and `--logfiles`
(file-operation logging) — see `PoseidonGame.exe --help` / `AppConfig.cpp`. Pass
`-NoLog` to `run-game.ps1` to disable capture; never launch `PoseidonGame.exe` in the
foreground of an AI session to "see" its output — use the log files.

**Always close what you open.** No human is at the keyboard to close game windows, and a
lingering instance poisons the next Trident connection ("connection error: eval failed").
Every automated command that spawns game instances (run-game.ps1, `tri test`) must
guarantee cleanup in the same command, e.g. append:
`powershell -NoProfile -Command "Get-Process PoseidonGame -ErrorAction SilentlyContinue | Stop-Process -Force"`
— and for run-game.ps1 launches, stop the specific PID once done reading logs. Run `tri`
with `--retries` (guerrilla_native_spawn's doMove assert is known-flaky) and check for
stray `PoseidonGame` processes before diagnosing connection failures.

## Tests

CTest drives both Catch2 unit tests and Trident integration tests:

```sh
ctest --test-dir build/win-x64-clang-rwdi --output-on-failure
ctest --test-dir build/win-x64-clang-rwdi -R "<test name>" --output-on-failure   # single test / regex
```

- **Unit** (`tests/unit/`, Catch2): suites `PoseidonFoundationTests`, `PoseidonCoreTests`, `PoseidonTests`, `PoseidonServerTests`, `PoseidonEvaluatorTests`, `PoseidonTetrisTests`. Catch2 tag filters like `[config]`, `[graphics]` partition `PoseidonTests`.
- **Integration** (`tests/integration/`, Trident): SQF-driven in-game scenarios. Needs game data. Build the runner first, then run:
  ```sh
  cargo build --manifest-path engine/Trident/Cargo.toml
  tri test -j6 --retries 2 tests/integration   # binary: engine/Trident/target/debug/tri[.exe]
  ```
  Copy `.trident.env.example` → `.trident.env`, set `OFPR_GAME_DIR` (build/dist dir with binaries) and `OFPR_DATA_DIR` (Demo game data; recommended layout `packages/Demo`, which is gitignored). Get free Demo data from Steam app 4819000.

  **On this machine** two game-data packages exist. `.trident.env` points `OFPR_DATA_DIR` at the full 1.99 install `ARMA Cold War Assault [Classic]` — the default for the `full_cwa`/`lobo` lanes — and the official remaster Demo package sits at `Arma Cold War Assault Demo [Remaster]`. Override the data dir per run for a Demo lane: `tri test --data-dir "D:\Arma_CWA\Arma Cold War Assault Demo [Remaster]" ...` (the runner does no globbing on the value, so the brackets are safe).

  **The three Demo-world Guerrilla tests don't run on either local package, and per the 2026-07-14 asset-targeting policy this is accepted, not a work item.** `guerrilla_capture_flip`, `guerrilla_spawn_domove` and `guerrilla_save_reload.seq` bind to the `demo` world (folder suffix `.Demo`), which Classic lacks; and their `mission.sqm` player is `SoldierGB`, which the Demo [Remaster] package does not ship (no `SoldierG*` roster except `SoldierGFakeE`, #13) — a `--data-dir` run against the Demo package boot-fails within ~60 ms on the strict-mode config error. Descriptor-sourced spawns are already package-proof (the plan-15 resolution pass), but the `mission.sqm` class is not descriptor-sourced, and remapping it for Demo data is out of scope now: the Demo asset set is no longer targeted. The runnable Guerrilla set here is the `full_cwa` tests on Classic (+ the `lobo` lane).

### Guerrilla integration-test preconditions (verified 2026-07-07)

- **Every Guerrilla mission now needs the shared script core installed** (issue #54 step B1, 2026-09-02): the managers live once at `guerrilla-mode/core` and are installed to `<GameDir>\gmcore` by `guerrilla-mode/install-missions.ps1`; each mission's `init.sqs` is a two-line `[] exec "\gmcore\init.sqs"` bootstrap, whose leading backslash resolves against the game data root rather than the mission folder. This holds even for the tests that boot a repo-root-relative template path: the mission folder no longer carries `scripts/`, so without a `<GameDir>\gmcore` the mission boots with zero managers and the asserts go red on missing `gm*` globals. Re-run the installer after editing the core.
- `ui/guerrilla_new_game_e2e` needs the mission templates **installed in the game dir**: plain `guerrilla-mode/install-missions.ps1` (issue #54 C4, 2026-09-02: the world gate now scans the pbo entry tables of `AddOns\`, `Dta\` and every `@*` mod folder next to the game dir, so Sinai and Lebanon80 install with no `-IncludeWorld`, which survives only as a manual override). The same run installs the shared script core (`gmcore`) and the **vanilla faction library** (`bin\guerrilla-factions.hpp` included from `bin\config-extra.cpp`, issue #54 A4). The @LoBo war rosters live in `tests/fixtures/mods-lobo/@lobofixup/bin/config.cpp`: a session that mounts `@LoBo` without `@lobofixup` offers no war factions on Sinai/Lebanon80. (`guerrilla_sinai_swap` and `guerrilla_lebanon80_boot` boot the repo-root-relative template path, so they need only the core + library install, not the templates.)
- The LoBo tests need a one-time `@LoBo` repair pass, idempotent and rerun after any @LoBo reinstall: **`dist/<build>/PoseidonTools.exe mod doctor "D:\Arma_CWA\@LoBo" --fix`** (drop `--fix` to report only; `--pbo <wildcard>` narrows the model scan). It repairs all three defect classes in one go with same-length in-place byte patches (the pbo header table never moves) and keeps the originals in `@LoBo/_ud-orig`:
  - **UNDEFINED_SCOPE_KEYWORD** — `LoBoWreck.pbo` and `LoBoPalObj.pbo` are the only two @LoBo addons that omit the `#define private 0 / protected 1 / public 2` header every other @LoBo config carries, so their `scope = public;` resolved to nothing and read back as 0. Eighteen finished wreck and poster classes were refused with `Cannot create '<class>': type is abstract`. `scripting/prop_spawn_safety` CASE 2b goes red if the repair has not been run.
  - **BURIED_MODEL_ORIGIN** — `LoBo_M60A1_wreck` and `LoBo_M60A1_wreck2` were authored with their model origin **above** the mesh, and a static prop is seated at `terrainY + shape->BoundingCenter().Y` (`Entity::PlaceOnSurface`, Static branch, `World/Simulation/Simul.cpp:1300`), so `createVehicle [x, y, 0]` buried both tanks whole (roof 0.45 m / 0.92 m under the sand). Not a missing LOD: no LoBoWreck model has a LandContact LOD, including the eight that seat correctly. `scripting/prop_spawn_safety` CASE 2d goes red, and the `p03_m60a1_wreck` / `p03b_m60a1_wreck2` probe frames come back as empty desert, if it has not been run.
  - **MALFORMED_FLOAT** — stock `@LoBo` ammo pbos carry `0.0.1` in eleven `tracerColor[]` arrays. The config reader now coerces such a token to its strtod prefix (see `ClassifyToleratedLiteral`), so this no longer hard-aborts under `--autotest`; the repair still normalises the bytes. On this machine `LoBoammo.pbo`/`LoBo_airammo.pbo` were already repaired in place on 2026-07-16 and are byte-identical to the `@lobofixup` fixture's shadows, so the shadows are inert; the fixture's `bin/config.cpp` (its `CfgAddons` preload roster and `CfgGuerrillaFactions`) is what still carries the lane.
  - `tools/lobo/fix-lobo-scope.ps1`, `tools/lobo/fix-lobo-model-origin.ps1` and `tests/fixtures/mods-lobo/@lobofixup/gen-patched-pbos.ps1` are now thin wrappers around `mod doctor`; the logic lives in `engine/Poseidon/Asset/Addon/ModDoctor.{hpp,cpp}` and is unit-tested (`[moddoctor]`).
- Run `tri` with `--retries` and low `-j`: `guerrilla_native_spawn` is known-flaky on its doMove displacement assert (recovers on retry), and parallel instances plus any manually closed window show up as `connection error: eval failed` cascades.

### Unit-test gotchas on Windows (verified 2026-06-28)

A clean build passes ~3536/3561 unit tests. The ~25 "failures" out of the box on this machine are **environment portability bugs in the tests, not engine regressions** — they reproduce on any fresh Windows/non-UTC checkout. Two causes, both fixed locally:

- **Hardcoded `/tmp/` paths (24 tests).** Ten test files (`test_paa_encoder`, `test_asset_preview`, `test_dds_converter`, `test_findfirst`, `test_gamePaths`, `test_crash_handler`, `test_mission_info`, `test_game_state_ext`, `test_dir_scanner`, `test_network_server`) write to literal `"/tmp/..."`. A native Windows exe resolves that to `\tmp\` **on the current drive**, so file writes (`WritePAA`, `saveToFile`, world-state writes) fail with no such directory. Fix: create `D:\tmp` (ctest runs from the `D:` drive). If you ever run the tests from another drive, create `\tmp` on that drive too.
- **`FormatTime` timezone assumption (1 test).** `AssetInfo::FormatTime` correctly renders asset mod-times in **local** time; the test in `test_asset_info.cpp` assumed UTC and asserted the string contained `"2020"` for `1577836800`, which renders as `2019-12-31 16:00:00` west of UTC. Fixed the test to assert the `YYYY-MM-DD HH:MM:SS` *format* instead (production code unchanged — local-time display is the desired behavior).

## Lint / format

These are custom CMake targets (not standalone scripts) — build them against a configured dir:

```sh
cmake --build build/win-x64-clang-rwdi --target Format      # clang-format check (--Werror)
cmake --build build/win-x64-clang-rwdi --target FormatFix   # clang-format in place
cmake --build build/win-x64-clang-rwdi --target Tidy        # clang-tidy
cmake --build build/win-x64-clang-rwdi --target Lint        # Format + Python (ruff, if uv present)
cmake --build build/win-x64-clang-rwdi --target FileSize    # warn >3000 lines, error >5000
```

Rust crates (`engine/Trident`, `mserver/*`) use the normal `cargo fmt` / `clippy` / `test` / `build` per their `Cargo.toml`.

## Architecture

**Engine (C++20 static libs, `engine/`)** — the stack apps link against:
- **Poseidon** (`engine/Poseidon/`, → `Poseidon.lib`) — the whole engine in one lib. Subsystems: `AI`, `Asset`, `Audio`, `Core`, `Dev`, `Foundation`, `Game`, `Graphics` (backend interface + factory), `IO` (streams, ParamFile, preproc), `Input`, `Network`, `Security`, `UI`, `World`, plus the SQF `Evaluator`.
- **PoseidonGL33** / **PoseidonOpenAL** — OpenGL 3.3 and OpenAL backends, linked only by client apps that need rendering/audio.
- **PoseidonFormats** — standalone C-API shared lib for P3D/PAA/PBO/RTM file formats; consumed by the Python Blender addon.

**Foundation** (`engine/Poseidon/Foundation/`) is the engine's own standard-library layer — custom Containers, Strings, Memory (mimalloc-backed), Threads, Math, Time, etc. — that the rest of the engine is written against instead of the STL. `PoseidonPCH.hpp` is the prelude precompiled-and-force-included into most targets; building with `POSEIDON_DISABLE_PCH=ON` removes the force-include to audit per-file include self-containment.

**Apps** (`apps/`): `cwr/Game` + `cwr/GameDemo` + `cwr/Server` (sharing `cwr/GameBase` startup), and tools under `tools/` (`Tools`/`PoseidonTools`, `Evaluator`, `Studio` ImGui shell, `TcPbo`, `TcLister`). Sample target `tetris/Tetris`, fuzz harnesses `fuzzers/Fuzzer`. GUI client targets link engine + GL/AL backends; tool targets link only the engine libs they need. Note the output name often differs from the target name (e.g. target `TcPbo` → `pbo`, `TcLister` → `poseidon`).

**Rust tooling**: `engine/Trident/` (the `tri` integration test runner / harness) and `mserver/` crates (`Archive`, `CLI`, `Client`, `MasterService` — public master-server service and CLI).

## Conventions that will surprise you

- **Legacy warning suppression is huge and intentional.** `CMakeLists.txt` disables ~50 clang warning classes globally (verified to suppress ~11,600 warnings). Don't "fix" code just because a normally-warned pattern appears — many are deliberate (e.g. `memcpy`/`memmove` on bitwise-movable types via the `ClassIsMovableZeroed` pattern, intentionally incomplete `switch` in AI logic, defensive tautological compares).
- **clang-cl vs GNU clang differ on `__FILE__`.** On GNU-driver clang, `-fmacro-prefix-map` rewrites `__FILE__` to repo-root-relative; on clang-cl it is left absolute on purpose (test helpers like `RepoPath()` walk an absolute `__FILE__` up to the repo root). Keep this asymmetry in mind when touching path/log/assert code.
- **PCH config is per-target PRIVATE**, not `REUSE_FROM`, because targets bake differing compile definitions into the PCH blob. See the `POSEIDON_PCH_TARGETS` list in the root `CMakeLists.txt`.
- The build adds targets in a specific order in the root `CMakeLists.txt`; new subdirectories must be registered there.
