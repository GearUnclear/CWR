<#
.SYNOPSIS
    Repairs @LoBo addon configs that use a scope keyword the file never defines.

.DESCRIPTION
    OFP-era configs write `scope = public;` and rely on a private preprocessor
    header at the top of the same file:

        #define private   0
        #define protected 1
        #define public    2

    Ninety-odd @LoBo addons carry that header. Two do not (LoBoWreck.pbo, 10
    classes, and LoBoPalObj.pbo, 1 class), and in those two the bare identifier
    resolves to nothing, so the config reader stores the string "public",
    CfgVehicles reads scope 0, and every class in the file becomes non-createable
    with `Cannot create '<class>': type is abstract`.

    THE REPAIR NOW LIVES IN THE ENGINE. This script is a thin wrapper around

        PoseidonTools mod doctor <LoBoDir> [--fix]

    which detects the defect generically (defect class UNDEFINED_SCOPE_KEYWORD,
    grouped under its MISSING_DEFINE_HEADER cause), rewrites the value in place
    padded back to the original byte length, backs the original up to
    <LoBoDir>/_ud-orig, and is idempotent. It repairs the malformed-float and
    buried-model-origin defect classes in the same pass, so this script and
    tools/lobo/fix-lobo-model-origin.ps1 now do the same work; either one is
    enough. See engine/Poseidon/Asset/Addon/ModDoctor.cpp.

    @LoBo is gitignored as third-party game data, so the repaired pbos live only
    on this machine. Rerun after reinstalling or updating @LoBo.

.PARAMETER LoBoDir
    The @LoBo mod folder. Default: D:\Arma_CWA\@LoBo

.PARAMETER Tools
    PoseidonTools executable.

.PARAMETER WhatIf
    Report what would change and write nothing (the tool's report mode).
#>
param(
    [string]$LoBoDir = 'D:\Arma_CWA\@LoBo',
    [string]$Tools = (Join-Path $PSScriptRoot '..\..\dist\x64-win-rwdi\PoseidonTools.exe'),
    [switch]$WhatIf
)
$ErrorActionPreference = 'Stop'

if (-not (Test-Path -LiteralPath $Tools)) { throw "PoseidonTools not found: $Tools (build it, or pass -Tools)" }

$doctorArgs = @('mod', 'doctor', $LoBoDir)
if (-not $WhatIf) { $doctorArgs += '--fix' }

& $Tools @doctorArgs
# Report mode exits 1 when defects were found; that is information, not failure.
if ($LASTEXITCODE -eq 2) { throw "mod doctor failed with I/O errors (exit 2)" }
exit 0
