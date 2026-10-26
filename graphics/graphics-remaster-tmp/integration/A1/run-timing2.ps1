# Timing rerun on the final A1 build, logging the CPU time of any Arma 3
# process before and after (an Arma 3 that was busy made the first full run
# not quiet).
$a = Get-Process arma3_x64 -ErrorAction SilentlyContinue
"arma3 before: " + ($(if ($a) { "pid $($a.Id) cpu $($a.CPU)" } else { 'none' })) + " at $(Get-Date -Format HH:mm:ss)"
Set-Location D:\Arma_CWA_worktrees\graphics-remaster\arma_CWR
& powershell -NoProfile -ExecutionPolicy Bypass -File tests\rendering\captures\run-timing.ps1 -GameDir dist/graphics-remaster-rwdi -DataJunction RemasterUD-gr-A1 -OutRoot D:\Arma_CWA_worktrees\graphics-remaster\tmp\integration\A1
$a = Get-Process arma3_x64 -ErrorAction SilentlyContinue
"arma3 after: " + ($(if ($a) { "pid $($a.Id) cpu $($a.CPU)" } else { 'none' })) + " at $(Get-Date -Format HH:mm:ss)"
