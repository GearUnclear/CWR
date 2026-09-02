<#
.SYNOPSIS
    Repairs @LoBo ODOL models whose whole mesh sits below their own origin, so
    they spawn underground.

.DESCRIPTION
    `LoBo_M60A1_wreck` and `LoBo_M60A1_wreck2` build, texture and draw perfectly,
    but `createVehicle [x, y, 0]` buries them: the M60A1 wreck's roof ends up
    0.45 m under the sand and its belly 3.39 m under, and the second variant is
    0.92 m / 3.19 m. Every other wreck in LoBoWreck.pbo seats correctly.

    WHAT ACTUALLY SEATS A STATIC PROP. All ten LoBoWreck classes derive from
    Camp -> Strategic -> Building -> Static, simulation "house", so the engine
    builds them as `Building` with `_static = true`. `Entity::PlaceOnSurface`
    (World/Simulation/Simul.cpp, Static branch) then takes

        pos += newTransform.Orientation() * GetShape()->BoundingCenter();

    i.e. the object is placed at terrainSurfaceY + boundingCenter.Y, which puts
    the model's AUTHORED ORIGIN on the ground. This is not a missing LandContact
    LOD: no LoBoWreck model has one, including the eight that seat correctly. It
    is the model's vertical origin.

    THE REPAIR NOW LIVES IN THE ENGINE. This script is a thin wrapper around

        PoseidonTools mod doctor <LoBoDir> [--fix] [--pbo <wildcard>]

    which reads every ODOL p3d with the engine's own reader (so the trailer
    offset is exact, not anchored heuristically), flags a model whose whole mesh
    is strictly below its own origin (boundingCenter.Y + minMax[1].Y < 0), and
    rewrites boundingCenter.Y := -minMax[0].Y as a four-byte in-place float, so
    the pbo file table does not move and no repack is involved. It is idempotent
    and discovers its own targets. The same pass also repairs the two config
    defect classes, so this script and tools/lobo/fix-lobo-scope.ps1 now do the
    same work; either one is enough. See engine/Poseidon/Asset/Addon/ModDoctor.cpp.

    @LoBo is gitignored as third-party game data, so the repaired pbos live only
    on this machine. Rerun after reinstalling or updating @LoBo.

.PARAMETER LoBoDir
    The @LoBo mod folder. Default: D:\Arma_CWA\@LoBo

.PARAMETER Pbo
    Wildcard limiting which addon pbos are scanned. Default '*' (all of them).
    Narrow it (e.g. 'LoBoWreck.pbo') to skip reading 2 GB of unrelated archives.

.PARAMETER Tools
    PoseidonTools executable.

.PARAMETER WhatIf
    Report what would change and write nothing (the tool's report mode).
#>
param(
    [string]$LoBoDir = 'D:\Arma_CWA\@LoBo',
    [string]$Pbo = '*',
    [string]$Tools = (Join-Path $PSScriptRoot '..\..\dist\x64-win-rwdi\PoseidonTools.exe'),
    [switch]$WhatIf
)
$ErrorActionPreference = 'Stop'

if (-not (Test-Path -LiteralPath $Tools)) { throw "PoseidonTools not found: $Tools (build it, or pass -Tools)" }

$doctorArgs = @('mod', 'doctor', $LoBoDir)
if (-not $WhatIf) { $doctorArgs += '--fix' }
if ($Pbo -ne '*') { $doctorArgs += @('--pbo', $Pbo) }

& $Tools @doctorArgs
# Report mode exits 1 when defects were found; that is information, not failure.
if ($LASTEXITCODE -eq 2) { throw "mod doctor failed with I/O errors (exit 2)" }
exit 0
