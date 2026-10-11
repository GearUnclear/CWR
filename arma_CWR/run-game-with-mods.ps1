<#
.SYNOPSIS
    Builds and launches PoseidonGame with mod folders mounted (@LoBo by default).

.DESCRIPTION
    A thin wrapper around run-game.ps1 that resolves mod folders to absolute
    paths, applies the @LoBo fixup rule, and hands the result to the engine as
    --mod / --mods-dir. Build, launch and console-output capture all stay in
    run-game.ps1, so this script only owns the mod part.

    WHY ABSOLUTE PATHS: the engine resolves a relative --mod entry against
    --mods-dir when one is given, otherwise against the process CWD, the -C
    game dir, then the managed user Mods dir (ModCollection.cpp,
    ResolveModPathList). An entry that resolves to nothing is FATAL at startup.
    Because the game launches detached, that failure would only surface later
    in the stderr log, so this script resolves and validates every entry up
    front and fails in the terminal instead.

    THE @LoBo FIXUP: tests/fixtures/mods-lobo/@lobofixup is a tracked fixture
    mod whose bin/config.cpp preloads the LoBo addons the Guerrilla templates
    spawn from and #includes the @LoBo war rosters
    (guerrilla-mode/config/lobo-factions.hpp). It must mount AFTER @LoBo.
    Whenever @LoBo is in the mod list this script appends it automatically
    (suppress with -NoLoBoFixup). It no longer needs any generated files:
    stock @LoBo's malformed "0.0.1" tracerColor floats used to abort the
    config parse, but the reader now coerces them at load (only
    --strict-config keeps them raw) and `PoseidonTools mod doctor <@LoBo>
    --fix` repairs the bytes in place. The fixture's optional shadow pbos
    from gen-patched-pbos.ps1 are used when present and skipped when not.

    MOUNT ORDER is left-to-right: later entries shadow earlier ones. The order
    you pass to -Mod is preserved, with the fixup appended at the end.

.PARAMETER Mod
    Mod folder(s) to mount, in mount order. An entry may be an absolute path,
    or a folder name / relative path resolved against -ModsDir, then the repo
    root, then -DataDir. Default: @LoBo

