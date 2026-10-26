<#
    Mist swarm lead: run one probe test (default agent 6's look_probe_mist6_base, the real ud_valley_mist
    staging, frames 240..389, head-recipe step) on a given overhaul build, then measure bandmetric,
    keep the frames as a compressed mkv and delete the PNGs. A copy of mist/6/run_probe.ps1 with the
    lead's own folder and junction.
    powershell -NoProfile -File run_probe_lead.ps1 -Tag fixed [-Test look_probe_mist6_base] [-From 30] [-GameDir <dist>]
    Output: tmp\video\mist\lead\runs\<Tag>\ (run.log, status.txt, band.txt, band.json, <Tag>.mkv, keep\)
#>
[CmdletBinding()]
param(
    [Parameter(Mandatory = $true)][string]$Tag,
    [string]$Test = 'look_probe_mist6_base',
    [string]$Prefix = 'valley_mist__ud__f',
    [int]$From = 30,
    [string]$Junction = 'RemasterUD-mist-lead',
    [string]$GameDir = 'D:\Arma_CWA_worktrees\look-director\arma_CWR\dist\look-director-rwdi',
    [string]$ShaderOverrideDir = '',
    [switch]$PostOnly
)
$ErrorActionPreference = 'Continue'
$mine = 'D:\Arma_CWA_worktrees\graphics-remaster\tmp\video\mist\lead'
$out = Join-Path $mine "runs\$Tag"
if (-not $PostOnly) {
    if (Test-Path -LiteralPath $out) { Remove-Item -LiteralPath $out -Recurse -Force }
}
New-Item -ItemType Directory -Force -Path $out | Out-Null
$engineRoot = 'D:\Arma_CWA_worktrees\video-work\arma_CWR'
$runner = Join-Path $engineRoot 'tests\rendering\captures\run-captures.ps1'
$testPath = "tests/rendering/captures/video/$Test.test.sqf"
Remove-Item Env:\POSEIDON_USER_DIR -ErrorAction SilentlyContinue
$t0 = Get-Date
"start $(Get-Date -Format HH:mm:ss) test=$testPath game=$GameDir override=$ShaderOverrideDir junction=$Junction" | Out-File -FilePath (Join-Path $out 'status.txt') -Encoding utf8
$runArgs = @('-NoProfile', '-ExecutionPolicy', 'Bypass', '-File', $runner, '-Tests', $testPath, '-GameDir', $GameDir,
    '-DataJunction', $Junction, '-OutDir', $out)
if ($ShaderOverrideDir) { $runArgs += @('-ShaderOverrideDir', $ShaderOverrideDir) }
$code = 'post-only'
if (-not $PostOnly) {
    & powershell @runArgs *> (Join-Path $out 'run.log')
    $code = $LASTEXITCODE
}
$pngs = @(Get-ChildItem -LiteralPath $out -Recurse -Filter "*$Prefix*.png" -ErrorAction SilentlyContinue | Sort-Object Name)
$n = $pngs.Count
"run exit=$code frames=$n minutes=$([math]::Round(((Get-Date) - $t0).TotalMinutes, 1)) out=$out" | Out-File -FilePath (Join-Path $out 'status.txt') -Append -Encoding utf8
if ($n -gt 40) {
    $frames = Join-Path $out 'frames'
    New-Item -ItemType Directory -Force -Path $frames | Out-Null
    foreach ($p in $pngs) { Move-Item -LiteralPath $p.FullName -Destination (Join-Path $frames ($p.Name -replace '^\d+_', '')) }
    & python 'D:\Arma_CWA_worktrees\graphics-remaster\tmp\video\mist\tools\bandmetric.py' $frames --from $From --json (Join-Path $out 'band.json') 2>&1 | Out-File -FilePath (Join-Path $out 'band.txt') -Encoding utf8
    $keep = Join-Path $out 'keep'
    New-Item -ItemType Directory -Force -Path $keep | Out-Null
    foreach ($k in @('f0271', 'f0326', 'f0327', 'f0328', 'f0329', 'f0330')) {
        $f = Join-Path $frames "$Prefix$($k.Substring(1)).png"
        if (Test-Path -LiteralPath $f) { Copy-Item -LiteralPath $f -Destination $keep }
    }
    $startNo = [int](($pngs[0].BaseName) -replace '^.*__f', '')
    & 'C:\ffmpeg\bin\ffmpeg.exe' -hide_banner -loglevel error -y -framerate 60 -start_number $startNo -i (Join-Path $frames "$Prefix%04d.png") -c:v libx264rgb -crf 4 -preset fast (Join-Path $out "$Tag.mkv")
    if ($LASTEXITCODE -eq 0 -and (Test-Path -LiteralPath (Join-Path $out "$Tag.mkv"))) {
        Remove-Item -LiteralPath $frames -Recurse -Force
        Get-ChildItem -LiteralPath $out -Recurse -Filter '*.png' | Where-Object { $_.DirectoryName -ne $keep } | Remove-Item -Force
    }
}
"done $(Get-Date -Format HH:mm:ss)" | Out-File -FilePath (Join-Path $out 'status.txt') -Append -Encoding utf8
