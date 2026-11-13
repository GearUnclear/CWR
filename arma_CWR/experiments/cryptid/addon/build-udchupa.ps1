# Builds the udchupa addon: stages config.cpp, the chupacabra MLOD and its
# texture, and packs @udchupa\addons\udchupa.pbo into the checkout's gitignored
# tmp\ at the git root, beside the Remaster shadows: a test whose data dir is
# <git root>\tmp\<shadow> mounts it with --mods-dir ".." --mod "@udchupa".
# Windows PowerShell 5.1 or later.
#
# The model and texture binaries are NOT in the repo (*.p3d and *.paa are
# gitignored and blocked by the game-data guard). They come from the art
# folder outside the repo; -Export re-exports the p3d from the .blend first.
#
#   powershell -NoProfile -File build-udchupa.ps1 [-Export] [-Texture256]
#       [-CustomMoves [-AnimDir <dir>] [-AllowMissingRtm]]
#
# -CustomMoves turns on CfgMovesChupa and class UDChupacabraAnim: the staged
# config.cpp gets #define UDCHUPA_CUSTOM_MOVES with moves_chupa.hpp inlined,
# and every RTM the move table names is packed under udchupa\anim\. A
# missing RTM stops the build (the engine would show the rest pose instead);
# -AllowMissingRtm packs anyway, for config checks only.
#
# Mount the result with: --mods-dir <git root>\tmp --mod @udchupa
param(
    [string]$ArtDir = 'D:\Arma_CWA_worktrees\cryptid-notes\art',
    # The re-exported p3d (texture retargeted to udchupa\chupa_co.paa).
    [string]$P3d = 'D:\Arma_CWA_worktrees\cryptid-notes\addon\build\chupacabra.p3d',
    [string]$Texture = '',
    [switch]$Texture256,
    [switch]$Export,
    [string]$Blender = 'C:\Program Files\Blender Foundation\Blender 4.3\blender.exe',
    [string]$Tools = 'D:\Arma_CWA\arma_CWR\dist\x64-win-rwdi\PoseidonTools.exe',
    [string]$OutModDir = '',
    [switch]$CustomMoves,
    [string]$AnimDir = 'D:\Arma_CWA_worktrees\cryptid-notes\anim\rtm',
    [switch]$AllowMissingRtm
)
$ErrorActionPreference = 'Stop'

$here = $PSScriptRoot
# <git root>\arma_CWR\experiments\cryptid\addon -> <git root>
$gitRoot = [IO.Path]::GetFullPath((Join-Path $here '..\..\..\..'))
if (-not $OutModDir) { $OutModDir = Join-Path $gitRoot 'tmp\@udchupa' }
if (-not $Texture) {
    $name = if ($Texture256) { 'chupa_co_256.paa' } else { 'chupa_co.paa' }
    $Texture = Join-Path $ArtDir "texture\$name"
}
$blend = Join-Path $ArtDir 'model\chupacabra.blend'

function Invoke-Checked([string]$exe, [string[]]$argv) {
    & $exe @argv
    if ($LASTEXITCODE -ne 0) { throw "$exe failed with exit $LASTEXITCODE" }
}

if ($Export) {
    $json = [IO.Path]::ChangeExtension($P3d, $null).TrimEnd('.') + '_p3d_export.json'
    Write-Host "[udchupa] exporting $blend -> $P3d"
    Invoke-Checked $Blender @('--background', '--factory-startup', $blend, '--python',
        (Join-Path $here 'export_udchupa_p3d.py'), '--', $P3d, $json)
}

foreach ($f in @($Tools, (Join-Path $here 'udchupa\config.cpp'), $P3d, $Texture)) {
    if (-not (Test-Path -LiteralPath $f)) { throw "missing input: $f" }
}

# The face texture must name the pbo prefix, or the model loads untextured.
$inspect = & $Tools model inspect -a $P3d | Out-String
if ($LASTEXITCODE -ne 0) { throw "model inspect failed on $P3d" }
if ($inspect -notmatch [regex]::Escape('udchupa\chupa_co.paa')) {
    throw "$P3d does not reference udchupa\chupa_co.paa; re-export with -Export"
}

$stage = Join-Path $gitRoot 'tmp\udchupa-stage\udchupa'
if (Test-Path -LiteralPath $stage) { Remove-Item -LiteralPath $stage -Recurse -Force }
New-Item -ItemType Directory -Force -Path $stage | Out-Null
$configSrc = Join-Path $here 'udchupa\config.cpp'
if (-not $CustomMoves) {
    Copy-Item -LiteralPath $configSrc -Destination (Join-Path $stage 'config.cpp')
} else {
    $movesSrc = Join-Path $here 'udchupa\moves_chupa.hpp'
    if (-not (Test-Path -LiteralPath $movesSrc)) { throw "missing input: $movesSrc" }
    $moves = [IO.File]::ReadAllText($movesSrc)
    $config = [IO.File]::ReadAllText($configSrc)
    $include = '#include "moves_chupa.hpp"'
    if (-not $config.Contains($include)) { throw "$configSrc has no line: $include" }
    $config = "#define UDCHUPA_CUSTOM_MOVES`n" + $config.Replace($include, $moves)
    [IO.File]::WriteAllText((Join-Path $stage 'config.cpp'), $config)

    # Every clip the move table names, packed as udchupa\anim\<name>.rtm.
    $clips = [regex]::Matches($moves, '\\udchupa\\anim\\([A-Za-z0-9_]+\.rtm)') |
        ForEach-Object { $_.Groups[1].Value } | Sort-Object -Unique
    $animStage = Join-Path $stage 'anim'
    New-Item -ItemType Directory -Force -Path $animStage | Out-Null
    $missing = @()
    foreach ($clip in $clips) {
        $src = Join-Path $AnimDir $clip
        if (Test-Path -LiteralPath $src) {
            Copy-Item -LiteralPath $src -Destination (Join-Path $animStage $clip)
        } else {
            $missing += $clip
        }
    }
    Write-Host "[udchupa] custom moves: $($clips.Count) clips named, $($clips.Count - $missing.Count) found in $AnimDir"
    if ($missing.Count -gt 0) {
        $list = $missing -join ', '
        if (-not $AllowMissingRtm) { throw "missing RTMs in ${AnimDir}: $list" }
        Write-Warning "packing without: $list (the creature shows its rest pose in those moves)"
    }
}
Copy-Item -LiteralPath $P3d -Destination (Join-Path $stage 'chupacabra.p3d')
Copy-Item -LiteralPath $Texture -Destination (Join-Path $stage 'chupa_co.paa')

$addons = Join-Path $OutModDir 'addons'
New-Item -ItemType Directory -Force -Path $addons | Out-Null
$pbo = Join-Path $addons 'udchupa.pbo'
if (Test-Path -LiteralPath $pbo) { Remove-Item -LiteralPath $pbo -Force }
Write-Host "[udchupa] packing $stage -> $pbo"
Invoke-Checked $Tools @('pbo', 'pack', $stage, $pbo)
Invoke-Checked $Tools @('pbo', 'list', $pbo)
Write-Host "[udchupa] done: $pbo"
