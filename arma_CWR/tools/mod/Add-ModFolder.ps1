<#
.SYNOPSIS
    F1 installer hook: take a downloaded mod folder from "picked on disk" to
    "mounted and checked" with no terminal required.

.DESCRIPTION
    Issue #54 (mod-intake sustainability umbrella), step F1: "a player can go
    from a downloaded mod folder to a new-game island without a terminal."
    This is the "add a mod folder" half; Add-Island.ps1 is the other half.
    Issue #29 (the player installer) does not exist yet - this script is
    written to be double-clicked (via Add-ModFolder.cmd) or wrapped by that
    future installer either way, so every step is printed as it runs and
    the exit code is the only thing a wrapper needs to check.

    Steps, in order:
      (a) VALIDATE - does -ModDir look like a mod folder (addons\ with a
          *.pbo, or bin\config.cpp)? See ModIntake.psm1's
          Test-ModFolderShape.
      (b) DOCTOR - `PoseidonTools mod doctor <dir>` in report mode. Exit 1
          means defects were found (not a failure - see ModDoctor's exit
          convention below); offer to --fix them (prompt, or -Fix, or
          -NonInteractive to skip). Exit 2 is a hard I/O error and stops
          the script.
      (c) PROBE - `PoseidonTools guerrilla probe --data-dir <GameDir> --mod
          <dir>`. A non-zero exit here is a WARNING, not a stop: content a
          mod ships legitimately unfinished (@LoBo's CoC diver classes miss
          their model files) fails the probe on purpose.
      (d) LINT - `PoseidonTools guerrilla lint --data-dir <GameDir> --mod
          <dir>`. Always run: a mod with no CfgGuerrillaFactions of its own
          still lints the package's existing faction table (factions=0 is a
          valid, unremarkable result for a mod that ships none).
      (e) MOUNT - record the mod's absolute path in <GameDir>\ud-mods.txt
          (idempotent, one path per line - see ModIntake.psm1's
          Add-UdModsEntry) and print the run-game-with-mods.ps1 invocation
          that mounts it. Neither run-game.ps1 nor
          run-game-with-mods.ps1 read a persisted mod list (their -Mod
          parameter is supplied fresh each launch) and this script does not
          own either of them, so ud-mods.txt is a NEW convention, not an
          existing one being reused - issue #29's installer is expected to
          read it to build its own launch command.
      (f) WORLDS - if the mod ships any *.wrp (found by scanning its pbo
          headers - the same header-only walk
          guerrilla-mode/install-missions.ps1 uses to gate template
          installs), list the guessed world class name(s) and point at
          Add-Island.ps1.

    ModDoctor's exit convention (engine/Poseidon/Asset/Addon/ModDoctor.cpp
    via ModCommand.cpp): 0 = nothing to do, 1 = defects found in report
    mode OR some findings were not patchable even after --fix (still not a
    hard failure), 2 = an I/O error (unreadable pbo, can't write, can't
    back up) - that one this script treats as fatal.

.PARAMETER ModDir
    The mod folder to add. Omit to pick one with a Windows folder browser
    (System.Windows.Forms.FolderBrowserDialog) - the double-click path.

.PARAMETER GameDir
    The Arma: Cold War Assault install (data) directory the mod will be
    checked and mounted against. Default: D:\Arma_CWA\ARMA Cold War Assault
    [Classic]

.PARAMETER Tools
    PoseidonTools executable. Default: the dist build next to this repo.

.PARAMETER Fix
    Apply mod doctor's --fix without asking.

