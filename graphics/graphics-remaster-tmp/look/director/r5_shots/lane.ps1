param([string]$Lane, [string]$Runs)
# $Runs: "outsub=test1,test2;outsub2=test3"
$root = 'D:/Arma_CWA_worktrees/graphics-remaster/tmp/look/director/r5_shots'
$eng = 'D:/Arma_CWA_worktrees/look-director/arma_CWR'
Remove-Item "$root/$Lane.done.txt" -ErrorAction SilentlyContinue
$sw = [Diagnostics.Stopwatch]::StartNew()
$start = Get-Date -Format o
"start $start" | Out-File -Encoding utf8 "$root/$Lane.time.txt"
$codes = @()
foreach ($run in ($Runs -split ';')) {
  $kv = $run -split '=', 2
  $out = "$root/$($kv[0])"
  $t0 = $sw.Elapsed.TotalSeconds
  & powershell -NoProfile -File "$eng/tests/rendering/captures/run-captures.ps1" -GameDir dist/look-director-rwdi -DataJunction "RemasterUD-look-director-$Lane" -Tests $kv[1] -OutDir $out *>&1 | Out-File -Append -Encoding utf8 "$root/$Lane.log"
  $codes += $LASTEXITCODE
  "run $($kv[0]) seconds=$([int]($sw.Elapsed.TotalSeconds - $t0)) exit=$LASTEXITCODE" | Out-File -Append -Encoding utf8 "$root/$Lane.time.txt"
}
"end $(Get-Date -Format o) seconds=$([int]$sw.Elapsed.TotalSeconds) exit=$($codes -join ',')" | Out-File -Append -Encoding utf8 "$root/$Lane.time.txt"
[IO.File]::WriteAllText("$root/$Lane.done.txt", "exit=$($codes -join ',')")
