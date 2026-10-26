<#
    Agent 3 (mist swarm): run one look_probe_mist3_<Test> test on the trailer's overhaul build,
    then measure bandmetric and the region series, keep the frames as a compressed mkv and delete the PNGs.
    powershell -NoProfile -File run_probe3.ps1 -Test static -Tag static-base -Junction RemasterUD-mist-3 [-ShaderOverrideDir <abs dir>]
    Output: tmp\video\mist\3\runs\<Tag>\ (run.log, status.txt, band.txt, band.json, series.txt, <Tag>.mkv)
#>
[CmdletBinding()]
param(
    [Parameter(Mandatory = $true)][string]$Test,
    [Parameter(Mandatory = $true)][string]$Tag,
    [int]$From = 15,
    [string]$Junction = 'RemasterUD-mist-3',
    [string]$GameDir = 'D:\Arma_CWA_worktrees\look-director\arma_CWR\dist\look-director-rwdi',
    [string]$ShaderOverrideDir = '',
    [switch]$CollectOnly
)
$ErrorActionPreference = 'Continue'
$mine = 'D:\Arma_CWA_worktrees\graphics-remaster\tmp\video\mist\3'
$out = Join-Path $mine "runs\$Tag"
if (-not $CollectOnly -and (Test-Path -LiteralPath $out)) { Remove-Item -LiteralPath $out -Recurse -Force }
New-Item -ItemType Directory -Force -Path $out | Out-Null
$engineRoot = 'D:\Arma_CWA_worktrees\video-work\arma_CWR'
$runner = Join-Path $engineRoot 'tests\rendering\captures\run-captures.ps1'
$testPath = "tests/rendering/captures/video/look_probe_mist3_$Test.test.sqf"
Remove-Item Env:\POSEIDON_USER_DIR -ErrorAction SilentlyContinue
$t0 = Get-Date
"start $(Get-Date -Format HH:mm:ss) test=$testPath override=$ShaderOverrideDir junction=$Junction" | Out-File -FilePath (Join-Path $out 'status.txt') -Encoding utf8
$runArgs = @('-NoProfile', '-ExecutionPolicy', 'Bypass', '-File', $runner, '-Tests', $testPath, '-GameDir', $GameDir,
    '-DataJunction', $Junction, '-OutDir', $out)
if ($ShaderOverrideDir) { $runArgs += @('-ShaderOverrideDir', $ShaderOverrideDir) }
$code = 0
if (-not $CollectOnly) { & powershell @runArgs *> (Join-Path $out 'run.log'); $code = $LASTEXITCODE }
$pngs = @(Get-ChildItem -LiteralPath $out -Recurse -Filter "*look_probe_mist3_${Test}__ud__f*.png" -ErrorAction SilentlyContinue | Sort-Object Name)
$n = $pngs.Count
"run exit=$code frames=$n minutes=$([math]::Round(((Get-Date) - $t0).TotalMinutes, 1)) out=$out" | Out-File -FilePath (Join-Path $out 'status.txt') -Append -Encoding utf8
if ($n -gt 40) {
    $frames = Join-Path $out 'frames'
    New-Item -ItemType Directory -Force -Path $frames | Out-Null
    foreach ($p in $pngs) { Move-Item -LiteralPath $p.FullName -Destination (Join-Path $frames ($p.Name -replace '^\d+_', '')) }
    $startNo = [int](($pngs[0].BaseName -replace '^.*__f', ''))
    & 'C:\ffmpeg\bin\ffmpeg.exe' -hide_banner -loglevel error -y -framerate 60 -start_number $startNo -i (Join-Path $frames "look_probe_mist3_${Test}__ud__f%04d.png") -c:v libx264rgb -crf 4 -preset fast (Join-Path $out "$Tag.mkv")
    & python 'D:\Arma_CWA_worktrees\graphics-remaster\tmp\video\mist\tools\bandmetric.py' $frames --from $From --json (Join-Path $out 'band.json') 2>&1 | Out-File -FilePath (Join-Path $out 'band.txt') -Encoding utf8
    & python (Join-Path $mine 'tools\series.py') (Join-Path $out "$Tag.mkv") --from $From --csv (Join-Path $out 'series.csv') 2>&1 | Out-File -FilePath (Join-Path $out 'series.txt') -Encoding utf8
    $keep = Join-Path $out 'keep'
    New-Item -ItemType Directory -Force -Path $keep | Out-Null
    foreach ($k in @('f0050', 'f0051', 'f0052', 'f0053')) {
        $f = Join-Path $frames "look_probe_mist3_${Test}__ud__$k.png"
        if (Test-Path -LiteralPath $f) { Copy-Item -LiteralPath $f -Destination $keep }
    }
    if ($LASTEXITCODE -eq 0 -and (Test-Path -LiteralPath (Join-Path $out "$Tag.mkv"))) {
        Remove-Item -LiteralPath $frames -Recurse -Force
        Get-ChildItem -LiteralPath $out -Recurse -Filter '*.png' | Where-Object { $_.DirectoryName -ne $keep } | Remove-Item -Force
    }
}
"done $(Get-Date -Format HH:mm:ss)" | Out-File -FilePath (Join-Path $out 'status.txt') -Append -Encoding utf8
