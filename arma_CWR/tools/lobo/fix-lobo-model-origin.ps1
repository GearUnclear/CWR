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
    (World/Simulation/Simul.cpp:1277-1301) then takes its Static branch:

        pos += newTransform.Orientation() * GetShape()->BoundingCenter();

    i.e. the object is placed at terrainSurfaceY + boundingCenter.Y, which puts
    the model's AUTHORED ORIGIN on the ground. The LandContact / Geometry /
    Level(0) chain that ObjGetPos and the non-static branch use is never reached
    for these props - and no LoBoWreck model has a LandContact LOD anyway
    (landContact = -1 in all ten), including the eight that seat correctly. So
    the seating defect is not a missing LOD. It is the model's vertical origin.

    Measured in game on Sinai at [11900, 9650] (terrain 11.970 m ASL), each prop
    landed at exactly surface + its own boundingCenter.Y, to 1 mm:

        LoBo_t54wrck      bc.Y  +0.522   asl 12.492
        LoBo_t55wrck      bc.Y  +1.053   asl 13.023
        LoBo_BTR60wreck1  bc.Y  +0.484   asl 12.454
        LoBo_uralwreck01  bc.Y  +1.128   asl 13.098
        LoBo_Shot_Wreck1  bc.Y  +1.379   asl 13.349
        LoBo_M60A1_wreck  bc.Y  -1.921   asl 10.050   <- buried
        LoBo_M60A1_wreck2 bc.Y  -2.057   asl  9.913   <- buried

    WHAT THIS SCRIPT CHANGES. In an ODOL p3d the vertex data is already stored
    auto-centred and `boundingCenter` records where the authored origin sits
    relative to that centre (Graphics/Rendering/Shape/Shape.hpp:666 - "-_boundingCenter
    is original (0,0,0) positioned in the new coordinate system"). Rewriting
    boundingCenter.Y is therefore exactly equivalent to having moved the model
    vertically in O2 before binarising: nothing else in the file describes the
    origin, the mesh, bounding sphere, geometry and collision data all live in
    auto-centred space and are untouched, and the ODOL load path takes minMax and
    boundingCenter verbatim from the file (World/Model/ShapeAdapter.cpp:542-557,
    no CalculateBoundingSphere re-centring for ODOL).

    The rule is self-discovering rather than a hardcoded model list: a model is
    broken when its whole mesh is at or below its own origin,

        boundingCenter.Y + minMax[1].Y <= 0

    which can never seat, and the repair sets

        boundingCenter.Y := -minMax[0].Y

    so the lowest vertex rests on the terrain. On this @LoBo install exactly two
    models out of every p3d in the mod match, and they are the two M60A1 wrecks.

    HOW THE PATCH IS APPLIED. p3ds are stored uncompressed inside these pbos and
    a float is rewritten with a float, so four bytes change in place: the pbo file
    table does not move and no repack is involved. Same technique as
    tools/lobo/fix-lobo-scope.ps1.

    FINDING THE FIELD. The model trailer follows the per-LOD resolution array at
    the end of the file (Asset/Formats/P3D/P3DStructures.hpp readModel), with
    minMax at +48/+60 and boundingCenter at +72. The trailer is not aligned, so
    the script anchors on the GEOMETRY_SPEC (1e13) resolution float, then walks
    forward to each possible end-of-resolutions and accepts an offset only when
    every one of these holds:

      * the resolution run is non-decreasing, non-negative, starts under 1e6, and the
        LOD indices in the last 12 bytes point at resolutions with the right
        special values (geometry 1e13, memory 1e15, landContact 2e15, ...)
      * minMax is finite, non-degenerate and symmetric about the origin
        (auto-centred models are symmetric by construction)
      * the bounding sphere radius lies between the largest half-extent and the
        bounding-box corner distance
      * the five bool fields read 0/1, mapType is in range
      * the mass array plus the trailing 4 floats and 12 LOD indices land exactly
        on the end of the file

    That resolves to exactly one offset per model; verified against
    PoseidonFormats' own reader for all ten LoBoWreck models.

    The script is idempotent (a repaired model no longer satisfies the buried
    test). @LoBo is gitignored as third-party game data, so the repaired pbos
    live only on this machine. THIS SCRIPT is the tracked artifact - rerun it
    after reinstalling or updating @LoBo, alongside fix-lobo-scope.ps1.

.PARAMETER LoBoDir
    The @LoBo mod folder. Default: D:\Arma_CWA\@LoBo

.PARAMETER Pbo
    Wildcard limiting which addon pbos are scanned. Default '*' (all of them).
    Narrow it (e.g. 'LoBoWreck.pbo') to skip reading 2 GB of unrelated archives.

.PARAMETER WhatIf
    Report what would change and write nothing.
#>
param(
    [string]$LoBoDir = 'D:\Arma_CWA\@LoBo',
    [string]$Pbo = '*',
    [switch]$WhatIf
)
$ErrorActionPreference = 'Stop'

$addons = Join-Path $LoBoDir 'addons'
if (-not (Test-Path -LiteralPath $addons)) { throw "Not found: $addons" }

$latin1 = [System.Text.Encoding]::GetEncoding(28591)
$backupDir = Join-Path $LoBoDir '_ud-orig'

# GEOMETRY_SPEC from Graphics/Rendering/Draw/SpecLods.hpp, as stored bytes.
$geomSpecBytes = [BitConverter]::GetBytes([float]1e13)
$geomSpecStr = $latin1.GetString($geomSpecBytes)

# tail LOD-index slot -> the resolution that slot must point at. geometryFire and
# geometryView are deliberately absent: binarisers alias them onto the geometry
# LOD, so they do not carry their own special value.
$specialForSlot = @{ 0 = 1e15; 8 = 2e15; 9 = 3e15; 10 = 4e15; 11 = 5e15 }

function Get-Float([byte[]]$b, [int]$o) { [BitConverter]::ToSingle($b, $o) }

function Test-Vec3Sane([byte[]]$b, [int]$o) {
    for ($i = 0; $i -lt 3; $i++) {
        $v = Get-Float $b ($o + 4 * $i)
        if ([float]::IsNaN($v) -or [float]::IsInfinity($v) -or [Math]::Abs($v) -ge 10000) { return $false }
    }
    return $true
}

# Is $o (absolute offset into the pbo bytes) the model trailer of the p3d that
# occupies [$start, $end)?
function Test-Trailer([byte[]]$b, [int]$o, [int]$end, [int[]]$lodIdx) {
    if ($o + 150 -gt $end) { return $false }
    if (-not (Test-Vec3Sane $b ($o + 48))) { return $false }
    if (-not (Test-Vec3Sane $b ($o + 60))) { return $false }
    if (-not (Test-Vec3Sane $b ($o + 72))) { return $false }
    $maxHalf = 0.0
    $corner2 = 0.0
    for ($i = 0; $i -lt 3; $i++) {
        $mn = Get-Float $b ($o + 48 + 4 * $i)
        $mx = Get-Float $b ($o + 60 + 4 * $i)
        if ($mx -le $mn) { return $false }
        # auto-centred models store a bounding box symmetric about the origin
        if ([Math]::Abs($mn + $mx) -gt 1e-3) { return $false }
        if ($mx -gt $maxHalf) { $maxHalf = $mx }
        $corner2 += [double]$mx * $mx
    }
    $bs = Get-Float $b ($o + 4)
    if ([float]::IsNaN($bs) -or $bs -le 0) { return $false }
    if ($bs -lt $maxHalf - 1e-3 -or $bs -gt [Math]::Sqrt($corner2) + 1e-3) { return $false }
    for ($i = 144; $i -lt 149; $i++) { if ($b[$o + $i] -gt 1) { return $false } }
    if ($b[$o + 149] -gt 32) { return $false }
    if ($o + 154 -gt $end) { return $false }
    $cnt = [BitConverter]::ToUInt32($b, $o + 150)
    if ($cnt -gt 4000000) { return $false }
    # massArray (LZSS-compressed at or above 1 KiB) + mass/invMass/armor/invArmor
    # + the 12 LOD indices must end exactly on the end of the file
    $avail = ($end - 28) - ($o + 154)
    $raw = [int64]$cnt * 4
    if ($raw -lt 1024) {
        if ($avail -ne $raw) { return $false }
    } else {
        if ($avail -le 0 -or $avail -ge $raw) { return $false }
    }
    return $true
}

# Given the offset of a GEOMETRY_SPEC float, is there a resolution array ending
# at $o that is consistent with the LOD indices in the tail?
function Test-Resolutions([byte[]]$b, [int]$resStart, [int]$n, [int]$geomIdx, [int[]]$lodIdx) {
    $prev = -1.0
    for ($i = 0; $i -lt $n; $i++) {
        $v = Get-Float $b ($resStart + 4 * $i)
        if ([float]::IsNaN($v) -or $v -lt 0) { return $false }
        if ($v -lt $prev) { return $false }
        $prev = $v
    }
    if ((Get-Float $b $resStart) -gt 1e6) { return $false }
    foreach ($slot in $specialForSlot.Keys) {
        $k = $lodIdx[$slot]
        if ($k -lt 0) { continue }
        if ($k -ge $n) { return $false }
        $want = $specialForSlot[$slot]
        $got = Get-Float $b ($resStart + 4 * $k)
        if ([Math]::Abs($got - $want) -gt $want * 0.01) { return $false }
    }
    return $true
}

$patchedFiles = 0
$patchedModels = 0
$scanned = 0
$skipped = 0

foreach ($pboFile in (Get-ChildItem -LiteralPath $addons -Filter '*.pbo' | Where-Object { $_.Name -like $Pbo } | Sort-Object Name)) {
    $bytes = [System.IO.File]::ReadAllBytes($pboFile.FullName)
    $text = $latin1.GetString($bytes)

    # --- pbo header table: asciiz name + 5 uint32, terminated by an empty name --
    $entries = @()
    $p = 0
    $ok = $true
    while ($true) {
        $z = $text.IndexOf([char]0, $p)
        if ($z -lt 0 -or $z + 21 -ge $bytes.Length) { $ok = $false; break }
        $name = $text.Substring($p, $z - $p)
        $p = $z + 1
        $size = [BitConverter]::ToUInt32($bytes, $p + 16)
        $p += 20
        if ($name.Length -eq 0) { break }
        $entries += , @($name, [int]$size, 0)
    }
    if (-not $ok) { Write-Warning "$($pboFile.Name): unreadable header table, skipped"; continue }
    $dataAt = $p
    foreach ($e in $entries) { $e[2] = $dataAt; $dataAt += $e[1] }

    $dirty = $false
    $report = @()

    foreach ($e in $entries) {
        $name = $e[0]; $size = $e[1]; $off = $e[2]
        if ($name -notlike '*.p3d') { continue }
        if ($size -lt 200 -or $off + $size -gt $bytes.Length) { continue }
        if ($latin1.GetString($bytes, $off, 4) -ne 'ODOL') { continue }
        $scanned++

        $end = $off + $size
        $lodIdx = New-Object 'System.Int32[]' 12
        for ($i = 0; $i -lt 12; $i++) { $v = [int]$bytes[$end - 12 + $i]; if ($v -gt 127) { $v -= 256 }; $lodIdx[$i] = $v }
        $geomIdx = $lodIdx[1]
        if ($geomIdx -lt 0) { continue }   # no geometry LOD: no 1e13 to anchor on

        # every GEOMETRY_SPEC float in the model, then every end-of-resolutions
        # that could follow it
        $found = @()
        $searchAt = $off
        while ($true) {
            $hit = $text.IndexOf($geomSpecStr, $searchAt, $end - $searchAt)
            if ($hit -lt 0 -or $hit + 4 -gt $end) { break }
            $searchAt = $hit + 1
            for ($n = $geomIdx + 1; $n -le 64; $n++) {
                $resStart = $hit - 4 * $geomIdx
                $trailer = $resStart + 4 * $n
                if ($resStart -lt $off -or $trailer + 150 -gt $end) { continue }
                if (-not (Test-Resolutions $bytes $resStart $n $geomIdx $lodIdx)) { continue }
                if (-not (Test-Trailer $bytes $trailer $end $lodIdx)) { continue }
                if ($found -notcontains $trailer) { $found += $trailer }
            }
        }
        if ($found.Count -eq 0) { $skipped++; Write-Verbose "$($pboFile.Name)/$name : no model trailer located, skipped"; continue }
        if ($found.Count -gt 1) { $skipped++; Write-Warning "$($pboFile.Name)/$name : ambiguous model trailer ($($found -join ', ')), skipped"; continue }

        $t = $found[0]
        $minY = Get-Float $bytes ($t + 52)
        $maxY = Get-Float $bytes ($t + 64)
        $bcY = Get-Float $bytes ($t + 76)
        # the whole mesh at or below the authored origin: this can never seat
        if ($bcY + $maxY -gt 0) { continue }

        $newY = [float](-$minY)
        $report += ("    {0}: boundingCenter.Y {1:N4} -> {2:N4}  (mesh {3:N4}..{4:N4}, was buried {5:N2} m)" -f `
                $name, $bcY, $newY, $minY, $maxY, [Math]::Abs($bcY + $maxY))
        if (-not $WhatIf) {
            [Array]::Copy([BitConverter]::GetBytes($newY), 0, $bytes, $t + 76, 4)
        }
        $dirty = $true
        $patchedModels++
    }

    if (-not $dirty) { continue }
    Write-Output ("{0}: {1} model(s)" -f $pboFile.Name, $report.Count)
    $report | ForEach-Object { Write-Output $_ }
    $patchedFiles++
    if ($WhatIf) { continue }

    New-Item -ItemType Directory -Force $backupDir | Out-Null
    $backup = Join-Path $backupDir $pboFile.Name
    if (-not (Test-Path -LiteralPath $backup)) {
        Copy-Item -LiteralPath $pboFile.FullName -Destination $backup
        Write-Output ("    backed up original -> {0}" -f $backup)
    }
    # @LoBo ships some pbos read-only (they came off a 2006 CD image).
    if ($pboFile.IsReadOnly) { Set-ItemProperty -LiteralPath $pboFile.FullName -Name IsReadOnly -Value $false }
    [System.IO.File]::WriteAllBytes($pboFile.FullName, $bytes)
}

if ($patchedModels -eq 0) {
    Write-Output ("Nothing to do: all {0} readable ODOL model(s) sit at or above their own origin ({1} not readable, see -Verbose)." -f ($scanned - $skipped), $skipped)
} elseif ($WhatIf) {
    Write-Output ("Would repair {0} model(s) in {1} pbo(s) (of {2} read, {3} skipped). Rerun without -WhatIf to apply." -f $patchedModels, $patchedFiles, ($scanned - $skipped), $skipped)
} else {
    Write-Output ("Repaired {0} model(s) in {1} pbo(s) (of {2} read, {3} skipped). Originals kept in {4}." -f $patchedModels, $patchedFiles, ($scanned - $skipped), $skipped, $backupDir)
}
exit 0
