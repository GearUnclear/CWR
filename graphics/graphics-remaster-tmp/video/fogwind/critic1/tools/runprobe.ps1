<#
    critic1: run one probe on a given build (optionally with a shader override), keep the frames as a
    compressed mkv (libx264rgb crf 4), delete the PNGs.
    powershell -NoProfile -File runprobe.ps1 -Probe fog2_pine_moving -Tag new -GameDir <dist> [-ShaderDir d]
#>
[CmdletBinding()]
param(
    [Parameter(Mandatory = $true)][string]$Probe,
    [Parameter(Mandatory = $true)][string]$Tag,
    [Parameter(Mandatory = $true)][string]$GameDir,
    [string]$ShaderDir = '',
    [string]$Junction = 'RemasterUD-fog-C1'
)
$ErrorActionPreference = 'Continue'
$mine = 'D:\Arma_CWA_worktrees\graphics-remaster\tmp\video\fogwind\critic1'
$out = Join-Path $mine "runs\$Probe-$Tag"
if (Test-Path -LiteralPath $out) { Remove-Item -LiteralPath $out -Recurse -Force }
New-Item -ItemType Directory -Force -Path $out | Out-Null
$engineRoot = 'D:\Arma_CWA_worktrees\video-work\arma_CWR'
$runner = Join-Path $engineRoot 'tests\rendering\captures\run-captures.ps1'
$testPath = "tests/rendering/captures/video/look_probe_$Probe.test.sqf"
Remove-Item Env:\POSEIDON_USER_DIR -ErrorAction SilentlyContinue
$t0 = Get-Date
"start $(Get-Date -Format HH:mm:ss) test=$testPath game=$GameDir override=$ShaderDir junction=$Junction" | Out-File -FilePath (Join-Path $out 'status.txt') -Encoding utf8
$runArgs = @('-NoProfile', '-ExecutionPolicy', 'Bypass', '-File', $runner, '-Tests', $testPath, '-GameDir', $GameDir,
    '-DataJunction', $Junction, '-OutDir', $out)
if ($ShaderDir) { $runArgs += @('-ShaderOverrideDir', $ShaderDir) }
& powershell @runArgs *> (Join-Path $out 'run.log'); $code = $LASTEXITCODE
$pngs = @(Get-ChildItem -LiteralPath $out -Recurse -Filter "*look_probe_${Probe}__ud__f*.png" -ErrorAction SilentlyContinue | Sort-Object Name)
$n = $pngs.Count
"run exit=$code frames=$n minutes=$([math]::Round(((Get-Date) - $t0).TotalMinutes, 1))" | Out-File -FilePath (Join-Path $out 'status.txt') -Append -Encoding utf8
if ($n -gt 40) {
    $frames = Join-Path $out 'frames'
    New-Item -ItemType Directory -Force -Path $frames | Out-Null
    foreach ($p in $pngs) { Move-Item -LiteralPath $p.FullName -Destination (Join-Path $frames ($p.Name -replace '^\d+_', '')) }
    $startNo = [int](($pngs[0].BaseName -replace '^.*__f', ''))
    & 'C:\ffmpeg\bin\ffmpeg.exe' -hide_banner -loglevel error -y -framerate 60 -start_number $startNo -i (Join-Path $frames "look_probe_${Probe}__ud__f%04d.png") -c:v libx264rgb -crf 4 -preset fast (Join-Path $out "$Probe-$Tag.mkv")
    if (Test-Path -LiteralPath (Join-Path $out "$Probe-$Tag.mkv")) {
        Remove-Item -LiteralPath $frames -Recurse -Force
        Get-ChildItem -LiteralPath $out -Recurse -Filter '*.png' | Remove-Item -Force
    }
}
$logs = @(Get-ChildItem -LiteralPath $out -Recurse -Filter 'game_stdout.log' -ErrorAction SilentlyContinue)
foreach ($l in $logs) {
    Select-String -Path $l.FullName -Pattern 'shader override', 'shader compile error', 'override loaded' -ErrorAction SilentlyContinue | Select-Object -First 8 | ForEach-Object { $_.Line } | Out-File -FilePath (Join-Path $out 'override.txt') -Append -Encoding utf8
}
"done $(Get-Date -Format HH:mm:ss)" | Out-File -FilePath (Join-Path $out 'status.txt') -Append -Encoding utf8
