# Builds the crush variant of the creature addon: the pipeline's uddedushka.pbo
# with a few config lines changed, packed into <git root>\tmp\@kj_crush (gitignored),
# to be mounted IN PLACE OF @uddedushka (never beside it: both carry the class).
# Windows PowerShell 5.1 or later. No game is started.
#
#   powershell -NoProfile -File build-kjcrush.ps1 [-Src <uddedushka.pbo>] [-Height 40]
#
# The lines, and why crush needs them (kaiju-notes\features\crush\NOTES.md):
#   CfgMovesDedushka >> States >> DedWalk:  interpolationRestart = 1;
#       The walk then starts at phase 0 every time he sets off. Without it the
#       walk takes over the phase the idle clip happened to be at
#       (SoldierOldMove.cpp:337-349), and no script can read the phase, so the
#       footfalls kj\crush\loop.sqs works out are in step with the legs only by
#       luck.
#   CfgVehicles >> UDDedushka:  kjHeight = <height in metres>;
#       Every crush size is a fraction of this (kj\crush\init.sqf, KJC_fnSize).
#   CfgVehicles >> UDDedushka:  armorHead, armorBody, armorHands, armorLegs = 20 (were 2, 2, 1, 2);
#       A zone's armour is armor x this. At 2 one Maverick on the head (raw hit
#       1900 against 2000) passes the 0.9 zone rule and kills him outright; at
#       20 nothing in the stock ammo table comes near.
#
# Everything binary stays out of the repo: staging goes to
# kaiju-notes\features\crush\build, the pbo to <git root>\tmp\@kj_crush.
param(
    [string]$Src = '',
    [double]$Height = 40,
    [double]$ZoneArmor = 20,
    [string]$Stage = 'D:\Arma_CWA_worktrees\kaiju-notes\features\crush\build',
    [string]$ModDir = '',
    [string]$Tools = 'D:\Arma_CWA\arma_CWR\dist\x64-win-rwdi\PoseidonTools.exe'
)
$ErrorActionPreference = 'Stop'
$gitRoot = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..\..\..\..\..\..'))
if (-not $Src) { $Src = Join-Path $gitRoot 'tmp\@uddedushka\addons\uddedushka.pbo' }
if (-not $ModDir) { $ModDir = Join-Path $gitRoot 'tmp\@kj_crush' }
foreach ($f in @($Src, $Tools)) { if (-not (Test-Path -LiteralPath $f)) { throw "missing input: $f" } }
$work = Join-Path $Stage 'uddedushka'
if (Test-Path -LiteralPath $work) { Remove-Item -LiteralPath $work -Recurse -Force }
[void][IO.Directory]::CreateDirectory($work)
& $Tools pbo extract $Src $work | Out-Null
if ($LASTEXITCODE -ne 0) { throw "pbo extract failed on $Src" }
$cfgPath = Join-Path $work 'config.cpp'
if (-not (Test-Path -LiteralPath $cfgPath)) { throw "no config.cpp in $Src" }
$cfg = [IO.File]::ReadAllText($cfgPath)

function Add-After([string]$text, [string]$pattern, [string]$line, [string]$what) {
    $m = [regex]::Match($text, $pattern)
    if (-not $m.Success) { throw "config.cpp has no $what; the patch does not fit this addon any more" }
    return $text.Insert($m.Index + $m.Length, $line)
}
if ($cfg -notmatch 'class\s+DedWalk\s*:\s*\w+\s*\{\s*interpolationRestart') {
    $cfg = Add-After $cfg 'class\s+DedWalk\s*:\s*\w+\s*\{' "`r`n            interpolationRestart = 1;   // crush: the walk starts at phase 0" 'States >> DedWalk'
}
if ($cfg -notmatch 'kjHeight') {
    $cfg = Add-After $cfg 'class\s+UDDedushka\s*:\s*\w+\s*\{' "`r`n        kjHeight = $Height;              // crush: his height in metres" 'CfgVehicles >> UDDedushka'
}
# Zone armour: a Maverick on the head or a bomb at his feet must not kill through the zone rule.
foreach ($zone in @('armorHead', 'armorBody', 'armorHands', 'armorLegs')) {
    $m = [regex]::Match($cfg, "$zone\s*=\s*[0-9.]+\s*;")
    if (-not $m.Success) { throw "config.cpp has no $zone; the patch does not fit this addon any more" }
    $cfg = $cfg.Remove($m.Index, $m.Length).Insert($m.Index, "$zone = $ZoneArmor;")
}
[IO.File]::WriteAllText($cfgPath, $cfg)
$addons = Join-Path $ModDir 'addons'
[void][IO.Directory]::CreateDirectory($addons)
$pbo = Join-Path $addons 'uddedushka.pbo'
if (Test-Path -LiteralPath $pbo) { Remove-Item -LiteralPath $pbo -Force }
& $Tools pbo pack $work $pbo | Out-Null
if ($LASTEXITCODE -ne 0) { throw "pbo pack failed" }
& $Tools pbo list $pbo
Write-Host "[kj_crush] done: $pbo (from $Src)"
