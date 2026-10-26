$out = 'D:/Arma_CWA_worktrees/graphics-remaster/tmp/look/director/r5_build'
Remove-Item "$out/unit_all.done" -ErrorAction SilentlyContinue
Set-Location 'D:\'
$exe = 'D:/Arma_CWA_worktrees/look-director/arma_CWR/build/look-director-rwdi/tests/unit/engine/Poseidon/PoseidonTests.exe'
& $exe *>&1 | Out-File -Encoding utf8 "$out/unit_all.txt"
$code = $LASTEXITCODE
[IO.File]::WriteAllText("$out/unit_all.done", "exit=$code")
