#requires -Version 7.0
$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest
$repo = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
$scratch = Join-Path ([IO.Path]::GetTempPath()) ('ud-setup-' + [guid]::NewGuid().ToString('N'))
$game = Join-Path $scratch 'Fresh [Classic]'
$lobo = Join-Path $scratch '@LoBo'
$setup = Join-Path $repo 'setup-guerrilla.ps1'
$doctor = Join-Path $PSScriptRoot 'doctor-stub.ps1'
function Put([string]$Path, [string]$Text) {
    [void][IO.Directory]::CreateDirectory([IO.Path]::GetDirectoryName($Path))
    [IO.File]::WriteAllText($Path, $Text)
}
function Assert([bool]$Condition, [string]$Message) { if (-not $Condition) { throw $Message } }
function Expect-Failure([scriptblock]$Action, [string]$Match) {
    try { & $Action } catch {
        Assert ($_.ToString() -like "*$Match*") "Unexpected failure: $_"
        return
    }
    throw "Expected failure: $Match"
}
try {
    foreach ($rel in @('BIN/CONFIG.BIN','BIN/RESOURCE.BIN','DTA/Data3D.pbo','Worlds/abel.wrp')) {
        Put (Join-Path $game $rel) 'synthetic fixture'
    }
    foreach ($name in @('LoBoammo.pbo','LoBo_airammo.pbo','LoBoWreck.pbo','LoBoPalObj.pbo')) {
        Put (Join-Path $lobo "AddOns/$name") 'synthetic fixture'
    }
    [void][IO.Directory]::CreateDirectory((Join-Path $lobo 'Bin'))
    # A valid PBO header declaring Sinai proves mod-world detection without real data.
    $memory = [IO.MemoryStream]::new()
    $writer = [IO.BinaryWriter]::new($memory)
    $writer.Write([Text.Encoding]::ASCII.GetBytes("sinai.wrp`0"))
    1..5 | ForEach-Object { $writer.Write([uint32]0) }
    $writer.Write([byte]0)
    1..5 | ForEach-Object { $writer.Write([uint32]0) }
    [IO.File]::WriteAllBytes((Join-Path $lobo 'AddOns/island.pbo'), $memory.ToArray())
    $writer.Dispose(); $memory.Dispose()
    Put (Join-Path $game 'BIN/config-extra.cpp') '// unrelated package configuration'
    Put (Join-Path $game 'BIN/resource-extra.cpp') '// preserve this override in backup'
    Put (Join-Path $game 'Missions/MyMission.Abel/mission.sqm') 'unrelated mission'
    & $setup -GameDir $game -LoBoDir $lobo -Tools $doctor -CheckOnly
    Assert (-not (Test-Path -LiteralPath (Join-Path $game 'fonts'))) 'CheckOnly wrote files'
    Expect-Failure { & $setup -GameDir $game -LoBoDir $lobo -Tools (Join-Path $scratch 'missing-tool') } 'Required file missing'
    Assert (-not (Test-Path -LiteralPath (Join-Path $game 'fonts'))) 'Preflight failure wrote files'
    Put (Join-Path $lobo 'Bin/config.cpp') '// custom mod config'
    Expect-Failure { & $setup -GameDir $game -LoBoDir $lobo -Tools $doctor } 'Custom LoBo config'
    # Exact fixture target only.
    Remove-Item -LiteralPath (Join-Path $lobo 'Bin/config.cpp')
    & $setup -GameDir $game -LoBoDir $lobo -Tools $doctor
    foreach ($rel in @('gmcore/init.sqs','Missions/Guerrilla.Abel/mission.sqm','Missions/Guerrilla.Sinai/mission.sqm','fonts/OFL.txt','BIN/UD_OPTIONS_APL-SA_NOTICE.txt')) {
        Assert (Test-Path -LiteralPath (Join-Path $game $rel)) "Missing installed output: $rel"
    }
    Assert (-not (Test-Path -LiteralPath (Join-Path $game 'gmcore/portraits'))) 'Developer portrait directory installed'
    $extra = [IO.File]::ReadAllText((Join-Path $game 'BIN/config-extra.cpp'))
    Assert ($extra.Contains('// unrelated package configuration')) 'Existing package config lost'
    Assert ($extra.Contains('#include "guerrilla-factions.hpp"')) 'Faction include missing'
    Assert (Test-Path -LiteralPath (Join-Path $lobo 'Bin/config.cpp')) 'LoBo factions missing'
    $backup = @(Get-ChildItem -LiteralPath (Join-Path $game '.ud-backups') -Recurse -File | Where-Object Name -EQ 'resource-extra.cpp')
    Assert ($backup.Count -eq 1) 'Expected original menu backup'
    Assert ([IO.File]::ReadAllText($backup[0].FullName) -eq '// preserve this override in backup') 'Wrong backup contents'
    Put (Join-Path $game 'gmcore/stale.sqs') 'stale core'
    Put (Join-Path $game 'gmcore/portraits/local.png') 'existing local image'
    Put (Join-Path $game 'Missions/Guerrilla.Demo/stale.sqs') 'world no longer available'
    & $setup -GameDir $game -LoBoDir $lobo -Tools $doctor
    Assert (-not (Test-Path -LiteralPath (Join-Path $game 'gmcore/stale.sqs'))) 'Stale core survived'
    Assert ([IO.File]::ReadAllText((Join-Path $game 'gmcore/portraits/local.png')) -eq 'existing local image') 'Excluded local image changed'
    Assert (-not (Test-Path -LiteralPath (Join-Path $game 'Missions/Guerrilla.Demo'))) 'Unavailable template survived'
    Assert (Test-Path -LiteralPath (Join-Path $game 'Missions/MyMission.Abel/mission.sqm')) 'Unrelated mission lost'
    $extra = [IO.File]::ReadAllText((Join-Path $game 'BIN/config-extra.cpp'))
    Assert (([regex]::Matches($extra, '#include "guerrilla-factions.hpp"')).Count -eq 1) 'Duplicate include'
    $manifest = Get-Content -LiteralPath (Join-Path $repo 'guerrilla-mode/runtime/manifest.json') -Raw | ConvertFrom-Json
    foreach ($entry in $manifest.files) {
        # Resolve the original uppercase BIN in a case-sensitive filesystem too.
        $relative = $entry.destination -replace '^bin/', 'BIN/'
        Assert ((Get-FileHash -LiteralPath (Join-Path $game $relative)).Hash -ieq $entry.sha256) "Payload drift: $relative"
    }
    Import-Module (Join-Path $repo 'tools/runtime/InstallTree.psm1') -Force
    Expect-Failure { Sync-InstallTree -Source (Join-Path $repo 'guerrilla-mode/core') -Destination $scratch -OwnerRoot $game } 'outside the owned'
    # Populated developer sources are ignored even when Git does not track their images.
    $source = Join-Path $scratch 'core-source'
    $mirror = Join-Path $game 'mirror-fixture'
    $cache = Join-Path $scratch 'external-cache/portraits/photo.png'
    Put (Join-Path $source 'portraits/generated.png') 'must not install'
    Put (Join-Path $source 'portraits/catalogue.json') 'must not install'
    Put (Join-Path $source 'scripts/portraits/keep.sqs') 'nested name stays'
    Put (Join-Path $source 'portraits-extra/keep.sqs') 'prefix stays'
    Put (Join-Path $source 'init.sqs') 'new core'
    Put (Join-Path $mirror 'stale.sqs') 'old core'
    Put $cache 'external cached image'
    Sync-InstallTree -Source $source -Destination $mirror -OwnerRoot $game -ExcludeRelativeDirectories @('portraits')
    Assert (-not (Test-Path -LiteralPath (Join-Path $mirror 'portraits'))) 'Generated portraits copied'
    Assert (-not (Test-Path -LiteralPath (Join-Path $mirror 'stale.sqs'))) 'Normal stale file survived exclusion'
    foreach ($rel in @('init.sqs','scripts/portraits/keep.sqs','portraits-extra/keep.sqs')) {
        Assert (Test-Path -LiteralPath (Join-Path $mirror $rel)) "Exclusion too broad: $rel"
    }
    Put (Join-Path $mirror 'portraits/generated.png') 'preserve destination'
    Sync-InstallTree -Source $source -Destination $mirror -OwnerRoot $game -ExcludeRelativeDirectories @('portraits')
    Assert ([IO.File]::ReadAllText((Join-Path $mirror 'portraits/generated.png')) -eq 'preserve destination') 'Excluded image overwritten'
    # Source archives omit portraits entirely. Preserve excluded destinations there too.
    $archiveSource = Join-Path $scratch 'archive-source'
    Put (Join-Path $archiveSource 'init.sqs') 'archive core'
    Put (Join-Path $mirror 'optional/portraits/old.png') 'nested excluded image'
    Sync-InstallTree -Source $archiveSource -Destination $mirror -OwnerRoot $game -ExcludeRelativeDirectories @('portraits','optional/portraits')
    Assert ([IO.File]::ReadAllText((Join-Path $mirror 'portraits/generated.png')) -eq 'preserve destination') 'Absent-source exclusion pruned'
    Assert (Test-Path -LiteralPath (Join-Path $mirror 'optional/portraits/old.png')) 'Excluded ancestor pruned'
    Assert ([IO.File]::ReadAllText($cache) -eq 'external cached image') 'External cache changed'
    foreach ($invalid in @('../portraits','/portraits','C:\portraits','portraits/../scripts','')) {
        Expect-Failure { Sync-InstallTree -Source $source -Destination $mirror -OwnerRoot $game -ExcludeRelativeDirectories @($invalid) } 'normalized relative path'
    }
    # Other users of the helper retain ordinary mirror semantics without exclusions.
    $plainMirror = Join-Path $game 'plain-mirror'
    Sync-InstallTree -Source $source -Destination $plainMirror -OwnerRoot $game
    Assert (Test-Path -LiteralPath (Join-Path $plainMirror 'portraits/generated.png')) 'Default mirror unexpectedly excludes portraits'
    Write-Output 'PASS: literal portrait exclusions, populated and archive sources, preserved local images and external cache.'
    Write-Output 'PASS: preflight, dry run, bracketed paths, case handling, backups, missions, LoBo factions, hashes and repeat install.'
} finally {
    $resolved = [IO.Path]::GetFullPath($scratch)
    $tempRoot = [IO.Path]::GetFullPath([IO.Path]::GetTempPath()).TrimEnd('/','\') + [IO.Path]::DirectorySeparatorChar
    if ($resolved.StartsWith($tempRoot) -and [IO.Path]::GetFileName($resolved) -like 'ud-setup-*') {
        if (Test-Path -LiteralPath $resolved) { Remove-Item -LiteralPath $resolved -Recurse -Force }
    }
}
