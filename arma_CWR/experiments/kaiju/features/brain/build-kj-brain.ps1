# Builds the brain's own mount of the creature addon, tmp\@kj_brain, so brain
# runs never depend on a mount another agent is rebuilding. Nothing of the
# pipeline's or the anim feature's is written. Windows PowerShell 5.1 or later.
#
#   powershell -NoProfile -File build-kj-brain.ps1 [-NoStride] [-From <stage folder>]
#
# Source, first that exists:
#   1. the anim feature's staged addon (kaiju-notes\features\anim\build\stage\uddedushka):
#      the proxy with the full move set (DedIdle, DedWalk, DedTurnL/R, DedRoar, ...);
#   2. the pipeline's built pbo (kaiju-notes\pipeline\build): DedStand and DedWalk only.
#
# One change is made to a COPY of the anim config, unless -NoStride:
#   class DedStride : DedWalk { relSpeedMin = 1; }   (and DedStride joins the
#   dedUp interpolation group)
# Why: a move started by playMove runs with the AI's wanted speed at zero, so
# the engine plays it at the move's relSpeedMin (SoldierOldSim.cpp:44-52,
# SoldierOldAI.cpp:1640-1648). DedWalk has relSpeedMin 0.6 so the AI can slow
# him at the end of a path; a scripted DedWalk therefore crawls at 0.6 rate.
# DedStride is the same clip at full rate under script. The brain uses it when
# the addon has it and falls back to DedWalk. This is the proposal to the anim
# feature (see kaiju-notes\features\brain\NOTES.md); this build is the proof.
#
# Output: kaiju-notes\features\brain\build\ (stage, pbo, logs) and the mount
# copy <git root>\tmp\@kj_brain\addons\uddedushka.pbo. Mount it IN PLACE OF
# @uddedushka / @kj_anim (all three carry the addon uddedushka).
param(
    [switch]$NoStride,
    [string]$From = '',
    [string]$OutDir = 'D:\Arma_CWA_worktrees\kaiju-notes\features\brain\build',
    [string]$Tools = 'D:\Arma_CWA\arma_CWR\dist\x64-win-rwdi\PoseidonTools.exe'
)
$ErrorActionPreference = 'Stop'
$here = $PSScriptRoot
$gitRoot = [IO.Path]::GetFullPath((Join-Path $here '..\..\..\..\..'))
$mount = Join-Path $gitRoot 'tmp\@kj_brain\addons'
$animStage = 'D:\Arma_CWA_worktrees\kaiju-notes\features\anim\build\stage\uddedushka'
$pipePbo = 'D:\Arma_CWA_worktrees\kaiju-notes\pipeline\build\@uddedushka\addons\uddedushka.pbo'
if ($From) { $animStage = $From }
[void][IO.Directory]::CreateDirectory($OutDir)
[void][IO.Directory]::CreateDirectory($mount)
$addons = Join-Path $OutDir '@kj_brain\addons'
[void][IO.Directory]::CreateDirectory($addons)
$pbo = Join-Path $addons 'uddedushka.pbo'

if (Test-Path -LiteralPath (Join-Path $animStage 'config.cpp')) {
    $stage = Join-Path $OutDir 'stage\uddedushka'
    if (Test-Path -LiteralPath $stage) { Remove-Item -LiteralPath $stage -Recurse -Force }
    [void][IO.Directory]::CreateDirectory((Split-Path $stage))
    Copy-Item -LiteralPath $animStage -Destination $stage -Recurse
    $cfgPath = Join-Path $stage 'config.cpp'
    $cfg = [IO.File]::ReadAllText($cfgPath)
    $what = 'anim move set'
    if (-not $NoStride) {
        $anchor = '        class DedTurnL : DedBase'
        $group = 'dedUp[] = {DED_COST, "DedIdle", "DedWalk", "DedTurnL", "DedTurnR"};'
        if ($cfg.IndexOf($anchor) -lt 0 -or $cfg.IndexOf($group) -lt 0) {
            throw "the anim config no longer has the DedTurnL state or the dedUp group as expected; rerun with -NoStride or update this script"
        }
        $stride = "        // Added by features\brain\build-kj-brain.ps1: the walk at full rate under playMove.`r`n" +
            "        class DedStride : DedWalk`r`n        {`r`n            relSpeedMin = 1;`r`n        };`r`n"
        $cfg = $cfg.Replace($anchor, $stride + $anchor)
        $cfg = $cfg.Replace($group, 'dedUp[] = {DED_COST, "DedIdle", "DedWalk", "DedTurnL", "DedTurnR", "DedStride"};')
        [IO.File]::WriteAllText($cfgPath, $cfg, (New-Object Text.UTF8Encoding($false)))
        $what = 'anim move set + DedStride'
    }
    if (Test-Path -LiteralPath $pbo) { Remove-Item -LiteralPath $pbo -Force }
    $log = Join-Path $OutDir 'pack.log'
    $old = $ErrorActionPreference; $ErrorActionPreference = 'Continue'
    & $Tools pbo pack $stage $pbo 2>&1 | ForEach-Object { "$_" } | Out-File -FilePath $log -Encoding utf8
    $ErrorActionPreference = $old
    if ($LASTEXITCODE -ne 0 -or -not (Test-Path -LiteralPath $pbo)) { Get-Content $log -Tail 20; throw "pbo pack failed (log: $log)" }
} elseif (Test-Path -LiteralPath $pipePbo) {
    Copy-Item -LiteralPath $pipePbo -Destination $pbo -Force
    $what = 'pipeline proxy (DedStand, DedWalk only)'
} else {
    throw "no source: neither $animStage nor $pipePbo exists"
}
Copy-Item -LiteralPath $pbo -Destination (Join-Path $mount 'uddedushka.pbo') -Force
Set-Content -LiteralPath (Join-Path $OutDir 'SOURCE.txt') -Value "$(Get-Date -Format s) $what from $animStage" -Encoding utf8
Write-Host "[kj_brain] $what"
Write-Host "[kj_brain] mounted copy: $(Join-Path $mount 'uddedushka.pbo')"
