<#
.SYNOPSIS
    Samples the GPU clock, load and power, and every running PoseidonGame, while
    a timing run goes (plan section 8 item 7; run-timing.ps1 starts it).

.DESCRIPTION
    Writes one CSV row every -Every seconds until -Seconds have passed or the
    file <Out>.stop appears: nvidia-smi's graphics utilisation, memory
    utilisation, graphics and memory clocks, power and P-state, then
    "<all PoseidonGame>/foreign<n>" where foreign counts the games whose command
    line does not carry -Junction (games of other sessions, which make a run's
    timings not quiet), and any Arma 3 process. summarize_timing.py joins the
    rows to the shots by time. The rows are stamped in whole seconds.
#>
param(
    [Parameter(Mandatory = $true)][string]$Out,
    [int]$Seconds = 5400,
    [int]$Every = 2,
    [string]$Junction = 'RemasterUD-gr-'
)
"time,util_gpu,util_mem,clock_gr,clock_mem,power_w,pstate,poseidon_games,other_gpu_apps" | Out-File -FilePath $Out -Encoding ascii
$end = (Get-Date).AddSeconds($Seconds)
while ((Get-Date) -lt $end) {
    $q = (& nvidia-smi --query-gpu=utilization.gpu,utilization.memory,clocks.gr,clocks.mem,power.draw,pstate --format=csv,noheader,nounits) -replace '\s', ''
    $all = @(Get-CimInstance Win32_Process -Filter "Name='PoseidonGame.exe'" -ErrorAction SilentlyContinue)
    $foreign = @($all | Where-Object { $_.CommandLine -notmatch [regex]::Escape($Junction) }).Count
    $games = "$($all.Count)/foreign$foreign"
    $other = (@(Get-Process arma3_x64, arma3 -ErrorAction SilentlyContinue | ForEach-Object { $_.ProcessName }) -join '+')
    "$((Get-Date).ToString('HH:mm:ss')),$q,$games,$other" | Out-File -FilePath $Out -Append -Encoding ascii
    if (Test-Path "$Out.stop") { break }
    Start-Sleep -Seconds $Every
}
