# Builds the uddedushka addon from a .blend: exports the model, converts its
# textures, makes the animation clips, stages config.cpp and packs
# <OutDir>\@uddedushka\addons\uddedushka.pbo. Windows PowerShell 5.1 or later.
#
#   powershell -NoProfile -File build-uddedushka.ps1 [-Blend <model.blend>] [-OutDir <folder>]
#       [-Objects a,b] [-Exclude a,b] [-Yaw 180] [-NoExport] [-NoInstall] [-DataDir <shadow>]
#
# -Blend     the model. Default: the pipeline proxy in kaiju-notes. A blend
#            with a "dedushka" collection of LOD collections is exported as it
#            is; any other blend (an art candidate) is wrapped into a loadable
#            statue, see export_dedushka_p3d.py.
# -OutDir    where every built file goes (p3d, png, paa, rtm, staging, pbo,
#            logs). Default: kaiju-notes\pipeline\build. Nothing binary is
#            written into the repo: *.p3d and *.paa are gitignored and blocked
#            by the game-data guard, and the art lives outside the repo.
# -Objects   wrap mode only: the mesh objects that are the creature.
# -Exclude   wrap mode only: mesh objects to leave out.
# -Yaw       wrap mode only: degrees to turn the model about the vertical. The
#            game wants it facing Blender -Y; the art candidates face +Y, so
#            the default is 180. Use 0 for a model that already faces -Y.
# -NoExport  reuse <OutDir>\dedushka.p3d and its export summary.
# -NoInstall do not copy the pbo to the mount folder.
# -DataDir   a Remaster shadow to run `guerrilla probe` against (a config and
#            model-path check, no game). Default: this checkout's own shadow,
#            skipped when it does not exist.
#
# The mount folder is <git root>\tmp\@uddedushka (gitignored), beside the
# Remaster shadow: a test whose data dir is <git root>\tmp\<shadow> mounts it
# with --mods-dir ".." --mod "@uddedushka". tri bakes its own repo root in at
# compile time, so a relative mount is the only one that follows the checkout.
# -ModDir picks another mount folder: make-stills.ps1 uses @uddedushka_photo,
# so photographing a candidate never swaps the model under another session
# (and never fails on a pbo that a running game holds open).
param(
    [string]$Blend = 'D:\Arma_CWA_worktrees\kaiju-notes\pipeline\proxy\dedushka_proxy.blend',
    [string]$OutDir = 'D:\Arma_CWA_worktrees\kaiju-notes\pipeline\build',
    [string[]]$Objects = @(),
    [string[]]$Exclude = @(),
    [double]$Yaw = 180,
    [switch]$NoExport,
    [switch]$NoInstall,
    [string]$DataDir = '',
    [string]$ModDir = '',
    [string]$Blender = 'C:\Program Files\Blender Foundation\Blender 4.3\blender.exe',
    [string]$Tools = 'D:\Arma_CWA\arma_CWR\dist\x64-win-rwdi\PoseidonTools.exe',
    [string]$Python = 'python'
)
$ErrorActionPreference = 'Stop'

$here = $PSScriptRoot
# <git root>\arma_CWR\experiments\kaiju\addon -> <git root>
$gitRoot = [IO.Path]::GetFullPath((Join-Path $here '..\..\..\..'))
if (-not $ModDir) { $ModDir = Join-Path $gitRoot 'tmp\@uddedushka' }
if (-not $DataDir) { $DataDir = Join-Path $gitRoot 'tmp\remaster-shadow-kaiju' }
$OutDir = [IO.Path]::GetFullPath($OutDir)
[void][IO.Directory]::CreateDirectory($OutDir)

# Runs a native tool, keeps its whole output in a log, fails on a bad exit code.
function Invoke-Logged([string]$log, [string]$exe, [string[]]$argv) {
    $old = $ErrorActionPreference
    $ErrorActionPreference = 'Continue'
    try { & $exe @argv 2>&1 | ForEach-Object { "$_" } | Out-File -FilePath $log -Encoding utf8 }
    finally { $ErrorActionPreference = $old }
    if ($LASTEXITCODE -ne 0) {
        Get-Content -LiteralPath $log -Tail 25 | ForEach-Object { Write-Host "    $_" }
        throw "$exe failed with exit $LASTEXITCODE (log: $log)"
    }
}

foreach ($f in @($Tools, (Join-Path $here 'uddedushka\config.cpp'))) {
    if (-not (Test-Path -LiteralPath $f)) { throw "missing input: $f" }
}

# 1. Model.
$p3d = Join-Path $OutDir 'dedushka.p3d'
$summaryPath = Join-Path $OutDir 'dedushka_export.json'
if (-not $NoExport) {
    foreach ($f in @($Blender, $Blend)) { if (-not (Test-Path -LiteralPath $f)) { throw "missing input: $f" } }
    foreach ($old in @($p3d, $summaryPath)) { if (Test-Path -LiteralPath $old) { Remove-Item -LiteralPath $old -Force } }
    $texDir = Join-Path $OutDir 'tex'
    if (Test-Path -LiteralPath $texDir) { Remove-Item -LiteralPath $texDir -Recurse -Force }
    Write-Host "[uddedushka] exporting $Blend"
    $argv = @('--background', '--factory-startup', '--python-exit-code', '3', $Blend, '--python',
        (Join-Path $here 'export_dedushka_p3d.py'), '--', $OutDir)
    if ($Objects.Count -gt 0) { $argv += @('--objects', ($Objects -join ',')) }
    if ($Exclude.Count -gt 0) { $argv += @('--exclude', ($Exclude -join ',')) }
    $argv += @('--yaw', "$Yaw")
    Invoke-Logged (Join-Path $OutDir 'export.log') $Blender $argv
}
foreach ($f in @($p3d, $summaryPath)) { if (-not (Test-Path -LiteralPath $f)) { throw "export left no $f" } }
$summary = Get-Content -LiteralPath $summaryPath -Raw | ConvertFrom-Json
Write-Host ("[uddedushka] mode {0}, {1:N1} m tall, skin: {2}" -f $summary.mode, $summary.height_m, $summary.skin)
foreach ($l in $summary.lods) {
    if ($l.resolution -lt 900) { Write-Host ("    LOD {0,-5} {1,6} points {2,6} faces" -f $l.name, $l.points, $l.faces) }
}
foreach ($w in $summary.warnings) { Write-Warning $w }

