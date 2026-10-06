#requires -Version 7.0
<#
.SYNOPSIS
Reproduce issue #42's unfinished save state without retaining a binary fixture.
.DESCRIPTION
Run guerrilla_assailant_save_reload.seq/01_save.test.sqf as an individual Trident
test with POSEIDON_USER_DIR pointing at a persistent test profile. This helper
copies its save, changing only the registered assailants' entity targetSide to
CIV. Copy the result over that TEST profile's assailants.fps, then run the exact
02_reload.test.sqf path with the same POSEIDON_USER_DIR. Those assertions verify
side repair, identity, inventory, groups and restored personal retaliation.

Config merge reads the original binary directly, preserving its float precision
and serialized script text. JSON is used only to resolve body IDs and verify the
result. Generated saves, JSON and overlays belong under tmp, never in Git.
#>
[CmdletBinding()]
param(
    [Parameter(Mandatory)][string]$InputSave,
    [Parameter(Mandatory)][string]$OutputSave,
    [Parameter(Mandatory)][string]$ToolsExe
)

$ErrorActionPreference = 'Stop'
$sourcePath = (Resolve-Path -LiteralPath $InputSave).Path
$toolPath = (Resolve-Path -LiteralPath $ToolsExe).Path
$outputPath = [System.IO.Path]::GetFullPath($OutputSave)
if ($sourcePath -eq $outputPath) {
    throw 'InputSave and OutputSave must differ; keep the original test save.'
}
$outputDirectory = Split-Path -Parent $outputPath
New-Item -ItemType Directory -Path $outputDirectory -Force | Out-Null
$sourceJson = "$outputPath.source.json"
$resultJson = "$outputPath.result.json"
$overlayPath = "$outputPath.overlay.cpp"

function Invoke-Config([string[]]$CommandArgs) {
    & $toolPath config @CommandArgs | Out-Null
    if ($LASTEXITCODE -ne 0) {
        throw "PoseidonTools config failed with exit code $LASTEXITCODE."
    }
}

function Find-BodyNodes($Node, [string[]]$NodePath, [int]$BodyId, $Found) {
    if ($Node -isnot [System.Collections.IDictionary]) { return }
    if ($Node.Contains('id') -and $Node.Contains('targetSide') -and
        $Node.Contains('type') -and [int]$Node['id'] -eq $BodyId) {
        $Found.Add([pscustomobject]@{ Node = $Node; Path = $NodePath })
    }
    foreach ($key in $Node.Keys) {
        if ($Node[$key] -is [System.Collections.IDictionary]) {
            Find-BodyNodes $Node[$key] ($NodePath + [string]$key) $BodyId $Found
        }
    }
}

function Format-Overlay($Node, [int]$Depth) {
    $indent = '    ' * $Depth
    foreach ($key in $Node.Keys) {
        if ($key -notmatch '^[A-Za-z_][A-Za-z0-9_]*$') {
            throw "Unexpected serialized class name: $key"
        }
        if ($Node[$key] -is [System.Collections.IDictionary]) {
            "$indent`class $key"
            "$indent{"
            Format-Overlay $Node[$key] ($Depth + 1)
            "$indent};"
        } else {
            "$indent$key=`"CIV`";"
        }
    }
}

Invoke-Config @('tojson', $sourcePath, '-o', $sourceJson)
$saveTree = Get-Content -LiteralPath $sourceJson -Raw | ConvertFrom-Json -AsHashtable
$records = $saveTree['GuerrillaAssailants']['Records']
if (-not $records -or [int]$records['items'] -lt 1) {
    throw 'The input save has no registered assailants.'
}
$overlay = [ordered]@{}
$changes = [System.Collections.Generic.List[object]]::new()
$seenIds = [System.Collections.Generic.HashSet[int]]::new()
for ($i = 0; $i -lt [int]$records['items']; $i++) {
    $record = $records["Item$i"]
    if (-not $record['body'] -or -not $record['body'].Contains('id')) {
        throw "Assailant record $i has no body reference."
    }
    $bodyId = [int]$record['body']['id']
    if (-not $seenIds.Add($bodyId)) { throw "Duplicate assailant body ID $bodyId." }
    $found = [System.Collections.Generic.List[object]]::new()
    Find-BodyNodes $saveTree @() $bodyId $found
    if ($found.Count -ne 1) {
        throw "Expected one serialized entity for body $bodyId; found $($found.Count)."
    }
    $body = $found[0]
    $side = [string]$body.Node['targetSide']
    if ($side -notin @('WEST', 'EAST', 'GUER') -or $side -ne $record['group']['side']) {
        throw "Body $bodyId is not a current assailant on its private group's side."
    }
    $entry = $overlay
    foreach ($part in $body.Path) {
        if (-not $entry.Contains($part)) { $entry[$part] = [ordered]@{} }
        $entry = $entry[$part]
    }
    $entry['targetSide'] = 'CIV'
    $changes.Add([pscustomobject]@{
        bodyId = $bodyId
        originalSide = $side
        path = ($body.Path -join ' >> ')
    })
    # This is the complete expected semantic change for verification below.
    $body.Node['targetSide'] = 'CIV'
}

Format-Overlay $overlay 0 | Set-Content -LiteralPath $overlayPath -Encoding utf8NoBOM
Invoke-Config @('merge', $sourcePath, $overlayPath, '-o', $outputPath)
Invoke-Config @('tojson', $outputPath, '-o', $resultJson)
$resultTree = Get-Content -LiteralPath $resultJson -Raw | ConvertFrom-Json -AsHashtable
if (($saveTree | ConvertTo-Json -Depth 100 -Compress) -cne
    ($resultTree | ConvertTo-Json -Depth 100 -Compress)) {
    throw 'Merged save differs beyond the registered bodies targetSide values.'
}
$changes | ConvertTo-Json -Depth 10 | Set-Content -LiteralPath "$outputPath.changes.json" -Encoding utf8NoBOM
Write-Output "Created legacy-state save: $outputPath ($($changes.Count) registered bodies changed to CIV)."
