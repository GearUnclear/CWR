$out = 'E:\look-lanes\look-surface\procs_r3.csv'
if (-not (Test-Path $out)) { "time,count_foreign,foreign_paths" | Out-File -Encoding utf8 $out }
while (-not (Test-Path 'E:\look-lanes\look-surface\procs_r3.stop')) {
  $p = Get-Process PoseidonGame* -ErrorAction SilentlyContinue | Where-Object { $_.Path -and $_.Path -notlike '*look-surface*' }
  $n = @($p).Count
  $paths = ($p | ForEach-Object { $_.Path }) -join ';'
  "$(Get-Date -Format HH:mm:ss),$n,$paths" | Out-File -Append -Encoding utf8 $out
  Start-Sleep -Seconds 5
}
