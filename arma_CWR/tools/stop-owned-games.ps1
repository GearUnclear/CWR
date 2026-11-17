<#
.SYNOPSIS
    Stops only the PoseidonGame.exe processes the caller owns, never another
    session's.

.DESCRIPTION
    Several AI sessions run Trident tests on this machine at once. A global
    `Get-Process PoseidonGame | Stop-Process` from one session kills the games
    of the others mid-run (their tri run then reports exit -1 with an empty
    stderr). This script picks its victims by ownership instead. A game is
    selected when ANY of the given selectors matches it:

      -OwnerPid <int>   the game's parent chain (Win32_Process ParentProcessId,
                        walked up to the root) contains this PID. Default: the
                        PID of the PowerShell running this script ($PID), which
                        only matches when the script is dot-sourced or run in
                        the same PowerShell that launched the games. From Git
                        Bash pass the Windows PID of your shell:
                        -OwnerPid $(cat /proc/$$/winpid)
      -DataDir <path>   the game's command line carries `-C <path>` (how tri
                        launches every game). Case and trailing slashes are
                        ignored. run-game.ps1 sets the working directory instead
                        of passing -C, so use -Pid for its launches.
      -Pid <int[]>      these exact PIDs (must be PoseidonGame processes).
      -Orphans          games whose parent process no longer exists (a crashed
                        tri run, or a run-game.ps1 launch whose shell exited).
                        Another session's run-game launch is also an orphan, so
                        prefer -Pid or -DataDir when you know them.

    When -DataDir, -Pid (alias of -ProcessId) or -Orphans is given, -OwnerPid is
    used only if it was passed explicitly.

    Output lists every PoseidonGame process with PID, parent, age and data dir,
    marked KILLED (or WOULD KILL under -DryRun/-WhatIf) or LEFT ALONE, so other
    sessions' instances stay visible. Exits 0 when there is nothing to do, and
    1 only when a selected process could not be stopped.

.EXAMPLE
    powershell -NoProfile -File tools\stop-owned-games.ps1 -DataDir D:\Arma_CWA\tmp\remaster-shadow-67
.EXAMPLE
    powershell -NoProfile -File tools\stop-owned-games.ps1 -Pid 12345
.EXAMPLE
    powershell -NoProfile -File tools\stop-owned-games.ps1 -DryRun
        Lists every running game and who would be selected (nothing is killed).
#>
[CmdletBinding(SupportsShouldProcess = $true)]
param(
    [int]$OwnerPid = $PID,
    [string[]]$DataDir = @(),
    [Alias('Pid', 'Id')][string[]]$ProcessId = @(),
    [switch]$Orphans,
    [switch]$DryRun
)

$ErrorActionPreference = 'Stop'
$dry = $DryRun -or $WhatIfPreference
# Honour -WhatIf through $dry only; left set, it makes module autoload print
# "What if: Set Alias" noise.
$WhatIfPreference = $false
# `powershell -File ... -Pid 1,2` delivers "1,2" as one string; split it here.
$ProcessId = @($ProcessId | ForEach-Object { $_ -split '[,\s]+' } | Where-Object { $_ } | ForEach-Object { [int]$_ })
$useOwner = $PSBoundParameters.ContainsKey('OwnerPid') -or
    (-not $DataDir -and -not $ProcessId -and -not $Orphans)

