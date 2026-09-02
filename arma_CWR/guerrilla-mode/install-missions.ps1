<#
.SYNOPSIS
    Installs the Guerrilla Mode shared script core and the mission templates
    into the game data directory.

.DESCRIPTION
    Installs two things:

      1. the SHARED SCRIPT CORE, guerrilla-mode/core -> <GameDir>\gmcore\
      2. every guerrilla-mode/mission/<Prefix>.<World> template ->
         <GameDir>\Missions\

    WHY THE CORE IS SEPARATE: there is exactly ONE copy of the manager
    scripts in the repo (guerrilla-mode/core), not one per template. A
    mission's own init.sqs is a two-line bootstrap that runs
    `[] exec "\gmcore\init.sqs"`. The LEADING BACKSLASH is what makes that
    reach outside the mission folder: OpenScript (Game/Scripting/Scripts.cpp)
    strips it and resolves the rest against the game data root, checking the
    mounted pbo banks first (a gmcore.pbo in any addons\ dir would serve too)
    and then a loose file on disk. So the loose directory <GameDir>\gmcore\
    sitting beside <GameDir>\Missions\ needs no engine change. The core is
    installed BEFORE the templates: a template without it boots into a
    mission with no managers.

    WHY THE TEMPLATE LOCATION: the new-game flow (main menu GUERRILLA button ->
    IDD_GUERRILLA_NEW_GAME) launches the template by resolving the relative
    path "missions\Guerrilla.<World>" against the game working directory --
    either a PBO "missions\Guerrilla.<World>.pbo" (FilePathExists) or an
    unbanked directory "missions\Guerrilla.<World>\mission.sqm"
    (QIFStreamB::FileExist). See DisplayMain::OnChildDestroyed, case
    IDD_GUERRILLA_NEW_GAME, engine/Poseidon/UI/OptionsUIApp.cpp (~line 883).
    The game working directory is the data dir (GameDir), so the unbanked
    form lands at <GameDir>\Missions\Guerrilla.<World>\.

    WORLD GATE: a template is only installed when its world is actually
    loadable in GameDir (a <GameDir>\Worlds\<World>.wrp exists). Installing
    a mission whose world is missing is a trap: the single-mission browser
    still lists it, and launching it silently keeps the CURRENT world, so
    the player spawns at the mission's coordinates on the WRONG island
    (typically in open sea) and drowns. Templates for worlds that ship in
    mod PBOs (e.g. Guerrilla.Sinai - Sinai lives inside @LoBo\addons\
    lost.pbo - and Guerrilla.Lebanon80 - Lebanon80 lives inside
    @LoBo\addons\LoBo_Leb.pbo - both undetectable by filename) must be
    named in -IncludeWorld to install.

    Idempotent: re-running mirrors the repo core and templates over any
    previous install (stale files inside <GameDir>\gmcore and inside each
    installed template are removed, and a previously installed template that
    is now world-gated out is REMOVED; unrelated missions are untouched).

.PARAMETER GameDir
    The Arma: Cold War Assault install (data) directory.
    Default: D:\Arma_CWA\ARMA Cold War Assault [Classic]

.PARAMETER IncludeWorld
    World name(s) to install even though <GameDir>\Worlds\<World>.wrp does
    not exist - for worlds that live inside (mod) PBOs. The game must then
    be launched with that mod loaded or the mission is unloadable.

.EXAMPLE
    .\install-missions.ps1
    .\install-missions.ps1 -GameDir 'C:\Games\ArmaCWA'
    .\install-missions.ps1 -IncludeWorld Sinai,Lebanon80
#>
param(
    [string]$GameDir = 'D:\Arma_CWA\ARMA Cold War Assault [Classic]',
    [string[]]$IncludeWorld = @()
)

$ErrorActionPreference = 'Stop'

if (-not (Test-Path -LiteralPath $GameDir -PathType Container)) {
    throw "GameDir not found: $GameDir"
}

$missionRoot = Join-Path $PSScriptRoot 'mission'
if (-not (Test-Path -LiteralPath $missionRoot -PathType Container)) {
    throw "Template source not found: $missionRoot"
}

