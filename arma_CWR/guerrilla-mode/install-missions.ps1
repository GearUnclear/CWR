<#
.SYNOPSIS
    Installs the Guerrilla Mode mission templates into the game data directory.

.DESCRIPTION
    Copies every guerrilla-mode/mission/Guerrilla.<World> template into
    <GameDir>\Missions\.

    WHY THAT LOCATION: the new-game flow (main menu GUERRILLA button ->
    IDD_GUERRILLA_NEW_GAME) launches the template by resolving the relative
    path "missions\Guerrilla.<World>" against the game working directory --
    either a PBO "missions\Guerrilla.<World>.pbo" (FilePathExists) or an
    unbanked directory "missions\Guerrilla.<World>\mission.sqm"
    (QIFStreamB::FileExist). See DisplayMain::OnChildDestroyed, case
    IDD_GUERRILLA_NEW_GAME, engine/Poseidon/UI/OptionsUIApp.cpp (~line 883).
    The game working directory is the data dir (GameDir), so the unbanked
    form lands at <GameDir>\Missions\Guerrilla.<World>\.

    Idempotent: re-running mirrors the repo templates over any previous
    install (stale files inside each installed template are removed;
    unrelated missions are untouched).

.PARAMETER GameDir
    The Arma: Cold War Assault install (data) directory.
    Default: D:\Arma_CWA\ARMA Cold War Assault

.EXAMPLE
    .\install-missions.ps1
    .\install-missions.ps1 -GameDir 'C:\Games\ArmaCWA'
#>
param(
    [string]$GameDir = 'D:\Arma_CWA\ARMA Cold War Assault'
)

$ErrorActionPreference = 'Stop'

if (-not (Test-Path -LiteralPath $GameDir -PathType Container)) {
    throw "GameDir not found: $GameDir"
}

$missionRoot = Join-Path $PSScriptRoot 'mission'
if (-not (Test-Path -LiteralPath $missionRoot -PathType Container)) {
    throw "Template source not found: $missionRoot"
}

$destRoot = Join-Path $GameDir 'Missions'
if (-not (Test-Path -LiteralPath $destRoot -PathType Container)) {
    New-Item -ItemType Directory -Path $destRoot | Out-Null
    Write-Output "Created: $destRoot"
}

$templates = Get-ChildItem -LiteralPath $missionRoot -Directory -Filter 'Guerrilla.*'
if (-not $templates) {
    throw "No Guerrilla.* templates under $missionRoot"
}

foreach ($tpl in $templates) {
    $dest = Join-Path $destRoot $tpl.Name
    # /MIR mirrors the template (removes stale files from previous installs);
    # scoped to this template's own folder only.
    $null = & robocopy $tpl.FullName $dest /MIR /NJH /NJS /NDL /NFL /NC /NS /NP
    if ($LASTEXITCODE -ge 8) {
        throw "robocopy failed for $($tpl.Name) (exit $LASTEXITCODE)"
    }
    $verb = if ($LASTEXITCODE -eq 0) { 'Up to date' } else { 'Installed' }
    Write-Output ("{0}: {1} -> {2}" -f $verb, $tpl.Name, $dest)
}

Write-Output "Done. Templates are picked up by the GUERRILLA new-game menu (missions\Guerrilla.<World>)."
# robocopy's informational exit codes (1 = files copied) must not leak as
# script failure.
exit 0
