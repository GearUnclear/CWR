# Set up Guerrilla Mode on another machine

You need this repository, **Classic 1.99 game data**, and the complete LoBo folder.
No files from the old developer machine are required. The Classic menu and
compatibility overlays, Options UI resources, and open-license fonts are now
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

## Prepare the supplied data

Keep Classic and LoBo in real directories (no directory junctions or symlinks).
LoBo may be beside the game folder or elsewhere. The server copy is
`mega_nc:/root/mobile-dev/mod_@lobo`; copy its entire contents, including
`deps/@LoBo_Deps`, to your chosen LoBo directory. That includes the recovered
73 Eastings files and the optional original download/readme archive.

From `arma_CWR`, replacing the example paths:

```powershell
pwsh -File ./setup-guerrilla.ps1 `
  -GameDir 'C:/Games/CWA Classic' `
  -LoBoDir 'C:/Games/@LoBo' `
  -Tools './dist/x64-win-rwdi/PoseidonTools.exe'
```

Add `-CheckOnly` to validate without installing. This targets Classic 1.99; do
not pass Remaster or Demo as `GameDir`. A Demo install is not needed for setup.

Setup installs the sky/cloud compatibility shim, UD menu, Options headers and
translations, five OFL fonts, faction library, shared `gmcore`, and missions for
detected islands. It generates LoBo's factions and runs `mod doctor --fix` on
the four known repair targets: LoBoammo, LoBo_airammo, LoBoWreck and LoBoPalObj.

Differing overlay files are backed up under `<GameDir>/.ud-backups/<run-id>`;
LoBo's generated config has a backup under `<LoBoDir>/.ud-backups`. Doctor keeps
original PBOs in `<LoBoDir>/_ud-orig`. The mission installer mirrors the repo's
core and named templates, removing stale files there; edit their repo sources,
not installed copies. Unrelated missions are preserved. A custom LoBo root
config must be merged before setup; it will not be silently replaced.
Correct any reported failure and rerun. It is safe to rerun after pulling
changes, but is not an atomic transaction across both data folders.

`runtime/manifest.json` checks the small runtime payload's SHA-256 hashes before
installation and checks installed files afterward. Update the relevant hash
after intentionally editing a payload file.

## Play

```powershell
pwsh -File ./play-guerrilla.ps1 `
  -GameDir 'C:/Games/CWA Classic' `
  -LoBoDir 'C:/Games/@LoBo' `
  -GameExe './dist/x64-win-rwdi/PoseidonGame.exe'
```

This launches the rebuilt engine and mounts LoBo plus `deps/@LoBo_Deps` when
present. Add `-Mission 'Missions/Guerrilla.Sinai'` for a direct launch. The old
`@lobofixup` shadow mod is not needed for normal play after setup.

## Development tests

Install Rust/Cargo for Trident. Copy `.trident.env.example` to `.trident.env` in
`arma_CWR`; set `OFPR_GAME_DIR` to the built binary directory and `OFPR_DATA_DIR`
to prepared Classic (forward slashes work in paths). Build Trident with
`cargo build --manifest-path engine/Trident/Cargo.toml`.

Some older LoBo tests still mount `tests/fixtures/mods-lobo/@lobofixup`. Generate
that ignored fixture on Windows with its `gen-patched-pbos.ps1`, passing
`-LoBoDir` and `-Tools`. Tests with relative `@LoBo` paths assume the mod is
beside Classic or the checkout; check the test's `--mods-dir` for other layouts.
Demo and full retail Remaster packages are only needed for their respective
integration/UI lanes, not normal Classic + LoBo play. See `../CLAUDE.md` for
those lanes' `packages/Demo` and `packages/Game` paths.

Run the setup regression test with `pwsh -File tests/runtime/test-setup.ps1`.
It uses temporary synthetic data; actual engine smoke tests require real data.

## Licenses and fonts

UD's compatibility configuration keeps the source license. The APL-SA text
resources in `thirdparty/classic-ui` and OFL fonts in `thirdparty/guerrilla-fonts`
have separate notices and are excluded from the engine's GPL grant. No base
game or LoBo PBOs are in Git.

The old Classic test bed used renamed Microsoft fonts. Fresh installs instead
use Roboto, Caveat, Unuaranga Kuriero, Vollkorn and Oswald from the Remaster,
matching the engine's mappings. Text appearance differs from that local
workaround. Those Microsoft font bytes are not included in the repository.
