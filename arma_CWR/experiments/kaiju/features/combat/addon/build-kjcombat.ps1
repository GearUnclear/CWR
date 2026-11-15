# Builds the combat variant of the creature addon: an existing uddedushka.pbo
# with the few config lines changed that decide who shoots at him, packed into
# <git root>\tmp\@kj_combat (gitignored), to be mounted IN PLACE OF @uddedushka
# (never beside it: both carry the class). No game is started.
# Windows PowerShell 5.1 or later.
#
#   powershell -NoProfile -File build-kjcombat.ps1 [-Src <uddedushka.pbo>] [-Armor 700] [-Structural 17] [-Zone 150] [-Side 2]
#
# Source, first that exists: the integration mount (tmp\@kj_dedushka: the anim
# feature's moves, DedStride, the engine's `class Kaiju` block), the brain's
# mount (tmp\@kj_brain), the pipeline's (tmp\@uddedushka). The source is only
# read. If the source mount has kjbeam.pbo (the beam feature's fullbright
# sprite, used here for sparks and the death flare) it is copied alongside.
#
# The lines, and why (kaiju-notes\features\combat\NOTES.md, "Who shoots him"):
#   side = 2            Resistance. Observers judge him by his CONFIG side until they have
#                       confirmed it (about 180 m for infantry), so it must be a side the
#                       garrison is hostile to. Put him in a Resistance group.
#   armor = 700         The AI fires a weapon at him only when its ammo "hit" passes two gates
#                       that read this value and his geometry radius (Target.cpp, WhatShootResult:
#                       hit >= 5.3 x sqrt(armor) x radius / 22.6 m, and hit >= armor / 10). At 700
#                       and the proxy's 19.7 m radius the cut is hit >= 122: tank guns (HEAT and
#                       HE), every launcher and missile, rockets. Rifles, machine guns, grenades,
#                       mortars and the 30 mm cannon (hit 100) stay silent. The cannon is kept out
#                       on purpose: measured at armor 120, where it passes, a Mi-24 strafed with
#                       the cannon and never fired its rockets; at 500 it fired all eight missiles
#                       and all 64 rockets. 700 keeps the cannon out for any model with a geometry
#                       radius over 16 m, and the 400 class of ammo in up to 64 m.
#   armorStructural = 17   The AI never reads it. Real toughness is armor x this (about 12 000):
#                       no stock round moves his damage value by more than 0.3 (a laser guided
#                       bomb), so no one blow kills the engine's Man on the stock binary either.
#   armorHead, Body, Hands, Legs = 150   Zone armour is armor x this x points. No zone rule
#                       (head or body over 0.9 is death, legs over 0.9 cannot stand) can fire.
#   kjHeight            added when the source has none (every script size is a fraction of it).
#
# Everything binary stays out of the repo: staging goes to
# kaiju-notes\features\combat\build, the pbo to <git root>\tmp\@kj_combat.
param(
    [string]$Src = '',
    [double]$Armor = 700,
    [double]$Structural = 17,
    [double]$Zone = 150,
    [int]$Side = 2,
    [double]$Height = 40,
    [string]$Stage = 'D:\Arma_CWA_worktrees\kaiju-notes\features\combat\build',
    [string]$ModDir = '',
    [string]$Tools = 'D:\Arma_CWA\arma_CWR\dist\x64-win-rwdi\PoseidonTools.exe'
)
$ErrorActionPreference = 'Stop'
$gitRoot = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..\..\..\..\..\..'))
if (-not $Src) {
    foreach ($m in @('@kj_dedushka', '@kj_brain', '@uddedushka')) {
        $c = Join-Path $gitRoot "tmp\$m\addons\uddedushka.pbo"
        if (Test-Path -LiteralPath $c) { $Src = $c; break }
    }
}
if (-not $ModDir) { $ModDir = Join-Path $gitRoot 'tmp\@kj_combat' }
foreach ($f in @($Src, $Tools)) { if (-not $f -or -not (Test-Path -LiteralPath $f)) { throw "missing input: $f" } }
[void][IO.Directory]::CreateDirectory($Stage)
# a snapshot first, so a rebuild of the source mount by its owner cannot tear the read
$snap = Join-Path $Stage 'source.pbo'
if ([IO.Path]::GetFullPath($Src) -ne [IO.Path]::GetFullPath($snap)) {
    Copy-Item -LiteralPath $Src -Destination $snap -Force
    $b0 = Join-Path (Split-Path -Parent $Src) 'kjbeam.pbo'
    if (Test-Path -LiteralPath $b0) { Copy-Item -LiteralPath $b0 -Destination (Join-Path $Stage 'kjbeam.pbo') -Force }
}
$work = Join-Path $Stage 'uddedushka'
if (Test-Path -LiteralPath $work) { Remove-Item -LiteralPath $work -Recurse -Force }
[void][IO.Directory]::CreateDirectory($work)
& $Tools pbo extract $snap $work | Out-Null
if ($LASTEXITCODE -ne 0) { throw "pbo extract failed on $Src" }
$cfgPath = Join-Path $work 'config.cpp'
if (-not (Test-Path -LiteralPath $cfgPath)) { throw "no config.cpp in $Src" }
$cfg = [IO.File]::ReadAllText($cfgPath)
# every creature class of the addon (UDDedushka, and UDDedushkaHound, UDDedushkaProxy when the source has them)
$starts = @([regex]::Matches($cfg, 'class\s+UDDedushka\w*\s*:\s*\w+\s*\{'))
if ($starts.Count -eq 0) { throw 'config.cpp has no CfgVehicles >> UDDedushka' }

