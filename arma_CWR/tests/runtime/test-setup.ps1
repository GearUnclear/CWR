#requires -Version 7.0
$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest
$repo = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
$scratch = Join-Path ([IO.Path]::GetTempPath()) ('ud-setup-' + [guid]::NewGuid().ToString('N'))
$game = Join-Path $scratch 'Fresh [Remaster]'
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
& (Join-Path $PSScriptRoot 'test-data-dir.ps1')
try {
    New-GameData $game
    [void][IO.Directory]::CreateDirectory((Join-Path $game 'DTAEXT'))
    Put (Join-Path $game 'BIN/remaster.cpp') '// official contract'
    # Copied-in rebuilt binaries are preserved alongside the official package.
    Put (Join-Path $game 'PoseidonGame.exe') 'rebuilt engine copied into Remaster'
    Put (Join-Path $game 'PoseidonGame') 'rebuilt engine copied into Remaster'
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
    # The Remaster Demo is refused before any write (no Abel, no GUER roster; not a data target).
    $loboState = Get-TreeState $lobo
    # The Demo lacks Abel; the refusal must still win over the missing-file and tool checks.
    $demo = Join-Path $scratch 'Demo [Remaster]'
    New-GameData $demo -NoWorld
    [void][IO.Directory]::CreateDirectory((Join-Path $demo 'dtaExt/equip'))
    foreach ($name in @('remaster.cpp','resource-extra.cpp','mainMenu.hpp','splashLogo.hpp')) {
        Put (Join-Path $demo "BIN/$name") "// Bohemia $name"
    }
    Put (Join-Path $demo 'PoseidonGameDemo.exe') 'official demo binary'
    $demoState = Get-TreeState $demo
    Expect-Failure { & $setup -GameDir $demo -LoBoDir $lobo -Tools (Join-Path $scratch 'missing-tool') -CheckOnly } 'Remaster Demo package'
    Expect-Failure { & $setup -GameDir $demo -LoBoDir $lobo -Tools $doctor } 'Remaster Demo package'
    Assert ((Get-TreeState $demo) -eq $demoState) 'Refused Demo folder was modified'
    # A Demo binary alone marks the Demo even where an Abel world is present.
    $demoAbel = Join-Path $scratch 'Demo with Abel [Remaster]'
    New-GameData $demoAbel
    Put (Join-Path $demoAbel 'BIN/mainMenu.hpp') '// Bohemia mainMenu.hpp'
    Put (Join-Path $demoAbel 'BIN/resource-extra.cpp') '// Bohemia resource-extra.cpp'
    Put (Join-Path $demoAbel 'PoseidonGameDemo') 'official demo binary'
    Expect-Failure { & $setup -GameDir $demoAbel -LoBoDir $lobo -Tools $doctor } 'Remaster Demo package'
    # Each marker alone, in any letter case, selects Remaster mode. Without Bohemia's
    # own bin/resource-extra.cpp to append to, that mode refuses before any write.
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
        Expect-Failure { & $setup -GameDir $root -LoBoDir $lobo -Tools $doctor } 'no bin/resource-extra.cpp'
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
    Assert ([IO.File]::ReadAllText((Join-Path $game 'BIN/remaster.cpp')) -eq '// official contract') 'Official contract changed'
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
        if ($entry.remaster -notin @('install','keep-existing')) { continue }
        # Resolve the original uppercase BIN in a case-sensitive filesystem too.
        $relative = $entry.destination -replace '^bin/', 'BIN/'
        Assert ((Get-FileHash -LiteralPath (Join-Path $game $relative)).Hash -ieq $entry.sha256) "Payload drift: $relative"
    }
    Assert (Test-Path -LiteralPath (Join-Path $game 'BIN/STRINGTABLE_UD.utf8.csv')) 'Remaster gap shard missing'
    Assert ([IO.File]::ReadAllText((Join-Path $game 'BIN/resource-extra.cpp')).Contains('#include "ud-mainmenu.hpp"')) 'Remaster menu include missing'

    # ---- Remaster mode: keep every Bohemia file, add UD's beside them (issue #67) ----
    $steam = Join-Path $scratch 'Remastered [Steam]'
    New-GameData $steam
    [void][IO.Directory]::CreateDirectory((Join-Path $steam 'dtaExt/equip'))
    Put (Join-Path $steam 'dtaExt/equip/w_m16.paa') 'official equipment art'
    foreach ($name in @('remaster.cpp','optionsShell.hpp','optionsUI.hpp','mainMenu.hpp','splashLogo.hpp','modsTable.hpp')) {
        Put (Join-Path $steam "BIN/$name") "// Bohemia $name"
    }
    # Bohemia's resource-extra/config-extra (LF, trailing newline like the shipped files).
    $bohemiaResource = "// Bohemia resource-extra.cpp`n#include `"optionsShell.hpp`"`n#include `"splashLogo.hpp`"`n#include `"modsTable.hpp`"`n#include `"mainMenu.hpp`"`n"
    $bohemiaConfig = "// Bohemia config-extra.cpp`nclass CfgLanguages { languages[]={`"English`"}; };`n"
    Put (Join-Path $steam 'BIN/resource-extra.cpp') $bohemiaResource
    Put (Join-Path $steam 'BIN/config-extra.cpp') $bohemiaConfig
    Put (Join-Path $steam 'BIN/STRINGTABLE.CSV') "LANGUAGE,English`nSTR_LEGACY_ONLY,legacy`n"
    # Bohemia's shard covers every UD Options key except the two lean actions, with its own text.
    $ourStrings = [IO.File]::ReadAllLines((Join-Path $repo 'thirdparty/classic-ui/STRINGTABLE_OPTIONS.utf8.csv'))
    $gapKeys = @('STR_USRACT_LEAN_LEFT', 'STR_USRACT_LEAN_RIGHT')
    $theirRows = @($ourStrings | Select-Object -Skip 1 | ForEach-Object { $_.Split(',')[0] } |
        Where-Object { $_ -notin $gapKeys } | ForEach-Object { "$_,Bohemia text" })
    $bohemiaShard = ((@('LANGUAGE,English') + $theirRows) -join "`n") + "`n"
    Put (Join-Path $steam 'BIN/STRINGTABLE_MAINMENU.utf8.csv') $bohemiaShard
    # Bohemia ships the UD fonts: one byte-identical, one differing, OFL its own text.
    [void][IO.Directory]::CreateDirectory((Join-Path $steam 'fonts'))
    Copy-Item -LiteralPath (Join-Path $repo 'thirdparty/guerrilla-fonts/cwr_body.ttf') -Destination (Join-Path $steam 'fonts/cwr_body.ttf')
    Put (Join-Path $steam 'fonts/cwr_title.ttf') 'Bohemia build of the title font'
    Put (Join-Path $steam 'fonts/OFL.txt') 'Bohemia OFL text'
    Put (Join-Path $steam 'Missions/01TakeTheCar.ABEL/mission.sqm') 'Bohemia mission'
    Put (Join-Path $steam 'PoseidonGame.exe') 'official binary'
    $steamBefore = @{}
    foreach ($file in Get-ChildItem -LiteralPath $steam -Recurse -File -Force) {
        $steamBefore[[IO.Path]::GetRelativePath($steam, $file.FullName)] = (Get-FileHash -LiteralPath $file.FullName).Hash
    }
    $steamState = Get-TreeState $steam
    & $setup -GameDir $steam -LoBoDir $lobo -Tools $doctor -CheckOnly
    Assert ((Get-TreeState $steam) -eq $steamState) 'Remaster CheckOnly wrote files'
    & $setup -GameDir $steam -LoBoDir $lobo -Tools $doctor
    $appended = @('BIN\resource-extra.cpp', 'BIN\config-extra.cpp') | ForEach-Object { $_ -replace '\\', [IO.Path]::DirectorySeparatorChar }
    foreach ($relative in $steamBefore.Keys) {
        if ($relative -in $appended) { continue }
        $path = Join-Path $steam $relative
        Assert (Test-Path -LiteralPath $path) "Bohemia file removed: $relative"
        Assert ((Get-FileHash -LiteralPath $path).Hash -eq $steamBefore[$relative]) "Bohemia file changed: $relative"
    }
    # The only mutation: appended include lines.
    $resourceAfter = [IO.File]::ReadAllText((Join-Path $steam 'BIN/resource-extra.cpp'))
    Assert ($resourceAfter.StartsWith($bohemiaResource)) 'Bohemia resource-extra.cpp prefix changed'
    $tail = $resourceAfter.Substring($bohemiaResource.Length)
    Assert (@($tail -split "`n" | Where-Object { $_.Trim() -and $_ -notmatch '^(//.*|#include "ud-(mainmenu|options)\.hpp")$' }).Count -eq 0) "Unexpected resource-extra tail: $tail"
    Assert (([regex]::Matches($resourceAfter, '#include "ud-mainmenu.hpp"')).Count -eq 1) 'Menu include missing or duplicated'
    Assert (([regex]::Matches($resourceAfter, '#include "ud-options.hpp"')).Count -eq 1) 'Options include missing or duplicated'
    # The Options shard comes after Bohemia's optionsShell.hpp include and after the menu.
    Assert ($resourceAfter.IndexOf('#include "ud-options.hpp"') -gt $resourceAfter.IndexOf('#include "ud-mainmenu.hpp"')) 'Options include precedes the menu include'
    $configAfter = [IO.File]::ReadAllText((Join-Path $steam 'BIN/config-extra.cpp'))
    Assert ($configAfter.StartsWith($bohemiaConfig)) 'Bohemia config-extra.cpp prefix changed'
    foreach ($library in @('guerrilla-factions.hpp', 'guerrilla-hq.hpp')) {
        Assert (([regex]::Matches($configAfter, "#include `"$library`"")).Count -eq 1) "Config include missing or duplicated: $library"
    }
    # UD's own files land beside Bohemia's; the Classic shims do not.
    foreach ($rel in @('BIN/ud-mainmenu.hpp','BIN/ud-options.hpp','fonts/UD_FONT_CREDITS.md','fonts/cwr_mono.ttf','gmcore/init.sqs','Missions/Guerrilla.Abel/mission.sqm','BIN/guerrilla-factions.hpp')) {
        Assert (Test-Path -LiteralPath (Join-Path $steam $rel)) "Missing Remaster install output: $rel"
    }
    foreach ($rel in @('BIN/optionsTemplates.hpp','BIN/optionsScrollList.hpp','BIN/optionsScrollListSlots.hpp','BIN/STRINGTABLE_OPTIONS.utf8.csv')) {
        Assert (-not (Test-Path -LiteralPath (Join-Path $steam $rel))) "Classic shim installed on the Remaster: $rel"
    }
    $udMenu = $manifest.files | Where-Object destination -EQ 'bin/ud-mainmenu.hpp'
    Assert ((Get-FileHash -LiteralPath (Join-Path $steam 'BIN/ud-mainmenu.hpp')).Hash -ieq $udMenu.sha256) 'UD menu payload drift'
    $udOptions = $manifest.files | Where-Object destination -EQ 'bin/ud-options.hpp'
    Assert ((Get-FileHash -LiteralPath (Join-Path $steam 'BIN/ud-options.hpp')).Hash -ieq $udOptions.sha256) 'UD options payload drift'
    # The gap shard holds exactly the keys Bohemia lacks, in our text.
    $gapShard = Join-Path $steam 'BIN/STRINGTABLE_UD.utf8.csv'
    Assert (Test-Path -LiteralPath $gapShard) 'UD gap shard missing'
    $gapRows = @([IO.File]::ReadAllLines($gapShard))
    Assert ($gapRows[0] -eq $ourStrings[0].Trim([char]0xFEFF)) 'Gap shard header wrong'
    Assert ((@($gapRows | Select-Object -Skip 1 | ForEach-Object { $_.Split(',')[0] }) -join ',') -eq ($gapKeys -join ',')) "Gap shard keys wrong: $($gapRows -join ' | ')"
    Assert ($gapRows[1] -eq ($ourStrings | Where-Object { $_.StartsWith('STR_USRACT_LEAN_LEFT,') })) 'Gap shard text is not ours'
    Assert (Test-Path -LiteralPath (Join-Path $steam 'BIN/UD_OPTIONS_APL-SA_NOTICE.txt')) 'Notice missing beside the gap shard'
    $remasterBackup = @(Get-ChildItem -LiteralPath (Join-Path $steam '.ud-backups') -Recurse -File | Where-Object Name -EQ 'resource-extra.cpp')
    Assert ($remasterBackup.Count -eq 1 -and [IO.File]::ReadAllText($remasterBackup[0].FullName) -eq $bohemiaResource) 'Pristine resource-extra backup missing'
    # Rerun: nothing appended twice, nothing rewritten, no new backups.
    $afterFirst = Get-TreeState $steam
    & $setup -GameDir $steam -LoBoDir $lobo -Tools $doctor
    Assert ((Get-TreeState $steam) -eq $afterFirst) 'Remaster rerun was not idempotent'
    # Steam update/verify restores Bohemia's pristine file: the includes come back once.
    [IO.File]::WriteAllText((Join-Path $steam 'BIN/resource-extra.cpp'), $bohemiaResource)
    & $setup -GameDir $steam -LoBoDir $lobo -Tools $doctor
    Assert ([IO.File]::ReadAllText((Join-Path $steam 'BIN/resource-extra.cpp')) -eq $resourceAfter) 'Re-append after a Steam restore differs'
    # An install from before the Options shard existed (menu include only) gains just that line.
    $menuOnly = $resourceAfter.Substring(0, $resourceAfter.IndexOf("`n// Uslu dur! Options additions"))
    [IO.File]::WriteAllText((Join-Path $steam 'BIN/resource-extra.cpp'), $menuOnly + "`n")
    & $setup -GameDir $steam -LoBoDir $lobo -Tools $doctor
    $upgraded = [IO.File]::ReadAllText((Join-Path $steam 'BIN/resource-extra.cpp'))
    Assert ($upgraded.StartsWith($menuOnly)) 'Upgrade rewrote the menu-only prefix'
    Assert (([regex]::Matches($upgraded, '#include "ud-mainmenu.hpp"')).Count -eq 1) 'Upgrade duplicated the menu include'
    Assert (([regex]::Matches($upgraded, '#include "ud-options.hpp"')).Count -eq 1) 'Upgrade did not add the options include once'
    [IO.File]::WriteAllText((Join-Path $steam 'BIN/resource-extra.cpp'), $resourceAfter)
    # Bohemia ships the missing keys: our shard retires so it cannot shadow theirs.
    [IO.File]::AppendAllText((Join-Path $steam 'BIN/STRINGTABLE_MAINMENU.utf8.csv'), "STR_USRACT_LEAN_LEFT,Bohemia lean`nSTR_USRACT_LEAN_RIGHT,Bohemia lean`n")
    & $setup -GameDir $steam -LoBoDir $lobo -Tools $doctor
    Assert (-not (Test-Path -LiteralPath $gapShard)) 'Covered gap shard was not retired'
    # Any other edit to a package file fails the run's own verification: a tool
    # double that touches one of Bohemia's headers mid-run must be caught.
    $tamper = Join-Path $scratch 'tamper-doctor.ps1'
    Put $tamper ("[IO.File]::AppendAllText('" + (Join-Path $steam 'BIN/modsTable.hpp') + "', '// edited')`nexit 0`n")
    Expect-Failure { & $setup -GameDir $steam -LoBoDir $lobo -Tools $tamper } 'Package file changed'
    Write-Output 'PASS: Remaster mode keeps every Bohemia file byte-identical apart from appended includes; gap shard, rerun idempotency, Steam-restore re-append and tamper detection.'
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
    Write-Output 'PASS: Demo packages and incomplete Remaster folders refused before any write (dtaExt, remaster.bin, mainMenu/splashLogo, any case); Remaster reruns pass.'
} finally {
    $resolved = [IO.Path]::GetFullPath($scratch)
    $tempRoot = [IO.Path]::GetFullPath([IO.Path]::GetTempPath()).TrimEnd('/','\') + [IO.Path]::DirectorySeparatorChar
    if ($resolved.StartsWith($tempRoot) -and [IO.Path]::GetFileName($resolved) -like 'ud-setup-*') {
        if (Test-Path -LiteralPath $resolved) { Remove-Item -LiteralPath $resolved -Recurse -Force }
    }
}
