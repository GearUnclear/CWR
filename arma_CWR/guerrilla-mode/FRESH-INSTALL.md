# Set up Guerrilla Mode on another machine

You need this repository, a game data folder and the complete LoBo folder. Two
data targets are supported:

1. **The official Arma: Cold War Assault Remaster** (Steam app 65790, the
   `Remastered` subfolder of the Steam install). This is the target.
2. **Classic 1.99** (legacy). Still installs and runs, but Classic
   compatibility is being retired: no new Classic-only shims are added.

No files from the old developer machine are required. The UD menu, the Classic
compatibility overlays, Options UI resources and open-license fonts are
versioned. Base game data and LoBo PBOs remain separate inputs.

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

From `arma_CWR`, replacing the example paths:

```powershell
pwsh -File ./setup-guerrilla.ps1 `
  -GameDir 'C:/Games/Steam/steamapps/common/ARMA Cold War Assault/Remastered' `
  -LoBoDir 'C:/Games/@LoBo' `
  -Tools './dist/x64-win-rwdi/PoseidonTools.exe'
```

Add `-CheckOnly` to validate without installing. Setup picks the mode from the
folder's layout and prints it (`Validated: official Remaster data ...` or
`Validated: Classic data (legacy) ...`).

Both modes install the faction library, the shared `gmcore`, and missions for
the islands they detect. They generate LoBo's factions and run
`mod doctor --fix` on the four known repair targets: LoBoammo, LoBo_airammo,
LoBoWreck and LoBoPalObj. The mission installer mirrors the repo's core and
named templates, removing stale files there; edit their repo sources, not
installed copies. Unrelated missions are preserved. A custom LoBo root config
must be merged before setup; it will not be silently replaced. Correct any
reported failure and rerun. It is safe to rerun after pulling changes, but is
not an atomic transaction across both data folders.

### Remaster mode (official Remaster)

Chosen when the folder has any official Remaster marker: a root `dtaExt`
folder, or Bohemia's `bin/remaster.bin`, `bin/mainMenu.hpp` or
`bin/splashLogo.hpp`.

**The rule: Bohemia's files stay byte-identical. The only change setup makes to
a file Bohemia ships is appended `#include` lines.** Concretely:

- Kept as shipped: `bin/remaster.cpp`, the `bin/options*.hpp` chain,
  `bin/mainMenu.hpp`, `splashLogo.hpp`, `modsTable.hpp`, every
  `STRINGTABLE*.csv`, and the `fonts/` Bohemia already ships (its `cwr_*.ttf`
  are the same fonts UD uses; its own `OFL.txt` stays).
- Appended, once each: `#include "ud-mainmenu.hpp"` to `bin/resource-extra.cpp`
  (by setup) and `#include "guerrilla-factions.hpp"` / `"guerrilla-hq.hpp"` to
  `bin/config-extra.cpp` (by `install-missions.ps1`), each under a one-line
  `// Uslu dur! ...` comment. A rerun finds the line and appends nothing.
- Added beside them (UD's own files): `bin/ud-mainmenu.hpp` (the UD menu),
  `bin/guerrilla-factions.hpp`, `bin/guerrilla-hq.hpp`,
  `bin/STRINGTABLE_UD_AIRCRAFT.utf8.csv` (UD-owned aircraft controls text),
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

After a Steam update or "verify integrity of game files", Bohemia's
`resource-extra.cpp` or `config-extra.cpp` may be restored without our lines.
The game then shows Bohemia's own menu. Rerun setup to append them again.
To uninstall, copy the earliest backups of those two files back and delete the
added files listed above.

The **Remaster Demo** package (Steam app 4819000, `PoseidonGameDemo.exe`, no
Abel world) is refused: its asset set is not a Guerrilla Mode target.

### Classic mode (legacy Classic 1.99)

Chosen when no Remaster marker is present. Setup installs the sky/cloud
compatibility shim (`bin/remaster.cpp`), our `bin/resource-extra.cpp` (which
includes the Options headers and the same `bin/ud-mainmenu.hpp`), the Options
headers and `STRINGTABLE_OPTIONS.utf8.csv`, and the five OFL fonts. Differing
overlay files are backed up under `<GameDir>/.ud-backups/<run-id>`.

### Both modes

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
  -GameDir 'C:/Games/Steam/steamapps/common/ARMA Cold War Assault/Remastered' `
  -LoBoDir 'C:/Games/@LoBo' `
  -GameExe './dist/x64-win-rwdi/PoseidonGame.exe'
```

This launches the rebuilt engine and mounts LoBo plus `deps/@LoBo_Deps` when
present. Add `-Mission 'Missions/Guerrilla.Sinai'` for a direct launch. The old
`@lobofixup` shadow mod is not needed for normal play after setup. Leave out
`-LoBoDir` to play the stock islands without LoBo.

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
pass setup already runs. Tests with relative `@LoBo` paths assume the mod is
beside the data folder or the checkout; check the test's `--mods-dir` for other
layouts. The `full_cwa`/`lobo` lanes still default to prepared Classic data.
The Demo and the unmodified retail Remaster are only needed for their own
integration/UI lanes; see `../CLAUDE.md` for the `packages/Demo` and
`packages/Game` paths.

Run the setup regression test with `pwsh -File tests/runtime/test-setup.ps1`.
It uses temporary synthetic data for both modes (a Remaster-shaped folder whose
every file is hashed before and after, reruns, a simulated Steam restore, and a
tamper check); actual engine smoke tests require real data.

## Licenses and fonts

UD's compatibility configuration and menu keep the source license. The APL-SA
text resources in `thirdparty/classic-ui` and OFL fonts in
`thirdparty/guerrilla-fonts` have separate notices and are excluded from the
engine's GPL grant. No base game or LoBo PBOs are in Git.

The old Classic test bed used renamed Microsoft fonts. Fresh installs instead
use Roboto, Caveat, Unuaranga Kuriero, Vollkorn and Oswald from the Remaster,
matching the engine's mappings. Text appearance differs from that local
workaround. Those Microsoft font bytes are not included in the repository.

## Original Arma 3 aircraft controls

Open **Options → Controls → Arma 3 controls** and confirm the preset to adopt the
original launch-era Arma 3 Classic aircraft keys. Existing saved aircraft layouts
remain in place until you apply the preset or reset an aircraft category. Controller
bindings are preserved. Under **Keyboard & Mouse**, **Helicopter** and **Plane**
can be edited or reset independently.

Both use W/S for nose down/up, A/D for bank and Q/E for pedals or rudder. In a
helicopter, Left Shift/Z raises/lowers the existing terrain-relative height target;
X toggles autohover. In a plane, Left Shift/Z increases/decreases requested speed;
G toggles supported retracting gear under manual control. K shows the aircraft
compass. Use the action menu for flaps and automatic landing. Mouse movement
continues to look around, and flight assistance remains unchanged.
