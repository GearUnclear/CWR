# The default game data directory for the launch and install scripts
# (run-game.ps1, run-game-with-mods.ps1, guerrilla-mode/install-missions.ps1).
# UD targets the full official Remaster through a setup-prepared SHADOW copy
# (tools/runtime/new-remaster-shadow.ps1, then setup-guerrilla.ps1); Classic
# 1.99 is a legacy fallback that is being retired. Works in Windows
# PowerShell 5.1 and PowerShell 7.
Set-StrictMode -Version Latest

function Get-UDDefaultDataDir {
    <#
    .SYNOPSIS
        The data dir a launcher or installer uses when none is given.
    .DESCRIPTION
        The first candidate that exists wins:
          1. $env:UD_DATA_DIR, a per-machine override (an error if it is set
             but missing).
          2. arma_CWR\packages\RemasterUD, the junction the remaster_ud test
             lane uses. Its TARGET is returned, because the installers refuse
             to write through a junction (Assert-PlainPath).
          3. <repo>\tmp\remaster-shadow-67, the shadow FULL-SCALE-SESSION.md
             builds.
          4. <repo>\tmp\remaster-shadow, the shadow FRESH-INSTALL.md builds.
          5. <repo>\ARMA Cold War Assault [Classic], with a warning.
        A shadow candidate only counts when it carries the Remaster's root
        dtaExt folder, so an empty leftover folder is skipped. The Steam
        folder itself is never a candidate: tests and setup must not write
        there. If nothing exists the Classic path is returned anyway, so the
        caller's own "not found" error names it.
    #>
    [CmdletBinding()]
    param(
        # The arma_CWR directory. Default: two levels above this module.
        [string]$EngineRoot = (Split-Path -Parent (Split-Path -Parent $PSScriptRoot))
    )
    $EngineRoot = [IO.Path]::GetFullPath($EngineRoot)
    $repoRoot = Split-Path -Parent $EngineRoot

    if ($env:UD_DATA_DIR) {
        if (-not (Test-Path -LiteralPath $env:UD_DATA_DIR -PathType Container)) {
            throw "UD_DATA_DIR is set but is not a directory: $env:UD_DATA_DIR"
        }
        return [IO.Path]::GetFullPath($env:UD_DATA_DIR)
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

    foreach ($candidate in $candidates) {
        if ((Test-Path -LiteralPath $candidate -PathType Container) -and
            (Test-Path -LiteralPath (Join-Path $candidate 'dtaExt') -PathType Container)) {
            return [IO.Path]::GetFullPath($candidate)
        }
    }

    $classic = Join-Path $repoRoot 'ARMA Cold War Assault [Classic]'
    $message = ("No prepared Remaster shadow found (looked at {0}). Falling back to the legacy Classic data dir {1}. " +
        "Classic support is being dropped: build a shadow with tools/runtime/new-remaster-shadow.ps1 and setup-guerrilla.ps1 " +
        "(guerrilla-mode/FULL-SCALE-SESSION.md, part A1), set UD_DATA_DIR, or pass the data dir explicitly.") -f ($candidates -join ', '), $classic
    Write-Warning $message
    return $classic
}

Export-ModuleMember -Function Get-UDDefaultDataDir
