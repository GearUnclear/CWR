<#
    Agent 2 (fogwind): run one look_probe_mist3_<Test> probe on the trailer's overhaul build with one of my
    psFogInject overrides, keep the frames as a compressed mkv (libx264rgb crf 4), run bandmetric, delete the PNGs.
    powershell -NoProfile -File run2.ps1 -Test static -Variant A [-Junction RemasterUD-fog-2]
    Output: tmp\video\fogwind\2\runs\<Test>-<Variant>\
#>
[CmdletBinding()]
param(
    [Parameter(Mandatory = $true)][string]$Test,
    [Parameter(Mandatory = $true)][string]$Variant,
    [string]$Junction = 'RemasterUD-fog-2',
    [string]$Probe = 'mist3',
    [string]$ShaderDir = '',
    [string]$GameDir = 'D:\Arma_CWA_worktrees\look-director\arma_CWR\dist\look-director-rwdi',
    [int]$From = 15
)
$ErrorActionPreference = 'Continue'
$mine = 'D:\Arma_CWA_worktrees\graphics-remaster\tmp\video\fogwind\2'
$tag = if ($Probe -eq 'mist3') { "$Test-$Variant" } else { "$Probe-$Test-$Variant" }
$out = Join-Path $mine "runs\$tag"
if (Test-Path -LiteralPath $out) { Remove-Item -LiteralPath $out -Recurse -Force }
New-Item -ItemType Directory -Force -Path $out | Out-Null
$shader = if ($ShaderDir) { $ShaderDir } else { Join-Path $mine "shaders\$Variant" }
$engineRoot = 'D:\Arma_CWA_worktrees\video-work\arma_CWR'
$runner = Join-Path $engineRoot 'tests\rendering\captures\run-captures.ps1'
$testPath = "tests/rendering/captures/video/look_probe_${Probe}_$Test.test.sqf"
Remove-Item Env:\POSEIDON_USER_DIR -ErrorAction SilentlyContinue
$t0 = Get-Date
"start $(Get-Date -Format HH:mm:ss) test=$testPath override=$shader junction=$Junction" | Out-File -FilePath (Join-Path $out 'status.txt') -Encoding utf8
$runArgs = @('-NoProfile', '-ExecutionPolicy', 'Bypass', '-File', $runner, '-Tests', $testPath, '-GameDir', $GameDir,
    '-DataJunction', $Junction, '-OutDir', $out, '-ShaderOverrideDir', $shader)
& powershell @runArgs *> (Join-Path $out 'run.log'); $code = $LASTEXITCODE
$pngs = @(Get-ChildItem -LiteralPath $out -Recurse -Filter "*look_probe_${Probe}_${Test}__ud__f*.png" -ErrorAction SilentlyContinue | Sort-Object Name)
$n = $pngs.Count
"run exit=$code frames=$n minutes=$([math]::Round(((Get-Date) - $t0).TotalMinutes, 1))" | Out-File -FilePath (Join-Path $out 'status.txt') -Append -Encoding utf8
if ($n -gt 40) {
    $frames = Join-Path $out 'frames'
    New-Item -ItemType Directory -Force -Path $frames | Out-Null
    foreach ($p in $pngs) { Move-Item -LiteralPath $p.FullName -Destination (Join-Path $frames ($p.Name -replace '^\d+_', '')) }
    $startNo = [int](($pngs[0].BaseName -replace '^.*__f', ''))
    & 'C:\ffmpeg\bin\ffmpeg.exe' -hide_banner -loglevel error -y -framerate 60 -start_number $startNo -i (Join-Path $frames "look_probe_${Probe}_${Test}__ud__f%04d.png") -c:v libx264rgb -crf 4 -preset fast (Join-Path $out "$tag.mkv")
    & python 'D:\Arma_CWA_worktrees\graphics-remaster\tmp\video\mist\tools\bandmetric.py' $frames --from $From --json (Join-Path $out 'band.json') 2>&1 | Out-File -FilePath (Join-Path $out 'band.txt') -Encoding utf8
    $keep = Join-Path $out 'keep'
    New-Item -ItemType Directory -Force -Path $keep | Out-Null
    foreach ($k in @('f0050', 'f0051')) {
        $f = Join-Path $frames "look_probe_${Probe}_${Test}__ud__$k.png"
        if (Test-Path -LiteralPath $f) { Copy-Item -LiteralPath $f -Destination $keep }
    }
    if (Test-Path -LiteralPath (Join-Path $out "$tag.mkv")) {
        Remove-Item -LiteralPath $frames -Recurse -Force
        Get-ChildItem -LiteralPath $out -Recurse -Filter '*.png' | Where-Object { $_.DirectoryName -ne $keep } | Remove-Item -Force
    }
}
Select-String -Path (Get-ChildItem -LiteralPath $out -Recurse -Include '*.log','*.rpt','*.txt' -ErrorAction SilentlyContinue).FullName -Pattern 'override' -SimpleMatch -ErrorAction SilentlyContinue | Select-Object -First 5 | ForEach-Object { $_.Line } | Out-File -FilePath (Join-Path $out 'override.txt') -Encoding utf8
"done $(Get-Date -Format HH:mm:ss)" | Out-File -FilePath (Join-Path $out 'status.txt') -Append -Encoding utf8
