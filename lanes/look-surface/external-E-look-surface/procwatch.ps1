$out = 'E:\look-lanes\look-surface\procs_t3.csv'
"time,count_foreign,foreign_paths" | Out-File -Encoding utf8 $out
while (-not (Test-Path 'E:\look-lanes\look-surface\procs_t3.stop')) {
  $p = Get-Process PoseidonGame* -ErrorAction SilentlyContinue | Where-Object { $_.Path -and $_.Path -notlike '*look-surface*' }
  $n = @($p).Count
  $paths = ($p | ForEach-Object { $_.Path }) -join ';'
  "$(Get-Date -Format HH:mm:ss),$n,$paths" | Out-File -Append -Encoding utf8 $out
  Start-Sleep -Seconds 10
}