.PARAMETER NonInteractive
    Never prompt (used by issue #29's installer and by tests). Implies
    "skip the fix" unless -Fix is also given.

.EXAMPLE
    .\Add-ModFolder.ps1
        Opens a folder picker, then validates/doctors/probes/lints/mounts
        whatever was chosen, asking before applying any doctor fix.

.EXAMPLE
    .\Add-ModFolder.ps1 -ModDir D:\Arma_CWA\@LoBo -NonInteractive
        Non-interactive report-only run: prints every step's findings,
        mounts @LoBo into ud-mods.txt, applies no fix.

.EXAMPLE
    .\Add-ModFolder.ps1 -ModDir 'C:\mods\@Other' -Fix
        Applies mod doctor --fix with no prompt if defects are found.
#>
param(
    [string]$ModDir,
    [string]$GameDir = 'D:\Arma_CWA\ARMA Cold War Assault [Classic]',
    [string]$Tools = (Join-Path $PSScriptRoot '..\..\dist\x64-win-rwdi\PoseidonTools.exe'),
    [switch]$Fix,
    [switch]$NonInteractive
)

$ErrorActionPreference = 'Stop'
Import-Module (Join-Path $PSScriptRoot 'ModIntake.psm1') -Force

function Write-Step {
    param([string]$Letter, [string]$Text)
    Write-Host ""
    Write-Host "== ($Letter) $Text ==" -ForegroundColor Cyan
}

function Fail {
    param([string]$Message)
    Write-Host "ERROR: $Message" -ForegroundColor Red
    exit 2
}

# ---- pick a folder with no terminal, if none was given ---------------------
if (-not $ModDir) {
    if ($NonInteractive) { Fail "-ModDir is required under -NonInteractive (no folder picker)." }
    Add-Type -AssemblyName System.Windows.Forms
    $dialog = New-Object System.Windows.Forms.FolderBrowserDialog
    $dialog.Description = 'Pick the mod folder to add (the one containing addons\ or bin\)'
    $dialog.ShowNewFolderButton = $false
    $result = $dialog.ShowDialog()
    if ($result -ne [System.Windows.Forms.DialogResult]::OK -or -not $dialog.SelectedPath) {
        Fail "No folder selected."
    }
    $ModDir = $dialog.SelectedPath
}
$ModDir = [System.IO.Path]::GetFullPath($ModDir)

if (-not (Test-Path -LiteralPath $Tools -PathType Leaf)) {
    Fail "PoseidonTools not found: $Tools (build it, or pass -Tools)"
}
if (-not (Test-Path -LiteralPath $GameDir -PathType Container)) {
    Fail "GameDir not found: $GameDir"
}

Write-Host "Mod folder: $ModDir"
Write-Host "Game dir:   $GameDir"

# ---- (a) validate -----------------------------------------------------------
Write-Step 'a' 'Validate mod folder shape'
$shape = Test-ModFolderShape -ModDir $ModDir
if (-not $shape.Looks) {
    Fail $shape.Reason
}
if ($shape.AddonsDir) {
    Write-Host ("Looks like a mod folder: {0} ({1} pbo(s))" -f $shape.AddonsDir, $shape.PboCount)
} else {
    Write-Host ("Looks like a bin-only overlay mod: {0}" -f $shape.ConfigCpp)
}

# ---- (b) doctor ---------------------------------------------------------
Write-Step 'b' 'mod doctor (defect report)'
& $Tools mod doctor $ModDir
$doctorExit = $LASTEXITCODE
if ($doctorExit -eq 2) {
    Fail "mod doctor failed with an I/O error (exit 2) - see the output above."
}
$defectsFound = ($doctorExit -eq 1)

if ($defectsFound) {
    $doFix = $false
    if ($Fix) {
        $doFix = $true
    } elseif ($NonInteractive) {
        Write-Host "Defects found; skipping --fix (-NonInteractive, pass -Fix to apply automatically)."
    } else {
        $answer = Read-Host "Defects found above. Apply --fix now? [Y/n]"
        $doFix = ($answer -eq '' -or $answer -match '^[Yy]')
    }
    if ($doFix) {
        Write-Host "Applying mod doctor --fix ..."
        & $Tools mod doctor $ModDir --fix
        $fixExit = $LASTEXITCODE
        if ($fixExit -eq 2) {
            Fail "mod doctor --fix failed with an I/O error (exit 2) - see the output above."
        }
        # fixExit 1 here means some findings were NOT patchable (e.g. a
        # compressed config entry) even though the patchable ones were
        # applied - informational, not a stop.
    }
} else {
    Write-Host "Nothing to do."
}

# ---- (c) probe ------------------------------------------------------------
Write-Step 'c' 'guerrilla probe (spawn gate)'
& $Tools guerrilla probe --data-dir $GameDir --mod $ModDir
if ($LASTEXITCODE -ne 0) {
    Write-Host ("WARNING: probe reported failing classes (exit {0}) - this can be legitimate " -f $LASTEXITCODE) -ForegroundColor Yellow
    Write-Host "(a mod may ship unfinished content on purpose, e.g. @LoBo's CoC diver classes)." -ForegroundColor Yellow
}

# ---- (d) lint ---------------------------------------------------------------
Write-Step 'd' 'guerrilla lint (faction descriptor audit)'
& $Tools guerrilla lint --data-dir $GameDir --mod $ModDir
if ($LASTEXITCODE -ne 0) {
    Write-Host ("NOTE: lint reported unresolved descriptor keys (exit {0}); see FACTION-PACKS.md." -f $LASTEXITCODE) -ForegroundColor Yellow
}

# ---- (e) mount ----------------------------------------------------------
Write-Step 'e' 'Mount (record in ud-mods.txt)'
$added = Add-UdModsEntry -GameDir $GameDir -ModDirPath $ModDir
$listPath = Get-UdModsListPath -GameDir $GameDir
if ($added) {
    Write-Host "Recorded in $listPath"
} else {
    Write-Host "Already recorded in $listPath"
}
$runWithMods = Join-Path (Split-Path (Split-Path $PSScriptRoot -Parent) -Parent) 'run-game-with-mods.ps1'
Write-Host "To launch with this mod mounted:"
Write-Host ("  {0} -Mod '{1}'" -f $runWithMods, $ModDir)
Write-Host "or, once issue #29's installer reads ud-mods.txt, just relaunch the game."

# ---- (f) worlds ---------------------------------------------------------
Write-Step 'f' 'World check'
$worlds = Get-ModWorldClasses -ModDir $ModDir
if ($worlds.Count -eq 0) {
    Write-Host "This mod ships no *.wrp - nothing to scaffold."
} else {
    Write-Host "This mod ships world(s):"
    foreach ($w in $worlds) {
        Write-Host ("  {0}  (from {1})" -f $w.World, $w.Source)
    }
    Write-Host "Run Add-Island.ps1 for each to make it a playable Guerrilla island, e.g.:"
    $addIsland = Join-Path $PSScriptRoot 'Add-Island.ps1'
    foreach ($w in $worlds) {
        Write-Host ("  {0} -World {1} -ModDir '{2}'" -f $addIsland, $w.World, $ModDir)
    }
}

Write-Host ""
Write-Host "Done." -ForegroundColor Green
exit 0