function Set-Value([string]$text, [string]$key, [string]$value) {
    $r = [regex]::Match($text, "(?m)^(\s*)$key\s*=\s*[-0-9.]+\s*;")
    if ($r.Success) {
        return $text.Remove($r.Index, $r.Length).Insert($r.Index, "$($r.Groups[1].Value)$key = $value;")
    }
    return "`r`n        $key = $value;" + $text
}
$fmt = [Globalization.CultureInfo]::InvariantCulture
$out = $cfg.Substring(0, $starts[0].Index + $starts[0].Length)
for ($i = 0; $i -lt $starts.Count; $i++) {
    $from = $starts[$i].Index + $starts[$i].Length
    $to = if ($i + 1 -lt $starts.Count) { $starts[$i + 1].Index + $starts[$i + 1].Length } else { $cfg.Length }
    $body = $cfg.Substring($from, $to - $from)
    $body = Set-Value $body 'side' ($Side.ToString($fmt))
    $body = Set-Value $body 'armor' ($Armor.ToString($fmt))
    $body = Set-Value $body 'armorStructural' ($Structural.ToString($fmt))
    foreach ($z in @('armorHead', 'armorBody', 'armorHands', 'armorLegs')) { $body = Set-Value $body $z ($Zone.ToString($fmt)) }
    if ($body -notmatch 'kjHeight') { $body = "`r`n        kjHeight = $($Height.ToString($fmt));" + $body }
    $out += $body
}
[IO.File]::WriteAllText($cfgPath, $out)
$addons = Join-Path $ModDir 'addons'
[void][IO.Directory]::CreateDirectory($addons)
$pbo = Join-Path $addons 'uddedushka.pbo'
if (Test-Path -LiteralPath $pbo) { Remove-Item -LiteralPath $pbo -Force }
& $Tools pbo pack $work $pbo | Out-Null
if ($LASTEXITCODE -ne 0) { throw 'pbo pack failed' }
$beam = Join-Path $Stage 'kjbeam.pbo'
$hasBeam = Test-Path -LiteralPath $beam
if ($hasBeam) { Copy-Item -LiteralPath $beam -Destination (Join-Path $addons 'kjbeam.pbo') -Force }
[IO.File]::WriteAllText((Join-Path $ModDir 'SOURCE.txt'), "from $Src`r`nside $Side armor $Armor armorStructural $Structural zones $Zone, kjbeam.pbo $hasBeam`r`n$(Get-Date -Format s)`r`n")
$show = [regex]::Matches($out, '(?m)^\s*(side|armor\w*|kjHeight|irTarget|cost)\s*=\s*[^;]+;') | ForEach-Object { $_.Value.Trim() }
Write-Host ("[kj_combat] classes: " + (($starts | ForEach-Object { ($_.Value -split '\s+')[1] }) -join ', '))
Write-Host ("[kj_combat] config: " + ($show -join ' '))
Write-Host "[kj_combat] done: $pbo (from $Src; kjbeam.pbo $hasBeam)"
