# Builds the uddedushka addon from a FINAL candidate (A or D, iteration 3) and mounts it in its own
# mod folder, <git root>\tmp\@kj_final_<letter>, to be used IN PLACE OF @uddedushka (never with it:
# both carry the addon uddedushka). A changed copy of experiments\kaiju\addon\build-uddedushka.ps1;
# the pipeline's files are only read. Windows PowerShell 5.1 or later.
#
#   powershell -NoProfile -File build-final.ps1 -Letter a|d [-Skin statue|bones] [-OutDir <folder>]
#       [-ModDir <mount>] [-NoExport] [-NoInstall]
#
# -Letter  a: art\candidates\A\iter3\dedushka_a.blend (Blender 4.3).
#          d: art\candidates\D\iter3\dedushka_D.obj (the model was built in Blender 5.2; its OBJ is
#             read by Blender 4.3, where the P3D add-on is proven).
# -Skin    statue: every vertex on the root bone, rigid clips (the pipeline's gen_dedushka_rtm.py).
#          bones: the model's own bone groups, its joint file copied beside the p3d; the clips and
#          the moves class then come from build-final-anim.ps1, which reads this build.
#          For D, bones needs <OutDir>\..\weights_d.json (dump_weights_52.py, run in Blender 5.2).
# -OutDir  every built file. Default kaiju-notes\features\finals\build\<letter>_<skin>.
#
# What differs from the pipeline's build: five sheets go from the artist's PNG straight to DXT1 PAA
# at their own size (four 4096, one 2048; the pipeline resized to 1024), and are cached by file
# time in <OutDir>\..\paa_<letter>; texture names are uddedushka\d<letter>_<sheet>.paa; the exporter
# is export_final_p3d.py (see its header).
param(
    [Parameter(Mandatory)][ValidateSet('a', 'd')][string]$Letter,
    [ValidateSet('statue', 'bones')][string]$Skin = 'statue',
    [string]$OutDir = '',
    [string]$ModDir = '',
    [switch]$NoExport,
    [switch]$NoInstall,
    [string]$DataDir = '',
    [string]$Blender = 'C:\Program Files\Blender Foundation\Blender 4.3\blender.exe',
    [string]$Tools = 'D:\Arma_CWA\arma_CWR\dist\x64-win-rwdi\PoseidonTools.exe',
    [string]$Python = 'python',
    [string]$Notes = 'D:\Arma_CWA_worktrees\kaiju-notes'
)
$ErrorActionPreference = 'Stop'
$here = $PSScriptRoot
# <git root>\arma_CWR\experiments\kaiju\features\finals -> <git root>
$gitRoot = [IO.Path]::GetFullPath((Join-Path $here '..\..\..\..\..'))
$pipe = [IO.Path]::GetFullPath((Join-Path $here '..\..\addon'))
$buildRoot = Join-Path $Notes 'features\finals\build'
if (-not $OutDir) { $OutDir = Join-Path $buildRoot "${Letter}_$Skin" }
if (-not $ModDir) { $ModDir = Join-Path $gitRoot "tmp\@kj_final_$Letter" }
if (-not $DataDir) { $DataDir = Join-Path $gitRoot 'tmp\remaster-shadow-kaiju' }
$OutDir = [IO.Path]::GetFullPath($OutDir)
[void][IO.Directory]::CreateDirectory($OutDir)
$free = (Get-PSDrive D).Free / 1GB
if ($free -lt 15) { throw ("D: has {0:N1} GB free, under the 15 GB floor" -f $free) }
$cand = Join-Path $Notes ("art\candidates\{0}\iter3" -f $Letter.ToUpper())
$rest = Join-Path $cand 'dedushka_rest.json'

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

# 1. Model.
$p3d = Join-Path $OutDir 'dedushka.p3d'
$summaryPath = Join-Path $OutDir 'dedushka_export.json'
if (-not $NoExport) {
    foreach ($old in @($p3d, $summaryPath)) { if (Test-Path -LiteralPath $old) { Remove-Item -LiteralPath $old -Force } }
    $argv = @('--background', '--factory-startup', '--python-exit-code', '3')
    if ($Letter -eq 'a') { $argv += (Join-Path $cand 'dedushka_a.blend') }
    $argv += @('--python', (Join-Path $here 'export_final_p3d.py'), '--', $OutDir, '--tag', $Letter, '--yaw', '180',
        '--skin', $Skin, '--rest', $rest)
    if ($Letter -eq 'd') {
        $argv += @('--obj', (Join-Path $cand 'dedushka_D.obj'))
        if ($Skin -eq 'bones') {
            $weights = Join-Path $buildRoot 'weights_d.json'
            if (-not (Test-Path -LiteralPath $weights)) { throw "D with bones needs $weights (dump_weights_52.py in Blender 5.2)" }
            $argv += @('--weights', $weights)
        }
    }
    Write-Host "[final $Letter] exporting ($Skin)"
    Invoke-Logged (Join-Path $OutDir 'export.log') $Blender $argv
}
foreach ($f in @($p3d, $summaryPath)) { if (-not (Test-Path -LiteralPath $f)) { throw "export left no $f" } }
$summary = Get-Content -LiteralPath $summaryPath -Raw | ConvertFrom-Json
Write-Host ("[final $Letter] {0}, {1:N1} m tall, skin: {2}" -f $summary.mode, $summary.height_m, $summary.skin)
foreach ($l in $summary.lods) {
    if ($l.resolution -lt 900) { Write-Host ("    LOD {0,-6} {1,6} points {2,6} faces" -f $l.name, $l.points, $l.faces) }
}
foreach ($w in $summary.warnings) { Write-Warning $w }

