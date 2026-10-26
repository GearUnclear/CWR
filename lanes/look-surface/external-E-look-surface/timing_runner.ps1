param([string]$Out, [int]$Passes = 3)
$tests = Get-ChildItem 'D:\Arma_CWA_worktrees\look-surface\arma_CWR\tests\rendering\captures' -Filter 'look_surface_t_*.test.sqf' | ForEach-Object { 'tests/rendering/captures/' + $_.Name }
Set-Location D:\Arma_CWA_worktrees\look-surface\arma_CWR
for ($pass = 1; $pass -le $Passes; $pass++) {
  foreach ($t in $tests) {
    $deadline = (Get-Date).AddMinutes(20)
    while ((Get-Date) -lt $deadline) {
      $quiet = $true
      for ($i = 0; $i -lt 3; $i++) {
        $f = Get-Process PoseidonGame* -ErrorAction SilentlyContinue | Where-Object { $_.Path -and $_.Path -notlike '*look-surface*' }
        if (@($f).Count -gt 0) { $quiet = $false; break }
        Start-Sleep -Seconds 5
      }
      if ($quiet) { break }
      Start-Sleep -Seconds 10
    }
    $name = [IO.Path]::GetFileNameWithoutExtension([IO.Path]::GetFileNameWithoutExtension($t))
    "$(Get-Date -Format HH:mm:ss) pass $pass $name" | Out-File -Append -Encoding utf8 "$Out.runner.log"
    powershell -NoProfile -File tests\rendering\captures\run-captures.ps1 -Tests $t -GameDir dist/look-surface-rwdi -DataJunction RemasterUD-look-surface -OutDir "$Out\p$pass-$name" *> "$Out.p$pass-$name.log"
  }
}
"done" | Out-File -Append -Encoding utf8 "$Out.runner.log"
