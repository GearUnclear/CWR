<#
.SYNOPSIS
    Regenerates this fixture mod's patched @LoBo pbo shadows.

.DESCRIPTION
    Stock @LoBo's LoBoammo.pbo and LoBo_airammo.pbo ship a malformed float
    literal ("0.0.1") in eleven CfgAmmo tracerColor[] arrays. The config reader
    coerces such a token to its strtod prefix now (see ClassifyToleratedLiteral
    in engine/Poseidon/IO/ParamFile/ParamFile.cpp), so a stock @LoBo no longer
    hard-aborts under --autotest; the shadows stay as belt and braces, and
    because the fixture's bin/config.cpp (its CfgAddons preload roster and
    CfgGuerrillaFactions) is what several lobo-lane tests assert against.

    This script copies the two ammo pbos into addons/ and then runs

        PoseidonTools mod doctor <fixture folder> --fix

    which detects the defect generically (defect class MALFORMED_FLOAT) and
    rewrites each token in place, padded back to the original byte length with
    fractional zeros: "0.0.1" -> "0.000". That is the value the 1.96 reader kept.
    (The earlier hand-rolled version of this script wrote "0.001", a different
    number that only happened to be the same length.) The pbo header table does
    not move, so no repack is involved.

    The patched pbos are THIRD-PARTY content (APL-SA LoBo mod data) and are
    gitignored (see .gitignore here) - they must never be committed to this GPL
    repo. Rerun this script if @LoBo is reinstalled or updated.

    STATE ON THIS MACHINE (checked 2026-09-02): @LoBo's own LoBoammo.pbo and
    LoBo_airammo.pbo were already repaired in place on 2026-07-16 (to "0.001"),
    so a rerun here copies already-clean pbos and the doctor finds nothing to do.
    A reinstall of stock @LoBo makes this script load-bearing again.

.PARAMETER LoBoDir
    The @LoBo mod folder. Default: D:\Arma_CWA\@LoBo

.PARAMETER Tools
    PoseidonTools executable.
#>
param(
    [string]$LoBoDir = 'D:\Arma_CWA\@LoBo',
    [string]$Tools = (Join-Path $PSScriptRoot '..\..\..\..\dist\x64-win-rwdi\PoseidonTools.exe')
)
$ErrorActionPreference = 'Stop'

if (-not (Test-Path -LiteralPath $Tools)) { throw "PoseidonTools not found: $Tools (build it, or pass -Tools)" }

$dest = Join-Path $PSScriptRoot 'addons'
New-Item -ItemType Directory -Force $dest | Out-Null
foreach ($name in 'LoBoammo.pbo', 'LoBo_airammo.pbo') {
    $src = Join-Path (Join-Path $LoBoDir 'addons') $name
    if (-not (Test-Path -LiteralPath $src)) { throw "Not found: $src" }
    $target = Join-Path $dest $name
    if (Test-Path -LiteralPath $target) { Set-ItemProperty -LiteralPath $target -Name IsReadOnly -Value $false }
    Copy-Item -LiteralPath $src -Destination $target -Force
    Set-ItemProperty -LiteralPath $target -Name IsReadOnly -Value $false
}

& $Tools mod doctor $PSScriptRoot --fix
if ($LASTEXITCODE -eq 2) { throw "mod doctor failed with I/O errors (exit 2)" }

# The doctor keeps pre-patch copies under <folder>/_ud-orig; here the source is
# always @LoBo, so the backups are dead weight in a gitignored fixture.
$backup = Join-Path $PSScriptRoot '_ud-orig'
if (Test-Path -LiteralPath $backup) { Remove-Item -LiteralPath $backup -Recurse -Force }

Write-Output "Done. Mount order: --mod `"@LoBo;...\@lobofixup`" (fixup AFTER @LoBo)."
exit 0