# 2. Textures: the artist's PNG -> PAA (DXT1, mips made by the tool), cached.
$paaDir = Join-Path $buildRoot "paa_$Letter"
[void][IO.Directory]::CreateDirectory($paaDir)
$paas = @()
foreach ($t in $summary.textures) {
    $paa = Join-Path $paaDir $t.paa
    $stale = (-not (Test-Path -LiteralPath $paa)) -or ((Get-Item -LiteralPath $paa).LastWriteTimeUtc -lt (Get-Item -LiteralPath $t.png).LastWriteTimeUtc)
    if ($stale) {
        Invoke-Logged (Join-Path $OutDir 'paa.log') $Tools @('image', 'convert', $t.png, $paa, '-f', 'DXT1')
        Write-Host "    converted $($t.paa)"
    }
    $paas += $paa
}
Write-Host "[final $Letter] $($paas.Count) textures"

# 3. Rigid clips (a statue); a boned build gets its clips from build-final-anim.ps1, but the
# pipeline's four are still packed so this pbo loads on its own.
$animDir = Join-Path $OutDir 'anim'
if (Test-Path -LiteralPath $animDir) { Remove-Item -LiteralPath $animDir -Recurse -Force }
Invoke-Logged (Join-Path $OutDir 'rtm.log') $Python @((Join-Path $pipe 'gen_dedushka_rtm.py'), '--out', $animDir,
    '--height', "$($summary.height_m)", '--depth', "$($summary.depth_forward_m)")

# 4. Stage.
$config = [IO.File]::ReadAllText((Join-Path $pipe 'uddedushka\config.cpp'))
$clips = [regex]::Matches($config, '\\uddedushka\\anim\\([A-Za-z0-9_]+\.rtm)') |
    ForEach-Object { $_.Groups[1].Value } | Sort-Object -Unique
$stage = Join-Path $OutDir 'stage\uddedushka'
if (Test-Path -LiteralPath $stage) { Remove-Item -LiteralPath $stage -Recurse -Force }
New-Item -ItemType Directory -Force -Path (Join-Path $stage 'anim') | Out-Null
Copy-Item -LiteralPath (Join-Path $pipe 'uddedushka\config.cpp') -Destination (Join-Path $stage 'config.cpp')
Copy-Item -LiteralPath $p3d -Destination (Join-Path $stage 'dedushka.p3d')
foreach ($paa in $paas) { Copy-Item -LiteralPath $paa -Destination $stage }
foreach ($clip in $clips) {
    $src = Join-Path $animDir $clip
    if (-not (Test-Path -LiteralPath $src)) { throw "config.cpp names $clip but gen_dedushka_rtm.py did not make it" }
    Copy-Item -LiteralPath $src -Destination (Join-Path $stage "anim\$clip")
}
$inspect = & $Tools model inspect -t $p3d | Out-String
if ($LASTEXITCODE -ne 0) { throw "model inspect failed on $p3d" }
$inspect | Out-File -FilePath (Join-Path $OutDir 'inspect.txt') -Encoding utf8
$named = [regex]::Matches($inspect, '(?im)uddedushka\\([a-z0-9_]+\.paa)') | ForEach-Object { $_.Groups[1].Value } | Sort-Object -Unique
foreach ($n in $named) {
    if (-not (Test-Path -LiteralPath (Join-Path $stage $n))) { throw "the model names uddedushka\$n but no such texture was built" }
}
Write-Host "[final $Letter] the model names: $($named -join ', ')"

# 5. Pack.
$addons = Join-Path $OutDir "@kj_final_$Letter\addons"
New-Item -ItemType Directory -Force -Path $addons | Out-Null
$pbo = Join-Path $addons 'uddedushka.pbo'
if (Test-Path -LiteralPath $pbo) { Remove-Item -LiteralPath $pbo -Force }
Invoke-Logged (Join-Path $OutDir 'pack.log') $Tools @('pbo', 'pack', $stage, $pbo)

# 6. Offline check: the config merges onto the stock one and the class resolves.
if (Test-Path -LiteralPath (Join-Path $DataDir 'BIN')) {
    $probeLog = Join-Path $OutDir 'probe.log'
    Invoke-Logged $probeLog $Tools @('guerrilla', 'probe', '--data-dir', $DataDir, '--mod', (Join-Path $OutDir "@kj_final_$Letter"))
    $probe = Get-Content -LiteralPath $probeLog | Where-Object { $_ -match 'PROBE|SUMMARY' }
    $probe | ForEach-Object { Write-Host "    $_" }
    if (-not ($probe -match 'PROBE UDDedushka ok')) { throw "guerrilla probe did not accept UDDedushka (log: $probeLog)" }
}

# 7. Mount copy beside the shadow.
if (-not $NoInstall) {
    $mount = Join-Path $ModDir 'addons'
    New-Item -ItemType Directory -Force -Path $mount | Out-Null
    Copy-Item -LiteralPath $pbo -Destination (Join-Path $mount 'uddedushka.pbo') -Force
    Write-Host "[final $Letter] mounted copy: $(Join-Path $mount 'uddedushka.pbo')"
}
Write-Host ("[final $Letter] done: $pbo ({0:N1} MB)" -f ((Get-Item -LiteralPath $pbo).Length / 1MB))