function Normalize-Dir([string]$p) {
    if (-not $p) { return '' }
    $p = $p.Trim().Trim('"') -replace '/', '\'
    try { $p = [IO.Path]::GetFullPath($p) } catch { }
    return $p.TrimEnd('\').ToLowerInvariant()
}

function Get-GameDataDir([string]$cmd) {
    if (-not $cmd) { return $null }
    # tri: ... -C D:\Arma_CWA\tmp\remaster-shadow-of1 --window ...  (also quoted form)
    $m = [regex]::Match($cmd, '(?:^|\s)-C\s+(?:"([^"]+)"|(\S+))')
    if ($m.Success) { if ($m.Groups[1].Success) { return $m.Groups[1].Value } else { return $m.Groups[2].Value } }
    return $null
}

$all = @{}
foreach ($p in @(Get-CimInstance Win32_Process)) { $all[[int]$p.ProcessId] = $p }
$games = @($all.Values | Where-Object { $_.Name -ieq 'PoseidonGame.exe' } | Sort-Object ProcessId)

if ($games.Count -eq 0) {
    Write-Host 'No PoseidonGame processes running. Nothing to do.'
    exit 0
}

# A parent counts only if it exists AND was created before the child; otherwise
# the PID was reused by an unrelated process and the real parent is gone.
function Get-LiveParent($proc) {
    $ppid = [int]$proc.ParentProcessId
    if (-not $all.ContainsKey($ppid)) { return $null }
    $parent = $all[$ppid]
    if ($parent.CreationDate -and $proc.CreationDate -and $parent.CreationDate -gt $proc.CreationDate) { return $null }
    return $parent
}

function Test-OwnedBy($proc, [int]$owner) {
    $cur = $proc
    $seen = @{}
    while ($cur) {
        $id = [int]$cur.ProcessId
        if ($seen.ContainsKey($id)) { break }
        $seen[$id] = $true
        $parent = Get-LiveParent $cur
        if (-not $parent) { break }
        if ([int]$parent.ProcessId -eq $owner) { return $true }
        $cur = $parent
    }
    return $false
}

$wantDirs = @($DataDir | ForEach-Object { Normalize-Dir $_ } | Where-Object { $_ })
$now = Get-Date
$rows = @()
foreach ($g in $games) {
    $gid = [int]$g.ProcessId
    $dir = Get-GameDataDir $g.CommandLine
    $reasons = @()
    if ($ProcessId -contains $gid) { $reasons += 'pid' }
    if ($dir -and $wantDirs -contains (Normalize-Dir $dir)) { $reasons += 'datadir' }
    if ($useOwner -and (Test-OwnedBy $g $OwnerPid)) { $reasons += "owner $OwnerPid" }
    $orphan = -not (Get-LiveParent $g)
    if ($Orphans -and $orphan) { $reasons += 'orphan' }
    $age = if ($g.CreationDate) { $now - $g.CreationDate } else { $null }
    $rows += [pscustomobject]@{
        Proc    = $g
        Pid     = $gid
        Parent  = "$([int]$g.ParentProcessId)$(if ($orphan) { ' (gone)' })"
        Age     = if ($age) { '{0:hh\:mm\:ss}' -f $age } else { '?' }
        DataDir = if ($dir) { $dir } else { '(no -C; cwd launch)' }
        Reasons = $reasons
    }
}

foreach ($n in $ProcessId) {
    if (-not ($rows | Where-Object { $_.Pid -eq $n })) {
        Write-Host "PID $n is not a running PoseidonGame process; ignored."
    }
}

$failed = 0
$selected = @($rows | Where-Object { $_.Reasons.Count -gt 0 })
foreach ($r in $rows) {
    if ($r.Reasons.Count -gt 0) {
        $why = $r.Reasons -join ','
        if ($dry) {
            $status = "WOULD KILL ($why)"
        } else {
            try {
                Stop-Process -Id $r.Pid -Force -ErrorAction Stop -Confirm:$false
                $status = "KILLED ($why)"
            } catch {
                if (Get-Process -Id $r.Pid -ErrorAction SilentlyContinue) {
                    $status = "FAILED ($why): $($_.Exception.Message)"; $failed++
                } else {
                    $status = "ALREADY EXITED ($why)"
                }
            }
        }
    } else {
        $status = 'LEFT ALONE'
    }
    Write-Host ('{0,-24} pid={1,-6} parent={2,-13} age={3,-9} dataDir={4}' -f $status, $r.Pid, $r.Parent, $r.Age, $r.DataDir)
}

if ($selected.Count -eq 0) { Write-Host 'Nothing selected. Nothing to do.' }
if ($failed -gt 0) { exit 1 }
exit 0
