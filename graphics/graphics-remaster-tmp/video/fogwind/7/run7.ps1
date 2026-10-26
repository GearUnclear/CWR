<#
    Agent 7 (fogwind swarm): run one probe test on the trailer's overhaul build with an optional psFogInject
    override, keep the frames as a crf-4 mkv (look) plus a lossless grey crop of the fog band (metrics),
    run bandmetric, delete the PNGs.
    powershell -NoProfile -File run7.ps1 -Tag <tag> -Test look_probe_mist3_static [-Shader <variant>]
    Output: tmp\video\fogwind\7\runs\<Tag>\
#>
[CmdletBinding()]
param(
    [Parameter(Mandatory = $true)][string]$Tag,
    [Parameter(Mandatory = $true)][string]$Test,
    [string]$Shader = '',
    [string]$Junction = 'RemasterUD-fog-7',
    [string]$GameDir = 'D:\Arma_CWA_worktrees\look-director\arma_CWR\dist\look-director-rwdi'
)
$ErrorActionPreference = 'Continue'
$mine = 'D:\Arma_CWA_worktrees\graphics-remaster\tmp\video\fogwind\7'
$out = Join-Path $mine "runs\$Tag"
if (Test-Path -LiteralPath $out) { Remove-Item -LiteralPath $out -Recurse -Force }
New-Item -ItemType Directory -Force -Path $out | Out-Null
$engineRoot = 'D:\Arma_CWA_worktrees\video-work\arma_CWR'
$runner = Join-Path $engineRoot 'tests\rendering\captures\run-captures.ps1'
$testPath = "tests/rendering/captures/video/$Test.test.sqf"
Remove-Item Env:\POSEIDON_USER_DIR -ErrorAction SilentlyContinue
$t0 = Get-Date
$runArgs = @('-NoProfile', '-ExecutionPolicy', 'Bypass', '-File', $runner, '-Tests', $testPath, '-GameDir', $GameDir,
    '-DataJunction', $Junction, '-OutDir', $out)
if ($Shader) { $runArgs += @('-ShaderOverrideDir', (Join-Path $mine "shaders\$Shader")) }
"start $(Get-Date -Format HH:mm:ss) test=$Test shader=$Shader" | Out-File -FilePath (Join-Path $out 'status.txt') -Encoding utf8
& powershell @runArgs *> (Join-Path $out 'run.log')
$code = $LASTEXITCODE
$pngs = @(Get-ChildItem -LiteralPath $out -Recurse -Filter '*__ud__f*.png' -ErrorAction SilentlyContinue | Sort-Object Name)
$n = $pngs.Count
"run exit=$code frames=$n minutes=$([math]::Round(((Get-Date) - $t0).TotalMinutes, 1))" | Out-File -FilePath (Join-Path $out 'status.txt') -Append -Encoding utf8
if ($n -gt 40) {
    $frames = Join-Path $out 'frames'
    New-Item -ItemType Directory -Force -Path $frames | Out-Null
    foreach ($p in $pngs) { Move-Item -LiteralPath $p.FullName -Destination (Join-Path $frames ('f' + ($p.BaseName -replace '^.*__f', '') + '.png')) }
    & python 'D:\Arma_CWA_worktrees\graphics-remaster\tmp\video\mist\tools\bandmetric.py' $frames --json (Join-Path $out 'band.json') 2>&1 | Out-File -FilePath (Join-Path $out 'band.txt') -Encoding utf8
    $keep = Join-Path $out 'keep'
    New-Item -ItemType Directory -Force -Path $keep | Out-Null
    $mid = $pngs.Count
    Get-ChildItem -LiteralPath $frames -Filter '*.png' | Sort-Object Name | Select-Object -Index 0, 60, 61 | ForEach-Object { Copy-Item -LiteralPath $_.FullName -Destination $keep }
    $startNo = [int](((Get-ChildItem -LiteralPath $frames -Filter '*.png' | Sort-Object Name | Select-Object -First 1).BaseName) -replace '^f', '')
    $ff = 'C:\ffmpeg\bin\ffmpeg.exe'
    & $ff -hide_banner -loglevel error -y -framerate 60 -start_number $startNo -i (Join-Path $frames 'f%04d.png') -c:v libx264rgb -crf 4 -preset fast (Join-Path $out "$Tag.mkv")
    & $ff -hide_banner -loglevel error -y -framerate 60 -start_number $startNo -i (Join-Path $frames 'f%04d.png') -vf 'crop=1280:440:640:400,format=gray' -c:v ffv1 (Join-Path $out "$Tag-band-gray.mkv")
    if ($LASTEXITCODE -eq 0 -and (Test-Path -LiteralPath (Join-Path $out "$Tag.mkv"))) {
        Remove-Item -LiteralPath $frames -Recurse -Force
    }
    Get-ChildItem -LiteralPath $out -Recurse -Filter '*.png' | Where-Object { $_.DirectoryName -ne $keep } | Remove-Item -Force
}
$log = Get-ChildItem -LiteralPath $out -Recurse -Filter '*.log' | Select-String -Pattern 'override' -SimpleMatch | Select-Object -First 5
$log | Out-File -FilePath (Join-Path $out 'override.txt') -Encoding utf8
"done $(Get-Date -Format HH:mm:ss)" | Out-File -FilePath (Join-Path $out 'status.txt') -Append -Encoding utf8
