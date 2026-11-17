# Runs several shoot takes one after another; after each, encodes its frames to mp4 and deletes them
# (raw frames are never kept). Usage: powershell -NoProfile -File queue-shoot.ps1 -Takes "kv_main1:main1,kv_church:church1"
param([Parameter(Mandatory)][string]$Takes, [int]$TimeoutSec = 7200)
$here = $PSScriptRoot
foreach ($item in ($Takes -split ',')) {
    $name, $tag = $item.Trim() -split ':'
    if (-not $tag) { $tag = $name }
    $o = powershell -NoProfile -File (Join-Path $here 'run-shoot.ps1') -Name $name -Tag $tag -TimeoutSec $TimeoutSec
    $o | Select-Object -Last 6 | ForEach-Object { Write-Host $_ }
    $line = $o | Where-Object { $_ -match '^KVOUT=' } | Select-Object -Last 1
    if ($line) {
        $out = $line -replace '^KVOUT=', ''
        python (Join-Path $here 'tools\encode_shots.py') $out | ForEach-Object { Write-Host $_ }
    }
    Write-Host "QUEUE-DONE $name $tag"
}
Write-Host "QUEUE-END"
