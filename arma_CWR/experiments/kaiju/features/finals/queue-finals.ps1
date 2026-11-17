# Runs the finals jobs listed in kaiju-notes\features\finals\logs\queue.txt one after another, each
# through run-finals.ps1 (so through the pipeline's run-tri.ps1: one game, hidden desktop, the
# UD-kaiju-tri mutex). One line per job: "<test> <tag>", for example "finals_a a_photo1". A finished
# job is appended to queue.done.txt with its exit code. The runner exits when no job is left; start
# it again after adding lines. Only one runner at a time (its own named mutex), so there is never
# more than one finals run parked on the kaiju turn.
#   Start-Process powershell -WindowStyle Hidden -ArgumentList '-NoProfile','-File','<this file>'
param([string]$Notes = 'D:\Arma_CWA_worktrees\kaiju-notes\features\finals')
$ErrorActionPreference = 'Continue'
$logs = Join-Path $Notes 'logs'
[void][IO.Directory]::CreateDirectory($logs)
$queue = Join-Path $logs 'queue.txt'
$done = Join-Path $logs 'queue.done.txt'
$log = Join-Path $logs 'queue.log'
$m = New-Object System.Threading.Mutex($false, 'UD-kaiju-finals-queue')
if (-not $m.WaitOne(0)) { Add-Content -LiteralPath $log -Value ("[{0}] another runner is active; this one exits" -f (Get-Date -Format HH:mm:ss)); exit 0 }
try {
    while ($true) {
        $finished = @()
        if (Test-Path -LiteralPath $done) { $finished = @(Get-Content -LiteralPath $done | ForEach-Object { ($_ -split '\s+')[1] }) }
        $next = $null
        foreach ($line in @(Get-Content -LiteralPath $queue -ErrorAction SilentlyContinue)) {
            $parts = $line.Trim() -split '\s+'
            if ($parts.Count -ge 2 -and $finished -notcontains $parts[1]) { $next = $parts; break }
        }
        if (-not $next) { break }
        Add-Content -LiteralPath $log -Value ("[{0}] start {1} {2}" -f (Get-Date -Format HH:mm:ss), $next[0], $next[1])
        & powershell -NoProfile -File (Join-Path $PSScriptRoot 'run-finals.ps1') -Test $next[0] -Tag $next[1] *>> (Join-Path $logs ("queue-" + $next[1] + ".out.txt"))
        $code = $LASTEXITCODE
        Add-Content -LiteralPath $done -Value ("{0} {1} exit {2} at {3}" -f $next[0], $next[1], $code, (Get-Date -Format HH:mm:ss))
        Add-Content -LiteralPath $log -Value ("[{0}] end {1} {2} exit {3}" -f (Get-Date -Format HH:mm:ss), $next[0], $next[1], $code)
    }
}
finally { $m.ReleaseMutex(); $m.Dispose() }
