<#
.SYNOPSIS
    Runs one head-wobble probe test (written by gen_repro.py) on one build and
    then runs metric.py on its frames. One game at a time; tri queues the rest.

.EXAMPLE
    powershell -NoProfile -File run_repro.ps1 -Side ud  -Agent 8 -Name base
.EXAMPLE
    powershell -NoProfile -File run_repro.ps1 -Side van -Agent 8 -Name base

    Output: D:\Arma_CWA_worktrees\graphics-remaster\tmp\video\heads\<Agent>\<side>-<Name>\
      (tri tree with the PNG frames, run.log, metric.json, metric.txt, crops_*.png)
    Junctions: packages\RemasterUD-head<Agent>-ud (video-work) / -van (vanilla-upstream);
    cleanup only ever stops games started on that junction.
#>
[CmdletBinding()]
param(
    [Parameter(Mandatory = $true)][ValidateSet('ud', 'van')][string]$Side,
    [Parameter(Mandatory = $true)][string]$Name,
    [string]$Agent = '8',
    [string]$Python = 'python',
    [switch]$NoMetric
)
$ErrorActionPreference = 'Continue'
$here = Split-Path -Parent $MyInvocation.MyCommand.Path
$heads = 'D:\Arma_CWA_worktrees\graphics-remaster\tmp\video\heads'
$out = Join-Path $heads "$Agent\$Side-$Name"
if (Test-Path -LiteralPath $out) { Remove-Item -LiteralPath $out -Recurse -Force }
New-Item -ItemType Directory -Force -Path $out | Out-Null
if ($Side -eq 'ud') {
    $engineRoot = 'D:\Arma_CWA_worktrees\video-work\arma_CWR'
    $gameDir = 'D:\Arma_CWA_worktrees\graphics-remaster\arma_CWR\dist\graphics-remaster-rwdi'
    $test = "tests/rendering/captures/video/look_probe_head${Agent}_$Name.test.sqf"
    $junction = "RemasterUD-head$Agent-ud"
    Remove-Item Env:\POSEIDON_USER_DIR -ErrorAction SilentlyContinue
} else {
    $engineRoot = 'D:\Arma_CWA_worktrees\vanilla-upstream'
    $gameDir = 'D:\Arma_CWA_worktrees\vanilla-upstream\dist\vanilla-upstream-rwdi'
    $test = "tests/rendering/captures/video/van_probe_head${Agent}_$Name.test.sqf"
    $junction = "RemasterUD-head$Agent-van"
    # frameRate=3 profile, as run_v2_lane.ps1 seeds the vanilla side
    $userDir = Join-Path $out 'user'
    Copy-Item -LiteralPath 'D:\Arma_CWA_worktrees\vanilla-upstream\tmp\video-userseed' -Destination $userDir -Recurse
    $env:POSEIDON_USER_DIR = $userDir
}
$runner = Join-Path $engineRoot 'tests\rendering\captures\run-captures.ps1'
$t0 = Get-Date
& powershell -NoProfile -ExecutionPolicy Bypass -File $runner -Tests $test -GameDir $gameDir -DataJunction $junction -OutDir $out *> (Join-Path $out 'run.log')
$code = $LASTEXITCODE
Remove-Item Env:\POSEIDON_USER_DIR -ErrorAction SilentlyContinue
$n = @(Get-ChildItem -LiteralPath $out -Recurse -Filter '*__f*.png' -ErrorAction SilentlyContinue).Count
"run exit=$code frames=$n minutes=$([math]::Round(((Get-Date) - $t0).TotalMinutes, 1)) out=$out" | Tee-Object -FilePath (Join-Path $out 'status.txt')
if (-not $NoMetric -and $n -gt 2) {
    & $Python (Join-Path $here 'metric.py') $out --out $out 2>&1 | Tee-Object -FilePath (Join-Path $out 'metric.txt')
}
