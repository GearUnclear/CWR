<#
.SYNOPSIS
    Build a shadow copy of the official Remaster folder for setup and engine
    tests, so nothing ever writes into the real (Steam) install.
.DESCRIPTION
    Windows only; runs on Windows PowerShell 5.1 (powershell.exe) and on
    PowerShell 7 (pwsh). The large read-only data folders
    (AddOns, Campaigns, DTA, Worlds, dtaExt, MPMissions, SPTemplates,
    Templates) become directory junctions to the source. Everything setup or
    the engine may write (BIN, fonts, Missions, the root files) is copied, so
    a write can never travel through a junction into Bohemia's files.
    setup-guerrilla.ps1 refuses to write through a junction anyway
    (Assert-PlainPath).

    -LoBoSource copies a LoBo folder to <Destination>\@LoBo as a real copy:
    setup writes LoBo's bin\config.cpp and mod doctor patches its pbos in
    place, so it must never be the original either.

    Existing destination entries are left alone; delete the shadow to rebuild.
.EXAMPLE
    pwsh -File tools/runtime/new-remaster-shadow.ps1 `
      -Source 'D:/SteamLibrary/steamapps/common/ARMA Cold War Assault/Remastered' `
      -Destination 'D:/Arma_CWA/tmp/remaster-shadow-67' `
      -LoBoSource 'D:/SteamLibrary/steamapps/common/ARMA Cold War Assault/Remastered/@LoBo'
.EXAMPLE
    powershell -NoProfile -File tools/runtime/new-remaster-shadow.ps1 `
      -Source 'D:/SteamLibrary/steamapps/common/ARMA Cold War Assault/Remastered' `
      -Destination 'D:/Arma_CWA/tmp/lobo-repair-4/runtime/official-shadow'
    The same on Windows PowerShell 5.1, here without LoBo.
#>
#requires -Version 5.1
[CmdletBinding()]
param(
    [Parameter(Mandatory)][string]$Source,
    [Parameter(Mandatory)][string]$Destination,
    [string]$LoBoSource
)
$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest
# $IsWindows exists only from PowerShell 6 on; Windows PowerShell 5.1 is Windows by definition.
if ([Environment]::OSVersion.Platform -ne [PlatformID]::Win32NT) { throw 'Directory junctions need Windows; on Linux copy the folder instead.' }
# Relative paths resolve against the PowerShell location (Set-Location), which
# Windows PowerShell 5.1 does not mirror into the process working directory.
$Source = [IO.Path]::GetFullPath($ExecutionContext.SessionState.Path.GetUnresolvedProviderPathFromPSPath($Source))
$Destination = [IO.Path]::GetFullPath($ExecutionContext.SessionState.Path.GetUnresolvedProviderPathFromPSPath($Destination))
if (-not (Test-Path -LiteralPath $Source -PathType Container)) { throw "Source missing: $Source" }
if ($Destination.StartsWith($Source.TrimEnd('\') + '\', [StringComparison]::OrdinalIgnoreCase)) {
    throw 'Destination must not sit inside the source package.'
}
[void][IO.Directory]::CreateDirectory($Destination)
$junctioned = @('AddOns','Campaigns','DTA','Worlds','dtaExt','MPMissions','SPTemplates','Templates')
foreach ($item in Get-ChildItem -LiteralPath $Source -Force) {
    if ($item.Name -like '@*') { continue }   # mods are opt-in via -LoBoSource
    $target = Join-Path $Destination $item.Name
    if (Test-Path -LiteralPath $target) { Write-Output "Kept: $target"; continue }
    if ($item.PSIsContainer -and ($junctioned -icontains $item.Name)) {
        [void](New-Item -ItemType Junction -Path $target -Target $item.FullName)
        Write-Output "Junction: $target -> $($item.FullName)"
    } elseif ($item.PSIsContainer) {
        Copy-Item -LiteralPath $item.FullName -Destination $target -Recurse
        Write-Output "Copied: $target"
    } elseif ($item.Extension -ine '.log') {
        Copy-Item -LiteralPath $item.FullName -Destination $target
        Write-Output "Copied: $target"
    }
}
if ($LoBoSource) {
    $lobo = Join-Path $Destination '@LoBo'
    if (Test-Path -LiteralPath $lobo) { Write-Output "Kept: $lobo" }
    else {
        Copy-Item -LiteralPath ([IO.Path]::GetFullPath($ExecutionContext.SessionState.Path.GetUnresolvedProviderPathFromPSPath($LoBoSource))) -Destination $lobo -Recurse
        Write-Output "Copied: $lobo"
    }
}
# Prove the writable folders are real.
foreach ($name in @('BIN','fonts','Missions')) {
    $path = Join-Path $Destination $name
    if ((Test-Path -LiteralPath $path) -and ((Get-Item -LiteralPath $path -Force).Attributes -band [IO.FileAttributes]::ReparsePoint)) {
        throw "Writable folder is a link: $path"
    }
}
Write-Output "Shadow ready: $Destination"
