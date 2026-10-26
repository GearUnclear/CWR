param([string]$Out, [int]$Seconds = 5400, [int]$Every = 2)
# Samples GPU load, clocks and power plus the tri slot holders and every
# running PoseidonGame / arma3 process while the A0 timing run is going.
"time,util_gpu,util_mem,clock_gr,clock_mem,power_w,pstate,poseidon_games,other_gpu_apps" | Out-File -FilePath $Out -Encoding ascii
$end = (Get-Date).AddSeconds($Seconds)
while ((Get-Date) -lt $end) {
    $q = (& nvidia-smi --query-gpu=utilization.gpu,utilization.memory,clocks.gr,clocks.mem,power.draw,pstate --format=csv,noheader,nounits) -replace '\s', ''
    $all = @(Get-CimInstance Win32_Process -Filter "Name='PoseidonGame.exe'" -ErrorAction SilentlyContinue)
    $foreign = @($all | Where-Object { $_.CommandLine -notmatch 'RemasterUD-gr-A0' }).Count
    $games = "$($all.Count)/foreign$foreign"
    $other = (@(Get-Process arma3_x64, arma3 -ErrorAction SilentlyContinue | ForEach-Object { $_.ProcessName }) -join '+')
    "$((Get-Date).ToString('HH:mm:ss')),$q,$games,$other" | Out-File -FilePath $Out -Append -Encoding ascii
    if (Test-Path "$Out.stop") { break }
    Start-Sleep -Seconds $Every
}
