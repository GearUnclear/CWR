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
# Classic-shaped data. DTA/DTAEXT.PBO is Classic's own and must not look like a root dtaExt.
function New-GameData([string]$Root, [string]$Bin = 'BIN', [switch]$NoWorld) {
    foreach ($rel in @("$Bin/CONFIG.BIN","$Bin/RESOURCE.BIN",'DTA/Data3D.pbo','DTA/DTAEXT.PBO','Worlds/abel.wrp')) {
        if (-not ($NoWorld -and $rel -like 'Worlds/*')) { Put (Join-Path $Root $rel) 'synthetic fixture' }
    }
}
# Every entry with its content hash, to prove a refused run wrote nothing.
function Get-TreeState([string]$Root) {
    (Get-ChildItem -LiteralPath $Root -Recurse -Force | Sort-Object FullName | ForEach-Object {
        $hash = if ($_.PSIsContainer) { 'dir' } else { (Get-FileHash -LiteralPath $_.FullName).Hash }
        [IO.Path]::GetRelativePath($Root, $_.FullName) + '=' + $hash
    }) -join "`n"
}
function Expect-Failure([scriptblock]$Action, [string]$Match) {
    try { & $Action } catch {
        Assert ($_.ToString() -like "*$Match*") "Unexpected failure: $_"
        return
    }
    throw "Expected failure: $Match"
}
try {
    New-GameData $game
    # A copied-in rebuilt engine binary is not a Remaster marker.
    Put (Join-Path $game 'PoseidonGame.exe') 'rebuilt engine copied into Classic'
    Put (Join-Path $game 'PoseidonGame') 'rebuilt engine copied into Classic'
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
    # Official packages ship BIN files our shims would replace; refuse them before any write.
    $loboState = Get-TreeState $lobo
    $steam = Join-Path $scratch 'Remastered [Steam]'
    New-GameData $steam
    [void][IO.Directory]::CreateDirectory((Join-Path $steam 'dtaExt/equip'))
    foreach ($name in @('remaster.cpp','resource-extra.cpp','optionsShell.hpp','mainMenu.hpp','splashLogo.hpp')) {
        Put (Join-Path $steam "BIN/$name") "// Bohemia $name"
    }
    Put (Join-Path $steam 'PoseidonGame.exe') 'official binary'
    $steamState = Get-TreeState $steam
    Expect-Failure { & $setup -GameDir $steam -LoBoDir $lobo -Tools $doctor } 'official Remaster or Demo package'
    Expect-Failure { & $setup -GameDir $steam -LoBoDir $lobo -Tools $doctor -CheckOnly } 'targets Classic 1.99'
    Assert ((Get-TreeState $steam) -eq $steamState) 'Refused Remaster folder was modified'
    Assert ([IO.File]::ReadAllText((Join-Path $steam 'BIN/remaster.cpp')) -eq '// Bohemia remaster.cpp') 'Official remaster.cpp replaced'
    Assert (-not (Test-Path -LiteralPath (Join-Path $steam '.ud-backups'))) 'Refused Remaster folder got backups'
    # The Demo lacks Abel; the refusal must still win over the missing-file and tool checks.
    $demo = Join-Path $scratch 'Demo [Remaster]'
    New-GameData $demo -NoWorld
    [void][IO.Directory]::CreateDirectory((Join-Path $demo 'dtaExt/equip'))
    foreach ($name in @('remaster.cpp','resource-extra.cpp','mainMenu.hpp','splashLogo.hpp')) {
        Put (Join-Path $demo "BIN/$name") "// Bohemia $name"
    }
    Put (Join-Path $demo 'PoseidonGameDemo.exe') 'official demo binary'
    $demoState = Get-TreeState $demo
    Expect-Failure { & $setup -GameDir $demo -LoBoDir $lobo -Tools (Join-Path $scratch 'missing-tool') -CheckOnly } 'official Remaster or Demo package'
    Expect-Failure { & $setup -GameDir $demo -LoBoDir $lobo -Tools $doctor } 'official Remaster or Demo package'
    Assert ((Get-TreeState $demo) -eq $demoState) 'Refused Demo folder was modified'
    # Each marker alone is enough, in any letter case.
    $variants = @(@('BIN','dtaExt'), @('bin','DTAEXT'), @('BIN','BIN/remaster.bin'), @('bin','bin/REMASTER.BIN'),
        @('BIN','BIN/MAINMENU.HPP'), @('bin','bin/splashLogo.hpp'))
    for ($i = 0; $i -lt $variants.Count; $i++) {
        $binName, $marker = $variants[$i]
        $root = Join-Path $scratch "Marker $i [Remaster]"
        New-GameData $root -Bin $binName
        if ($marker.Contains('/')) { Put (Join-Path $root $marker) 'official package file' }
        else { [void][IO.Directory]::CreateDirectory((Join-Path $root $marker)) }
        $state = Get-TreeState $root
        Expect-Failure { & $setup -GameDir $root -LoBoDir $lobo -Tools $doctor } (Split-Path -Leaf $marker)
        Assert ((Get-TreeState $root) -eq $state) "Refused folder was modified: $marker"
    }
    Assert ((Get-TreeState $lobo) -eq $loboState) 'Refused setup changed LoBo'
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
    Assert ($extra.Contains('#include "guerrilla-hq.hpp"')) 'HQ include missing'
    Assert (Test-Path -LiteralPath (Join-Path $game 'BIN/guerrilla-hq.hpp')) 'HQ classes missing'
    Assert (Test-Path -LiteralPath (Join-Path $lobo 'Bin/config.cpp')) 'LoBo factions missing'
    $backup = @(Get-ChildItem -LiteralPath (Join-Path $game '.ud-backups') -Recurse -File | Where-Object Name -EQ 'resource-extra.cpp')
    Assert ($backup.Count -eq 1) 'Expected original menu backup'
    Assert ([IO.File]::ReadAllText($backup[0].FullName) -eq '// preserve this override in backup') 'Wrong backup contents'
    Put (Join-Path $game 'gmcore/stale.sqs') 'stale core'
    Put (Join-Path $game 'gmcore/portraits/local.png') 'existing local image'
    Put (Join-Path $game 'Missions/Guerrilla.Demo/stale.sqs') 'world no longer available'
    # Our installed bin/remaster.cpp must not make a Classic rerun look like a Remaster.
    Assert (Test-Path -LiteralPath (Join-Path $game 'BIN/remaster.cpp')) 'Shim missing before rerun'
    & $setup -GameDir $game -LoBoDir $lobo -Tools $doctor -CheckOnly
    & $setup -GameDir $game -LoBoDir $lobo -Tools $doctor
    Assert (-not (Test-Path -LiteralPath (Join-Path $game 'gmcore/stale.sqs'))) 'Stale core survived'
    Assert ([IO.File]::ReadAllText((Join-Path $game 'gmcore/portraits/local.png')) -eq 'existing local image') 'Excluded local image changed'
    Assert (-not (Test-Path -LiteralPath (Join-Path $game 'Missions/Guerrilla.Demo'))) 'Unavailable template survived'
    Assert (Test-Path -LiteralPath (Join-Path $game 'Missions/MyMission.Abel/mission.sqm')) 'Unrelated mission lost'
    $extra = [IO.File]::ReadAllText((Join-Path $game 'BIN/config-extra.cpp'))
    Assert (([regex]::Matches($extra, '#include "guerrilla-factions.hpp"')).Count -eq 1) 'Duplicate include'
    Assert (([regex]::Matches($extra, '#include "guerrilla-hq.hpp"')).Count -eq 1) 'Duplicate HQ include'
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
    Write-Output 'PASS: Remaster/Demo packages refused before any write (dtaExt, remaster.bin, mainMenu/splashLogo, any case); Classic reruns and copied engine binaries pass.'
} finally {
    $resolved = [IO.Path]::GetFullPath($scratch)
    $tempRoot = [IO.Path]::GetFullPath([IO.Path]::GetTempPath()).TrimEnd('/','\') + [IO.Path]::DirectorySeparatorChar
    if ($resolved.StartsWith($tempRoot) -and [IO.Path]::GetFileName($resolved) -like 'ud-setup-*') {
        if (Test-Path -LiteralPath $resolved) { Remove-Item -LiteralPath $resolved -Recurse -Force }
    }
}