.PARAMETER ModsDir
    Base directory that relative -Mod entries resolve against, and the
    directory the in-game MODS screen scans.
    Default: D:\Arma_CWA (the repo root's parent layout, where @LoBo lives)

.PARAMETER NoLoBoFixup
    Do not auto-append the @lobofixup fixture mod when @LoBo is mounted, e.g.
    to check what a session with @LoBo alone (rosters from
    tools/lobo/install-lobo-factions.ps1 only) offers.

.PARAMETER DataDir
    The Arma: Cold War Assault install (data) directory, passed through.
    Default: D:\Arma_CWA\ARMA Cold War Assault [Classic]

.PARAMETER Preset
    CMake configure preset / build dir name, passed through.
    Default: win-x64-clang-rwdi

.PARAMETER Mission
    Optional mission folder to boot straight into, resolved relative to
    DataDir (e.g. 'Missions\Guerrilla.Sinai'). Passed through. Omit to boot to
    the main menu.

.PARAMETER SkipBuild
    Skip the build step and launch whatever is staged in dist/. Passed through.

.PARAMETER LogDir
    Directory for the captured output files. Passed through.
    Default: <repo>\logs

.PARAMETER NoLog
    Disable all output capture. Passed through.

.EXAMPLE
    .\run-game-with-mods.ps1
        Build + launch to the main menu with @LoBo (+ fixup) mounted.

.EXAMPLE
    .\run-game-with-mods.ps1 -Mission 'Missions\Guerrilla.Sinai'
        Jump straight into the Sinai template (its world lives in @LoBo).

.EXAMPLE
    .\run-game-with-mods.ps1 -Mod '@LoBo','C:\mods\@other' -SkipBuild
        Mount two mods, no rebuild. @other shadows @LoBo where they collide.
#>
param(
    [string[]]$Mod = @('@LoBo'),
    [string]$ModsDir = 'D:\Arma_CWA',
    [switch]$NoLoBoFixup,
    [string]$DataDir = 'D:\Arma_CWA\ARMA Cold War Assault [Classic]',
    [string]$Preset = 'win-x64-clang-rwdi',
    [string]$Mission,
    [switch]$SkipBuild,
    [string]$LogDir = "$PSScriptRoot\logs",
    [switch]$NoLog
)

$ErrorActionPreference = 'Stop'
$RepoRoot = $PSScriptRoot

if (-not $Mod -or $Mod.Count -eq 0) {
    throw "-Mod is empty; use run-game.ps1 for an unmodded launch."
}
if (-not (Test-Path -LiteralPath $ModsDir -PathType Container)) {
    throw "ModsDir not found: $ModsDir"
}

# Resolve one -Mod entry to an absolute directory, mirroring the engine's
# search order but failing here (in the terminal) rather than at startup.
function Resolve-ModPath {
    param([string]$Entry)

    if ([System.IO.Path]::IsPathRooted($Entry)) {
        if (Test-Path -LiteralPath $Entry -PathType Container) {
            return [System.IO.Path]::GetFullPath($Entry)
        }
        throw "Mod not found: $Entry"
    }

    foreach ($root in @($ModsDir, $RepoRoot, $DataDir)) {
        $candidate = Join-Path $root $Entry
        if (Test-Path -LiteralPath $candidate -PathType Container) {
            return [System.IO.Path]::GetFullPath($candidate)
        }
    }
    throw ("Mod '{0}' not found under any of: {1}, {2}, {3}" -f $Entry, $ModsDir, $RepoRoot, $DataDir)
}

$resolved = @()
foreach ($entry in $Mod) {
    $path = Resolve-ModPath $entry
    if ($resolved -contains $path) {
        Write-Host "Skipping duplicate mod entry: $path"
        continue
    }
    $resolved += $path
    # Not fatal: a mod may legitimately ship only bin/ or dta/ overrides.
    if (-not (Test-Path -LiteralPath (Join-Path $path 'addons') -PathType Container)) {
        Write-Warning "Mod has no addons\ subfolder (mounting anyway): $path"
    }
}

# @LoBo fixup: the fixture's bin/config.cpp must mount AFTER @LoBo (see .DESCRIPTION).
$fixupDir = Join-Path $RepoRoot 'tests\fixtures\mods-lobo\@lobofixup'
$hasLoBo = @($resolved | Where-Object { (Split-Path $_ -Leaf) -eq '@LoBo' }).Count -gt 0
if ($hasLoBo -and -not $NoLoBoFixup -and ($resolved -notcontains [System.IO.Path]::GetFullPath($fixupDir))) {
    $resolved += [System.IO.Path]::GetFullPath($fixupDir)
}

# The engine takes ONE --mod option whose value is a ';'-separated list; it is
# a scalar option, so a repeated flag would not accumulate. Start-Process does
# not quote array elements, so a value containing a space needs its own quotes.
$modValue = $resolved -join ';'
if ($modValue -match '\s') { $modValue = '"' + $modValue + '"' }
$modsDirValue = [System.IO.Path]::GetFullPath($ModsDir)
if ($modsDirValue -match '\s') { $modsDirValue = '"' + $modsDirValue + '"' }

Write-Host "Mods (mount order, later shadows earlier):"
$resolved | ForEach-Object { Write-Host "  $_" }

# Delegate build + launch + log capture to run-game.ps1.
$runGame = Join-Path $RepoRoot 'run-game.ps1'
if (-not (Test-Path -LiteralPath $runGame)) {
    throw "run-game.ps1 not found next to this script: $runGame"
}

$forward = @{
    DataDir   = $DataDir
    Preset    = $Preset
    LogDir    = $LogDir
    ExtraArgs = @('--mods-dir', $modsDirValue, '--mod', $modValue)
}
if ($Mission)  { $forward.Mission = $Mission }
if ($SkipBuild) { $forward.SkipBuild = $true }
if ($NoLog)     { $forward.NoLog = $true }

& $runGame @forward
