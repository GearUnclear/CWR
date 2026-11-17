# Builds the kjbeam addon (the drawn parts of Dedushka's breath) from nothing:
# models and textures by gen_kjbeam.py, PNG to PAA, config, pack, mount copy.
# Windows PowerShell 5.1 or later. No Blender, no game.
#
#   powershell -NoProfile -File build-kjbeam.ps1 [-OutDir <folder>] [-NoInstall]
#
# -OutDir    every built file (p3d, png, paa, staging, pbo, logs). Default:
#            kaiju-notes\features\beam\build. Nothing binary goes into the repo.
# -NoInstall do not copy the pbo to the mount folder <git root>\tmp\@kj_beam.
#
# A test mounts it beside the creature: --mods-dir ".." --mod "@uddedushka" --mod "@kj_beam"
# and names both addons in its mission.sqm addOns[].
param(
    [string]$OutDir = 'D:\Arma_CWA_worktrees\kaiju-notes\features\beam\build',
    [switch]$NoInstall,
    [string]$ModDir = '',
    [string]$DataDir = '',
    [string]$Tools = 'D:\Arma_CWA\arma_CWR\dist\x64-win-rwdi\PoseidonTools.exe',
    [string]$Python = 'python'
)
$ErrorActionPreference = 'Stop'
$here = $PSScriptRoot
# <git root>\arma_CWR\experiments\kaiju\features\beam\addon -> <git root>
$gitRoot = [IO.Path]::GetFullPath((Join-Path $here '..\..\..\..\..\..'))
if (-not $ModDir) { $ModDir = Join-Path $gitRoot 'tmp\@kj_beam' }
if (-not $DataDir) { $DataDir = Join-Path $gitRoot 'tmp\remaster-shadow-kaiju' }
$OutDir = [IO.Path]::GetFullPath($OutDir)
[void][IO.Directory]::CreateDirectory($OutDir)

function Invoke-Logged([string]$log, [string]$exe, [string[]]$argv) {
    $old = $ErrorActionPreference
    $ErrorActionPreference = 'Continue'
    try { & $exe @argv 2>&1 | ForEach-Object { "$_" } | Out-File -FilePath $log -Encoding utf8 -Append }
    finally { $ErrorActionPreference = $old }
    if ($LASTEXITCODE -ne 0) {
        Get-Content -LiteralPath $log -Tail 25 | ForEach-Object { Write-Host "    $_" }
        throw "$exe failed with exit $LASTEXITCODE (log: $log)"
    }
}
foreach ($f in @($Tools, (Join-Path $here 'kjbeam\config.cpp'), (Join-Path $here 'gen_kjbeam.py'))) {
    if (-not (Test-Path -LiteralPath $f)) { throw "missing input: $f" }
}
foreach ($l in @('gen.log', 'paa.log', 'pack.log', 'probe.log')) {
    $p = Join-Path $OutDir $l
    if (Test-Path -LiteralPath $p) { Remove-Item -LiteralPath $p -Force }
}

# 1. Models and PNGs.
Invoke-Logged (Join-Path $OutDir 'gen.log') $Python @((Join-Path $here 'gen_kjbeam.py'), $OutDir)
Get-Content -LiteralPath (Join-Path $OutDir 'gen.log') | ForEach-Object { Write-Host "    $_" }
$summary = Get-Content -LiteralPath (Join-Path $OutDir 'kjbeam_gen.json') -Raw | ConvertFrom-Json

# 2. Stage.
$stage = Join-Path $OutDir 'stage\kjbeam'
if (Test-Path -LiteralPath $stage) { Remove-Item -LiteralPath $stage -Recurse -Force }
New-Item -ItemType Directory -Force -Path $stage | Out-Null
Copy-Item -LiteralPath (Join-Path $here 'kjbeam\config.cpp') -Destination (Join-Path $stage 'config.cpp')
foreach ($m in $summary.models.PSObject.Properties.Name) {
    Copy-Item -LiteralPath (Join-Path $OutDir $m) -Destination $stage
}
foreach ($t in $summary.textures) {
    Invoke-Logged (Join-Path $OutDir 'paa.log') $Tools @('image', 'convert', $t.png, (Join-Path $stage $t.paa), '-f', $t.format)
}
Write-Host "[kjbeam] $($summary.textures.Count) textures converted"

# 3. Pack.
$addons = Join-Path $OutDir '@kj_beam\addons'
New-Item -ItemType Directory -Force -Path $addons | Out-Null
$pbo = Join-Path $addons 'kjbeam.pbo'
if (Test-Path -LiteralPath $pbo) { Remove-Item -LiteralPath $pbo -Force }
Invoke-Logged (Join-Path $OutDir 'pack.log') $Tools @('pbo', 'pack', $stage, $pbo)
& $Tools pbo list $pbo
if ($LASTEXITCODE -ne 0) { throw "pbo list failed on $pbo" }

# 4. Offline check: the config merges onto the stock one (no game).
if (Test-Path -LiteralPath (Join-Path $DataDir 'BIN')) {
    $probeLog = Join-Path $OutDir 'probe.log'
    $old = $ErrorActionPreference
    $ErrorActionPreference = 'Continue'
    & $Tools guerrilla probe --data-dir $DataDir --mod (Join-Path $OutDir '@kj_beam') 2>&1 | ForEach-Object { "$_" } | Out-File -FilePath $probeLog -Encoding utf8
    $ErrorActionPreference = $old
    Get-Content -LiteralPath $probeLog | Where-Object { $_ -match 'KJBeam|SUMMARY|error|Error' } | Select-Object -First 20 | ForEach-Object { Write-Host "    $_" }
}

# 5. Mount copy beside the shadow.
if (-not $NoInstall) {
    $mount = Join-Path $ModDir 'addons'
    New-Item -ItemType Directory -Force -Path $mount | Out-Null
    Copy-Item -LiteralPath $pbo -Destination (Join-Path $mount 'kjbeam.pbo') -Force
    Write-Host "[kjbeam] mounted copy: $(Join-Path $mount 'kjbeam.pbo')"
}
Write-Host "[kjbeam] done: $pbo"
