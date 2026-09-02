# Mod intake hooks (issue #54 step F1)

Two standalone, double-clickable PowerShell workflows that take a player
from "downloaded mod folder" to "playable Guerrilla island" with no
terminal. They are the F1 hooks issue #29's future installer wraps ("add a
mod folder" and "add an island"); until that installer exists, either
script can be run directly or via its `.cmd` launcher.

## Add-ModFolder.ps1 / Add-ModFolder.cmd

Checks a mod folder and mounts it.

1. **Validate** - does it look like a mod folder (an `addons\`/`AddOns\`
   dir with at least one `*.pbo`, or a `bin\config.cpp`)?
2. **Doctor** - `PoseidonTools mod doctor <dir>` (report mode). If defects
   are found, offers `--fix` (prompt, `-Fix`, or `-NonInteractive` to skip).
3. **Probe** - `PoseidonTools guerrilla probe --data-dir <GameDir> --mod
   <dir>`. A non-zero exit is shown as a warning, not a stop - a mod can
   legitimately ship unfinished content (e.g. @LoBo's CoC diver classes
   fail this check on purpose, missing their model files).
4. **Lint** - `PoseidonTools guerrilla lint --data-dir <GameDir> --mod
   <dir>`. Always run, whether or not the mod ships its own
   `CfgGuerrillaFactions` - it also reports on the package's existing
   faction table.
5. **Mount** - records the mod's absolute path in `<GameDir>\ud-mods.txt`
   (idempotent) and prints the `run-game-with-mods.ps1 -Mod <path>`
   invocation to launch with it.
6. **Worlds** - if the mod ships any `*.wrp` (found by scanning its pbo
   headers, not by extracting anything), lists the guessed world class
   name(s) and points at `Add-Island.ps1`.

With no `-ModDir`, it opens a Windows folder picker instead of requiring a
command line - the double-click path.

```powershell
# double-click Add-ModFolder.cmd, or:
.\Add-ModFolder.ps1                                   # folder picker
.\Add-ModFolder.ps1 -ModDir D:\Arma_CWA\@LoBo          # prompts before --fix
.\Add-ModFolder.ps1 -ModDir 'C:\mods\@Other' -Fix      # applies --fix, no prompt
.\Add-ModFolder.ps1 -ModDir 'C:\mods\@Other' -NonInteractive   # report-only, never prompts
```

Parameters: `-ModDir`, `-GameDir` (default `D:\Arma_CWA\ARMA Cold War
Assault [Classic]`), `-Tools` (default the dist `PoseidonTools.exe`
next to this repo), `-Fix`, `-NonInteractive`. Exit 0 on success, 2 on a
hard error (bad folder, tool not found, `mod doctor` I/O error).

## Add-Island.ps1 / Add-Island.cmd

Turns one `CfgWorlds` world into an installed Guerrilla Mode island.

1. **Resolve `-World`.** If omitted, scans the package plus `-ModDir` (or
   its default - see below) for `*.wrp` entries by reading pbo headers, and
   either prompts for a pick by number or, under `-NonInteractive`, prints
   the list and exits 2.
2. **Scaffold** - `PoseidonTools guerrilla scaffold --world <W> --data-dir
   <GameDir> [--mod ...] --out <OutDir> --outposts <N> [--keep-zones]` and
   shows its zone summary. A refusal (no `Names` block, world not found,
   unreadable `.wrp`, ...) prints the tool's own message and exits 2.
3. **Install** - runs `guerrilla-mode/install-missions.ps1 -GameDir
   <GameDir>` (installs the shared script core, the faction library, and
   every world-gated template, including the one just scaffolded).
   Skippable with `-NoInstall`.
4. Prints where the template landed and that GUERRILLA in the main menu
   now lists the island.

```powershell
# double-click Add-Island.cmd, or:
.\Add-Island.ps1                                # lists worlds, prompts for a pick
.\Add-Island.ps1 -World Cain -Outposts 2        # scaffold + install
.\Add-Island.ps1 -World Sinai -ModDir 'D:\Arma_CWA\@LoBo' -NoInstall   # scaffold only
```

Parameters: `-World`, `-ModDir` (string[], default: the lines of
`<GameDir>\ud-mods.txt` if it exists, else every `@*` folder beside
`GameDir`), `-GameDir`, `-Tools`, `-Outposts` (default 3), `-KeepZones`,
`-Install` (default on) / `-NoInstall`, `-OutDir` (override for
verification against a scratch directory - a template written anywhere
else will not be picked up by `install-missions.ps1`), `-NonInteractive`.
Exit 0 on success, 2 on a hard error or an unresolved world pick.

## The `ud-mods.txt` convention

Neither `run-game.ps1` nor `run-game-with-mods.ps1` (both in the repo
root) read a persisted mod list - their `-Mod` parameter is supplied fresh
per launch, and nothing in the repo already had a "list of mods to mount"
file. `Add-ModFolder.ps1`'s mount step introduces `<GameDir>\ud-mods.txt`:
one absolute mod-folder path per line, written idempotently. It is
consumed by:

- `Add-Island.ps1`'s `-ModDir` default, so scaffolding an island from a
  mod you already added doesn't need `-ModDir` typed again.
- Issue #29's future installer, which is expected to read it to build its
  own launch command (`run-game-with-mods.ps1 -Mod <line1>,<line2>,...`)
  instead of re-deriving the mounted mod set some other way.

It is a plain text file, safe to hand-edit (blank lines and `#` comments
are ignored on read).

## ModIntake.psm1

Shared helpers both scripts import: `Get-PboEntryNames` (the pbo
header-only entry-table walk, generalised from
`guerrilla-mode/install-missions.ps1`'s `Get-PboWorldEntries` to take an
extension instead of being hardcoded to `*.wrp`), `Test-ModFolderShape`,
`Get-ModWorldClasses` / `Get-PackageWorldClasses` (the world pickers), and
the `ud-mods.txt` read/write functions. Not meant to be imported by
anything outside this directory; the pbo walk is a header-only, ~KB-sized
read even against a 100+ MB pbo, the same way the install script's copy is.

## Verifying without a live game

Both scripts accept `-NonInteractive` for scripted/CI-style runs, and
`Add-Island.ps1` accepts `-OutDir`/`-NoInstall` specifically so a
verification run never touches the live game directory's `Missions\` or
this repo's tracked `guerrilla-mode/mission/`:

```powershell
.\Add-ModFolder.ps1 -ModDir D:\Arma_CWA\@LoBo -NonInteractive
.\Add-Island.ps1 -World Cain -NonInteractive -NoInstall -Outposts 2 -OutDir <scratch dir>
```
