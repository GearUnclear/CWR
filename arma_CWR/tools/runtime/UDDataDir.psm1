# Full official Remaster package validation shared by launch and install scripts.
# Compatible with Windows PowerShell 5.1 and PowerShell 7. This checks the
# package layout, not a numeric release or whether UD setup has been run.
Set-StrictMode -Version Latest

function Resolve-UDPackageChild {
    param([string]$Parent, [string]$Name)
    if (Test-Path -LiteralPath $Parent -PathType Container) {
        $entries = @(Get-ChildItem -LiteralPath $Parent -Force | Where-Object Name -IEQ $Name)
        if ($entries.Count -gt 1) { throw "Case-ambiguous entries for $Name in $Parent" }
        if ($entries.Count -eq 1) { return $entries[0].FullName }
    }
    return Join-Path $Parent $Name
}

function Assert-UDRemasterDataDir {
    [CmdletBinding()]
    param([Parameter(Mandatory)][string]$DataDir)
    $DataDir = $ExecutionContext.SessionState.Path.GetUnresolvedProviderPathFromPSPath($DataDir)
    $help = 'UD requires the full official Remaster. Classic 1.99 is no longer supported. Create a Remaster shadow with tools/runtime/new-remaster-shadow.ps1, then run setup-guerrilla.ps1.'
    if (-not (Test-Path -LiteralPath $DataDir -PathType Container)) { throw "Data directory missing: $DataDir. $help" }
    $bin = Resolve-UDPackageChild $DataDir 'bin'
    # remaster.cpp and PoseidonGame are deliberately not markers: old Classic
    # setup installed a shim and a rebuilt executable can be copied anywhere.
    $found = @()
    if (Test-Path -LiteralPath (Resolve-UDPackageChild $DataDir 'dtaExt') -PathType Container) { $found += 'dtaExt' }
    foreach ($name in @('remaster.bin','mainMenu.hpp','splashLogo.hpp')) {
        if (Test-Path -LiteralPath (Resolve-UDPackageChild $bin $name) -PathType Leaf) { $found += "bin/$name" }
    }
    foreach ($name in @('PoseidonGameDemo.exe','PoseidonGameDemo','demo')) {
        if (Test-Path -LiteralPath (Resolve-UDPackageChild $DataDir $name)) {
            throw "Remaster Demo package is not supported ($name in $DataDir). $help"
        }
    }
    if (-not $found.Count) { throw "No official Remaster marker in $DataDir. $help" }
    $worlds = Resolve-UDPackageChild $DataDir 'worlds'
    if (-not (Test-Path -LiteralPath (Resolve-UDPackageChild $worlds 'abel.wrp') -PathType Leaf)) {
        throw "Remaster Demo package or incomplete full package: no worlds/abel.wrp in $DataDir. $help"
    }
    if (-not (Test-Path -LiteralPath (Resolve-UDPackageChild $bin 'resource-extra.cpp') -PathType Leaf)) {
        throw "Remaster markers ($($found -join ', ')) but no bin/resource-extra.cpp in $DataDir. Verify or reinstall the package. $help"
    }
    foreach ($relative in @('bin/config.bin','bin/resource.bin','dta/data3d.pbo')) {
        $path = $DataDir
        foreach ($part in $relative.Split('/')) { $path = Resolve-UDPackageChild $path $part }
        if (-not (Test-Path -LiteralPath $path -PathType Leaf)) { throw "Incomplete Remaster package; missing $relative in $DataDir. $help" }
    }
    return $DataDir
}

function Get-UDDefaultDataDir {
    # Prefer the first valid full Remaster package; an explicit override fails closed.
    [CmdletBinding()]
    param(
        # The arma_CWR directory. Default: two levels above this module.
        [string]$EngineRoot = (Split-Path -Parent (Split-Path -Parent $PSScriptRoot))
    )
    $EngineRoot = $ExecutionContext.SessionState.Path.GetUnresolvedProviderPathFromPSPath($EngineRoot)
    $repoRoot = Split-Path -Parent $EngineRoot

    if ($env:UD_DATA_DIR) {
        try { return Assert-UDRemasterDataDir -DataDir $env:UD_DATA_DIR }
        catch { throw "UD_DATA_DIR: $_" }
    }

    $candidates = @()
    $junction = Join-Path $EngineRoot 'packages\RemasterUD'
    if (Test-Path -LiteralPath $junction -PathType Container) {
        $item = Get-Item -LiteralPath $junction -Force
        $target = $null
        if ($item.Attributes -band [IO.FileAttributes]::ReparsePoint) {
            # .Target is a string in PowerShell 7 and a string array in 5.1.
            $target = @($item.Target)[0]
            if ($target -and -not [IO.Path]::IsPathRooted($target)) {
                $target = Join-Path (Split-Path -Parent $junction) $target
            }
        }
        if ($target) { $candidates += $target } else { $candidates += $junction }
    }
    $candidates += (Join-Path $repoRoot 'tmp\remaster-shadow-67')
    $candidates += (Join-Path $repoRoot 'tmp\remaster-shadow')

    $reasons = @()
    foreach ($candidate in $candidates) {
        try { return Assert-UDRemasterDataDir -DataDir $candidate }
        catch { $reasons += "${candidate}: $_" }
    }
    throw ("No valid full official Remaster shadow found. Create a shadow with tools/runtime/new-remaster-shadow.ps1, " +
        "then run setup-guerrilla.ps1; set UD_DATA_DIR or pass the prepared shadow explicitly. Checked: " + ($reasons -join '; '))
}

Export-ModuleMember -Function Get-UDDefaultDataDir, Assert-UDRemasterDataDir
