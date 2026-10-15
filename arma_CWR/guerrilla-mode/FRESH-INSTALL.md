# Set up Guerrilla Mode on another machine

You need this repository, the **full official Arma: Cold War Assault Remastered**
data (Steam app 65790, the `Remastered` subfolder), and the complete LoBo folder.
Classic 1.99 and the Remaster Demo are unsupported for UD gameplay and are
refused by setup and the UD launch helpers.

No files from the old developer machine are required. The UD menu, Options text
and open-license fonts are versioned. Base game data and LoBo PBOs remain
separate inputs. Keep existing Classic installations, profiles and saves intact;
setup does not convert them. Bohemia documents separate Remaster profiles and
incompatible Classic savegames in its [official release notes](https://old.bohemia.net/blog/Arma-Cold-War-Assault-Remastered-Out-Now).

## Build

Install Git, PowerShell 7 (`pwsh`), CMake, Clang, Ninja, ccache and vcpkg. On
Windows, clang-cl also needs the Windows SDK and Visual Studio C++ build tools.
Set `VCPKG_ROOT` to your vcpkg checkout and put the tools on PATH. See
[`../CLAUDE.md`](../CLAUDE.md#build) and the CMake presets for platform details.
From the clone's `arma_CWR` directory:

```powershell
cmake --preset win-x64-clang-rwdi
cmake --build build/win-x64-clang-rwdi --target PoseidonGame PoseidonTools
```

On Linux use `linux-x64-clang-rwdi`; binaries are staged in `dist/x64-linux-rwdi`
without `.exe`. The setup scripts require PowerShell 7 on either platform.

## Prepare the data

Keep the game folder and LoBo in real directories (no directory junctions or
symlinks on the path setup writes to). LoBo may be beside the game folder,
inside it, or elsewhere. The server copy is `mega_nc:/root/mobile-dev/mod_@lobo`;
copy its entire contents, including `deps/@LoBo_Deps`, to your chosen LoBo
directory. That includes the recovered 73 Eastings files and the optional
original download/readme archive.

First make a separate shadow/copy of the full Remaster. **Never run setup against
the Steam folder.** On Windows, follow the shadow command in [Development tests](#development-tests);
on Linux, copy the full data to a separate directory. Only read-only data folders
may be junctions; `BIN`, fonts, missions and LoBo must be writable real copies.
From `arma_CWR`, replacing the example paths with that shadow and your LoBo copy:

```powershell
pwsh -File ./setup-guerrilla.ps1 `
  -GameDir 'D:/Arma_CWA/tmp/remaster-shadow' `
  -LoBoDir 'C:/Games/@LoBo' `
  -Tools './dist/x64-win-rwdi/PoseidonTools.exe'
```

Add `-CheckOnly` to validate without installing. Setup validates the full
Remaster layout and prints `Validated: official Remaster data ...`. A Classic,
Demo or incomplete layout fails before installation.

Setup installs the faction library, the shared `gmcore`, and missions for
the islands they detect. They generate LoBo's factions and run
`mod doctor --fix` on the four known repair targets: LoBoammo, LoBo_airammo,
LoBoWreck and LoBoPalObj. The mission installer mirrors the repo's core and
named templates, removing stale files there; edit their repo sources, not
installed copies. Unrelated missions are preserved. A custom LoBo root config
must be merged before setup; it will not be silently replaced. Correct any
reported failure and rerun. It is safe to rerun after pulling changes, but is
not an atomic transaction across both data folders.

### Required Remaster layout

The folder must have an official Remaster marker: a root `dtaExt`
folder, or Bohemia's `bin/remaster.bin`, `bin/mainMenu.hpp` or
`bin/splashLogo.hpp`. The same guard also requires `bin/config.bin`,
`bin/resource.bin`, `bin/resource-extra.cpp`, `dta/data3d.pbo` and
`worlds/abel.wrp`, and refuses Demo markers even if Abel is present.
Names are resolved case-insensitively; ambiguous duplicate spellings are refused.
A marker alone does not establish a complete or authentic release.

**The rule: Bohemia's files stay byte-identical. The only change setup makes to
a file Bohemia ships is appended `#include` lines.** Concretely:

- Kept as shipped: `bin/remaster.cpp`, the `bin/options*.hpp` chain,
  `bin/mainMenu.hpp`, `splashLogo.hpp`, `modsTable.hpp`, every
  `STRINGTABLE*.csv`, and the `fonts/` Bohemia already ships (its `cwr_*.ttf`
  are the same fonts UD uses; its own `OFL.txt` stays).
- Appended, once each: `#include "ud-mainmenu.hpp"` and
  `#include "ud-options.hpp"` to `bin/resource-extra.cpp` (by setup) and
  `#include "guerrilla-factions.hpp"` / `"guerrilla-hq.hpp"` to
  `bin/config-extra.cpp` (by `install-missions.ps1`), each under a one-line
  `// Uslu dur! ...` comment. A rerun finds each line and appends nothing; an
  older install that has only the menu line gains the Options line.
- Added beside them (UD's own files): `bin/ud-mainmenu.hpp` (the UD menu),
  `bin/ud-options.hpp` (the UD Options additions, see below),
  `bin/guerrilla-factions.hpp`, `bin/guerrilla-hq.hpp`,
  `fonts/UD_FONT_CREDITS.md`, any UD font Bohemia does not ship, `gmcore/`,
  the `Missions/Guerrilla.*` and reference templates, and
  `bin/STRINGTABLE_UD.utf8.csv` with its `bin/UD_OPTIONS_APL-SA_NOTICE.txt`.
  The stringtable shard holds only the UD Options keys none of Bohemia's
  shards define (Arma 3 controls preset, gamepad steering, lean, temporary
  zoom). The engine loads `STRINGTABLE_*` shards in name order and a later
  shard wins a duplicate key, so shipping our whole Options shard would
  overwrite Bohemia's newer text. Setup prints the missing keys, and retires
  the shard when Bohemia's shards cover them all.
- Not installed: the Classic shims (`remaster.cpp`, `resource-extra.cpp`,
  Options headers, `STRINGTABLE_OPTIONS.utf8.csv`).

Before writing, setup snapshots every package file outside LoBo and its own
output, and fails the run if anything other than those appended lines changed.
`resource-extra.cpp` and `config-extra.cpp` are backed up under
`<GameDir>/.ud-backups/<run-id>` before an append; a rerun with nothing to
append makes no backup.

Why the menu works this way: the engine reads exactly one
`bin/resource-extra.cpp` and parses it, includes and all, as one config. On
the Remaster that file is Bohemia's (Options, mods table, splash, main menu),
and it already defines `RscDisplayMainRemaster`. The UD menu therefore uses its
own class, `RscDisplayMainUD`, which the engine prefers when present, and
Bohemia's menu stays loaded but unused. MODS stays reachable through the
button the engine injects next to QUIT GAME.

The UD-only Options controls work the same way. Bohemia's `optionsShell.hpp`
defines a six-row Controls page (`RscOptionsPageControls`) without UD's
"Arma 3 controls" preset row (idc 1407), and redefining that class in the same
parse would be a "Member already defined" error. `bin/ud-options.hpp` instead
defines `RscOptionsPageControlsUD`, inheriting Bohemia's page and its
`OptIdxMenuItem` template, adding the row and moving Close down one; the
Controls page mounts the UD class when it is present and the stock page
otherwise (`SelectControlsPageResourceClass`, `UI/Options/ControlsPage.cpp`).
Only that row is resource-driven: the other UD Options texts (gamepad
steering, lean, temporary zoom) are rows the engine builds itself and need
only their keys from the gap stringtable shard.
`tests/integration/ui/remaster/remaster_ud_options_controls` covers
the row on the shadow; `tests/unit/.../UI/test_controls_page_resource.cpp`
covers the parse.

After a Steam update or "verify integrity of game files", Bohemia's
`resource-extra.cpp` or `config-extra.cpp` may be restored without our lines.
The game then shows Bohemia's own menu. Rerun setup to append them again.
To uninstall, copy the earliest backups of those two files back and delete the
added files listed above.

The **Remaster Demo** package (Steam app 4819000, `PoseidonGameDemo.exe`, no
Abel world) is refused: its asset set is not a Guerrilla Mode target.

Classic shims retained under `runtime/classic` are historical source, not an
installation target. A copied UD executable or an old shim named
`bin/remaster.cpp` does not establish a full Remaster package.

### Payload verification and backups

LoBo's generated config has a backup under `<LoBoDir>/.ud-backups`. Doctor
keeps original PBOs in `<LoBoDir>/_ud-orig`.

`runtime/manifest.json` checks the small runtime payload's SHA-256 hashes
before installation and checks installed files afterward. Each entry's
`remaster` field says how Remaster mode treats it (`install`,
`keep-existing`, `string-gap`, `skip`). Update the relevant hash after
intentionally editing a payload file; setup refuses to run on a mismatch.

## Play

```powershell
pwsh -File ./play-guerrilla.ps1 `
  -GameDir 'D:/Arma_CWA/tmp/remaster-shadow' `
  -LoBoDir 'C:/Games/@LoBo' `
  -GameExe './dist/x64-win-rwdi/PoseidonGame.exe'
```

This launches the rebuilt engine and mounts LoBo plus `deps/@LoBo_Deps` when
present. Add `-Mission 'Missions/Guerrilla.Sinai'` for a direct launch. The old
`@lobofixup` shadow mod is not needed for normal play after setup. Leave out
`-LoBoDir` to play the stock islands without LoBo. The game starts detached;
the script prints its PID and writes the engine log, stdout and stderr to
`logs/game-<ts>.log`, `.stdout.log` and `.stderr.log` (crash traces land in
stderr). Add `-Dev` for the dev panel's Console and Profile tabs, `-NoLog` to
skip the logs, or `-LogDir <dir>` to write them elsewhere.

## Development tests

Install Rust/Cargo for Trident. Copy `.trident.env.example` to `.trident.env` in
`arma_CWR`; set `OFPR_GAME_DIR` to the built binary directory and `OFPR_DATA_DIR`
to prepared game data (forward slashes work in paths). Build Trident with
`cargo build --manifest-path engine/Trident/Cargo.toml`.

**Remaster lane** (`tests/integration/ui/remaster`, tag `remaster_ud`): pinned
to `packages/RemasterUD`, a gitignored junction to a *shadow copy* of the
Remaster folder that setup has prepared. Never point tests or setup at the
Steam install itself. Build the shadow with real copies of everything setup
writes and junctions for the large read-only data folders:

```powershell
pwsh -File tools/runtime/new-remaster-shadow.ps1 `
  -Source 'C:/Games/Steam/steamapps/common/ARMA Cold War Assault/Remastered' `
  -Destination 'D:/Arma_CWA/tmp/remaster-shadow' `
  -LoBoSource 'C:/Games/Steam/steamapps/common/ARMA Cold War Assault/Remastered/@LoBo'
pwsh -File ./setup-guerrilla.ps1 -GameDir 'D:/Arma_CWA/tmp/remaster-shadow' `
  -LoBoDir 'D:/Arma_CWA/tmp/remaster-shadow/@LoBo' -Tools './dist/x64-win-rwdi/PoseidonTools.exe'
cmd /c mklink /J packages\RemasterUD D:\Arma_CWA\tmp\remaster-shadow
tri test --retries 2 tests/integration/ui/remaster
```

Delete a shadow's junctions with `cmd /c rmdir <link>` (removes the link only)
before deleting the folder; a recursive delete that follows a junction would
reach the Steam files.

Some older LoBo tests still mount `tests/fixtures/mods-lobo/@lobofixup` for its
tracked `bin/config.cpp` (addon preload roster and LoBo factions); it needs no
generation step, and the LoBo repair it once stood in for is the `mod doctor`
pass setup already runs. Unpinned tests mount `@LoBo` with `--mods-dir "."`,
so they expect the mod INSIDE the data dir, where `new-remaster-shadow.ps1`
puts it, and reach repo fixtures through tri's `${repo_root}` token (the
engine checkout root), so they run on any shadow under `tmp\` as well as on
`packages\RemasterUD`. The `full_cwa`/`lobo` tags are retained identifiers, not a Classic support promise.
The example `.trident.env` now defaults to `packages/RemasterUD`. Update existing
local `.trident.env` files that still point to Classic, or pass `--data-dir <shadow>`
for unpinned tests. These lanes still need execution on full Remaster data;
changing the default is not runtime acceptance.
The Demo and the unmodified retail Remaster are only needed for their own
integration/UI lanes; see `../CLAUDE.md` for the `packages/Demo` and
`packages/Game` paths.

Run the setup regression test with `pwsh -File tests/runtime/test-setup.ps1`.
It exercises the production helpers using temporary synthetic Remaster data and
unsupported-layout rejection cases. Remaster fixture files are hashed before and
after; reruns, simulated Steam restore and tamper checks cover file preservation.
The installer hashes `bin` and fonts and compares size/timestamps for large data
folders. These checks do not prove game startup or real-save compatibility.

## Verified contract and remaining checks

Source review date: **2026-10-15**, delivery baseline `a6fe940`, with upstream
`occupier-forces` verified at `197a7d3`. The runtime policy is full official
Remastered only. `engine/Poseidon/Foundation/Platform/VersionNo.h` reports
`APP_VERSION_TEXT = "3.05"`; `CMakeLists.txt` has project version `1.0.0`.
These describe this source build, not a verified installed Steam release.
**The current supported Steam build/depot version remains unresolved**: this
server has no full game data or Steam manifest, and no release-number allowlist
is established by the source. Do not infer compatibility from either number.

The package guard checks layout, not asset contents, binary identity, or successful
engine configuration. The engine loads its Remaster config through
`ConfigurationSystem::InitializeGameConfiguration` / `ParseRemaster`. Keep the
full package's own `remaster.cpp`/`remaster.bin` contract; do not substitute the
historical Classic shim. `play-guerrilla.ps1 -GameExe` must name the rebuilt UD
`PoseidonGame`; arbitrary renamed executables are not authenticated by data checks.

Classification: **A** retained format/identifier used by Remastered; **B** historical
reference; **C** unsupported Classic behavior to retire; **D** confirmed Remastered
incompatibility; **E** needs runtime or release evidence. This batch confirms
policy contradictions (C), not a new binary/runtime defect (D).

| Component | Verified requirement and old assumption | Action / class | Headless evidence | Remaining runtime check |
| --- | --- | --- | --- | --- |
| Default data selection | Full Remaster layout; previously returned Classic when shadows were missing and accepted any existing env directory | Remove Classic fallback; validate override and candidates (C) | Production-helper cases in `tests/runtime/test-data-dir.ps1` | Windows junction target resolution and actual prepared shadow |
| Explicit setup / launch / mission install | Same layout guard before build, logs, install or launch; explicit paths previously bypassed default checks | Shared guard in setup, play, run-game, run-game-with-mods and install-missions (C) | Synthetic explicit-path rejection with intercepted process launch | Actual `PoseidonGame` startup; CLI/mod argument handling |
| Runtime payload | Preserve Bohemia resources; manifest already distinguishes install, keep-existing, string-gap, skip | Remove Classic install branch; retain skipped historical sources (B/C), licensed gap strings and font policies (A) | `test-setup.ps1`: hashes, backup/include append, rerun, restore and tamper checks | Menu, Options, fonts, localization and audio on full package |
| Build configuration | CMake describes Remastered; platform presets choose compiler/OS, not a Classic runtime | No preset/ABI changes (A); `3.05` vs `1.0.0` is not a defect | Source inspection of CMake, presets and version header | Record official build ID and verify built executable against that package (E) |
| Trident defaults / CI | `OFPR_*` variables are runner identifiers; explicit `data_dir` pins override defaults | Example now uses RemasterUD; retain Demo-specific engine pins and `full_cwa`/`lobo` tag names (A/C) | Installer tests run without proprietary data; test default inspected | Execute full_cwa/lobo and remaster_ud on shadow; old `packages/Remaster` CTest registration remains issue #91 (E) |
| Addons / packaging | `AddonSystem::CheckVersion` treats `requiredVersion` as a minimum; LoBo fixture's `1.30` is not a runtime selector | Keep minimum and `requiredAddons` identifiers (A) | Source inspection of `Asset/Addon/AddonSystem.cpp` and `@lobofixup/bin/config.cpp` | Load actual repaired LoBo dependencies and report missing assets (E) |
| Missions / scripts | Mission `version=11` matches `MissionsVersion=11`; SQS/SQF and PBO names are consumed by current engine | Keep formats and command identifiers (A) | Source/fixture inspection; no gameplay execution | Boot Abel and Sinai, exercise script APIs, save/reload |
| Profiles / save formats | `GamePaths` still uses CWR / ColdWarAssault identifiers, env overrides and optional old-paths; serialization versions are independent of runtime version | Preserve paths, user data and schema/migration behavior (A); no conversion promised | Source inspection of `GameBase.cpp`, `GamePaths.cpp`, `SaveVersion.hpp` | Verify Windows/Linux profile locations and authentic UD saves with save owner (E) |

Before calling this runtime-compatible, on a machine with the full package:

1. Record Steam app/build/depot IDs and the built UD executable version/hash.
   Create a separate shadow and retain original profiles/saves. Do not attempt
   Classic-save conversion; Bohemia's release notes above distinguish those saves.
2. Run setup `-CheckOnly`, setup and a repeat install on the shadow. Confirm
   originals remain unchanged apart from the documented backed-up includes, and
   confirm Classic/Demo paths fail before writes.
3. Exercise default discovery, explicit paths and env overrides on Windows and
   Linux, including bracketed paths and Windows read-only data junctions. Confirm
   the launched executable and profile directory, then stop only that process.
4. Run `tri test --retries 2 tests/integration/ui/remaster` plus the applicable
   `full_cwa`/`lobo` lanes using prepared Remaster data. Check menu, Options, fonts,
   addon load, Abel/Sinai boots, rendering, audio and gameplay.
5. With the save/schema owner, validate authentic UD campaign save/reload and
   supported older UD records in disposable profile copies. Headless fixtures
   do not establish real-save migration or full Steam-release compatibility.

## Licenses and fonts

UD's compatibility configuration and menu keep the source license. The APL-SA
text resources in `thirdparty/classic-ui` and OFL fonts in
`thirdparty/guerrilla-fonts` have separate notices and are excluded from the
engine's GPL grant. No base game or LoBo PBOs are in Git.

The old Classic test bed used renamed Microsoft fonts. Fresh installs instead
use Roboto, Caveat, Unuaranga Kuriero, Vollkorn and Oswald from the Remaster,
matching the engine's mappings. Text appearance differs from that local
workaround. Those Microsoft font bytes are not included in the repository.
