# Runs several integration tests one after another (each takes the kaiju turn afresh through
# run-kj.ps1), and writes one line per run to kaiju-notes\integration\logs\queue.txt.
#
#   powershell -NoProfile -File run-queue.ps1 -Jobs "kj_blows|proxy|blows-proxy; kj_demo|proxy|demo-proxy; kj_demo||demo-a|3"
#
# A job is  name|params|tag|runs|stock   (params: comma list of presets; runs default 1; stock: "stock").
param([Parameter(Mandatory)][string]$Jobs, [string]$NotesDir = 'D:\Arma_CWA_worktrees\kaiju-notes\integration', [int]$TimeoutSec = 0)
$here = $PSScriptRoot
[void][IO.Directory]::CreateDirectory((Join-Path $NotesDir 'logs'))
$log = Join-Path $NotesDir 'logs\queue.txt'
foreach ($job in ($Jobs -split ';' | ForEach-Object { $_.Trim() } | Where-Object { $_ })) {
    $f = $job -split '\|'
    $name = $f[0]
    $argv = @('-NoProfile', '-File', (Join-Path $here 'run-kj.ps1'), '-Name', $name, '-NotesDir', $NotesDir)
    if ($TimeoutSec -gt 0) { $argv += @('-TimeoutSec', $TimeoutSec) }
    if ($f.Count -gt 1 -and $f[1]) { $argv += @('-Params', $f[1]) }
    if ($f.Count -gt 2 -and $f[2]) { $argv += @('-Tag', $f[2]) }
    if ($f.Count -gt 3 -and $f[3]) { $argv += @('-Runs', $f[3]) }
    if ($f.Count -gt 4 -and $f[4] -eq 'stock') { $argv += '-Stock' }
    $t0 = Get-Date
    $out = & powershell @argv 2>&1 | ForEach-Object { "$_" }
    $code = $LASTEXITCODE
    $kj = ($out | Where-Object { $_ -match '^KJOUT=' }) -join ' ; '
    Add-Content -LiteralPath $log -Value ("{0} job {1} exit {2} in {3:N0} s : {4}" -f (Get-Date -Format s), $job, $code, ((Get-Date) - $t0).TotalSeconds, $kj) -Encoding utf8
    Write-Host "job $job exit $code : $kj"
}
