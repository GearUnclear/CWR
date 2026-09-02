<#
.SYNOPSIS
    F1 installer hook: turn one CfgWorlds world into a playable Guerrilla
    Mode island with no terminal required.

.DESCRIPTION
    Issue #54 (mod-intake sustainability umbrella), step F1's other half -
    Add-ModFolder.ps1 gets a mod mounted and checked; this script takes a
    world it (or the base package) offers and makes it choosable from the
    GUERRILLA main-menu button. Issue #29's future installer is expected to
    wrap this the same way.

    Steps:
      1. Resolve -World. If not given, list every world the package plus
         -ModDir offer (scanning pbo headers for *.wrp, the same header-only
         walk install-missions.ps1 and Add-ModFolder.ps1 use) and let the
         player pick by number. -NonInteractive with no -World fails (there
         is no terminal to prompt in the real installer flow, so this
         matches its constraint).
      2. Run `PoseidonTools guerrilla scaffold --world <W> --data-dir
         <GameDir> [--mod ...] --out <OutDir> [--outposts N]
         [--keep-zones]` and show its zone summary. A refusal (missing
         Names block, unreadable .wrp, etc: the tool's own stderr message)
         stops the script with exit 2 - there is nothing this wrapper can
         usefully add on top of that message.
      3. Run guerrilla-mode/install-missions.ps1 -GameDir <GameDir>, which
         installs the shared script core + faction library and auto-
         detects worlds inside mod pbos, so the freshly scaffolded template
         actually boots. Skippable with -NoInstall (verification/testing:
         install-missions.ps1 rewrites the live game dir, which is unsafe
         to do for real while other sessions are mid-test).
      4. Print where the template landed and that GUERRILLA now lists the
         island.

.PARAMETER World
    CfgWorlds class of the world to scaffold (e.g. Abel, Sinai, Cain). Omit
    to pick from a discovered list.

.PARAMETER ModDir
    Mod folder(s) to mount while scaffolding (and to search when -World is
    omitted). Default: the lines of <GameDir>\ud-mods.txt if that file
    exists (see Add-ModFolder.ps1's mount step), else every "@*" folder
    sitting next to GameDir.

.PARAMETER GameDir
    The Arma: Cold War Assault install (data) directory.
    Default: D:\Arma_CWA\ARMA Cold War Assault [Classic]

.PARAMETER Tools
    PoseidonTools executable. Default: the dist build next to this repo.

.PARAMETER Outposts
    OUTPOST zones to place (passed through to `guerrilla scaffold
    --outposts`). Default 3.

.PARAMETER KeepZones
    Keep an existing template's class Zones block verbatim across a re-run
    (passed through as `guerrilla scaffold --keep-zones`).

.PARAMETER Install
    Run install-missions.ps1 after scaffolding. Default on.

.PARAMETER NoInstall
    Skip the install-missions.ps1 step (equivalent to -Install:$false).
    Use for a dry run against a scratch -OutDir, or when another session is
    mid-test against the live game dir.

.PARAMETER OutDir
    Override for where the template is written. Default:
    <repo>\guerrilla-mode\mission\Guerrilla.<World>, the convention
    install-missions.ps1 expects. Only meant to be overridden for
    verification against a scratch directory - a template written anywhere
    else will not be picked up by install-missions.ps1.

.PARAMETER NonInteractive
    Never prompt. With no -World, print the discovered world list and exit
    2 instead of prompting for a pick.

.EXAMPLE
    .\Add-Island.ps1
        Lists worlds found in the package + default mods, prompts for a
        pick, scaffolds it, then installs.

.EXAMPLE
    .\Add-Island.ps1 -World Cain -Outposts 2
        Scaffolds Cain into guerrilla-mode/mission/Guerrilla.Cain and
        installs it.

.EXAMPLE
    .\Add-Island.ps1 -World Sinai -ModDir 'D:\Arma_CWA\@LoBo' -NoInstall
        Scaffolds Sinai (which lives inside @LoBo's pbos) without touching
        the live game dir's Missions\.
#>
param(
    [string]$World,
    [string[]]$ModDir,
    [string]$GameDir = 'D:\Arma_CWA\ARMA Cold War Assault [Classic]',
    [string]$Tools = (Join-Path $PSScriptRoot '..\..\dist\x64-win-rwdi\PoseidonTools.exe'),
    [ValidateRange(0, 32)][int]$Outposts = 3,
    [switch]$KeepZones,
    [switch]$Install = $true,
    [switch]$NoInstall,
    [string]$OutDir,
    [switch]$NonInteractive
)

$ErrorActionPreference = 'Stop'
Import-Module (Join-Path $PSScriptRoot 'ModIntake.psm1') -Force

function Fail {
    param([string]$Message)
    Write-Host "ERROR: $Message" -ForegroundColor Red
    exit 2
}

if (-not (Test-Path -LiteralPath $Tools -PathType Leaf)) {
    Fail "PoseidonTools not found: $Tools (build it, or pass -Tools)"
}
if (-not (Test-Path -LiteralPath $GameDir -PathType Container)) {
    Fail "GameDir not found: $GameDir"
}
if ($NoInstall) { $Install = $false }

# ---- -ModDir default: ud-mods.txt, else every "@*" beside GameDir ---------
if (-not $ModDir -or $ModDir.Count -eq 0) {
    $fromList = Get-UdModsEntries -GameDir $GameDir
    if ($fromList.Count -gt 0) {
        $ModDir = $fromList
        Write-Host "Using mods from $(Get-UdModsListPath -GameDir $GameDir):"
        foreach ($m in $ModDir) { Write-Host "  $m" }
    } else {
        $parent = Split-Path $GameDir.TrimEnd('\') -Parent
        $ModDir = @()
        if ($parent -and (Test-Path -LiteralPath $parent -PathType Container)) {
            $ModDir = @([System.IO.Directory]::GetDirectories($parent, '@*'))
        }
        if ($ModDir.Count -gt 0) {
            Write-Host "Using mods found beside GameDir:"
            foreach ($m in $ModDir) { Write-Host "  $m" }
        }
    }
}
$ModDir = @($ModDir | Where-Object { $_ -and (Test-Path -LiteralPath $_ -PathType Container) })

# ---- 1. resolve -World ------------------------------------------------------
if (-not $World) {
    Write-Host "No -World given; scanning for worlds offered by the package + mods..."
    $candidates = Get-PackageWorldClasses -GameDir $GameDir -ModDir $ModDir
    if ($candidates.Count -eq 0) {
        Fail "No worlds found (no loose Worlds\*.wrp, no *.wrp inside AddOns\, Dta\, or any mod folder)."
    }
    Write-Host ""
    Write-Host "Worlds available:"
    for ($i = 0; $i -lt $candidates.Count; $i++) {
        Write-Host ("  [{0}] {1}  (from {2})" -f ($i + 1), $candidates[$i].World, $candidates[$i].Source)
    }
    if ($NonInteractive) {
        Write-Host ""
        Write-Host "No -World given under -NonInteractive; pick one of the above and pass -World <name>."
        exit 2
    }
    Write-Host ""
    $pick = Read-Host "Pick a world by number"
    $idx = 0
    if (-not [int]::TryParse($pick, [ref]$idx) -or $idx -lt 1 -or $idx -gt $candidates.Count) {
        Fail "Invalid pick: $pick"
    }
    $World = $candidates[$idx - 1].World
    Write-Host "Selected: $World"
}

# ---- 2. scaffold ------------------------------------------------------------
$RepoRoot = Split-Path $PSScriptRoot -Parent
if (-not $OutDir) {
    $OutDir = Join-Path $RepoRoot "guerrilla-mode\mission\Guerrilla.$World"
}
Write-Host ""
Write-Host "== Scaffolding $World -> $OutDir ==" -ForegroundColor Cyan

$scaffoldArgs = @('guerrilla', 'scaffold', '--world', $World, '--data-dir', $GameDir, '--out', $OutDir,
                  '--outposts', $Outposts)
foreach ($m in $ModDir) { $scaffoldArgs += @('--mod', $m) }
if ($KeepZones) { $scaffoldArgs += '--keep-zones' }

& $Tools @scaffoldArgs
if ($LASTEXITCODE -ne 0) {
    # The tool's own stderr already explains the refusal (missing Names
    # block, world not found in CfgWorlds - with the package's real world
    # list attached, unreadable .wrp, ...); nothing to add on top of it.
    Fail "guerrilla scaffold failed (exit $LASTEXITCODE) - see its message above."
}

# ---- 3. install -------------------------------------------------------------
if ($Install) {
    Write-Host ""
    Write-Host "== Installing (script core, faction library, mission templates) ==" -ForegroundColor Cyan
    $installScript = Join-Path $RepoRoot 'guerrilla-mode\install-missions.ps1'
    if (-not (Test-Path -LiteralPath $installScript -PathType Leaf)) {
        Fail "install-missions.ps1 not found: $installScript"
    }
    & $installScript -GameDir $GameDir
    if ($LASTEXITCODE -ne 0) {
        Fail "install-missions.ps1 failed (exit $LASTEXITCODE) - see its output above."
    }
    Write-Host ""
    Write-Host "Template installed. GUERRILLA in the main menu now lists $World." -ForegroundColor Green
} else {
    Write-Host ""
    Write-Host "Template written to $OutDir (not installed: -NoInstall)."
    Write-Host "Run guerrilla-mode\install-missions.ps1 -GameDir '$GameDir' to install it for real."
}

exit 0
