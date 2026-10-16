#requires -Version 7.0
$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest
$engine = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
Import-Module (Join-Path $engine 'tools/runtime/UDDataDir.psm1') -Force
$root = Join-Path ([IO.Path]::GetTempPath()) ('ud-data-dir-' + [guid]::NewGuid().ToString('N'))
$oldOverride = $env:UD_DATA_DIR
$failures = @()
function Start-Process { throw 'Unexpected process launch' }
function cmake { throw 'Unexpected build invocation' }
function Put([string]$Path, [string]$Text = 'fixture') {
    [void][IO.Directory]::CreateDirectory([IO.Path]::GetDirectoryName($Path))
    [IO.File]::WriteAllText($Path, $Text)
}
function Check([string]$Name, [scriptblock]$Action) {
    try { & $Action; Write-Output "PASS: $Name" } catch { $script:failures += "$Name : $_"; Write-Output "FAIL: $Name : $_" }
}
function Reject([scriptblock]$Action, [string]$Match = 'full official Remaster') {
    try { & $Action | Out-Null } catch {
        if ($_.ToString() -notlike "*$Match*") { throw "Unexpected error: $_" }
        return
    }
    throw "Accepted unsupported package; expected $Match"
}
function Tree-State([string]$Path) {
    (Get-ChildItem -LiteralPath $Path -Recurse -Force | Sort-Object FullName | ForEach-Object {
        $hash = if ($_.PSIsContainer) { 'dir' } else { (Get-FileHash -LiteralPath $_.FullName).Hash }
        $_.FullName + '=' + $hash
    }) -join "`n"
}
function Package([string]$Path, [string]$Marker = 'BIN/MAINMENU.HPP') {
    foreach ($file in @('BIN/CONFIG.BIN','BIN/RESOURCE.BIN','BIN/resource-extra.cpp','DTA/Data3D.pbo','Worlds/ABEL.WRP')) { Put (Join-Path $Path $file) }
    if ($Marker -ieq 'dtaExt') { [void][IO.Directory]::CreateDirectory((Join-Path $Path $Marker)) }
    else { Put (Join-Path $Path $Marker) }
}
try {
    $env:UD_DATA_DIR = $null
    $fakeEngine = Join-Path $root 'arma_CWR'
    [void][IO.Directory]::CreateDirectory($fakeEngine)
    $classic = Join-Path $root 'ARMA Cold War Assault [Classic]'
    foreach ($file in @('BIN/CONFIG.BIN','BIN/RESOURCE.BIN','BIN/remaster.cpp','BIN/resource-extra.cpp','BIN/ud-mainmenu.hpp','DTA/Data3D.pbo','DTA/DTAEXT.PBO','Worlds/abel.wrp','gmcore/init.sqs','fonts/cwr_body.ttf','PoseidonGame.exe','PoseidonGame')) { Put (Join-Path $classic $file) }
    Check 'no default must refuse Classic fallback' { Reject { Get-UDDefaultDataDir -EngineRoot $fakeEngine } }
    $env:UD_DATA_DIR = $classic
    Check 'explicit environment Classic refused' { Reject { Get-UDDefaultDataDir -EngineRoot $fakeEngine } }
    $shadow = Join-Path $root 'tmp/remaster-shadow'
    Package $shadow
    Check 'invalid override does not fall through to valid shadow' { Reject { Get-UDDefaultDataDir -EngineRoot $fakeEngine } }
    $env:UD_DATA_DIR = Join-Path $root 'missing'
    Check 'missing override reports source' { Reject { Get-UDDefaultDataDir -EngineRoot $fakeEngine } 'UD_DATA_DIR' }
    $env:UD_DATA_DIR = $null
    Check 'alternate marker default and bracket paths' {
        if ((Get-UDDefaultDataDir -EngineRoot $fakeEngine) -ne $shadow) { throw 'Wrong shadow selected' }
    }
    $incomplete = Join-Path $root 'tmp/remaster-shadow-67'
    [void][IO.Directory]::CreateDirectory((Join-Path $incomplete 'dtaExt'))
    Check 'incomplete default skipped for full package' {
        if ((Get-UDDefaultDataDir -EngineRoot $fakeEngine) -ne $shadow) { throw 'Selected incomplete shadow' }
    }
    $env:UD_DATA_DIR = $incomplete
    Check 'incomplete override refused' { Reject { Get-UDDefaultDataDir -EngineRoot $fakeEngine } }
    $lobo = Join-Path $root '@LoBo'
    [void][IO.Directory]::CreateDirectory($lobo)
    foreach ($name in @('LoBoammo.pbo','LoBo_airammo.pbo','LoBoWreck.pbo','LoBoPalObj.pbo')) { Put (Join-Path $lobo "AddOns/$name") }
    $dummy = Join-Path $root 'never-run.ps1'
    Put $dummy "throw 'Executable must not run'"
    $classicState = Tree-State $classic
    foreach ($script in @('setup-guerrilla.ps1','run-game.ps1','run-game-with-mods.ps1','play-guerrilla.ps1','guerrilla-mode/install-missions.ps1')) {
        Check "explicit Classic refused by $script" {
            $launchParams = switch ($script) {
                'play-guerrilla.ps1' { @{ GameDir=$classic; GameExe=$dummy; NoLog=$true } }
                'setup-guerrilla.ps1' { @{ GameDir=$classic; LoBoDir=$lobo; Tools=$dummy; CheckOnly=$true } }
                'guerrilla-mode/install-missions.ps1' { @{ GameDir=$classic } }
                default { @{ DataDir=$classic; SkipBuild=$true; NoLog=$true } }
            }
            Reject { & (Join-Path $engine $script) @launchParams }
        }
    }
    Check 'Classic setup refuses before writing' {
        Reject { & (Join-Path $engine 'setup-guerrilla.ps1') -GameDir $classic -LoBoDir $lobo -Tools $dummy }
    }
    Check 'refused explicit paths did not write files' {
        $after = Tree-State $classic
        if ($after -ne $classicState) { throw 'Classic folder changed' }
    }
    $markerIndex = 0
    foreach ($marker in @('dtaExt','DTAEXT','BIN/remaster.bin','BIN/MAINMENU.HPP','BIN/splashLogo.hpp')) {
        $package = Join-Path $root ('Full [Remaster] ' + $markerIndex++ + ' ' + $marker.Replace('/','-'))
        Package $package $marker
        $env:UD_DATA_DIR = $package
        Check "valid marker $marker" {
            if ((Get-UDDefaultDataDir -EngineRoot $fakeEngine) -ne $package) { throw 'Wrong override selected' }
        }
        Put (Join-Path $package 'PoseidonGameDemo')
        Check "Demo with Abel refused ($marker)" { Reject { Get-UDDefaultDataDir -EngineRoot $fakeEngine } 'Remaster Demo package' }
    }
    Check 'relative override uses PowerShell location' {
        Push-Location -LiteralPath $root
        try {
            $env:UD_DATA_DIR = 'tmp/remaster-shadow'
            if ((Get-UDDefaultDataDir -EngineRoot $fakeEngine) -ne $shadow) { throw 'Wrong relative directory' }
        } finally { Pop-Location }
    }
    Check 'real helper rejects missing base payload' {
        Remove-Item -LiteralPath (Join-Path $shadow 'BIN/CONFIG.BIN')
        $env:UD_DATA_DIR = $shadow
        Reject { Get-UDDefaultDataDir -EngineRoot $fakeEngine } 'missing bin/config.bin'
        Put (Join-Path $shadow 'BIN/CONFIG.BIN')
    }
    if (-not $IsWindows) {
        Check 'case-ambiguous package entries refused' {
            Put (Join-Path $shadow 'BIN/config.bin')
            $env:UD_DATA_DIR = $shadow
            Reject { Get-UDDefaultDataDir -EngineRoot $fakeEngine } 'Case-ambiguous'
            Remove-Item -LiteralPath (Join-Path $shadow 'BIN/config.bin')
        }
        Check 'relative package link target takes precedence' {
            $env:UD_DATA_DIR = $null
            $target = Join-Path $root 'Linked [Remaster]'
            Package $target
            [void][IO.Directory]::CreateDirectory((Join-Path $fakeEngine 'packages'))
            New-Item -ItemType SymbolicLink -Path (Join-Path $fakeEngine 'packages/RemasterUD') -Target '../../Linked [Remaster]' | Out-Null
            if ((Get-UDDefaultDataDir -EngineRoot $fakeEngine) -ne $target) { throw 'Package link target not selected' }
        }
    }
} finally {
    $env:UD_DATA_DIR = $oldOverride
    if (Test-Path -LiteralPath $root) { Remove-Item -LiteralPath $root -Recurse -Force }
}
if ($failures.Count) { throw ($failures -join "`n") }