# 2. Textures: PNG -> PAA (DXT1, mips made by the tool).
$paas = @()
foreach ($t in $summary.textures) {
    $paa = Join-Path (Split-Path -Parent $t.png) $t.paa
    Invoke-Logged (Join-Path $OutDir 'paa.log') $Tools @('image', 'convert', $t.png, $paa, '-f', 'DXT1')
    $paas += $paa
}
Write-Host "[uddedushka] $($paas.Count) textures converted"

# 3. Animation clips for this model's own joints (or rigid ones for a statue).
$animDir = Join-Path $OutDir 'anim'
if (Test-Path -LiteralPath $animDir) { Remove-Item -LiteralPath $animDir -Recurse -Force }
$argv = @((Join-Path $here 'gen_dedushka_rtm.py'), '--out', $animDir, '--height', "$($summary.height_m)",
    '--depth', "$($summary.depth_forward_m)")
if ($summary.rest_json) { $argv += @('--rest', $summary.rest_json) }
Invoke-Logged (Join-Path $OutDir 'rtm.log') $Python $argv
Get-Content -LiteralPath (Join-Path $OutDir 'rtm.log') | ForEach-Object { Write-Host "    $_" }

# 4. Stage. Every clip the config names must exist, or the engine shows the
# rest pose in that move without an error.
$config = [IO.File]::ReadAllText((Join-Path $here 'uddedushka\config.cpp'))
$clips = [regex]::Matches($config, '\\uddedushka\\anim\\([A-Za-z0-9_]+\.rtm)') |
    ForEach-Object { $_.Groups[1].Value } | Sort-Object -Unique
$stage = Join-Path $OutDir 'stage\uddedushka'
if (Test-Path -LiteralPath $stage) { Remove-Item -LiteralPath $stage -Recurse -Force }
New-Item -ItemType Directory -Force -Path (Join-Path $stage 'anim') | Out-Null
Copy-Item -LiteralPath (Join-Path $here 'uddedushka\config.cpp') -Destination (Join-Path $stage 'config.cpp')
Copy-Item -LiteralPath $p3d -Destination (Join-Path $stage 'dedushka.p3d')
foreach ($paa in $paas) { Copy-Item -LiteralPath $paa -Destination $stage }
foreach ($clip in $clips) {
    $src = Join-Path $animDir $clip
    if (-not (Test-Path -LiteralPath $src)) { throw "config.cpp names $clip but gen_dedushka_rtm.py did not make it" }
    Copy-Item -LiteralPath $src -Destination (Join-Path $stage "anim\$clip")
}
# Every texture the model names must be in the pbo, or it loads untextured.
$inspect = & $Tools model inspect -t $p3d | Out-String
if ($LASTEXITCODE -ne 0) { throw "model inspect failed on $p3d" }
$named = [regex]::Matches($inspect, '(?im)uddedushka\\([a-z0-9_]+\.paa)') | ForEach-Object { $_.Groups[1].Value } | Sort-Object -Unique
foreach ($n in $named) {
    if (-not (Test-Path -LiteralPath (Join-Path $stage $n))) { throw "the model names uddedushka\$n but no such texture was built" }
}

# 5. Pack.
$addons = Join-Path $OutDir '@uddedushka\addons'
New-Item -ItemType Directory -Force -Path $addons | Out-Null
$pbo = Join-Path $addons 'uddedushka.pbo'
if (Test-Path -LiteralPath $pbo) { Remove-Item -LiteralPath $pbo -Force }
Invoke-Logged (Join-Path $OutDir 'pack.log') $Tools @('pbo', 'pack', $stage, $pbo)
& $Tools pbo list $pbo
if ($LASTEXITCODE -ne 0) { throw "pbo list failed on $pbo" }

# 6. Offline check: the config merges onto the stock one and the class resolves.
if (Test-Path -LiteralPath (Join-Path $DataDir 'BIN')) {
    $probeLog = Join-Path $OutDir 'probe.log'
    Invoke-Logged $probeLog $Tools @('guerrilla', 'probe', '--data-dir', $DataDir, '--mod', (Join-Path $OutDir '@uddedushka'))
    $probe = Get-Content -LiteralPath $probeLog | Where-Object { $_ -match 'PROBE|SUMMARY' }
    $probe | ForEach-Object { Write-Host "    $_" }
    if (-not ($probe -match 'PROBE UDDedushka ok')) { throw "guerrilla probe did not accept UDDedushka (log: $probeLog)" }
} else {
    Write-Host "[uddedushka] no shadow at $DataDir, config probe skipped"
}

# 7. Mount copy beside the shadow.
if (-not $NoInstall) {
    $mount = Join-Path $ModDir 'addons'
    New-Item -ItemType Directory -Force -Path $mount | Out-Null
    Copy-Item -LiteralPath $pbo -Destination (Join-Path $mount 'uddedushka.pbo') -Force
    Write-Host "[uddedushka] mounted copy: $(Join-Path $mount 'uddedushka.pbo')"
}
Write-Host "[uddedushka] done: $pbo"