# ---- 1. the shared script core -> <GameDir>\gmcore ------------------------
# Installed first and unconditionally: it carries no world data, so no world
# gate applies, and every template's two-line init.sqs is dead without it.
$coreRoot = Join-Path $PSScriptRoot 'core'
if (-not (Test-Path -LiteralPath $coreRoot -PathType Container)) {
    throw "Script core source not found: $coreRoot"
}
$coreDest = Join-Path $GameDir 'gmcore'
$null = & robocopy $coreRoot $coreDest /MIR /NJH /NJS /NDL /NFL /NC /NS /NP
if ($LASTEXITCODE -ge 8) {
    throw "robocopy failed for the script core (exit $LASTEXITCODE)"
}
$coreVerb = if ($LASTEXITCODE -eq 0) { 'Up to date' } else { 'Installed' }
Write-Output ("{0}: script core -> {1}" -f $coreVerb, $coreDest)

# ---- 2. the mission templates -> <GameDir>\Missions -----------------------
$destRoot = Join-Path $GameDir 'Missions'
if (-not (Test-Path -LiteralPath $destRoot -PathType Container)) {
    # [System.IO.Directory]::CreateDirectory is fully literal: New-Item -Path
    # would treat the bracketed data-dir parent ([Classic]) as a glob and fail.
    [void][System.IO.Directory]::CreateDirectory($destRoot)
    Write-Output "Created: $destRoot"
}

# Every template below is script-free: none of them carries a scripts\
# directory any more, they all reach into <GameDir>\gmcore instead.
# Guerrilla.<World> = the playable campaign templates (two-line init.sqs, the
# whole core); Showcase.<World> = the issue #9 systems-showcase missions (same
# two-line init.sqs + a mission-local showcase\ overlay); Qrf.<World>,
# Market.<World> and Undercover.<World> = the reference slices, each with its
# own bootstrap init.sqs that execs only the ONE core policy script it exists
# to demonstrate (\gmcore\scripts\qrf.sqs / market.sqs / undercover.sqs, plus
# lib.sqs) and keeps its debug menu and HUD mission-local. Showcase.*,
# Undercover.*, Qrf.* and Market.* are surfaced as direct main-menu launch
# buttons (kReferenceMissions, UI/OptionsUIApp.cpp) that only appear when the
# mission is installed.
$templates = Get-ChildItem -LiteralPath $missionRoot -Directory |
    Where-Object { $_.Name -match '^(Guerrilla|Showcase|Undercover|Qrf|Market)\.' }
if (-not $templates) {
    throw "No Guerrilla.*/Showcase.*/Undercover.*/Qrf.*/Market.* templates under $missionRoot"
}

foreach ($tpl in $templates) {
    $dest = Join-Path $destRoot $tpl.Name

    # World gate (see .DESCRIPTION): template name is <Prefix>.<World>.
    $world = $tpl.Name.Substring($tpl.Name.IndexOf('.') + 1)
    $wrp = Join-Path $GameDir "Worlds\$world.wrp"
    if (-not (Test-Path -LiteralPath $wrp) -and $IncludeWorld -notcontains $world) {
        if (Test-Path -LiteralPath $dest) {
            Remove-Item -LiteralPath $dest -Recurse -Force -Confirm:$false
            Write-Output ("Removed stale install: {0} (world '{1}' not in {2}\Worlds; use -IncludeWorld {1} for mod-PBO worlds)" -f $tpl.Name, $world, $GameDir)
        } else {
            Write-Output ("Skipped: {0} (world '{1}' not in {2}\Worlds; use -IncludeWorld {1} for mod-PBO worlds)" -f $tpl.Name, $world, $GameDir)
        }
        continue
    }
    # /MIR mirrors the template (removes stale files from previous installs);
    # scoped to this template's own folder only.
    $null = & robocopy $tpl.FullName $dest /MIR /NJH /NJS /NDL /NFL /NC /NS /NP
    if ($LASTEXITCODE -ge 8) {
        throw "robocopy failed for $($tpl.Name) (exit $LASTEXITCODE)"
    }
    $verb = if ($LASTEXITCODE -eq 0) { 'Up to date' } else { 'Installed' }
    Write-Output ("{0}: {1} -> {2}" -f $verb, $tpl.Name, $dest)
}

Write-Output "Done. Templates are picked up by the GUERRILLA new-game menu (missions\Guerrilla.<World>) and load their managers from $coreDest."
# robocopy's informational exit codes (1 = files copied) must not leak as
# script failure.
exit 0
