# Tests

## Structure

| Directory | Framework | What |
|-----------|-----------|------|
| `unit/` | Catch2 / ImGui Test Engine | C++ unit tests for engine and apps |
| `integration/` | Trident (tri) | SQF-driven in-game scenarios |
| `smoke/` | Pester | Boot log verification |
| `fixtures/` | - | Test data files (P3D, PAA, PBO, RTM, configs, audio, etc.) |

## Unit Test Suites (`unit/`)

| Suite | Location | Tests |
|-------|----------|-------|
| PoseidonFoundationTests | `unit/engine/Poseidon/Foundation/` | Low-level Poseidon foundations (containers, strings, memory, threads, IO/runtime glue) |
| PoseidonCoreTests | `unit/engine/Poseidon/` | Folded Poseidon core support coverage: evaluator, config/ParamFile, QStream, locale/stringtable, preproc, CSV |
| PoseidonTests | `unit/engine/Poseidon/` | Engine - P3D formats, graphics, audio, core, AI, entities |
| PoseidonTests (`[config]`) | `unit/engine/Poseidon/Core/` | Config store, profiles |
| PoseidonTests (`[graphics]`) | `unit/engine/Poseidon/Graphics/` | Graphics + shader compilation coverage |
| PoseidonServerTests | `unit/apps/Server/` | Server simulate mode |
| PoseidonEvaluatorTests | `unit/apps/Evaluator/` | SQF evaluation and evaluator tooling layer |

## Running

Build first with a CMake preset from the repository root:

```sh
cmake --preset linux-x64-clang-rwdi
cmake --build build/linux-x64-clang-rwdi
```

Run CTest against the build directory:

```sh
ctest --test-dir build/linux-x64-clang-rwdi --output-on-failure
ctest --test-dir build/linux-x64-clang-rwdi -R "<test name>" --output-on-failure
```

Use the matching build directory for other presets, for example
`build/win-x64-clang-rwdi` on Windows.

## Smoke Suite (`smoke/`)

`tests/smoke/*.tests.ps1` are Pester v5 (also runs under v6) tests that boot the
real `PoseidonGame`/`PoseidonServer` binaries (`--check` mode, `--auto-screenshot`,
or a live `--harness` session) and assert on their JSONL log output, on-disk cfg
files, or rendered pixels. They share one helper module,
`tests/cli/TestHelpers.psm1`, which every file imports via
`Import-Module "$PSScriptRoot/../cli/TestHelpers.psm1"`.

Each file takes a mandatory `-Preset` parameter (a CMake configure preset name,
e.g. `win-x64-clang-rwdi`) used to locate the built binaries under
`dist/<dist-name>/`. Run one file directly:

```powershell
Invoke-Pester -Container (New-PesterContainer -Path tests/smoke/boot_logs.tests.ps1 -Data @{ Preset = 'win-x64-clang-rwdi' }) -Output Detailed
```

or the whole directory by building a container per file and passing them all to
`Invoke-Pester -Container`. Works under both Windows PowerShell 5.1 and
PowerShell 7+.

The suite self-skips per test (`Set-ItResult -Skipped`) rather than failing when
its prerequisites are missing: no built `PoseidonGame`/`PoseidonServer` for the
given preset, no `AddOns`/`Addons` folder in the resolved game-data directory (see
`Get-GameDataDir` in `TestHelpers.psm1` for how that directory is found), or no
`tri` (Trident) binary for the tests that drive a live UI flow. This makes it safe
to run with `-Preset` pointed at a preset that has never been built - every test
reports Skipped instead of erroring.

Trident integration, screenshot, and other game-data-backed tests need a local
`.trident.env` file. Copy `.trident.env.example` to `.trident.env`, set
`OFPR_GAME_DIR` to the build or distribution directory containing the binaries,
and set `OFPR_DATA_DIR` to the Demo game data. The recommended local layout is
`packages/Demo`; the whole `packages/` tree is ignored by Git.

Build the Trident CLI before running integration tests:

```sh
cargo build --manifest-path engine/Trident/Cargo.toml
```

The binary is `tri` and lives under `engine/Trident/target/debug/` for the
default Cargo build. Put that directory on `PATH`, or call the binary by relative
path:

```sh
tri test  -j6 --retries 2 tests/integration
./engine/Trident/target/debug/tri test -j6 --retries 2 tests/integration
```

On Windows, use `engine\Trident\target\debug\tri.exe` when calling it directly.
