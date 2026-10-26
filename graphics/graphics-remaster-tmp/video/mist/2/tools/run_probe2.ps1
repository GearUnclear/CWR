<#
    Agent 2 (mist swarm): run one look_probe_mist2_<Tag> test on the trailer's overhaul build,
    then measure bandmetric, keep the frames as a compressed mkv and delete the PNGs.
    Based on tmp\video\mist\6\run_probe.ps1 (its PNG filter missed the NNN_ prefix; fixed here).
    powershell -NoProfile -File run_probe2.ps1 -Tag w4 [-From 30]
    Output: tmp\video\mist\2\runs\<Tag>\ (run.log, status.txt, band.txt, band.json, pacing.txt, <Tag>.mkv)
#>
[CmdletBinding()]
param(
    [Parameter(Mandatory = $true)][string]$Tag,
    [int]$From = 30,
    [string]$Junction = 'RemasterUD-mist-2',
    [string]$GameDir = 'D:\Arma_CWA_worktrees\look-director\arma_CWR\dist\look-director-rwdi',
    [string]$ShaderOverrideDir = ''
)
$ErrorActionPreference = 'Continue'
$mine = 'D:\Arma_CWA_worktrees\graphics-remaster\tmp\video\mist\2'
$out = Join-Path $mine "runs\$Tag"
if (Test-Path -LiteralPath $out) { Remove-Item -LiteralPath $out -Recurse -Force }
New-Item -ItemType Directory -Force -Path $out | Out-Null
$engineRoot = 'D:\Arma_CWA_worktrees\video-work\arma_CWR'
$runner = Join-Path $engineRoot 'tests\rendering\captures\run-captures.ps1'
$test = "tests/rendering/captures/video/look_probe_mist2_$Tag.test.sqf"
Remove-Item Env:\POSEIDON_USER_DIR -ErrorAction SilentlyContinue
$t0 = Get-Date
$runArgs = @('-NoProfile', '-ExecutionPolicy', 'Bypass', '-File', $runner, '-Tests', $test, '-GameDir', $GameDir,
    '-DataJunction', $Junction, '-OutDir', $out)
if ($ShaderOverrideDir) { $runArgs += @('-ShaderOverrideDir', $ShaderOverrideDir) }
& powershell @runArgs *> (Join-Path $out 'run.log')
$code = $LASTEXITCODE
$pngs = @(Get-ChildItem -LiteralPath $out -Recurse -Filter '*valley_mist__ud__f*.png' -ErrorAction SilentlyContinue | Sort-Object Name)
$n = $pngs.Count
"run exit=$code frames=$n minutes=$([math]::Round(((Get-Date) - $t0).TotalMinutes, 1)) out=$out" | Out-File -FilePath (Join-Path $out 'status.txt') -Encoding utf8
$log = Join-Path $out 'udcap.log'
if (Test-Path -LiteralPath $log) {
    & python 'D:\Arma_CWA_worktrees\graphics-remaster\tmp\video\mist\2\tools\parse_framelog.py' $log (Join-Path $out 'frames.json') 2>&1 | Out-File -FilePath (Join-Path $out 'pacing.txt') -Encoding utf8
}
if ($n -gt 40) {
    $frames = Join-Path $out 'frames'
    New-Item -ItemType Directory -Force -Path $frames | Out-Null
    foreach ($p in $pngs) {
        $short = $p.Name -replace '^\d+_', ''
        Move-Item -LiteralPath $p.FullName -Destination (Join-Path $frames $short)
    }
    & python 'D:\Arma_CWA_worktrees\graphics-remaster\tmp\video\mist\tools\bandmetric.py' $frames --from $From --json (Join-Path $out 'band.json') 2>&1 | Out-File -FilePath (Join-Path $out 'band.txt') -Encoding utf8
    $keep = Join-Path $out 'keep'
    New-Item -ItemType Directory -Force -Path $keep | Out-Null
    foreach ($k in @('f0271', 'f0326', 'f0327', 'f0328', 'f0329', 'f0330')) {
        $f = Join-Path $frames "valley_mist__ud__$k.png"
        if (Test-Path -LiteralPath $f) { Copy-Item -LiteralPath $f -Destination $keep }
    }
    $first = @(Get-ChildItem -LiteralPath $frames -Filter '*.png' | Sort-Object Name)[0]
    $startNo = [int]($first.BaseName -replace '^.*__f', '')
    & 'C:\ffmpeg\bin\ffmpeg.exe' -hide_banner -loglevel error -y -framerate 60 -start_number $startNo -i (Join-Path $frames 'valley_mist__ud__f%04d.png') -c:v libx264rgb -crf 4 -preset fast (Join-Path $out "$Tag.mkv")
    if ($LASTEXITCODE -eq 0 -and (Test-Path -LiteralPath (Join-Path $out "$Tag.mkv"))) {
        Remove-Item -LiteralPath $frames -Recurse -Force
        Get-ChildItem -LiteralPath $out -Recurse -Include '*.png', '*.bmp' | Where-Object { $_.DirectoryName -ne $keep } | Remove-Item -Force
    }
}
"done $(Get-Date -Format HH:mm:ss)" | Out-File -FilePath (Join-Path $out 'status.txt') -Append -Encoding utf8
