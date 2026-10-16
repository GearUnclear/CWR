<#
.SYNOPSIS
    Prepare the full official Remaster data and a LoBo mod
    for the built UD engine.
.DESCRIPTION
    Requires PowerShell 7 on Windows or Linux. The install mode follows the
    GameDir layout (Classic 1.99 is refused):

    Remaster  A folder with any official Remaster marker (root dtaExt, or
              Bohemia's bin/remaster.bin, bin/mainMenu.hpp or
              bin/splashLogo.hpp). Bohemia's files are kept: remaster.cpp,
              resource-extra.cpp, the options*.hpp chain, the STRINGTABLE
              shards and the fonts they already ship. Setup adds only UD's
              own files beside them (bin/ud-mainmenu.hpp, bin/ud-options.hpp,
              font credits, a STRINGTABLE_UD.utf8.csv shard holding just the
              keys Bohemia's shards lack, missions, gmcore, faction library).
              The ONLY change to a Bohemia file is appended #include lines:
              two in bin/resource-extra.cpp (by this script: the UD menu and
              the UD Options additions) and two in bin/config-extra.cpp (by
              install-missions.ps1). Each is appended once; reruns add
              nothing. Setup snapshots every
              pre-existing package file first and fails if anything else
              changed. The Remaster Demo package is refused.

    Installs missions and factions and runs the built mod doctor.
    Files setup replaces or appends to are backed up beneath each data
    folder's .ud-backups directory first. Original PBO backups are managed by
    mod doctor under LoBo/_ud-orig. No base game PBO is copied. -CheckOnly
    validates every input without writing or running the doctor.
#>
#requires -Version 7.0
[CmdletBinding()]
param(
    [Parameter(Mandatory)][string]$GameDir,
    [Parameter(Mandatory)][string]$LoBoDir,
    [Parameter(Mandatory)][string]$Tools,
    [switch]$CheckOnly
)
$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest
Import-Module (Join-Path $PSScriptRoot 'tools/runtime/InstallTree.psm1') -Force
Import-Module (Join-Path $PSScriptRoot 'tools/runtime/UDDataDir.psm1') -Force

function Resolve-Child {
    param([string]$Parent, [string]$Name)
    if (Test-Path -LiteralPath $Parent -PathType Container) {
        $matches = @(Get-ChildItem -LiteralPath $Parent -Force | Where-Object Name -IEQ $Name)
        if ($matches.Count -gt 1) { throw "Case-ambiguous entries for $Name in $Parent" }
        if ($matches.Count -eq 1) { return $matches[0].FullName }
    }
    return Join-Path $Parent $Name
}
function Require-File {
    param([string]$Path)
    if (-not (Test-Path -LiteralPath $Path -PathType Leaf)) { throw "Required file missing: $Path" }
}
function Resolve-Relative {
    param([string]$Parent, [string]$Relative)
    foreach ($part in $Relative.Split('/')) { $Parent = Resolve-Child $Parent $part }
    return $Parent
}
function Save-Existing {
    param([string]$Path, [string]$Root)
    if (-not (Test-Path -LiteralPath $Path -PathType Leaf)) { return }
    $relative = [IO.Path]::GetRelativePath($Root, $Path)
    if ($relative.StartsWith('..') -or [IO.Path]::IsPathRooted($relative)) { throw "Backup outside root: $Path" }
    $backup = Join-Path (Join-Path $Root ".ud-backups/$runId") $relative
    Assert-PlainPath $backup
    [void][IO.Directory]::CreateDirectory([IO.Path]::GetDirectoryName($backup))
    [IO.File]::Copy($Path, $backup, $false)
    Write-Output "Backup: $backup"
}
function Get-FileSha {
    param([string]$Path)
    return (Get-FileHash -LiteralPath $Path -Algorithm SHA256).Hash
}
# First CSV field of every line: the stringtable key (keys never hold commas).
function Get-StringKeys {
    param([string]$Path)
    $keys = [Collections.Generic.HashSet[string]]::new([StringComparer]::OrdinalIgnoreCase)
    foreach ($line in [IO.File]::ReadAllLines($Path)) {
        $key = $line.Split(',')[0].Trim().Trim([char]0xFEFF).Trim('"')
        if ($key -and $key -ine 'LANGUAGE') { [void]$keys.Add($key) }
    }
    return , $keys
}
function Test-AppendOnlyTail {
    param([byte[]]$Before, [byte[]]$After)
    if ($After.Length -lt $Before.Length) { return $false }
    for ($i = 0; $i -lt $Before.Length; $i++) { if ($After[$i] -ne $Before[$i]) { return $false } }
    $tail = [Text.Encoding]::UTF8.GetString($After, $Before.Length, $After.Length - $Before.Length)
    foreach ($line in $tail -split "`r?`n") {
        if ($line.Trim() -and $line -notmatch '^\s*(//.*|#include\s*"[^"]+")\s*$') { return $false }
    }
    return $true
}

$GameDir = $ExecutionContext.SessionState.Path.GetUnresolvedProviderPathFromPSPath($GameDir)
$LoBoDir = $ExecutionContext.SessionState.Path.GetUnresolvedProviderPathFromPSPath($LoBoDir)
$Tools = $ExecutionContext.SessionState.Path.GetUnresolvedProviderPathFromPSPath($Tools)
foreach ($dir in @($GameDir, $LoBoDir)) {
    if (-not (Test-Path -LiteralPath $dir -PathType Container)) { throw "Directory missing: $dir" }
    Assert-PlainPath $dir
}
if ($GameDir -eq $LoBoDir) { throw 'GameDir and LoBoDir must be separate folders.' }
# Reject Classic, Demo and incomplete packages before planning or writing files.
$GameDir = Assert-UDRemasterDataDir -DataDir $GameDir
$bin = Resolve-Child $GameDir 'bin'
$resourceExtra = Resolve-Child $bin 'resource-extra.cpp'
Require-File $Tools
Require-File (Resolve-Child $bin 'config.bin')
Require-File (Resolve-Child $bin 'resource.bin')
Require-File (Resolve-Relative $GameDir 'dta/data3d.pbo')
Require-File (Resolve-Relative $GameDir 'worlds/abel.wrp')
$addons = Resolve-Child $LoBoDir 'addons'
foreach ($name in @('LoBoammo.pbo','LoBo_airammo.pbo','LoBoWreck.pbo','LoBoPalObj.pbo')) {
    Require-File (Resolve-Child $addons $name)
}
foreach ($item in Get-ChildItem -LiteralPath $LoBoDir -Recurse -Force) {
    if ($item.Attributes -band [IO.FileAttributes]::ReparsePoint) { throw "Linked LoBo content: $($item.FullName)" }
}

$manifest = Get-Content -LiteralPath (Join-Path $PSScriptRoot 'guerrilla-mode/runtime/manifest.json') -Raw | ConvertFrom-Json
$plan = @()
$stringSource = $null
$noticeEntry = $null
foreach ($entry in $manifest.files) {
    $source = Join-Path $PSScriptRoot $entry.source
    Require-File $source
    if ((Get-FileHash -LiteralPath $source -Algorithm SHA256).Hash -ine $entry.sha256) {
        throw "Runtime source hash mismatch: $source (update manifest after intentional edits)"
    }
    # install = UD-owned, kept current;
    # keep-existing = Bohemia may ship it, so an existing file is never replaced;
    # string-gap = feeds the generated STRINGTABLE_UD shard; skip = Classic shim.
    $policy = [string]$entry.remaster
    if ($policy -notin @('install', 'keep-existing', 'string-gap', 'skip')) {
        throw "Manifest entry $($entry.destination) has no valid remaster policy: '$policy'"
    }
    if ($policy -eq 'skip') { continue }
    if ($policy -eq 'string-gap') {
        if ($entry.destination -like '*.csv') { $stringSource = $source } else { $noticeEntry = $entry }
        continue
    }
    $target = Resolve-Relative $GameDir $entry.destination
    Assert-PlainPath $target
    if (Test-Path -LiteralPath $target -PathType Container) { throw "Expected a file: $target" }
    $plan += [pscustomobject]@{ Source=$source; Target=$target; Hash=$entry.sha256; Policy=$policy }
}
foreach ($relative in @('bin/config-extra.cpp','bin/guerrilla-factions.hpp','bin/guerrilla-hq.hpp','gmcore','Missions')) {
    $path = Resolve-Relative $GameDir $relative
    Assert-PlainPath $path
    if (Test-Path -LiteralPath $path -PathType Container) {
        foreach ($item in Get-ChildItem -LiteralPath $path -Recurse -Force) { Assert-PlainPath $item.FullName }
    }
}

# Remaster: the UD stringtable shard carries only keys Bohemia's shards lack. The
# engine loads every STRINGTABLE_*.csv sibling in name order and a later shard
# wins a duplicate key, so shipping our whole Options shard would overwrite
# Bohemia's newer Options text with the Demo-era copy.
$udStrings = $null
$udStringsText = $null
$udStrings = Resolve-Child $bin 'STRINGTABLE_UD.utf8.csv'
Assert-PlainPath $udStrings
$theirKeys = [Collections.Generic.HashSet[string]]::new([StringComparer]::OrdinalIgnoreCase)
foreach ($csv in Get-ChildItem -LiteralPath $bin -File -Force) {
    if ($csv.Name -notmatch '^stringtable.*\.csv$' -or $csv.Name -ieq 'STRINGTABLE_UD.utf8.csv') { continue }
    $theirKeys.UnionWith((Get-StringKeys $csv.FullName))
}
$sourceLines = [IO.File]::ReadAllLines($stringSource)
$gapLines = @(foreach ($line in $sourceLines | Select-Object -Skip 1) {
    $key = $line.Split(',')[0].Trim().Trim('"')
    if ($key -and -not $theirKeys.Contains($key)) { $line }
})
if ($gapLines.Count -gt 0) {
    $udStringsText = (@($sourceLines[0].Trim([char]0xFEFF)) + $gapLines) -join "`n"
    $udStringsText += "`n"
    $gapKeys = @($gapLines | ForEach-Object { $_.Split(',')[0] })
    Write-Output "Stringtable: Bohemia's shards lack $($gapLines.Count) UD key(s): $($gapKeys -join ', ')"
} else {
    Write-Output "Stringtable: Bohemia's shards cover every UD Options key; no UD shard needed."
}

$loboConfig = Resolve-Relative $LoBoDir 'bin/config.cpp'
Assert-PlainPath $loboConfig
if (Test-Path -LiteralPath $loboConfig) {
    $text = [IO.File]::ReadAllText($loboConfig)
    if ($text -notmatch '^// GENERATED by arma_CWR/tools/lobo/install-lobo-factions.ps1') {
        throw "Custom LoBo config needs merging before setup: $loboConfig"
    }
}
if (Test-Path -LiteralPath (Resolve-Relative $LoBoDir 'bin/config.bin')) {
    throw 'LoBo has a bin/config.bin that the generated config would shadow; merge it first.'
}
$deps = Join-Path $LoBoDir 'deps/@LoBo_Deps'
$modDirs = @($LoBoDir)
if (Test-Path -LiteralPath $deps -PathType Container) { $modDirs += $deps }
else { Write-Warning 'LoBo has no deps/@LoBo_Deps. For the recovered dependency set, use the complete server LoBo folder.' }
$dataName = 'official Remaster data (Bohemia files kept)'
Write-Output "Validated: $dataName, LoBo, tools, and $($plan.Count) runtime files."
if ($CheckOnly) { return }

$runId = (Get-Date -Format 'yyyyMMdd-HHmmss') + '-' + [guid]::NewGuid().ToString('N').Substring(0,8)

# Remaster: snapshot every pre-existing package file (outside LoBo and our own
# backups) so the run can prove it changed nothing of Bohemia's except the
# appended include lines. bin/ and fonts/ are hashed; the large data folders are
# compared by length and timestamp.
$snapshot = $null
$snapshot = @{}
# UD-owned paths (earlier runs' output, which a rerun may legitimately update)
# are not package files: LoBo, backups, gmcore, the mission templates, and the
# bin/ and fonts/ files only UD writes.
$skipRoots = @([IO.Path]::GetFullPath($LoBoDir).TrimEnd('\','/'), (Join-Path $GameDir '.ud-backups'),
    (Resolve-Child $GameDir 'gmcore'))
$missionsDir = Resolve-Child $GameDir 'Missions'
foreach ($template in Get-ChildItem -LiteralPath (Join-Path $PSScriptRoot 'guerrilla-mode/mission') -Directory) {
    $skipRoots += Resolve-Child $missionsDir $template.Name
}
$udBinFiles = @($plan | Where-Object Policy -EQ 'install' | ForEach-Object Target) +
    @($udStrings, (Resolve-Relative $GameDir $noticeEntry.destination),
      (Resolve-Child $bin 'guerrilla-factions.hpp'), (Resolve-Child $bin 'guerrilla-hq.hpp'))
$skipRoots += $udBinFiles
$skipRoots = @($skipRoots | ForEach-Object { [IO.Path]::GetFullPath($_).TrimEnd('\','/') })
$pending = [Collections.Generic.Stack[string]]::new()
$pending.Push($GameDir)
while ($pending.Count -gt 0) {
    $dir = $pending.Pop()
    foreach ($item in Get-ChildItem -LiteralPath $dir -Force) {
        $full = $item.FullName.TrimEnd('\','/')
        if ($skipRoots | Where-Object { $_ -ieq $full }) { continue }
        if ($item.PSIsContainer) { $pending.Push($item.FullName); continue }
        $relative = [IO.Path]::GetRelativePath($GameDir, $item.FullName)
        $top = $relative.Split([char[]]@('\','/'))[0]
        $hashed = $top -ieq 'bin' -or $top -ieq 'fonts'
        $snapshot[$relative] = [pscustomobject]@{
            Path = $item.FullName; Length = $item.Length; Time = $item.LastWriteTimeUtc
            Hash = if ($hashed) { Get-FileSha $item.FullName } else { $null }
            Bytes = if ($item.Name -ieq 'resource-extra.cpp' -or $item.Name -ieq 'config-extra.cpp') { [IO.File]::ReadAllBytes($item.FullName) } else { $null }
        }
    }
}

foreach ($entry in $plan) {
    $exists = Test-Path -LiteralPath $entry.Target
    if ($exists -and (Get-FileSha $entry.Target) -ieq $entry.Hash) { continue }
    if ($exists -and $entry.Policy -eq 'keep-existing') {
        Write-Output "Kept package file: $($entry.Target)"
        continue
    }
    Save-Existing $entry.Target $GameDir
    [void][IO.Directory]::CreateDirectory([IO.Path]::GetDirectoryName($entry.Target))
    [IO.File]::Copy($entry.Source, $entry.Target, $true)
    Write-Output "Installed: $($entry.Target)"
}
if ($null -ne $udStringsText) {
    $current = if (Test-Path -LiteralPath $udStrings) { [IO.File]::ReadAllText($udStrings) } else { $null }
    if ($current -cne $udStringsText) {
        Save-Existing $udStrings $GameDir
        [IO.File]::WriteAllText($udStrings, $udStringsText, [Text.UTF8Encoding]::new($false))
        Write-Output "Installed: $udStrings"
    }
    $notice = Resolve-Relative $GameDir $noticeEntry.destination
    Assert-PlainPath $notice
    if (-not (Test-Path -LiteralPath $notice) -or (Get-FileSha $notice) -ine $noticeEntry.sha256) {
        Save-Existing $notice $GameDir
        [IO.File]::Copy((Join-Path $PSScriptRoot $noticeEntry.source), $notice, $true)
        Write-Output "Installed: $notice"
    }
} elseif (Test-Path -LiteralPath $udStrings) {
    # Bohemia now covers every key; retire our shard so it cannot shadow theirs.
    Save-Existing $udStrings $GameDir
    [IO.File]::Delete($udStrings)
    Write-Output "Removed (now covered by Bohemia's shards): $udStrings"
}
# The one edit to a Bohemia file: append the UD includes, each once. The
# menu (RscDisplayMainUD) and the Options additions (RscOptionsPageControlsUD,
# the Arma 3 controls row) each carry their own class name so they parse
# beside Bohemia's classes in that one file; the engine prefers them when
# present. An install from before ud-options.hpp existed gets only the
# missing line.
$udIncludes = @(
    @{ Name = 'ud-mainmenu.hpp'; Comment = 'main menu' },
    @{ Name = 'ud-options.hpp'; Comment = 'Options additions (Arma 3 controls row)' })
$backedUp = $false
foreach ($include in $udIncludes) {
    $resourceText = [IO.File]::ReadAllText($resourceExtra)
    if ($resourceText -match ('(?m)^\s*#include\s*"' + [regex]::Escape($include.Name) + '"')) {
        Write-Output "Up to date: $resourceExtra includes $($include.Name)"
        continue
    }
    if (-not $backedUp) { Save-Existing $resourceExtra $GameDir; $backedUp = $true }
    $append = "`n// Uslu dur! $($include.Comment); appended by arma_CWR/setup-guerrilla.ps1 (Remaster mode).`n" +
              "#include `"$($include.Name)`"`n"
    [IO.File]::AppendAllText($resourceExtra, $append)
    Write-Output "Appended: $($include.Name) include -> $resourceExtra"
}
# install-missions appends its two includes to Bohemia's config-extra.cpp;
# back it up only when that append is still to come.
$configExtra = Resolve-Child $bin 'config-extra.cpp'
if (Test-Path -LiteralPath $configExtra) {
    $configText = [IO.File]::ReadAllText($configExtra)
    foreach ($library in @('guerrilla-factions.hpp', 'guerrilla-hq.hpp')) {
        if ($configText -notmatch ('(?m)^\s*#include\s*"' + [regex]::Escape($library) + '"')) {
            Save-Existing $configExtra $GameDir
            break
        }
    }
}
Save-Existing $loboConfig $LoBoDir
# Invoke in-process child script scopes; their explicit exit 0 returns to the caller.
& (Join-Path $PSScriptRoot 'guerrilla-mode/install-missions.ps1') -GameDir $GameDir -ModDir $modDirs
if ($LASTEXITCODE -ne 0) { throw 'Mission installation failed.' }
& (Join-Path $PSScriptRoot 'tools/lobo/install-lobo-factions.ps1') -LoBoDir $LoBoDir
if ($LASTEXITCODE -ne 0) { throw 'LoBo faction installation failed.' }
# A full scan reports unrelated compressed configs as NOT_PATCHABLE (exit 1).
# Gate setup on the four known LoBo repair targets instead of treating those
# unrelated limitations as broken installation data.
foreach ($name in @('LoBoammo.pbo','LoBo_airammo.pbo','LoBoWreck.pbo','LoBoPalObj.pbo')) {
    & $Tools mod doctor $LoBoDir --fix --pbo $name
    if ($LASTEXITCODE -ne 0) { throw "LoBo repair failed for $name (exit $LASTEXITCODE). See doctor output." }
}
foreach ($entry in $plan) {
    if ($entry.Policy -eq 'keep-existing') {
        Require-File $entry.Target
    } elseif ((Get-FileSha $entry.Target) -ine $entry.Hash) {
        throw "Installed file verification failed: $($entry.Target)"
    }
}
foreach ($relative in $snapshot.Keys) {
    $before = $snapshot[$relative]
    if (-not (Test-Path -LiteralPath $before.Path -PathType Leaf)) { throw "Package file removed: $($before.Path)" }
    if ($null -ne $before.Bytes) {
        if (-not (Test-AppendOnlyTail $before.Bytes ([IO.File]::ReadAllBytes($before.Path)))) {
            throw "Package file changed beyond appended #include lines: $($before.Path)"
        }
        continue
    }
    $item = Get-Item -LiteralPath $before.Path -Force
    $same = if ($null -ne $before.Hash) { (Get-FileSha $before.Path) -ieq $before.Hash }
            else { $item.Length -eq $before.Length -and $item.LastWriteTimeUtc -eq $before.Time }
    if (-not $same) { throw "Package file changed: $($before.Path)" }
}
Write-Output "Verified: $($snapshot.Count) package files unchanged apart from appended #include lines."
Write-Output "Setup complete (Remaster). Launch the rebuilt PoseidonGame with these game/mod folders."
Write-Output ('Mods: ' + ($modDirs -join ';'))
