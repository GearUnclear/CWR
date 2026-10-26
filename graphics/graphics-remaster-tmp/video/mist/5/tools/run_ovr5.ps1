# Agent 5: after the first batch, run the shader-override probes one at a time.
$here = 'D:\Arma_CWA_worktrees\graphics-remaster\tmp\video\mist\5'
while (-not (Select-String -LiteralPath (Join-Path $here 'batch.txt') -Pattern 'batch finished' -Quiet)) { Start-Sleep -Seconds 5 }
foreach ($t in @('fogvis1', 'nobeams')) {
    "$(Get-Date -Format HH:mm:ss) start $t" | Out-File -FilePath (Join-Path $here 'batch.txt') -Append -Encoding utf8
    & powershell -NoProfile -ExecutionPolicy Bypass -File (Join-Path $here 'tools\run_probe5.ps1') -Tag $t -ShaderOverrideDir (Join-Path $here "shaders\$t")
    $band = Get-Content -LiteralPath (Join-Path $here "runs\$t\band.txt") -ErrorAction SilentlyContinue | Select-Object -Last 1
    "$(Get-Date -Format HH:mm:ss) done $t $band" | Out-File -FilePath (Join-Path $here 'batch.txt') -Append -Encoding utf8
}
"$(Get-Date -Format HH:mm:ss) ovr finished" | Out-File -FilePath (Join-Path $here 'batch.txt') -Append -Encoding utf8
